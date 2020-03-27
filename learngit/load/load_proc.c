#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "load_main.h"

/***********************************************************
 * Function: 获取每个文件对应的表名 及附加字符串
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [O]addStr              :返回入库记录的附加字符串，如${FILE_NAME},${SYSDATE}等字段。
 *      [I]DBSession           :ORACLE事务指针
 *      [IO]FI                 :待入库的文件对应的字段信息, 求出FI->table_def.table_name的值
 *      [I]inFileName          :待入库的文件名
 * Returns:
 *      0                      :无错误
 *      -1                     :出错
 **********************************************************/
short GetTableStr(char *outStr, char *addStr,
                  DB_SESSION *DBSession, FILE_INFO *FI, char *inFileName)
{
    int i;
    char tmpStr[300], tmpStr1[300], tmpStr2[300];
    TABLE_RECORD tmpRec[1], vaRec[1];

    /*获取表名*/
    sprintf(tmpStr, "select %s from dual", FI->table);
    sprintf(vaRec[0].field, inFileName);    
    if( GetRowFromTable(outStr, DBSession, tmpRec, 1, tmpStr, vaRec, FI->table_fileFlag)!=0 )
    {
        #ifdef DEBUG_INFO
            printf("Error on get table_name (%s)\n%s\n", outStr, tmpStr);
        #endif
        sprintf(outStr, "Error on get table_name (%s)", tmpStr);
        return -1;
    }
    sprintf(FI->table_def.table_name, "%s", tmpRec[0].field);
//printf("file=%s,line=%d,table=%s,FI->table_def.table_name=%s\n",__FILE__,__LINE__,FI->table,FI->table_def.table_name);
    memset(tmpStr1, '\0', sizeof(tmpStr1));
    for(i=0; i<FI->fieldNum; i++)
    {
        if( (FI->field[i].format[0]=='\0')||(1 == FI->field[i].funcFlag) )
            continue;
        if(strcmp(FI->field[i].format, "${FILE_NAME}")==0)
            sprintf(tmpStr, "%s", inFileName+FI->field[i].pos-1);
        else if(strcmp(FI->field[i].format, "${SYSDATE}")==0)
        {
            GetSysDateTime(tmpStr2);
            sprintf(tmpStr, "%s", tmpStr2+FI->field[i].pos-1);
        }
        else
            sprintf(tmpStr, "%s", FI->field[i].format+FI->field[i].pos-1);

        tmpStr[FI->field[i].len] = '\0';
        while(FI->field[i].len > (int)strlen(tmpStr))
            strcat(tmpStr, " ");
        strcat(tmpStr1, tmpStr);
    }
    sprintf(addStr, "%s", tmpStr1);
    return 0;
}

/***********************************************************
 * Function: 采用Insert方式入库
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [I]addStr              :入库记录的附加字符串，如${FILE_NAME},${SYSDATE}等字段。
 *      [O]totalNum            :返回入库的总记录数
 *      [I]infp                :待入库的文件的文件指针
 *      [I]errfp               :错单对应的行数、及错误信息暂时记录在此文件中
 *      [I]FI                  :待入库的文件对应的字段信息
 *      [I]DBSession           :ORACLE事务指针
 * Returns:
 *      0                      :无错误
 *      -1                     :出错
 **********************************************************/
short loadInsert(char *outStr, char *addStr, long *totalNum,
                 SFILE *infp, FILE *errfp, FILE_INFO *FI, DB_SESSION *DBSession)
{
    char recStr[MAX_REC_STR_LEN], tmpStr[MAX_REC_STR_LEN];
    char tmpSql[4000], tmpIns[2000];
    char tmpStr1[2000], tmpStr2[1000],tmpStr3[1000];
    int  i,j=1,n1=0,n2=0,k,len;
    char *dataRec, *p, *tmpP;
    int  curRowNum=0, colNum, dataLen, iArr[300];
    char szBuf[2048];
    char szInDateTime[14+1];    //入库时间   
    char strPartDate[8+1];
    int num = 0;
    char tmpPartDate[8+1];
    char m_month[2+1];
    memset(szBuf,0,sizeof(szBuf));
    memset(szInDateTime,0,sizeof(szInDateTime));
    memset(tmpStr3,0,sizeof(tmpStr3));
    memset(strPartDate,0,sizeof(strPartDate));
    memset(tmpPartDate,0,sizeof(tmpPartDate));
    memset(m_month,0,sizeof(m_month));
		
    getSysDate(szInDateTime);

    /*生成INSERT语句*/
    k = 0;
    for(i=0; i<FI->fieldNum; i++)
    {
        n1+=sprintf(tmpStr1+n1, "%s,", FI->field[i].field_name);
        switch(FI->field[i].funcFlag)
        {
          case 0:
            n2+=sprintf(tmpStr2+n2, ":f%d,", j++);
            if( strcasecmp(FI->field[i].field_name,"startdate")==0 )
            {
               sprintf(tmpPartDate, "%d", FI->field[i].pos);
             }
            break;
          
          case 1:
            n2+=sprintf(tmpStr2+n2, "%s,", FI->field[i].format+3);
	    k++;
            break;
          
          case 2:
            n2+=sprintf(tmpStr2+n2, "%s,", FI->field[i].format+3);
            j++;
            break;
        }
    }
    tmpStr1[strlen(tmpStr1)-1]='\0';
    tmpStr2[strlen(tmpStr2)-1]='\0';


    colNum = FI->fieldNum + 1 - k;      /*需绑定的字段个数*/
    dataLen = MAX_LOAD_ROW_LEN*MAX_LOAD_FIELD_LEN*colNum*sizeof(char);
    dataRec = malloc(dataLen);
    if (dataRec == (char *)0)
    {
        sprintf(outStr, "malloc(%d) failure.\tError occurred at File %s:%d",
            dataLen, __FILE__, __LINE__);
        #ifdef DEBUG_INFO
            printf("%s\n", outStr);
        #endif
        return -1;
    }

    /*准备入库*/
    tmpP = tmpStr + strlen(addStr);
    memset(dataRec, 0, dataLen);
    *totalNum = 0;
    p = sgets( recStr, sizeof(recStr), infp );
    len = strlen(recStr);
    if(recStr[len-2] == 13 )
       recStr[len-2] = '\0';
    else if(recStr[len-3] == 13)
       recStr[len-3] = '\0';
    else
       recStr[len-1] = '\0';
    num= atoi(tmpPartDate);
    getTextField(strPartDate,recStr,'|',num-1);
    snprintf(m_month,3,"%s",strPartDate+4);
//printf("FILE=%s,LINE=%d,strPartDate=%s,num=%d\n",__FILE__,__LINE__,strPartDate,num);
    //根据话单时间入相应月份表
    sprintf(tmpIns, "insert into %s%s(%s) values(", FI->table_def.table_name,m_month,tmpStr1);
    sprintf(tmpSql, "%s%s)", tmpIns, tmpStr2);
    sprintf(tmpStr3, "%s|%s", szInDateTime, recStr);
    memset(recStr,0,sizeof(recStr));
    sprintf(recStr, "%s", tmpStr3);
    /*printf("[%s][%d]........tmpStr3=[%s],recStr=[%s],szInDateTime=[%s]\n",__FILE__,__LINE__,tmpStr3,recStr,szInDateTime);*/

    while(p!=NULL)
    {
        memset(tmpStr, 0, sizeof(tmpStr));
        /*去掉字符串的换行符（10），20071110加入*/
        len = strlen(recStr);
        if(recStr[len-2] == 13)
            recStr[len-2] = '\0';
        else
            recStr[len-1] = '\0';
        sprintf(tmpStr, "%s%s", addStr, recStr);
				
	/*printf("[%s][%d]........tmpStr=[%s],addStr=[%s],recStr=[%s]\n",__FILE__,__LINE__,tmpStr,addStr,recStr);*/
        /*取下一条记录*/
        memset(recStr,'\0',sizeof(recStr));
        p = sgets( recStr, sizeof(recStr), infp );
        
    	sprintf(tmpStr3, "%s|%s", szInDateTime, recStr);
    	memset(recStr,0,sizeof(recStr));
    	sprintf(recStr, "%s", tmpStr3);
	/*printf("[%s][%d]........recStr=[%s]\n",__FILE__,__LINE__,recStr);*/
        /*将记录插入数组，等待入库*/
        if( (p!=NULL) || (FI->tailNum==0) )  /*此记录不是尾记录*/
        {
            if(0 == FI->segChar[0]) /*定长*/
            {
                for(i=0; i<FI->fieldNum; i++)
                {
                    if(1 == FI->field[i].funcFlag)
                        continue;

                    if(FI->dfield[i].flag_fld==FLD_INLINE)
                        strncat(dataRec+(curRowNum*colNum + i)*MAX_LOAD_FIELD_LEN,
                             tmpStr + FI->dfield[i].begpos_fld-1, FI->dfield[i].maxlen_fld);
                    else /*去空格*/
                    {
                        tmpStr1[0]='\0';
                        strncat(tmpStr1, tmpStr + FI->dfield[i].begpos_fld-1, FI->dfield[i].maxlen_fld);
                        tmpStr1[FI->dfield[i].maxlen_fld] = '\0';
                        CutChar( tmpStr1, ' ', 'C' );
                        strcat(dataRec+(curRowNum*colNum + i)*MAX_LOAD_FIELD_LEN,tmpStr1);
                    }
                }
            }
            else  /*变长*/
            {
                SplitStrM(tmpP, FI->segChar, &k, 300, iArr);
                for(i=0; i<FI->fieldNum; i++)
                {
                    if(1 == FI->field[i].funcFlag) /*直接使用oracle函数*/
                        continue;

                    if( (0 == FI->field[i].format[0]) ||  /*数据取自文件记录*/
                        (2 == FI->field[i].funcFlag) )
                    {
                        if(k >= FI->field[i].pos)
                        {
                            if(FI->dfield[i].flag_fld==FLD_INLINE)
                                strcat(dataRec+(curRowNum*colNum + i)*MAX_LOAD_FIELD_LEN,
                                     tmpP + iArr[FI->field[i].pos-1]);
                            else /*去空格*/
                            {
                                strcpy(tmpStr1, tmpP + iArr[FI->field[i].pos-1]);
                                CutChar( tmpStr1, ' ', 'C' );
                                strcat(dataRec+(curRowNum*colNum + i)*MAX_LOAD_FIELD_LEN,tmpStr1);
                            }
                        }
                    }
                    else   /*数据取系统时间、文件名、或者常量*/
                    {
                        if(FI->dfield[i].flag_fld==FLD_INLINE)
                            strncat(dataRec+(curRowNum*colNum + i)*MAX_LOAD_FIELD_LEN,
                                 tmpStr + FI->dfield[i].begpos_fld-1, FI->dfield[i].maxlen_fld);
                        else /*去空格*/
                        {
                            tmpStr1[0]='\0';
                            strncat(tmpStr1, tmpStr + FI->dfield[i].begpos_fld-1, FI->dfield[i].maxlen_fld);
                            tmpStr1[FI->dfield[i].maxlen_fld] = '\0';
                            CutChar( tmpStr1, ' ', 'C' );
                            strcat(dataRec+(curRowNum*colNum + i)*MAX_LOAD_FIELD_LEN,tmpStr1);
                        }
                    }
                }
            }

            (*totalNum)++;
            curRowNum++;
        }

        /*如果符合入库条件*/
        if( (curRowNum==MAX_LOAD_ROW_LEN) || ((p==NULL)&&(curRowNum>0)) )
        {
        	/*printf("[%s][%d]........tmpStr1=[%s],tmpSql=[%s],dataRec=[%s]\n",__FILE__,__LINE__,tmpStr1,tmpSql,dataRec);*/
            if(ExeMultiSql(tmpStr1, DBSession, tmpSql, dataRec,
                   MAX_LOAD_FIELD_LEN, curRowNum, colNum, MAX_LOAD_ROW_LEN)<0)
            {
                sprintf(outStr, "Insert Error (%s)", tmpStr1);
		printf("[%s][%d].......出错的话单[%s]\n",__FILE__,__LINE__,dataRec);
                //#ifdef DEBUG_INFO
                    printf("%s\n1出错语句是:\n", outStr);
                    n2=sprintf(tmpStr1, "%s'%s'", tmpIns, dataRec);
                    for(i=1; i<FI->fieldNum; i++)
                        n2+=sprintf(tmpStr1+n2, ",'%s'", dataRec+i*MAX_LOAD_FIELD_LEN);
                    printf("%s)\n",tmpStr1);
                //#endif
		//fprintf(errfp, "%-10d(%d): %s\n", n1, n1, dataRec+j+1);
		printf("[%s][%d].............dataRec=[%s]\n",__FILE__,__LINE__,dataRec);
                //ExecuteSql(tmpStr, DBSession, "rollback");
                //free(dataRec);
                //return -1;
            }

            /*查找错单和重单*/
            for(i=0;i<curRowNum;i++)
            {
                j = ((i+1)*colNum -1)*MAX_LOAD_FIELD_LEN;
                if(dataRec[j]!='0')
                {
                    n1 = *totalNum - curRowNum + i+ 1;
                    if(dataRec[j]=='2')  /*错单*/
                    {
                        fprintf(errfp, "%-10d(%d): %s\n", n1, n1, dataRec+j+1);
			printf("[%s][%d].....出错话单=[%s],recStr=[%s]\n",__FILE__,__LINE__,dataRec,recStr);
                        /*sprintf(szBuf,"%-10d(%d): %s\n", n1, n1, dataRec+j+1);
                        swriteln(szBuf,strlen(szBuf),errfp);*/

                        #ifdef DEBUG_INFO
                            printf("%s\n2出错语句是:\n", dataRec+j+1);
                            n2=sprintf(tmpStr1, "%s'%s'", tmpIns, dataRec+(i*colNum)*MAX_LOAD_FIELD_LEN);
                            for(k=1; k<FI->fieldNum; k++)
                            n2+=sprintf(tmpStr1+n2, ",'%s'", dataRec+(i*colNum+k)*MAX_LOAD_FIELD_LEN);
                            printf("%s)\n",tmpStr1);
		
                        #endif
                    }
                    else                 /*重单*/
                    {
                        fprintf(errfp, "%-10d1\n", n1);
                        /*sprintf(szBuf,"%-10d1\n", n1);
                        swriteln(szBuf,strlen(szBuf),errfp);*/
                      }
                }
            }
            memset(dataRec, 0, dataLen);
            curRowNum=0;
        }
    }
    free(dataRec);
    return 0;
}

/***********************************************************
 * Function: 采用direct方式入库
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [I]addStr              :入库记录的附加字符串，如${FILE_NAME},${SYSDATE}等字段。
 *      [O]totalNum            :返回入库的总记录数
 *      [I]infp                :待入库的文件的文件指针
 *      [I]errfp               :错单对应的行数、及错误信息暂时记录在此文件中
 *      [I]FI                  :待入库的文件对应的字段信息
 *      [I]lctl                :direct方式入库使用的控制信息
 * Returns:
 *      0                      :无错误
 *      -1                     :出错
 **********************************************************/
short loadDirect(char *outStr, char *addStr, long *totalNum,
                 SFILE *infp, FILE *errfp, FILE_INFO *FI, struct loadctl  *lctl)
{
    /* initialize the load */
    if(init_load(outStr, lctl, &(FI->table_def))<0)
        return -1;

    /*在direct方式下，tailNum只支持0 1两种情况，不能大于1*/
    if(simple_load(outStr, totalNum, errfp, lctl, infp, addStr, FI)<0)
        return -1;

    return 0;
}

/***********************************************************
 * Function: 将指定文件入库
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [I]PT                  :本进程相关参数信息
 *      [I]FI                  :待入库的文件对应的字段信息
 *      [I]DBSession           :ORACLE事务指针
 *      [I]lctl                :direct方式入库使用的控制信息
 *      [I]fileName            :待入库的文件名
 * Returns:
 *      0                      :无错误
 *      -1                     :出错
 **********************************************************/
short processFile(char *outStr, PARAM_TABLE *PT, FILE_INFO *FI,
                  DB_SESSION *DBSession, struct loadctl  *lctl, char *fileName)
{
    char addStr[1000], tmpStr[1000], recStr[MAX_REC_STR_LEN];
    char inFile[MAX_FILE_NAME_LEN];
    char bakFile[MAX_FILE_NAME_LEN];
    char errFile[MAX_FILE_NAME_LEN], errtFile[MAX_FILE_NAME_LEN];
    char dupFile[MAX_FILE_NAME_LEN], duptFile[MAX_FILE_NAME_LEN];
    char outFile[MAX_FILE_NAME_LEN], outtFile[MAX_FILE_NAME_LEN];
    char tmpErr[MAX_FILE_NAME_LEN], logFile[MAX_FILE_NAME_LEN];
    char TEMP_ERR_LOG[100];
    /*FILE  *infp, *tmpfp, *errfp=NULL, *dupfp=NULL, *outfp=NULL, *logfp=NULL;*/
    SFILE  *infp,  *dupfp=NULL, *outfp=NULL;
    FILE  *tmpfp, *errfp=NULL,*logfp=NULL;
    long totalNum=0;
    int  i, errNum=0, dupNum=0, curNo=0, errNo;
    short flag;

    sprintf(TEMP_ERR_LOG, ".TEMP.ERR.%s", PT->procID);

    /*取附加字符串、表名*/
    if(GetTableStr(outStr, addStr, DBSession, FI, fileName)<0)
        return -1;

    /*生成各类文件名*/
    sprintf(inFile, "%s%s", PT->inPath, fileName);
    sprintf(bakFile, "%s%s", PT->bakPath, fileName);
    sprintf(errFile, "%s%s.err", PT->errPath, fileName);
    sprintf(dupFile, "%s%s.dup", PT->dupPath, fileName);
    sprintf(errtFile, "%s%s.err", PT->errPath, TEMP_ERR_LOG);
    sprintf(duptFile, "%s%s.dup", PT->dupPath, TEMP_ERR_LOG);
    sprintf(tmpErr, "%s%s", PT->proclogPath, TEMP_ERR_LOG);
		
		//printf("[%s][%d].........errPath=[%s].duptFile=[%s],tmpErr=[%s]\n",__FILE__,__LINE__,errtFile,duptFile,tmpErr);
		
    if(PT->outPath[0]!='\0')
    {
        sprintf(outFile, "%s%s%s%s", PT->outPath, PT->outPreFix, fileName, PT->outPostFix);
        sprintf(outtFile, "%s%s.out", PT->outPath, TEMP_ERR_LOG);
        outfp = sopen(outtFile,"w", &strSdfs);
    }
		

    /*打开源文件*/
    /*infp=fopen(inFile,"r");*/
    infp=sopen(inFile,"r", &strSdfs);
    if(NULL == infp)
    	printf("[%s][%d].........infp\n",__FILE__,__LINE__);
    	
		/*printf("[%s][%d].........headNum=[%d],\n",__FILE__,__LINE__,FI->headNum);  
		printf("[%s][%d].........fileName=[%s],file_head=[%s]\n",__FILE__,__LINE__,fileName,FI->file_head);  	
		*/
    for(i=0; i<FI->headNum; i++)
   	{	
   			/*printf("[%s][%d].........fileName=[%s],file_head=[%s]\n",__FILE__,__LINE__,FI->headNum,FI->file_head);*/
        sgets(recStr, sizeof(recStr), infp);  /*跳过头记录*/
     }
		
		/*printf("[%s][%d]....headNum=[%d],recStr=[%s]\n",__FILE__,__LINE__,FI->headNum,recStr);*/
    /*入库*/
    tmpfp=fopen(tmpErr,"w");
    
    if(FI->flagDirect==1)  /*direct 方式*/
        flag = loadDirect(outStr, addStr, &totalNum, infp, tmpfp, FI, lctl);
    else                   /*insert 方式*/
        flag = loadInsert(outStr, addStr, &totalNum, infp, tmpfp, FI, DBSession);

    fclose(tmpfp);
    if(flag<0)
    {
        sclose(infp);
        return -1;
    }

    /*处理错单、重单, 生成合法文件*/
    tmpfp=fopen(tmpErr,"r");
    srewind(infp);
    
    for(i=0; i<FI->headNum; i++)
        sgets(recStr, sizeof(recStr), infp);   /*跳过头记录*/
    while( fgets(tmpStr, 1000, tmpfp) !=NULL )
    {
        /*合法话单*/
        tmpStr[9]='\0';
        errNo = atoi(tmpStr)-1;
        for(i=curNo; i<errNo; i++)
        {
            memset(recStr, '\0', sizeof(recStr));
            sgets(recStr, MAX_REC_STR_LEN, infp);
            if(outfp!=NULL)
                sputs(recStr, outfp);
        }
        curNo = errNo + 1;

        memset(recStr, '\0', sizeof(recStr));
        sgets(recStr, MAX_REC_STR_LEN, infp);

        if(tmpStr[10]!='1')   /*错单*/
        {
            tmpStr[10+strlen(tmpStr+10)-1] = '\0';
            Writelog(LEVEL_WARN, "%s:%s", fileName, tmpStr+10);
            if(errfp==NULL)
                errfp = fopen(errtFile, "w");

            fwrite(recStr, sizeof(recStr), strlen(recStr), errfp );
            errNum++;
        }
        else
        {
            if(dupfp==NULL)
                dupfp = sopen(duptFile, "w", &strSdfs);
            //fprintf(dupfp, "%s", recStr);
            swriteln(recStr, strlen(recStr), dupfp);
            dupNum++;
        }
    }
    fclose(tmpfp);
    remove(tmpErr);

    if(outfp!=NULL)
    {
        for(i=curNo; i<totalNum; i++)
        {
            memset(recStr, '\0', sizeof(recStr));
            sgets(recStr, MAX_REC_STR_LEN, infp);
            sputs(recStr, outfp);
        }
    }
    sclose(infp);

    /*准备写日志*/
    GetSysDateTime(tmpStr);
    sprintf(recStr, "%-50s T: %d A: %d E: %d D: %d Time:%14s\n", fileName,\
                 totalNum, totalNum-errNum-dupNum, errNum, dupNum,tmpStr);
    sprintf(logFile, "%sTEMP_LOG.%s", PT->proclogPath, FI->proc_id );
    logfp=fopen(logFile,"a+");

    /*转移文件*/
    if(errfp!=NULL)
    {
        fclose(errfp);
        rename(errtFile, errFile);
    }
    if(dupfp!=NULL)
    {
        sclose(dupfp);
        srename(duptFile, dupFile, &strSdfs);
    }
    if(outfp!=NULL)
    {
        sclose(outfp);
        srename(outtFile, outFile, &strSdfs);
    }

    if(srename(inFile, bakFile, &strSdfs)!=0)
    {
        sprintf(tmpStr, "mv %s %s", inFile, PT->bakPath);
        system(tmpStr);
    }

    /*写日志*/
    fwrite(recStr, sizeof(char), strlen(recStr), logfp );
    fclose(logfp);

    /*commit*/
    if(FI->flagDirect==1)  /*direct 方式*/
        flag = finish_load(outStr, lctl);
    else                   /*insert 方式*/
        flag = ExecuteSql(outStr, DBSession, "commit");

    return 0;
}

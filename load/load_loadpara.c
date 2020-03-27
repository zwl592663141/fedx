
/***********************************************************
 * Copyright (c) 1999,2002 .
 * Descriptions:
 *      _loadpara.c
 *      加载相关参数.
 * Known bugs:
 *
 * Change log:
 *        who          MM/DD/YYYY        why
 *        wangjs       03/20/2005        create
 **********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "load_main.h"

/***********************************************************
 * Function: 获取命令行参数.
 * Parameter:
 *      [O]exePath              :主进程所在路径
 *      [O]exeName              :主进程名
 *      [O]lockFile             :LOCK文件名
 *      [I]argc                 :参数个数
 *      [I]argv                 :参数序列
 * Returns:
 *      0                       :程序继续运行
 *      -1                      :程序需要退出
 **********************************************************/
short GetArgParam(char *exePath, char *exeName, char *lockFile, int argc, char *argv[])
{
    char tmpStr[100], cmdStr[100];
    FILE *stream;

    /*获取可执行文件名及路径*/
    SeparatePathFile(exePath, exeName, argv[0]);

    if ( (argc != 4) && (argc != 2) )
    {
        printf("please input %s -h to view help\n",exeName);
        return -1;
    }

    if (argc == 4 )
    {
        sprintf( lockFile, "%s.%s.pid", exePath, argv[1]);
        return 0;
    }

    /*显示帮助信息*/
    if (strcmp(argv[1], "-h") == 0)
    {
        printf( "Usage:\n");
        printf("\t%s proc_id login_file dbname\n",exeName);
        printf("\t%s -h :to view help\n",exeName);
        printf("\t%s -v :to view version\n",exeName);
        printf("\t%s -qproc_id :to stop the program of proc_id.\n",exeName );
        return -1;
    }
    /*显示版本信息*/
    else if (strcmp(argv[1], "-v") == 0)
    {
        printf("\t%s Version %s \n", exeName, SOFT_VERSION);
        printf("\tCompiled at %s %s\n", __DATE__, __TIME__);
        printf("\tCopyrights hold by Si-tech company\n");
        exit(0);
    }
    /*退出程序*/
    else if (strncmp(argv[1], "-q", 2) == 0)
    {
        sprintf( lockFile, "%s.%s.pid", exePath, argv[1]+2);
        if(CheckUniqueCopy(lockFile) != 0)     /*程序已运行*/
        {
            stream = fopen(lockFile,"r");
            if(stream!=NULL)
            {
                memset(tmpStr, 0, sizeof(tmpStr));
                fgets(tmpStr,sizeof(tmpStr),stream);
                fclose(stream);
                sprintf(cmdStr, "kill %s", tmpStr);
                system(cmdStr);
            }
            printf("The Program '%s' exit success.\n",argv[1]+2);
        }
        else
            printf("The Program '%s' not exist.\n",argv[1]+2);
        return -1;
    }
    /*非法参数*/
    else
    {
        printf("please input %s -h to view help\n",exeName);
        return -1;
    }
}

/***********************************************************
 * Function: 查找指定的变量.
 * Parameter:
 *      [O]outStr                :输出的操作结果描述
 *      [O]outV                  :输出指定变量对应的值
 *      [I]inParam               :指定的变量名称
 *      [I]rec                   :存放变量值的数组
 *      [I]rowNum                :数组的总长度
 * Returns:
 *      0                         :找到
 *      -1                        :未找到
 **********************************************************/
short GetParamValue(char *outStr, char *outV, char *inParam, TABLE_RECORD *rec, int rowNum)
{
    int i;

    for(i=0; i<rowNum; i++)
    {
        if(strcmp(inParam, (rec+2*i)->field)==0)
        {
            sprintf(outV, "%s", (rec+2*i+1)->field);
            return 0;
        }
    }
    printf("[%s][%d].......outV=[%s],inParam=[%s]\n",__FILE__,__LINE__,outV,inParam);
    sprintf(outStr, "the param \"%s\" is not found", inParam);
    return -1;
}

/********  new  add  end **********/
/**********************************************************
Function:               getZkpNodeInfo()
Description:    获取zookeper返回的节点信息（根据；分隔）
Input:                  void* pTaskEntry, 任务入口
Output:                 无
Return:                 void*
Others:
**********************************************************/

int getZkpNodeInfo(char *nodeInfo,int index,char *subInfo)
{
             char tmp_str[1024+1];
             strcpy(tmp_str,nodeInfo);
             char *str=tmp_str;
             int tag=0;
             char *token=strsep(&str,";");
                 while( token != NULL )
       {
         if(tag==index)
         {
                 strcpy(subInfo,token);
                 break;
         }
         tag++;
         token=strsep(&str,";");

       }

       return 0;
}

int  mkdir_p(const char * filename)
{
	const char * pdir = filename;
	char tempdir[1024+1] = {0};

  char szError[256];


	while(*pdir)
	{
		if( (*pdir == '\\') || (*pdir == '/') )
		{
			if( (pdir - filename) > 1024)
				break;

			strncpy(tempdir,filename,pdir - filename);
			tempdir[pdir-filename] = '\0';

			if( tempdir[0] != 0 && access(tempdir,00) != 0)
			{
				if( 0 != smkdir(tempdir,0755,&strSdfs) )
				{
					if( saccess(tempdir,00,&strSdfs) != 0 )
					{
						fprintf(stderr, "mkdir %s error!\n", tempdir);
						return 1;
					}
				}
			}
		}
		pdir++;
	}
	return 0;
}
int Mkdir_SubFile(char *chDir_name, PARAM_TABLE *PT)
{
	int iRet=0;
	
	sprintf(chDir_name,"%s/%s/",chDir_name,PT->stZookeeper.chProcessName);
	
	
	printf("[%s][%d]Mkdir_SubFile [%s] create begin\n",__FILE__,__LINE__, chDir_name);

	
	if(0 != mkdir_p(chDir_name))
	{
		printf("[%s][%d]Mkdir_SubFile error ! [%s] create failed ...\n",__FILE__,__LINE__, chDir_name);
		return -1;
	}

  return 0;
	
}
/***********************************************************
 * Function: 从param表中读取入口参数.
 * Parameter:
 *      [O]outStr                 :输出的操作结果描述
 *      [O]PT                     :存储参数的结构体指针
 *      [I]DBSession              :数据库事务指针
 *      [I]inTableName            :参数表表名
 *      [I]procID                 :本进程的进程标识
 * Returns:
 *      0                         :成功
 *      -1                        :操作失败
 **********************************************************/
short GetParamFromTable(char *outStr, PARAM_TABLE *PT,
                        DB_SESSION *DBSession, char *inTableName, char *procID)
{
    char tmpSql[300], tmpStr[100], *p, tmpOut[300];
    int i, rowNum;
    TABLE_RECORD tmpRec[2];
    TABLE_RECORD *rec;
    short flag;
    int Ret;
 	 	char szNodeInfo[1024];                                                                                           
  	char szClusterInfo[1024];                                                                                                   
  	memset(szNodeInfo,0,sizeof(szNodeInfo));                                                                         
  	memset(szClusterInfo,0,sizeof(szClusterInfo));     

    i = 0;
    i += sprintf(tmpSql + i, "select param_name,nvl(trim(param_value),'@') ");
    i += sprintf(tmpSql + i, "from %s ", inTableName);
    i += sprintf(tmpSql + i, "where proc_name='%s' ", procID);
    i += sprintf(tmpSql + i, "order by proc_name ");

    rowNum = GetCountOfTable(outStr, DBSession, tmpSql);/*取表的行数*/
    flag=GetRowFromTable(outStr, DBSession, tmpRec, 2, tmpSql, NULL, 0);
    if(flag != 0)
    {
        #ifdef DEBUG_INFO
            printf("ERR SQL: %s\n", tmpSql);
        #endif
        return -1;
    }

    /*取出所有参数*/
    rec = (TABLE_RECORD *)malloc( rowNum * 2 * sizeof( TABLE_RECORD ) );
    if (rec == (TABLE_RECORD *)0)
    {
        sprintf(outStr, "malloc(%d) failure.\tError occurred at File %s:%d",
            rowNum * 2 * sizeof( TABLE_RECORD ), __FILE__, __LINE__);
        return -1;
    }
    memset(rec, 0, rowNum * 2 * sizeof( TABLE_RECORD ) );
    memset(PT,  0, sizeof(PARAM_TABLE));
    for(i=0; i< rowNum; i++)
    {
        sprintf( (rec+i*2)->field, "%s", tmpRec[0].field);
        sprintf( (rec+i*2+1)->field, "%s", tmpRec[1].field);

        flag = FetchData(outStr, DBSession);
        if(flag != 0) break;
    }

    /*取日志路径*/
    if( (GetParamValue(outStr, PT->runlogPath, "runlog_path", rec, rowNum) !=0)
         )
    {
        free(rec);
        return -1;
    }
    /*初始化日志文件*/
    Initlog(PT->runlogPath, procID);
    Writelog(LEVEL_INFO, "Program started!");

    /*取相关参数*/
    if( (GetParamValue(outStr, PT->inPath, "in_path", rec, rowNum) !=0)||
        (GetParamValue(outStr, PT->bakPath, "bak_path", rec, rowNum) !=0)||
        (GetParamValue(outStr, PT->dupPath, "dup_path", rec, rowNum) !=0)||
        (GetParamValue(outStr, PT->errPath, "err_path", rec, rowNum) !=0)||
        (GetParamValue(outStr, tmpOut, "out_path", rec, rowNum) !=0)||
        (GetParamValue(outStr, PT->proclogPath, "proclog_path", rec, rowNum) !=0)||
        (GetParamValue(outStr, PT->sysTable, "run_table", rec, rowNum) !=0)||
        (GetParamValue(outStr, tmpStr, "run_flag", rec, rowNum) !=0) ||
        (GetParamValue(outStr, PT->stZookeeper.chCluster, "cluster", rec, rowNum) !=0) ||
        (GetParamValue(outStr, PT->stZookeeper.chAttr, "attr", rec, rowNum) !=0)||
        (GetParamValue(outStr, PT->stZookeeper.chService, "service", rec, rowNum) !=0) ||
        (GetParamValue(outStr, PT->bLoadFile, "b_load_file", rec, rowNum) !=0)||
        (GetParamValue(outStr, PT->bLoadField, "b_load_field", rec, rowNum) !=0))
    {
        free(rec);
        return -1;
    }
    free(rec);
    PT->runFlag = tmpStr[0];

		if( 0 != InitInfo(1, PT->stZookeeper.chProcessName, PT->stZookeeper.chCluster, PT->stZookeeper.chAttr, PT->stZookeeper.chService) )      
 	 	{                                                                                                                
			printf("ZKClient InitClientInfo error!!!\n");
			return -1;                                                                                                      
  	} 
                                                                
  	if( 0 != Register( szNodeInfo, szClusterInfo, 1) )                                                               
  	{                                                                                                                
			return -1;                                                                                                
  	}                                                                                                                
   
 		/*解析zookeeper注册后的buf*/                                                                                                                
                                                                                    
  	getZkpNodeInfo(szNodeInfo,1,PT->stZookeeper.chInport);                                                               
  	getZkpNodeInfo(szNodeInfo,2,PT->stZookeeper.chProcessName);  
  	
  	//printf("[%s][%d]=Mkdir_SubFile error! Ret=[%d]  inPath=[%s/%s]\n",__FILE__,__LINE__, Ret,PT->inPath,PT->stZookeeper.chProcessName);
  	
  	Ret = Mkdir_SubFile(PT->inPath,PT);
		if( 0 != Ret )
		{
			printf("[%s][%d]=Mkdir_SubFile error! Ret=[%d]  inPath=[%s/%s]\n",__FILE__,__LINE__, Ret,PT->inPath,PT->stZookeeper.chProcessName);
			return -1 ;
		}	 
		 	
  	Ret = Mkdir_SubFile(PT->bakPath,PT);
		if( 0 != Ret )
		{
			printf("[%s][%d]=Mkdir_SubFile error! Ret=[%d]  bakPath=[%s/%s]\n",__FILE__,__LINE__, Ret,PT->bakPath,PT->stZookeeper.chProcessName);
			return -1 ;
		}	   
			
  	Ret = Mkdir_SubFile(PT->dupPath,PT);
		if( 0 != Ret )
		{
			printf("[%s][%d]=Mkdir_SubFile error! Ret=[%d]  dupPath=[%s/%s]\n",__FILE__,__LINE__, Ret,PT->dupPath,PT->stZookeeper.chProcessName);
			return -1 ;
		}	 
		  	
  	Ret = Mkdir_SubFile(PT->errPath,PT);
		if( 0 != Ret )
		{
			printf("[%s][%d]=Mkdir_SubFile error! Ret=[%d]  errPath=[%s/%s]\n",__FILE__,__LINE__, Ret,PT->errPath,PT->stZookeeper.chProcessName);
			return -1 ;
		}	 
		  	
    /*检查路径*/
    if( (CheckValidPath(outStr, PT->inPath) !=0)||
        (CheckValidPath(outStr, PT->bakPath) !=0)||
        (CheckValidPath(outStr, PT->dupPath) !=0)||
        (CheckValidPath(outStr, PT->errPath) !=0)||
        (CheckValidPath(outStr, PT->proclogPath) !=0) )
        return -1;

    /*输出文件*/
    if(strcmp(tmpOut, "@")==0)
        PT->outPath[0]='\0';
    else
    {
        p=strchr(tmpOut, '*');
        if(p==NULL)
        {
            sprintf(outStr, "out_path error, can't find '*'.  [%s]", tmpOut);
            return -1;
        }
        SeparatePathFile( PT->outPath, tmpStr, tmpOut);
        sprintf(PT->outPostFix, "%s", p+1);
        strncpy(PT->outPreFix, tmpStr, strlen(tmpStr)-strlen(p));
        if(CheckValidPath(outStr, PT->outPath) !=0)
            return -1;
    }
    Ret = Mkdir_SubFile(PT->outPath,PT);
		if( 0 != Ret )
		{
			printf("[%s][%d]=Mkdir_SubFile error! Ret=[%d]  outPath=[%s/%s]\n",__FILE__,__LINE__, Ret,PT->outPath,PT->stZookeeper.chProcessName);
			return -1 ;
		}	   
			
    sprintf(PT->procID, "%s", procID);

                                                          
   
    return 0;
}

/***********************************************************
 * Function: 取字段定义表信息.
 * Parameter:
 *      [O]outStr                 :输出的操作结果描述
 *      [O]outStruct              :输出的参数表数据
 *      [I]DBSession              :数据库事务指针
 *      [I]inFileName             :文件类型定义表的表名
 *      [I]inFieldName            :字段配置表的表名
 *      [I]procID                 :本进程的进程标识名
 * Returns:
 *      >=0                       :参数表行数
 *      -1                        :操作失败
 **********************************************************/
int LoadFileInfo(char *outStr, FILE_INFO **outStruct, DB_SESSION *DBSession,
                 char *inFileName, char *inFieldName, char *procID)
{
    char tmpSql[300];
    char *p, tmpStr[100], *tmpP;
    int outRowNum, tmpColCount, i, j=0, len;
    TABLE_RECORD tmpRec[50];
    short flag;

    i = 0;
    sprintf(tmpSql,"select file_head from %s where proc_name='%s' and field_id in (select distinct id from %s)",
        inFileName, procID, inFieldName);
    outRowNum = GetCountOfTable(outStr, DBSession, tmpSql);
    if(outRowNum==0)
        sprintf(outStr, "NO DATA FOUND");

    if( outRowNum<=0)
    {
        #ifdef DEBUG_INFO
            printf("ERR SQL: %s\n", tmpSql);
        #endif
        return -1;
    }

    /*取出所有参数*/
    i = 0;
    i += sprintf(tmpSql + i, "select file_head,nvl(date_format,'@'),table_name,seq,field_name,pos,len,data_type,nvl(format,'@'),load_type,nvl(cancel_line,'0,0'), a.field_id ");
    i += sprintf(tmpSql + i, "from %s a,%s b where proc_name='%s' and a.field_id=b.id  order by file_head,seq", inFileName, inFieldName, procID);

    tmpColCount = GetColNumFromSql(tmpSql);
    if(GetRowFromTable(outStr, DBSession, tmpRec, tmpColCount, tmpSql, NULL, 0)!=0)
    {
        #ifdef DEBUG_INFO
            printf("ERR SQL: %s\n", tmpSql);
        #endif
        return -1;
    }

    if(*outStruct != NULL)
    {
        free(*outStruct);
        *outStruct = NULL;
    }
    *outStruct = (FILE_INFO *)malloc( outRowNum * sizeof( FILE_INFO ) );
    if (*outStruct == (FILE_INFO *)0)
    {
        sprintf(outStr, "malloc(%d) failure.\tError occurred at File %s:%d",
            outRowNum * sizeof( FILE_INFO ), __FILE__, __LINE__);
        return -1;
    }
    memset(*outStruct, 0, outRowNum * sizeof( FILE_INFO ) );

    i=-1;
    while(1)
    {
        if((i==-1)||(strcmp((*outStruct+i)->file_head, tmpRec[0].field)!=0))
        {
            i++;
            sprintf( (*outStruct+i)->proc_id, "%s", procID);
            sprintf( (*outStruct+i)->file_head, "%s", tmpRec[0].field);
            if(tmpRec[1].field[0]!='@')
                sprintf( (*outStruct+i)->date_format, "%s", tmpRec[1].field);
            (*outStruct+i)->flagDirect = atoi(tmpRec[9].field);            
            
            /*待入库表名*/
            sprintf( (*outStruct+i)->table, "%s", tmpRec[2].field);
            if(strstr((*outStruct+i)->table, "'${FILE_NAME}'") != NULL)
            {
                (*outStruct+i)->table_fileFlag=1;
                strRep((*outStruct+i)->table, "'${FILE_NAME}'", ":v1");
            }
            else
                (*outStruct+i)->table_fileFlag=0;  /*表名和文件名无关*/            

            /*取头尾记录行数*/
            sprintf(tmpStr, "%s", tmpRec[10].field);
            p = strchr(tmpStr, ',');
            if(p==NULL)
            {
                (*outStruct+i)->headNum=atoi(tmpStr);
                (*outStruct+i)->tailNum=0;
            }
            else
            {
                (*outStruct+i)->tailNum=atoi(p+1);
                p[0]=0;
                (*outStruct+i)->headNum=atoi(tmpStr);                
            }
            
            /*对于尾记录，本程序只支持尾记录最多为1行的情况*/
            if((*outStruct+i)->tailNum > 1)
                (*outStruct+i)->tailNum = 1;

            /*判断是否变长格式*/
            tmpP = strchr(tmpRec[11].field, '[');
            len = strlen(tmpRec[11].field);
            if( (NULL != tmpP) && (']' == tmpRec[11].field[len-1]) )
            {
                tmpRec[11].field[len-1] = 0;
                if(0 == strcmp(tmpP+1, "\\t") )
                    strcpy((*outStruct+i)->segChar, "\t");
                else
                    strcpy((*outStruct+i)->segChar, tmpP+1);
            }
            else
                (*outStruct+i)->segChar[0] = 0;  /*定长格式*/
            
            j=0;
        }
        (*outStruct+i)->field[j].seq = atoi(tmpRec[3].field);
        sprintf( (*outStruct+i)->field[j].field_name, "%s", tmpRec[4].field);
        (*outStruct+i)->field[j].pos = atoi(tmpRec[5].field);
        (*outStruct+i)->field[j].len = atoi(tmpRec[6].field);
        sprintf( (*outStruct+i)->field[j].data_type, "%s", tmpRec[7].field);
        sprintf( (*outStruct+i)->field[j].format, "%s", (strcmp(tmpRec[8].field,"@")==0)?"":tmpRec[8].field);
        j++;
        
        (*outStruct+i)->fieldNum = j;

        flag = FetchData(outStr, DBSession);
        if(flag != 0) break;
    }

    return outRowNum;
}

/***********************************************************
 * Function: 取系统运行状态.
 * Parameter:
 *      [O]outStr                 :输出的操作结果描述
 *      [I]DBSession              :数据库事务指针
 *      [I]inTableName            :调度表表名
 *      [I]stopFlag               :是否是退出时调用 0 否，1是
 *      [I]procId                 :进程标识
 * Returns:
 *      >=0                       :系统运行状态
 *      -1                        :操作失败
 **********************************************************/
int GetSysStatus(char *outStr, DB_SESSION *DBSession, char *inTableName, short stopFlag, char *procId)
{
    char tmpSql[300], tmpStr[200];
    TABLE_RECORD tmpRec[10];
    int id, runFlag;
    short flag;

    /*单次执行，无需判断状态*/
    if(gs_paramTable.runFlag=='0')
        return 0;

    /*如果是退出时执行*/
    if(stopFlag==1)
    {
        sprintf(tmpSql, "update %s set process_status='1' where program_name='%s'",inTableName, procId);
        ExecuteSql(outStr, DBSession, tmpSql);
        ExecuteSql(outStr, DBSession, "commit");
        return 0;
    }

    sprintf(tmpSql, "select operator_id,process_status from %s where program_name='%s' ", inTableName, procId);
    flag=GetRowFromTable(tmpStr, DBSession, tmpRec, 2, tmpSql, NULL, 0);
    if(flag!=0)
    {
        sprintf(outStr, "Get operator_id of %s error [%s]", procId, tmpStr);
        #ifdef DEBUG_INFO
            printf("ERR SQL: %s\n%s\n", tmpSql,outStr);
        #endif
        return -1;
    }
    else
    {
        id = atoi(tmpRec[0].field);
        runFlag = atoi(tmpRec[1].field);
        
        if( (id!=1) && ( (id==2)||(runFlag!=0) ) )
        {
            sprintf(tmpSql, "update %s set operator_id='0',process_status='0' where program_name='%s'",inTableName, procId);
            if(ExecuteSql(tmpStr, DBSession, tmpSql)<0)
            {
                sprintf(outStr, "%s [%s]", tmpStr, inTableName);
                #ifdef DEBUG_INFO
                    printf("ERR SQL: %s\n%s\n", tmpSql,outStr);
                #endif
                return -1;
            }
            ExecuteSql(outStr, DBSession, "commit");
        }
        return id;
    }
}

/***********************************************************
 * Function: 加载所有表.
 * Parameter:
 *      [O]outStr                 :输出的操作结果描述
 *      [I]DBSession              :数据库事务指针
 *      [I]procID                 :本进程的进程标识
 * Returns:
 *       0                        :成功
 *       -1                       :失败
 **********************************************************/
short GetAllParam(char *outStr, DB_SESSION *DBSession, char *procID)
{
    int i,j,tmpI, pos;
    FILE_INFO *tmpF;

    /*读参数表，初始化入口参数*/
    if(GetParamFromTable(outStr, &gs_paramTable, DBSession, TABLE_PARAM, procID)!=0)
    {
        sprintf(outStr, "%s [%s]", outStr, TABLE_PARAM);
        return -1;
    }

    /*读文件类型，及其对应的话单格式*/
    if( (gi_FileNum=LoadFileInfo(outStr, &gs_FileInfo, DBSession, gs_paramTable.bLoadFile, gs_paramTable.bLoadField, procID) )<=0)
    {
        sprintf(outStr, "%s [%s,%s]", outStr, gs_paramTable.bLoadFile,gs_paramTable.bLoadField);
        return -1;
    }

    /*取direct方式的字段定义*/
    for(i=0;i<gi_FileNum;i++)
    {
        tmpF = (gs_FileInfo+i);
        tmpI=0;
        for(j=0;j<tmpF->fieldNum;j++)
        {
            if(tmpF->field[j].format[0]!='\0')
            {
                tmpF->dfield[j].begpos_fld = tmpI + 1;
                tmpI += tmpF->field[j].len;
            }
        }

        pos = 1;
        for(j=0;j<tmpF->fieldNum;j++)
        {
            sprintf(tmpF->column[j].name_col, "%s", tmpF->field[j].field_name);
            tmpF->column[j].id_col = 0;
            tmpF->column[j].exttyp_col = SQLT_CHR;
            tmpF->column[j].date_col = (strcmp(tmpF->field[j].data_type,"date")==0)?1:0;

            if(tmpF->field[j].format[0]=='\0')
            {
                if(0 == tmpF->segChar[0]) /*定长*/
                    tmpF->dfield[j].begpos_fld = tmpI + tmpF->field[j].pos;
                else
                {
                    tmpF->dfield[j].begpos_fld = tmpI + pos;
                    pos += tmpF->field[j].len;
                }
            }
            tmpF->dfield[j].maxlen_fld = tmpF->field[j].len;
            tmpF->dfield[j].endpos_fld = tmpF->dfield[j].begpos_fld+tmpF->dfield[j].maxlen_fld-1;

            tmpF->dfield[j].flag_fld = ( (strcmp(tmpF->field[j].data_type,"varchar")==0)||(strcmp(tmpF->field[j].data_type,"date")==0) )?FLD_INLINE|FLD_STRIP_TRAIL_BLANK:FLD_INLINE;

            if(0 == strncmp("$F_", tmpF->field[j].format, 3))
                tmpF->field[j].funcFlag = 1;
            else if(0 == strncmp("$V_", tmpF->field[j].format, 3))
                tmpF->field[j].funcFlag = 2;
            else
                tmpF->field[j].funcFlag = 0;
        }
        sprintf(tmpF->table_def.owner,"%s",gs_dbInfo.dbUser);
        sprintf(tmpF->table_def.date_mask,"%s",tmpF->date_format);
        tmpF->table_def.col_num = tmpF->fieldNum;
        tmpF->table_def.column = (struct col_def *)(&(tmpF->column[0]));
        tmpF->table_def.field = (struct fld_def *)(&(tmpF->dfield[0]));
    }
    return 0;
}


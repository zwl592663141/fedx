#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "load_main.h"


#ifdef UNIX_OS
  #include <unistd.h>
  #include <signal.h>
#endif

/***********************************************************
 * Function: 退出程序，断开数据库，释放内存.
 * Parameter:
 *      [I]inExitStatus      :退出选项
 * Returns:
 *      none
 **********************************************************/
void ExitProgram(int inExitStatus)
{
    char outStr[500];
    char tmpStr[200];
    time_t tmTmp;
    char oldFileName[500];
    char newFileName[500];

    /*异常退出时，将临时日志改为正式日志*/
    time(&tmTmp);
    strftime(tmpStr, 15, "%Y%m%d%H%M%S", localtime(&tmTmp));

    sprintf(oldFileName,"%sTEMP_LOG.%s", gs_paramTable.proclogPath, gs_FileInfo->proc_id);
    sprintf(newFileName,"%s%s.%s.%s",gs_paramTable.proclogPath,gs_FileInfo->proc_id,tmpStr,"log");
    rename(oldFileName,newFileName);

    ExecuteSql(outStr, &(gs_dbInfo.dbSession), "rollback");

    /*置停止标志*/
    GetSysStatus(outStr, &(gs_dbInfo.dbSession), gs_paramTable.sysTable, 1, gs_paramTable.procID);
    /* 关闭与数据库的连接*/
    DisconnectDBMS(&(gs_dbInfo.dbSession));
    cleanup(&gs_dpctl, (sb4)0);

    /*释放内存*/
    if(gs_FileInfo != NULL)
        free(gs_FileInfo);

    /* 恢复原来的中断键处理 */
    #ifdef UNIX_OS
        signal( SIGTERM, SIG_DFL );
        signal( SIGSEGV, SIG_DFL );
        signal( SIGBUS, SIG_DFL );
    #endif

    Writelog(LEVEL_INFO, "Program stopped![%d]", inExitStatus);
    exit(inExitStatus);
}

/***********************************************************
 * Function: 主程序
 **********************************************************/
int main(int argc, char *argv[])
{
    char outStr[500], tmpStr[300];
    char exePath[MAX_FILE_NAME_LEN];         /*可执行文件路径*/
    char exeName[MAX_FILE_NAME_LEN];         /*可执行文件名*/
    char lockFile[MAX_FILE_NAME_LEN];        /*锁文件所在目录及文件名*/
    int  i, j, flagStatus;
    char procID[50];                         /*进程标识*/
    char oldFileName[500];                  /*旧文件名*/
    char newFileName[500];                  /*新文件名*/
    char fileHead[20+1];
    char fileName[500+1];
    short flag;
    time_t  tmTmp;
    long preTime, nowTime;
    int len = 0;
    Writelog(LEVEL_ERR, "PROGRAM is start ...");
    printf("PROGRAM is start ...\n");
    memset(fileHead,0,sizeof(fileHead));
		
		char szError[40+1];
    memset( &strSdfs, 0, sizeof(SDFS));
    if( -1 == sinit("h", &strSdfs, szError))
    {
        printf("SDFS start failed...\n");
        sdestory( &(strSdfs) );
    }
    printf("GetArgParam is start ...\n");
    /*获取命令行参数*/
    if(GetArgParam(exePath, exeName, lockFile, argc, argv)!=0)
        exit(0);
    printf("CheckUniqueCopy is start ...\n");
    /*检查进程是否运行*/
 /*   if(CheckUniqueCopy(lockFile) != 0)
    {
        printf("Programm already running.\n");
        exit(0);
    }*/
    printf("获取数据库信息 is start ...\n");
    sprintf(procID, argv[1]);
    sprintf(gs_dbInfo.dbFile,argv[2]);
    sprintf(gs_dbInfo.dbServer,argv[3]);

    /*获取数据库信息*/
    if(GetLoginInfo(gs_dbInfo.dbFile, gs_dbInfo.dbUser, gs_dbInfo.dbPass)!=0)
    {
        printf("Error on load login file: %s\n", gs_dbInfo.dbFile);
        exit(0);
    }

    /*登录数据库*/
    if( ConnectDBMS(outStr, &(gs_dbInfo.dbSession), gs_dbInfo.dbUser,
                   gs_dbInfo.dbPass, gs_dbInfo.dbServer) != 0)
    {
        printf("%s(%s/%s@%s)\n", outStr,gs_dbInfo.dbUser, gs_dbInfo.dbPass, gs_dbInfo.dbServer);
        exit(0);
    }

    /*截获退出信号*/
    #ifdef UNIX_OS
        signal( SIGTERM, ExitProgram );
        signal( SIGSEGV, ExitProgram );
        signal( SIGBUS, ExitProgram );
    #endif

    /*加载所有参数*/
    if(GetAllParam(outStr, &(gs_dbInfo.dbSession), procID)<0)
    {
        printf("%s\n", outStr);
        Writelog(LEVEL_ERR, outStr);
        ExitProgram(0);
    }
		
    /*direct方式,登录数据库*/
    if(Connect_DBDP(outStr,&gs_dpctl,gs_dbInfo.dbUser,gs_dbInfo.dbPass, gs_dbInfo.dbServer)<0)
    {
        printf("%s\n", outStr);
        Writelog(LEVEL_ERR, outStr);
        ExitProgram(0);
    }
		
    /*申请文件名列表的内存*/
    gc_FileList = (char *)malloc(MAX_FILE_NUM*MAX_FLIST_LEN);
    if(gc_FileList == NULL)
    {
        sprintf(outStr, "malloc fail, the parameter of \"%d\" is too large", MAX_FILE_NUM*MAX_FLIST_LEN);
        Writelog(LEVEL_ERR, outStr);
        ExitProgram(0);
    }
		
		//printf("[%s][%d]......( gs_FileInfo+i)->headNum=[%d]\n",__FILE__,__LINE__,( gs_FileInfo+i)->headNum);
    /*处理文件*/
    time(&tmTmp);
    preTime = tmTmp / LOG_INTERVAL;
    while(1)
    {
        TAG_START:
        flag=0;
        for(i=0; i<gi_FileNum; i++)
        {
            /*设置日期格式*/
            if( ((gs_FileInfo+i)->date_format[0]!=0) && ((gs_FileInfo+i)->flagDirect==0) )
            {
                sprintf(tmpStr, "ALTER SESSION SET NLS_DATE_FORMAT = '%s'", (gs_FileInfo+i)->date_format);
                ExecuteSql(outStr, &(gs_dbInfo.dbSession), tmpStr);
            }
						
            /*取文件列表，逐个处理文件*/
            memset(gc_FileList, '\0', MAX_FILE_NUM*MAX_FLIST_LEN );
            GetFileList(gc_FileList, gs_paramTable.inPath, (gs_FileInfo+i)->file_head, MAX_FLIST_LEN, MAX_FILE_NUM);
            for(j=0; j<MAX_FILE_NUM; j++)
            {
                /*检查调度状态*/
                flagStatus = GetSysStatus(outStr, &(gs_dbInfo.dbSession), gs_paramTable.sysTable, 0, procID);

                switch( flagStatus )
                {
                  case 0: /*正常*/
                    break;
                  case 1: /*退出*/
                    {
                        Writelog(LEVEL_ERR, "Exit program because run status values is 1");
                        ExitProgram(0);
                    }
                  case 2: /*重新初始化*/
                    {
                        Writelog(LEVEL_INFO, "Re initialize the parameter.");
                        if(GetAllParam(outStr, &(gs_dbInfo.dbSession), procID)<0)
                        {
                            Writelog(LEVEL_ERR, outStr);
                            ExitProgram(0);
                        }
                        goto TAG_START;
                    }
                  case -1: /*出错*/
                    {
                        Writelog(LEVEL_ERR, outStr);
                        ExitProgram(0);
                    }
                  default:
                    {}
                }
								
                if(gc_FileList[MAX_FLIST_LEN*j]=='\0')
                    break;
                    
		//判断处理文件和配置文件头是否为同一类文件，如果不是不进行处理
		getTextField(fileHead,(gs_FileInfo+i)->file_head,'.',1);
		len = strlen(fileHead);
    		memset(fileName,0,sizeof(fileName));
		strncpy(fileName,gc_FileList + MAX_FLIST_LEN*j,len);
		printf("[%s][%d]....fileHead=[%s],filename=[%s]\n",__FILE__,__LINE__,fileHead,fileName);
		if(0 != strncmp(fileHead,fileName,strlen(fileHead)))
			continue;
								
                /*处理一个文件*/
                printf("[%s][%d]....file_head=[%s],filename=[%s]\n",__FILE__,__LINE__,(gs_FileInfo+i)->file_head,gc_FileList + MAX_FLIST_LEN*j);
                if(processFile(outStr, &(gs_paramTable), gs_FileInfo+i,
                        &(gs_dbInfo.dbSession), &gs_dpctl, gc_FileList + MAX_FLIST_LEN*j)<0)
                {
                    Writelog(LEVEL_ERR, "%s,file name is:%s",outStr, gc_FileList + MAX_FLIST_LEN*j);
                    ExitProgram(0);
                }
                flag=1;
            }
        }

        /*判断是否输出日志*/
        time(&tmTmp);
        nowTime = tmTmp / LOG_INTERVAL;
        if(nowTime != preTime)
        {
            preTime = nowTime;
            strftime(tmpStr, 15, "%Y%m%d%H%M%S", localtime(&tmTmp));
            sprintf(oldFileName,"%sTEMP_LOG.%s", gs_paramTable.proclogPath, gs_FileInfo->proc_id);
            sprintf(newFileName,"%s%s.%s.%s",gs_paramTable.proclogPath,gs_FileInfo->proc_id,tmpStr,"log");
            rename(oldFileName,newFileName);
        }

        /*还有文件，继续循环*/
        if(flag==1)    
            continue;  
        
        /*如果不需要循环，处理完毕，退出*/
        if(gs_paramTable.runFlag=='0')    
            break;    

        #ifdef UNIX_OS
            sleep(10);   /*程序进入休眠*/
        #endif
    }

    ExitProgram(0);
    exit(0);
}

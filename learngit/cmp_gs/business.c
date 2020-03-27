/*
Copyright (C), 1995-2004, Si-Tech Information Technology Ltd. 
file_name:	business.c
author:		caoshun
version:	1.02
date:		2004-11-23
description:
			话单核对-业务函数-代码文件
others:
history:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mydefine.h"
#include "o2n_cmp.h"
#include "business.h"
#include "function.h"

#include "CookieFunc.h"
#include "ErrLog.h"
#include "IniFile.h"

/* 本地全局变量 */

/* 系统数据*/
struct stApp_Data g_AppData;
/* 命令行参数 */
struct stApp_Argv g_AppArgv;
/* 日志对象 */
struct stErrLog g_RunLog;
/* 配置文件对象 */
struct stIniFile_File g_IniFile;
/*线程对象*/
struct stApp_Thread g_AppThread[MAX_THREAD_NUMBER];
/* 内存泄漏句柄数 */
int g_MemLeak;

/*
function:   busi_init()
description:
            初始化应用系统所有全局数据
Input:      
Output:
Return:     
others: 
*/
void busi_init(void)
{
	/* 初始化全局变量 */
	memset(&g_AppData, 0, sizeof(g_AppData));
	memset(&g_AppArgv, 0, sizeof(g_AppArgv));
	memset(g_AppThread, 0, sizeof(g_AppThread));
	
	/* 取当前系统日期与时间 */
	cf_time_getcurrent(g_AppData.chDealDate);

	/* 日志对象初始化 */
	ErrLog_Init(&g_RunLog);

	/* 配置文件对象初始化 */
	IniFile_Init(&g_IniFile);

	g_MemLeak = 0;

	memset(&g_AppThread, 0, sizeof(g_AppThread));
}

/*
function:   busi_write_runlog()
description:
            输出运行控制信息到屏幕或日志文件
Input:      
            errmsg:无回车的输出信息
            file:__FILE__
            line:__LINE__
Output:
Return:     
			0:成功，other:不成功
others: 
*/
int busi_write_runlog(char *errmsg, char *file, int line)
{
	int iRcode;
	
#ifdef DEBUG
	fprintf(stderr, "%s,%d,%s\n", file, line, errmsg);
#else
	fprintf(stderr, "%s,%d,%s[%d]\n", file, line, errmsg,errno);
#endif

	if(ErrLog_ifLogFile(&g_RunLog) == 0)
	{
		iRcode = ErrLog_Log(&g_RunLog, errmsg, file, line);
		if(iRcode)
		{
			fprintf(stderr, "%s\n", ERROR_APP_0020);
			return -1;
		}
	}
	
	return 0;
}

/*
function:   busi_version()
description:
			显示程序的版本信息与使用说明
Input:      
Output:
Return:     
others: 
*/
void busi_version(void)
{
	printf("Copyrights(C) 1995-2006 SI-TECH CO.,LTD\n");
	printf("The iBOSS Billing %s. Version %s\n", APPLICATION_NAME, APPLICATION_VERSION);
	printf("Usage:%s <cfg_file>\n", APPLICATION_NAME);
}


/*
function:   busi_set_runlog()
description:
			设置运行日志文件
Input:      
Output:
            0:成功，other:不成功
Return:     
others: 
*/
int busi_set_runlog(void)
{
	int iRcode;
	char chFileName[512+1];

	/* 打开日志文件 */
	memset(chFileName, 0, sizeof(chFileName));

	sprintf(chFileName, "%s/%s.%s.log", g_AppData.chPath_Log, APPLICATION_NAME, g_AppData.chDealDate);
	iRcode=ErrLog_SetLogFile(&g_RunLog, chFileName);
if(iRcode)
	{
		fprintf(stderr, "%s\n", ERROR_APP_0020);
		return -1;
	}
	
	/* 日志文件格式设置 */
	ErrLog_SetLogFormat(&g_RunLog,
		ERRLOG_FORMAT_TIME
		|
		ERRLOG_FORMAT_FILE
		|
		ERRLOG_FORMAT_LINE
	);
	
	return 0;
}

/*
function:   busi_usage()
description:
            分析命令行参数个数与内容是否合法，否则显示帮助信息。
Input:      
            argc:命令行参数个数
            argv:命令行参数列表
Output:
Return:     0:成功，other:不成功
others: 
*/
int busi_usage(int argc, char *argv[])
{
	if(argc == 1)
	{
		busi_version();
		return -1;
	}

	if(argc == 2)
	{
		if(strcmp(argv[1], "-v") == 0)
		{
			busi_version();
			return -1;
		}
	}

	if(argc != 2)
	{
		busi_version();
		return -1;
	}

	strcpy(g_AppArgv.chConfigFile, argv[1]);

	return 0;
}

/*
function:   busi_read_cfgfile()
description:
            读配置文件中的数据
Input:      
            argc:命令行参数个数
            argv:命令行参数列表
Output:
Return:     0:成功，other:不成功
others: 
*/
int busi_read_cfgfile(int argc, char *argv[])
{
	int iRcode;
	/* 读配置文件 */
	iRcode = IniFile_ReadFile(&g_IniFile, g_AppArgv.chConfigFile);
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_0010);
		return -1;
	}

	return 0;
}

/*
function:   busi_check_cfgfile()
description:
            分析配置文件中内容的合法性，并设置相关数据。
Input:      
Output:
Return:     0:成功，other:不成功
others: 
*/
int busi_check_cfgfile(void)
{
	struct stIniFile_Group *system_group, *path_info_group, *record_info_group, *new_index_field_group, *old_index_field_group, *fee_field_list_group, *loop_group;
	struct stIniFile_Key   *key1, *key2, *key3, *key4, *key5, *key6, *key7, *key8, *loop_key;
	int    iRcode, iCount, iLoop;
	char   chSscanf[64+1], chBuffer1[16+1], chBuffer2[16+1];

	MEMSET(chSscanf);
	sprintf(chSscanf,"file{%%[^,],%%[^}]}");

	/* 读取配置文件中GROUP */
	iRcode=IniFile_File_GetGroupByName(&g_IniFile, &system_group, "system")
		|| IniFile_File_GetGroupByName(&g_IniFile, &path_info_group, "path_info")
		|| IniFile_File_GetGroupByName(&g_IniFile, &record_info_group, "record_info")
		|| IniFile_File_GetGroupByName(&g_IniFile, &new_index_field_group, "new_index_field")
		|| IniFile_File_GetGroupByName(&g_IniFile, &old_index_field_group, "old_index_field")
		|| IniFile_File_GetGroupByName(&g_IniFile, &fee_field_list_group, "fee_field_list")
	;
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_0011);
		return -1;
	}

	/* system GROUP内子元素分析 */
	iRcode=IniFile_Group_GetKeyByName(system_group, &key1, "max_thread_number")
		|| (strcmp(key1->value, "") == 0)
		|| cf_string_ifdigital(key1->value)
	;

	if(iRcode)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_0011);

		return -1;
	}
	else
	{
		g_AppData.iThreadNumber = atoi(key1->value);
		if(g_AppData.iThreadNumber > MAX_THREAD_NUMBER)
		{
			BUSI_WRITE_RUNLOG(ERROR_APP_0011);
		
			return -1;		
		}	
	}
printf("/system GROUP内子元素分析/-->end\n");
	/* path_info GROUP内子元素分析 */
	iRcode=IniFile_Group_GetKeyByName(path_info_group, &key1, "log")
		|| IniFile_Group_GetKeyByName(path_info_group, &key2, "old_no_exist")
		|| IniFile_Group_GetKeyByName(path_info_group, &key3, "new_no_exist")
		|| IniFile_Group_GetKeyByName(path_info_group, &key4, "fee_cmp")
		|| IniFile_Group_GetKeyByName(path_info_group, &key5, "new_in")
		|| IniFile_Group_GetKeyByName(path_info_group, &key6, "new_out")
		|| IniFile_Group_GetKeyByName(path_info_group, &key7, "old_in")
		|| IniFile_Group_GetKeyByName(path_info_group, &key8, "old_out")
		|| (strcmp(key1->value, "") == 0)
		|| (strcmp(key2->value, "") == 0)
		|| (strcmp(key3->value, "") == 0)
		|| (strcmp(key4->value, "") == 0)
		|| (strcmp(key5->value, "") == 0)	
		|| (strcmp(key6->value, "") == 0)	
		|| (strcmp(key7->value, "") == 0)	
		|| (strcmp(key8->value, "") == 0)	
		|| cf_file_ifdir(key1->value)
		|| cf_file_ifdir(key2->value)
		|| cf_file_ifdir(key3->value)
		|| cf_file_ifdir(key4->value)
		|| cf_file_ifdir(key5->value)
		|| cf_file_ifdir(key6->value)
		|| cf_file_ifdir(key7->value)
		|| cf_file_ifdir(key8->value)		
	;
/*	
printf("==[%s],==[%s],==[%s],==[%s],==[%s],==[%s],==[%s],==[%s],==[%d]\n",key1->value,key2->value,key3->value,key4->value,key5->value,key6->value,key7->value,key8->value,iRcode);				
*/	
	
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_0011);
	
		return -1;
	}
	else
	{
		strcpy(g_AppData.chPath_Log, key1->value);
		strcpy(g_AppData.chPath_OldNoExist, key2->value);
		strcpy(g_AppData.chPath_NewNoExist, key3->value);
		strcpy(g_AppData.chPath_FeeCmp, key4->value);
		strcpy(g_AppData.chPath_NewIn, key5->value);
		strcpy(g_AppData.chPath_NewOut, key6->value);
		strcpy(g_AppData.chPath_OldIn, key7->value);
		strcpy(g_AppData.chPath_OldOut, key8->value);
	}
printf("/ path_info GROUP内子元素分析/-->end\n");
	/* record_info GROUP内子元素分析 */
	iRcode=IniFile_Group_GetKeyByName(record_info_group, &key1, "new_record_size")
		|| IniFile_Group_GetKeyByName(record_info_group, &key2, "old_record_size")
		|| IniFile_Group_GetKeyByName(record_info_group, &key3, "max_record_number")
		|| (strcmp(key1->value, "") == 0)
		|| (strcmp(key2->value, "") == 0)
		|| (strcmp(key3->value, "") == 0)		
		|| cf_string_ifdigital(key1->value)
		|| cf_string_ifdigital(key2->value)		
	;
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_0011);
printf("%s\n",ERROR_APP_0011);
		return -1;
		
	}
	else
	{	
		g_AppData.iNewRecordSize = atoi(key1->value);
		g_AppData.iOldRecordSize = atoi(key2->value);
		g_AppData.iMaxRecordNumber = atoi(key3->value);


		if(
			g_AppData.iNewRecordSize > MAX_RECORD_LEN
			||
			g_AppData.iOldRecordSize > MAX_RECORD_LEN
			||
			g_AppData.iMaxRecordNumber > MAX_FILE_RECORD_NUMBER
		)
		{
			BUSI_WRITE_RUNLOG(ERROR_APP_0011);
		
			return -1;
		}
	}
printf("/ record_info GROUP内子元素分析/-->end\n");	
	/* new_index_field GROUP内子元素分析 */
	iCount = IniFile_Group_CountKey(new_index_field_group);
	if(iCount > MAX_INDEX_FIELD_NUMBER)
	{
		/* 业务支持个数超过系统最大值 */
		BUSI_WRITE_RUNLOG(ERROR_APP_0011);
		return -1;
	}
	else
	{
		for(iLoop = 0; iLoop < iCount; iLoop++)
		{
			iRcode=IniFile_Group_GetKeyByIndex(new_index_field_group, &loop_key, iLoop)
				|| (strcmp(loop_key->value, "") == 0)
			;
			if(iRcode)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_0011);
				return -1;
			}
			
			sscanf(loop_key->value, chSscanf, chBuffer1, chBuffer2);
			g_AppData.stNewIndexField[iLoop].iBeginPosition = atoi(chBuffer1);
			g_AppData.stNewIndexField[iLoop].iSeekSize = atoi(chBuffer2);

			if(g_AppData.stNewIndexField[iLoop].iSeekSize > MAX_FIELD_LEN)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_0011);
				return -1;
			}	
		}

		g_AppData.iIndexFieldNumber = iCount;
	}
printf("/new_index_field GROUP内子元素分析/-->end\n");	
	/* old_index_field GROUP内子元素分析 */
	iCount = IniFile_Group_CountKey(old_index_field_group);
	if(iCount > MAX_INDEX_FIELD_NUMBER)
	{
		/* 业务支持个数超过系统最大值 */
		BUSI_WRITE_RUNLOG(ERROR_APP_0011);
		return -1;
	}
	else
	{
		if(iCount != g_AppData.iIndexFieldNumber)
		{
			/* 结点数不一样 */
			BUSI_WRITE_RUNLOG(ERROR_APP_0011);
			return -1;
		}

		for(iLoop = 0; iLoop < iCount; iLoop++)
		{
			iRcode=IniFile_Group_GetKeyByIndex(old_index_field_group, &loop_key, iLoop)
				|| (strcmp(loop_key->value, "") == 0)
			;
			if(iRcode)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_0011);
				return -1;
			}
			
			sscanf(loop_key->value, chSscanf, chBuffer1, chBuffer2);
			g_AppData.stOldIndexField[iLoop].iBeginPosition = atoi(chBuffer1);
			g_AppData.stOldIndexField[iLoop].iSeekSize = atoi(chBuffer2);

			if(g_AppData.stOldIndexField[iLoop].iSeekSize > MAX_FIELD_LEN)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_0011);
				return -1;
			}	
		}
	}
printf("/ old_index_field GROUP内子元素分析/-->end\n");	
	/* fee_field_list GROUP内子元素分析 */
	iCount = IniFile_Group_CountKey(fee_field_list_group);

	if(iCount > MAX_FEE_FIELD_NUMBER)
	{
		/* 业务支持个数超过系统最大值 */
		BUSI_WRITE_RUNLOG(ERROR_APP_0011);
		
		return -1;
	}
	else
	{		
		for(iLoop = 0; iLoop < iCount; iLoop++)
		{
printf("/fee==[%d]内子元素分析/-->begin\n",iLoop);				
			iRcode=IniFile_Group_GetKeyByIndex(fee_field_list_group, &loop_key, iLoop)
				|| (strcmp(loop_key->value, "") == 0)
			;
			if(iRcode)
			{	
				BUSI_WRITE_RUNLOG(ERROR_APP_0011);
				return -1;
			}
	
			iRcode = IniFile_File_GetGroupByName(&g_IniFile, &loop_group, loop_key->value);
			if(iRcode)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_0011);
				return -1;
			}

			iRcode=IniFile_Group_GetKeyByName(loop_group, &key1, "path")
				|| IniFile_Group_GetKeyByName(loop_group, &key2, "new_data")
				|| IniFile_Group_GetKeyByName(loop_group, &key3, "old_data")
				|| IniFile_Group_GetKeyByName(loop_group, &key4, "new_unit")
				|| IniFile_Group_GetKeyByName(loop_group, &key5, "old_unit")
				|| IniFile_Group_GetKeyByName(loop_group, &key6, "field_type")
				|| (strcmp(key1->value, "") == 0)
				|| (strcmp(key2->value, "") == 0)
				|| (strcmp(key3->value, "") == 0)
				|| (strcmp(key4->value, "") == 0)
				|| (strcmp(key5->value, "") == 0)
				|| (strcmp(key6->value, "") == 0)
			;
/*printf("[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d]\n",IniFile_Group_GetKeyByName(loop_group, &key1, "path"), IniFile_Group_GetKeyByName(loop_group, &key2, "new_data"),IniFile_Group_GetKeyByName(loop_group, &key3, "old_data"),IniFile_Group_GetKeyByName(loop_group, &key4, "new_unit"),IniFile_Group_GetKeyByName(loop_group, &key5, "old_unit"), \
       IniFile_Group_GetKeyByName(loop_group, &key6, "field_type"),(strcmp(key1->value, "") == 0),(strcmp(key2->value, "") == 0),(strcmp(key3->value, "") == 0),(strcmp(key4->value, "") == 0),(strcmp(key5->value, "") == 0),(strcmp(key6->value, "") == 0));			
			
printf("==[%s],==[%s],==[%s],==[%s],==[%s],==[%s],==[%d]\n",key1->value,key2->value,key3->value,key4->value,key5->value,key6->value,iRcode);				
*/
			if(iRcode)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_0011);
				return -1;
			}
	
			strcpy(g_AppData.stFeeField[iLoop].chPath, key1->value);
			
			g_AppData.stFeeField[iLoop].stNew.iUnit = atoi(key4->value);
			g_AppData.stFeeField[iLoop].stOld.iUnit = atoi(key5->value);

			sscanf(key2->value, chSscanf, chBuffer1, chBuffer2);
			g_AppData.stFeeField[iLoop].stNew.iBeginPosition = atoi(chBuffer1);
			g_AppData.stFeeField[iLoop].stNew.iSeekSize = atoi(chBuffer2);

			if(g_AppData.stFeeField[iLoop].stNew.iSeekSize > MAX_FIELD_LEN)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_0011);
				return -1;
			}			

			sscanf(key3->value, chSscanf, chBuffer1, chBuffer2);
			g_AppData.stFeeField[iLoop].stOld.iBeginPosition = atoi(chBuffer1);
			g_AppData.stFeeField[iLoop].stOld.iSeekSize = atoi(chBuffer2);

			if(g_AppData.stFeeField[iLoop].stOld.iSeekSize > MAX_FIELD_LEN)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_0011);
				return -1;
			}

			if(strcmp(key6->value, "number") == 0)
			{
			
				g_AppData.stFeeField[iLoop].iFieldType = APP_FIELDTYPE_NUMBER;
			}
			else
			{
				if(strcmp(key6->value, "string") == 0)
				{
					g_AppData.stFeeField[iLoop].iFieldType = APP_FIELDTYPE_STRING;
				}
				else
				{
					BUSI_WRITE_RUNLOG(ERROR_APP_0011);
					return -1;
				}
			}
printf("/fee==[%d]内子元素分析/-->end\n",iLoop);		
		}
	
		g_AppData.iFeeFieldNumber = iCount;
	}
printf("/fee_field_list GROUP内子元素分析/-->end\n");	
	return 0;
}

/*
function:   busi_do_prefix()
description:
            任务处理前，一些事务处理。
Input:      
Output:
Return:     0:成功，other:不成功
others: 
*/
int  busi_do_prefix(void)
{
	int iRcode;
	int iLoop;

	/* 设置日志文件 */
	iRcode = busi_set_runlog();
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_9999);
		return -1;
	}

	iRcode = busi_do_MallocMem();
	if(iRcode)
	{
		return -1;
	}

	pthread_mutex_init(&g_AppData.iThreadMutex, NULL);

	/*建立线程*/
	for(iLoop = 0; iLoop < g_AppData.iThreadNumber; iLoop++)
	{
		memset(g_AppThread[iLoop].chThreadId, 0, sizeof(g_AppThread[iLoop].chThreadId));
		sprintf(g_AppThread[iLoop].chThreadId, "%d", iLoop);

		iRcode = pthread_create(&(g_AppThread[iLoop].iThreadId), NULL, busi_do_PthreadTask, (void *)g_AppThread[iLoop].chThreadId);
		if(iRcode)
		{
			BUSI_WRITE_RUNLOG(ERROR_APP_0032);
			return -1;
		}
	}
	return 0;
}

/*
function:   busi_do_postfix()
description:
            任务处理后，一些事务处理。
Input:      
Output:
Return:     0:成功，other:不成功
others: 
*/
int  busi_do_postfix(void)
{
	return 0;
}

/*
function:   busi_do_File()
description:
            核对两个文件
Input:      
Output:
Return:     0:成功，other:不成功
others: 
*/
int busi_do_File(struct stApp_Thread *pstObj)
{
	int iRcode, iLoop, iCount;
	FILE *fpNewFile, *fpOldFile;
	char chNewFile[512+1];
	char chOldFile[512+1];
	/*
	char chFileRecordBuffer[1024+1];
	*/
	char *pchBuffer;

	int iNewCurrentRecord, iOldCurrentRecord;
	
	/*********************************************************/	

	/*初始化话单缓冲区*/
	for(iLoop = 0; iLoop < g_AppData.iMaxRecordNumber; iLoop++)
	{
		pstObj->stNewRecordList[iLoop].iFlag = 0;
		pstObj->stOldRecordList[iLoop].iFlag = 0;

		/*
		memset(pstObj->stNewRecordList[iLoop].pchBuffer, 0, g_AppData.iNewRecordSize+1);
		memset(pstObj->stOldRecordList[iLoop].pchBuffer, 0, g_AppData.iOldRecordSize+1);
		*/
	}
	//printf("busi_do_File进程号[%d]线程号[%d]新任务[%s]开始\n", getpid(), pthread_self(), pstObj->chRelationFlag);	
	/*打开新旧话单文件*/
	printf("NewFile [%s/%s]\n", g_AppData.chPath_NewIn, pstObj->chNewFile);
	sprintf(chNewFile, "%s/%s", g_AppData.chPath_NewIn, pstObj->chNewFile);
	fpNewFile = fopen(chNewFile, "r");
	if(fpNewFile == NULL)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_9999);
		return -1;
	}

	sprintf(chOldFile, "%s/%s", g_AppData.chPath_OldIn, pstObj->chOldFile);
	fpOldFile = fopen(chOldFile, "r");
	if(fpOldFile == NULL)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_9999);
		return -1;
	}

	/*读取新话单文件中全部记录到缓冲中*/
	iCount = 0;
	/*
	MEMSET(chFileRecordBuffer);
	*/
	pchBuffer = pstObj->stNewRecordList[iCount].pchBuffer;
	fgets(pchBuffer, 4096, fpNewFile);
	while(!feof(fpNewFile))
	{
		pstObj->stNewRecordList[iCount].iFlag = 1;

		iCount++;
		if(iCount >= g_AppData.iMaxRecordNumber)
		{
			fclose(fpOldFile);
			fclose(fpNewFile);
			BUSI_WRITE_RUNLOG(ERROR_APP_0030);
			return -1;
		}

		/*
		memcpy(pstObj->stNewRecordList[iCount].pchBuffer, chFileRecordBuffer, g_AppData.iNewRecordSize);
		*/
		/*
		MEMSET(chFileRecordBuffer);
		*/
		pchBuffer = pstObj->stNewRecordList[iCount].pchBuffer;
		fgets(pchBuffer, 4096, fpNewFile);
	}
	
	pstObj->iNewFileRecordCount = iCount;

	/*读取旧话单文件中全部记录到缓冲中*/
	iCount = 0;
	/*
	MEMSET(chFileRecordBuffer);
	*/
	pchBuffer = pstObj->stOldRecordList[iCount].pchBuffer;
	fgets(pchBuffer, 4096, fpOldFile);
	while(!feof(fpOldFile))
	{
		if(iCount >= g_AppData.iMaxRecordNumber)
		{
			fclose(fpOldFile);
			fclose(fpNewFile);
			BUSI_WRITE_RUNLOG(ERROR_APP_0030);
			return -1;
		}

		/*
		memcpy(pstObj->stOldRecordList[iCount].pchBuffer, chFileRecordBuffer, g_AppData.iOldRecordSize);
		*/
		pstObj->stOldRecordList[iCount].iFlag = 1;
		
		iCount++;
		/*
		MEMSET(chFileRecordBuffer);
		*/
		pchBuffer = pstObj->stOldRecordList[iCount].pchBuffer;
		fgets(pchBuffer, 4096, fpOldFile);
	}
	
	pstObj->iOldFileRecordCount = iCount;
	
	/*关闭新旧话单文件*/
	fclose(fpOldFile);
	fclose(fpNewFile);

	/*********************************************************/

	/*事先分析新旧所有话单中的索引信息*/
	/*提高核对性能*/
	busi_do_PreProcessNewIndexData(pstObj);
	busi_do_PreProcessOldIndexData(pstObj);

	/*********************************************************/
//printf("pstObj->iNewFileRecordCount=%d\n",pstObj->iNewFileRecordCount);
	iNewCurrentRecord = 1;
	for(;;)
	{
		if(g_AppData.iThreadExitFlag == 1)
		{
			return -1;
		}
	//printf("iNewCurrentRecord=%d\n",iNewCurrentRecord);
		if(iNewCurrentRecord > pstObj->iNewFileRecordCount)
		{
			/*已经处理完所有新话单文件中的记录*/
			break;
		}

		/*处理新话单记录列表中下一条未处理记录*/	
			
		/*拿当前新话单记录与旧话单记录列表中所有有效记录对比*/
		iOldCurrentRecord = busi_do_FindMacthRecord(pstObj, iNewCurrentRecord-1);
		if(iOldCurrentRecord > 0)
		{
			/*找到匹配记录*/

			/*对比所有的费用字段项*/
			iRcode = busi_do_MatchRecordFee(
				pstObj,
				pstObj->stNewRecordList[iNewCurrentRecord-1].pchBuffer,
				pstObj->stOldRecordList[iOldCurrentRecord-1].pchBuffer
			);
			if(iRcode < 0)
			{
				return -1;
			}
			
			/*删除新话单列表中此匹配记录*/
			pstObj->stNewRecordList[iNewCurrentRecord-1].iFlag = 0;

			/*删除旧话单列表中此匹配记录*/
			pstObj->stOldRecordList[iOldCurrentRecord-1].iFlag = 0;
		}
		else
		{
		}

		iNewCurrentRecord++;
	}
	
	/*将新话单记录中所有未处理的记录写入文件(新话单有旧话单没有)*/
	iRcode = busi_do_WriteFile_OldNoExist(pstObj);
	if(iRcode)
	{
		return -1;
	}
		
	/*将旧话单记录中所有未处理的记录写入文件(新话单没有旧话单有)*/
	iRcode = busi_do_WriteFile_NewNoExist(pstObj);
	if(iRcode)
	{
		return -1;
	}
		
	/*显示核对统计信息*/
	iRcode = busi_do_PrintResult(pstObj);
	if(iRcode)
	{
		return -1;
	}
			
	return 0;
}

/*
function:   busi_do_PrintResult()
description:
            显示核对最终统计结果
Input:      
Output:
Return:
others: 
*/
int  busi_do_PrintResult(struct stApp_Thread *pstObj)
{
	int iRcode;
	int iLoop;

	iRcode = pthread_mutex_lock(&(g_AppData.iThreadMutex));
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_0034);
		return -1;
	}

	BUSI_WRITE_RUNLOG("APP:MSG:++++++++++++++++++++++++++++++++++");
	sprintf(g_RunLog.LogBuffer, "APP:MSG:关联标识{%s}", pstObj->chRelationFlag);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:新话单文件{%s}", pstObj->chNewFile);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:旧话单文件{%s}", pstObj->chOldFile);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);	

	sprintf(g_RunLog.LogBuffer, "APP:MSG:新话单文件记录总数{%d}", pstObj->iNewFileRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:旧话单文件记录总数{%d}", pstObj->iOldFileRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:新旧话单文件记录匹配总数{%d}", pstObj->iMathchRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:旧话单没有新话单有的话单记录总数{%d}", pstObj->iOldNoExistCmpCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:新话单没有旧话单有的话单记录总数{%d}", pstObj->iNewNoExistCmpCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);

	for(iLoop = 0; iLoop < g_AppData.iFeeFieldNumber; iLoop++)
	{
		sprintf(g_RunLog.LogBuffer, "APP:MSG:新旧话单费用项{%d}不匹配的记录总数{%d}", iLoop+1, pstObj->stFeeField[iLoop].iNoCmpCount);
		BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	}

	pthread_mutex_unlock(&(g_AppData.iThreadMutex));

	return 0;
}

/*
function:   busi_do_AllFilePrintResult()
description:
            显示所有文件核对最终统计结果
Input:      
Output:
Return:
others: 
*/
void  busi_do_AllFilePrintResult(void)
{
	int iLoop;

	BUSI_WRITE_RUNLOG("APP:MSG:++++++++++++++++++++++++++++++++++");
	sprintf(g_RunLog.LogBuffer, "APP:MSG:所有核对文件新话单文件记录总数{%ld}", g_AppData.iAllFileNewFileRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:所有核对文件旧话单文件记录总数{%ld}", g_AppData.iAllFileOldFileRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:所有核对文件新旧话单文件记录匹配总数{%ld}", g_AppData.iAllFileMathchRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:所有核对文件旧话单没有新话单有的话单记录总数{%ld}", g_AppData.iAllFileOldNoExistCmpCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:所有核对文件新话单没有旧话单有的话单记录总数{%ld}", g_AppData.iAllFileNewNoExistCmpCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);

	for(iLoop = 0; iLoop < g_AppData.iFeeFieldNumber; iLoop++)
	{
		sprintf(g_RunLog.LogBuffer, "APP:MSG:所有核对文件新旧话单费用项{%d}不匹配的记录总数{%ld}", iLoop+1, g_AppData.stFeeField[iLoop].iAllFileNoCmpCount);
		BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	}
}

/*
function:   busi_do_PreProcessNewIndexData()
description:
			分析所有新话单记录中索引字段值
Input:
Output:
Return:
others:
*/
void busi_do_PreProcessNewIndexData(struct stApp_Thread *pstObj)
{
	int iLoop, iLoop1;

	/*分析所有新话单记录中索引字段值*/
	for(iLoop = 0; iLoop < pstObj->iNewFileRecordCount; iLoop++)
	{
		for(iLoop1 = 0; iLoop1 < g_AppData.iIndexFieldNumber; iLoop1++)
		{
			MEMSET(&(pstObj->stNewRecordList[iLoop].chIndexData[iLoop1]));
			
			memcpy(
				&(pstObj->stNewRecordList[iLoop].chIndexData[iLoop1]),
				(pstObj->stNewRecordList[iLoop].pchBuffer+(g_AppData.stNewIndexField[iLoop1].iBeginPosition-1)),
				g_AppData.stNewIndexField[iLoop1].iSeekSize
			);

			cf_string_trim(
				pstObj->stNewRecordList[iLoop].chIndexData[iLoop1]
			);
		}
	}
}

/*
function:   busi_do_PreProcessOldIndexData()
description:
			分析所有旧话单记录中索引字段值
Input:
Output:
Return:
others:
*/
void busi_do_PreProcessOldIndexData(struct stApp_Thread *pstObj)
{
	int iLoop, iLoop1;

	/*分析所有旧话单记录中索引字段值*/
	for(iLoop = 0; iLoop < pstObj->iOldFileRecordCount; iLoop++)
	{
		for(iLoop1 = 0; iLoop1 < g_AppData.iIndexFieldNumber; iLoop1++)
		{
			MEMSET(&(pstObj->stOldRecordList[iLoop].chIndexData[iLoop1]));
			
			memcpy(
				&(pstObj->stOldRecordList[iLoop].chIndexData[iLoop1]),
				(pstObj->stOldRecordList[iLoop].pchBuffer+(g_AppData.stOldIndexField[iLoop1].iBeginPosition-1)),
				g_AppData.stOldIndexField[iLoop1].iSeekSize
			);

			cf_string_trim(
				pstObj->stOldRecordList[iLoop].chIndexData[iLoop1]
			);
		}
	}
}

/*
function:   busi_do_FindMacthRecord()
description:
			拿当前新话单记录与旧话单记录列表中所有有效记录对比
Input:
			pchNewData 新话单记录
Output:		<= 0 没有找到匹配记录, > 0, 匹配记录号(从1开始)
Return:
others:
*/
int  busi_do_FindMacthRecord(struct stApp_Thread *pstObj, int iNewLoop)
{
	int iLoop, iLoop1;
		
	/*将新话单记录中索引字段与所有旧话单文件中记录对比*/
	for(iLoop = 0; iLoop < pstObj->iOldFileRecordCount; iLoop++)
	{
		/*该记录结点是否有效*/
		if(pstObj->stOldRecordList[iLoop].iFlag == 0)
		{
			continue;
		}

		/*将所有索引字段对比*/
		for(iLoop1 = 0; iLoop1 < g_AppData.iIndexFieldNumber; iLoop1++)
		{

/*		
#ifdef DEBUG
printf("new[%s]\n", pstObj->stNewRecordList[iNewLoop].chIndexData[iLoop1]);
printf("old[%s]\n", pstObj->stOldRecordList[iLoop].chIndexData[iLoop1]);
#endif
*/

			if(
				strcmp(
				pstObj->stNewRecordList[iNewLoop].chIndexData[iLoop1],
				pstObj->stOldRecordList[iLoop].chIndexData[iLoop1]
				) != 0
			)
			{
				break;
			}
		}
	
		if(iLoop1 >= g_AppData.iIndexFieldNumber)
		{
			/*所有索引字段都匹配成功*/
			return iLoop+1;
		}
	}

	return 0;
}

/*
function:   busi_do_MatchRecordFee()
description:
			对比新旧话单记录中费用字段
Input:
			pNewData 新话单记录
			pOldData 旧话单记录
Output:		<0 内部错误, =0 不相等, >0 相等
Return:
others:
*/
int busi_do_MatchRecordFee(struct stApp_Thread *pstObj, char *pchNewData, char *pchOldData)
{
	int iRcode;
	int iLoop;
	int iCmpFlag;

	for(iLoop = 0; iLoop < g_AppData.iFeeFieldNumber; iLoop++)
	{
		MEMSET1(pstObj->stFeeField[iLoop].stNew);
		MEMSET1(pstObj->stFeeField[iLoop].stOld);

		/*取新话单记录单项费用*/
		memcpy(
			pstObj->stFeeField[iLoop].stNew.chData,
			pchNewData+(g_AppData.stFeeField[iLoop].stNew.iBeginPosition-1),
			g_AppData.stFeeField[iLoop].stNew.iSeekSize
		);

		/*取旧话单记录单项费用*/
		memcpy(
			pstObj->stFeeField[iLoop].stOld.chData,
			pchOldData+(g_AppData.stFeeField[iLoop].stOld.iBeginPosition-1),
			g_AppData.stFeeField[iLoop].stOld.iSeekSize
		);

		if(g_AppData.stFeeField[iLoop].iFieldType == APP_FIELDTYPE_NUMBER)
		{
			pstObj->stFeeField[iLoop].stNew.lData = atol(pstObj->stFeeField[iLoop].stNew.chData) * g_AppData.stFeeField[iLoop].stNew.iUnit;
			pstObj->stFeeField[iLoop].stOld.lData = atol(pstObj->stFeeField[iLoop].stOld.chData) * g_AppData.stFeeField[iLoop].stOld.iUnit;

/*
#ifdef DEBUG
		printf("number fee->%ld,%ld\n", pstObj->stFeeField[iLoop].stNew.lData, pstObj->stFeeField[iLoop].stOld.lData);
#endif		
*/

			if(pstObj->stFeeField[iLoop].stNew.lData == pstObj->stFeeField[iLoop].stOld.lData)
			{
				iCmpFlag = 1;
			}
			else
			{
				iCmpFlag = 0;
			}
		}
		else
		{
			cf_string_trim(pstObj->stFeeField[iLoop].stNew.chData);
			cf_string_trim(pstObj->stFeeField[iLoop].stOld.chData);

/*			
#ifdef DEBUG
		printf("string fee->%s,%s\n", pstObj->stFeeField[iLoop].stNew.chData, pstObj->stFeeField[iLoop].stOld.chData);
#endif
*/

			if(strcmp(pstObj->stFeeField[iLoop].stNew.chData, pstObj->stFeeField[iLoop].stOld.chData) == 0)
			{
				iCmpFlag = 1;
			}
			else
			{
				iCmpFlag = 0;
			}
		}
		
		if(iCmpFlag == 0)
		{
			/*当前费用字段项对比不成功*/

			/*统计-当前费用对比不相同记录数加1*/
			pstObj->stFeeField[iLoop].iNoCmpCount ++;

			/*将新旧话单记录数据写入文件(费用不相同)*/
			iRcode = busi_do_WriteFile_NoMatchFee(pstObj, iLoop, pchNewData, pchOldData);
			if(iRcode)
			{
				return -1;
			}

			return 0;
		}
		
	}

	/*统计-匹配记录数加1*/
	pstObj->iMathchRecordCount ++;

	/*将新旧话单记录数据写入文件(费用相同)*/
	iRcode = busi_do_WriteFile_MatchRecord(pstObj, pchNewData, pchOldData);
	if(iRcode)
	{
		return -1;
	}

	return 1;
}

int  busi_do_OpenFile_MatchRecord(struct stApp_Thread *pstObj)
{
	char chFileName[512+1];

	sprintf(chFileName, "%s/%s.ok.record.%s.%s", g_AppData.chPath_FeeCmp, APPLICATION_NAME, pstObj->chRelationFlag, g_AppData.chDealDate);
	pstObj->fpMatchRecord = fopen(chFileName, "w");
	if(pstObj->fpMatchRecord == NULL)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_9997);
		return -1;
	}

	setvbuf(pstObj->fpMatchRecord, pstObj->chFileBuffer_MatchRecord, _IOFBF, MAX_FILE_WRITEBUFFER);

	return 0;
}

int  busi_do_CloseFile_MatchRecord(struct stApp_Thread *pstObj)
{
	fclose(pstObj->fpMatchRecord);
	pstObj->fpMatchRecord = NULL;
	return 0;
}

int  busi_do_OpenFile_NewNoExist(struct stApp_Thread *pstObj)
{
	char chFileName[512+1];
	
	sprintf(chFileName, "%s/%s.error.record.%s.%s", g_AppData.chPath_NewNoExist, APPLICATION_NAME, pstObj->chRelationFlag, g_AppData.chDealDate);
	pstObj->fpNewNoExist = fopen(chFileName, "w");
	if(pstObj->fpNewNoExist == NULL)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_9997);
		return -1;
	}

	setvbuf(pstObj->fpNewNoExist, pstObj->chFileBuffer_NewNoExist, _IOFBF, MAX_FILE_WRITEBUFFER);
	
	return 0;
}

int  busi_do_CloseFile_NewNoExist(struct stApp_Thread *pstObj)
{
	fclose(pstObj->fpNewNoExist);
	pstObj->fpNewNoExist = NULL;
	return 0;
}

int  busi_do_OpenFile_OldNoExist(struct stApp_Thread *pstObj)
{
	char chFileName[512+1];
	
	sprintf(chFileName, "%s/%s.error.record.%s.%s", g_AppData.chPath_OldNoExist, APPLICATION_NAME, pstObj->chRelationFlag, g_AppData.chDealDate);
	pstObj->fpOldNoExist = fopen(chFileName, "w");
	if(pstObj->fpOldNoExist == NULL)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_9997);
		return -1;
	}

	setvbuf(pstObj->fpOldNoExist, pstObj->chFileBuffer_OldNoExist, _IOFBF, MAX_FILE_WRITEBUFFER);
	
	return 0;
}

int  busi_do_CloseFile_OldNoExist(struct stApp_Thread *pstObj)
{
	fclose(pstObj->fpOldNoExist);
	pstObj->fpOldNoExist = NULL;
	return 0;
}

int  busi_do_OpenFile_NoMatchFee(struct stApp_Thread *pstObj)
{
	int iLoop;
	char chFileName[512+1];

	for(iLoop = 0; iLoop < g_AppData.iFeeFieldNumber; iLoop++)
	{
		sprintf(chFileName, "%s/%s.error.fee.%s.%s", g_AppData.stFeeField[iLoop].chPath, APPLICATION_NAME, pstObj->chRelationFlag, g_AppData.chDealDate);
		pstObj->fpNoMatchFee[iLoop] = fopen(chFileName, "w");
		if(pstObj->fpNoMatchFee[iLoop] == NULL)
		{
			 char aa[300];
			memset(aa,0,sizeof(aa));
			sprintf(aa,"%s,%d\n",chFileName,errno);
			BUSI_WRITE_RUNLOG(aa);
			BUSI_WRITE_RUNLOG(ERROR_APP_9997);
			return -1;
		}

		setvbuf(pstObj->fpNoMatchFee[iLoop], pstObj->chFileBuffer_NoMatchFee[iLoop], _IOFBF, MAX_FILE_WRITEBUFFER);
	}
	
	return 0;
}

int  busi_do_CloseFile_NoMatchFee(struct stApp_Thread *pstObj)
{
	int iLoop;

	for(iLoop = 0; iLoop < g_AppData.iFeeFieldNumber; iLoop++)
	{
		fclose(pstObj->fpNoMatchFee[iLoop]);
		pstObj->fpNoMatchFee[iLoop] = NULL;
	}
	
	return 0;
}

int  busi_do_WriteFile_NoMatchFee(struct stApp_Thread *pstObj, int iIndex, char *pchNewData, char *pchOldData)
{
	fputs(APP_SPLIT_LINE, pstObj->fpNoMatchFee[iIndex]);
	fputs(pchNewData, pstObj->fpNoMatchFee[iIndex]);
	fputs(pchOldData, pstObj->fpNoMatchFee[iIndex]);
	return 0;
}

int  busi_do_WriteFile_MatchRecord(struct stApp_Thread *pstObj, char *pchNewData, char *pchOldData)
{
	fputs(APP_SPLIT_LINE, pstObj->fpMatchRecord);
	fputs(pchNewData, pstObj->fpMatchRecord);
	fputs(pchOldData, pstObj->fpMatchRecord);
	return 0;
}

int  busi_do_WriteFile_OldNoExist(struct stApp_Thread *pstObj)
{
	int iLoop;

	for(iLoop = 0; iLoop < pstObj->iNewFileRecordCount; iLoop++)
	{
		if(pstObj->stNewRecordList[iLoop].iFlag == 0)
		{
			continue;
		}

		fputs(pstObj->stNewRecordList[iLoop].pchBuffer, pstObj->fpOldNoExist);
		pstObj->iOldNoExistCmpCount++;
	}
	
	return 0;
}

int  busi_do_WriteFile_NewNoExist(struct stApp_Thread *pstObj)
{
	int iLoop;

	for(iLoop = 0; iLoop < pstObj->iOldFileRecordCount; iLoop++)
	{
		if(pstObj->stOldRecordList[iLoop].iFlag == 0)
		{
			continue;
		}

		fputs(pstObj->stOldRecordList[iLoop].pchBuffer, pstObj->fpNewNoExist);
		pstObj->iNewNoExistCmpCount++;
	}
	
	return 0;
}

/*
function:   busi_do_Path()
description:
            核对两个目录下所有文件
Input:      
Output:
Return:     0:成功，other:不成功
others: 
*/
int busi_do_Path(void)
{
	int iRcode;
	int iLoop;
	int iFlag;
	DIR  *pOldDir, *pNewDir;
	struct dirent *pOldDirent, *pNewDirent;
	char chNewFile[512+1], chOldFile[512+1], chRelationFlag[32+1];

	pOldDir = opendir(g_AppData.chPath_OldIn);
	if(pOldDir == NULL)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_9999);
		return -1;
	}

	while((pOldDirent = readdir(pOldDir)) != NULL)
	{
		if(g_AppData.iThreadExitFlag == 1)
		{
			break;
		}
		
		if(strcmp(pOldDirent->d_name, "." ) == 0)  continue;
		if(strcmp(pOldDirent->d_name, "..") == 0)  continue;

		MEMSET(chNewFile);
		MEMSET(chOldFile);
		MEMSET(chRelationFlag);

		/*如果文件名没有关联信息，则跳过此文件*/
		iRcode = busi_do_GetRelationFlag(pOldDirent->d_name, chRelationFlag);
		if(iRcode)
		{
			continue;
		}
		else
		{
			strcpy(chOldFile, pOldDirent->d_name);
		}

		pNewDir = opendir(g_AppData.chPath_NewIn);
		if(pNewDir == NULL)
		{
			BUSI_WRITE_RUNLOG(ERROR_APP_9999);
			return -1;
		}

		while((pNewDirent = readdir(pNewDir)) != NULL)
		{
			if(strcmp(pNewDirent->d_name, "." ) == 0)  continue;
			if(strcmp(pNewDirent->d_name, "..") == 0)  continue;

			/*匹配关联的文件*/
			if(strstr(pNewDirent->d_name, chRelationFlag) == NULL)
			{
				continue;
			}
			else
			{
				strcpy(chNewFile, pNewDirent->d_name);
			}

			/*找有效的空闭线程*/
			for(;;)
			{
				if(g_AppData.iThreadExitFlag == 1)
				{
					break;
				}		
				
				for(iLoop = 0; iLoop < g_AppData.iThreadNumber; iLoop++)
				{
					if(g_AppThread[iLoop].iBusyFlag == 0)
					{
						break;
					}
				}

				if(iLoop < g_AppData.iThreadNumber)
				{
					break;
				}
				else
				{
#ifdef DEBUG				
					sprintf(g_RunLog.LogBuffer, "APP:STA:无空闲进程线程");
					BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
#endif					
					sleep(APP_THREAD_BUSY_SLEEP);
				}
			}

			if(g_AppData.iThreadExitFlag == 1)
			{
				break;
			}				

			strcpy(g_AppThread[iLoop].chNewFile, chNewFile);
			strcpy(g_AppThread[iLoop].chOldFile, chOldFile);
			strcpy(g_AppThread[iLoop].chRelationFlag, chRelationFlag);
			//printf("g_AppThread[%d].chNewFile=%s\n",iLoop,g_AppThread[iLoop].chNewFile);
			//printf("g_AppThread[%d].chOldFile=%s\n",iLoop,g_AppThread[iLoop].chOldFile);
			//printf("g_AppThread[%d].chRelationFlag=%s\n",iLoop,g_AppThread[iLoop].chRelationFlag);

			g_AppThread[iLoop].iBusyFlag = 1;
			
			/*只处理第一次与之匹配的文件*/
			break;
		}
		
		closedir(pNewDir);
	}

	closedir(pOldDir);

	/*****************************************************/
#ifdef DEBUG	
	printf("等待所有线程任务处理完毕\n");
#endif
	
	/*等待所有线程任务都结束*/
	for(iLoop = 0; iLoop < g_AppData.iThreadNumber; iLoop++)
	{
		while(g_AppData.iThreadExitFlag != 1)
		{
			if(g_AppThread[iLoop].iBusyFlag != 0)
			{
				sleep(APP_THREAD_BUSY_SLEEP);
			}
			else
			{
				break;
			}
		}
	}

#ifdef DEBUG	
	printf("所有线程任务处理完毕\n");
#endif

	if(g_AppData.iThreadExitFlag == 1)
	{
printf("====zhaoro");
		BUSI_WRITE_RUNLOG(ERROR_APP_0035);
		return -1;
	}

	/*****************************************************/
	/*显示没有匹配处理的文件*/
	
	pOldDir = opendir(g_AppData.chPath_OldIn);
	if(pOldDir == NULL)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_9999);
		return -1;
	}

	iFlag = 0;
	while((pOldDirent = readdir(pOldDir)) != NULL)
	{
		if(strcmp(pOldDirent->d_name, "." ) == 0)  continue;
		if(strcmp(pOldDirent->d_name, "..") == 0)  continue;

		if(iFlag == 0)
		{
			BUSI_WRITE_RUNLOG("APP:MSG:++++++++++++++++++++++++++++++++++");
		}
		iFlag = 1;

		sprintf(g_RunLog.LogBuffer, "APP:MSG:{%s/%s}文件没有处理", g_AppData.chPath_OldIn, pOldDirent->d_name);
		BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	}

	closedir(pOldDir);

	pNewDir = opendir(g_AppData.chPath_NewIn);
	if(pNewDir == NULL)
	{
		BUSI_WRITE_RUNLOG(ERROR_APP_9999);
		return -1;
	}

	iFlag = 0;
	while((pNewDirent = readdir(pNewDir)) != NULL)
	{
		if(strcmp(pNewDirent->d_name, "." ) == 0)  continue;
		if(strcmp(pNewDirent->d_name, "..") == 0)  continue;

		if(iFlag == 0)
		{
			BUSI_WRITE_RUNLOG("APP:MSG:++++++++++++++++++++++++++++++++++");
		}
		iFlag = 1;
	
		sprintf(g_RunLog.LogBuffer, "APP:MSG:{%s/%s}文件没有处理", g_AppData.chPath_NewIn, pNewDirent->d_name);
		BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	}

	closedir(pNewDir);
	/*****************************************************/

	/*输出所有核对文件的统计信息*/
	busi_do_AllFilePrintResult();
	
	return 0;
}

/*
function:   busi_do_GetRelationFlag()
description:
            取旧话单文件名中的关联信息
Input:      
Output:
Return:     0:成功，other:不成功
others: 
*/
int busi_do_GetRelationFlag(char *chOldFile, char *chRelationFlag)
{
	char chTmp1[128+1];

	sscanf(chOldFile, "%[^.].%s", chTmp1, chRelationFlag);

	if(strcmp(chRelationFlag, "") == 0)
	{
		return -1;
	}
	
	return 0;
}

/*
function:   busi_do_MallocMem()
description:
            申请内存空间
Input:      
Output:
Return:
others: 
*/
int busi_do_MallocMem(void)
{
	int iThreadLoop;
	int iLoop;
printf("=========zhaorh===MaxRecord[%d]================\n",g_AppData.iMaxRecordNumber);

	for(iThreadLoop = 0; iThreadLoop < g_AppData.iThreadNumber; iThreadLoop++)
	{
		g_AppThread[iThreadLoop].stNewRecordList = (struct stData_CmpRecord *)calloc(sizeof(struct stData_CmpRecord), g_AppData.iMaxRecordNumber);
		if(g_AppThread[iThreadLoop].stNewRecordList == NULL)
		{
			BUSI_WRITE_RUNLOG(ERROR_APP_9998);
			return -1;
		}
		else
		{
			g_MemLeak++;
		}
		
		g_AppThread[iThreadLoop].stOldRecordList = (struct stData_CmpRecord *)calloc(sizeof(struct stData_CmpRecord), g_AppData.iMaxRecordNumber);
		if(g_AppThread[iThreadLoop].stOldRecordList == NULL)
		{
			BUSI_WRITE_RUNLOG(ERROR_APP_9998);
			return -1;
		}
		else
		{
			g_MemLeak++;
		}	
		
		for(iLoop = 0; iLoop < g_AppData.iMaxRecordNumber; iLoop++)
		{
			g_AppThread[iThreadLoop].stNewRecordList[iLoop].iFlag = 0;
			g_AppThread[iThreadLoop].stNewRecordList[iLoop].pchBuffer = (char *)calloc(g_AppData.iNewRecordSize+1, 1);
			if(g_AppThread[iThreadLoop].stNewRecordList[iLoop].pchBuffer == NULL)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_9998);
				return -1;
			}
			else
			{
				g_MemLeak++;
			}

			g_AppThread[iThreadLoop].stOldRecordList[iLoop].iFlag = 0;
			g_AppThread[iThreadLoop].stOldRecordList[iLoop].pchBuffer = (char *)calloc(g_AppData.iOldRecordSize+1, 1);
			if(g_AppThread[iThreadLoop].stOldRecordList[iLoop].pchBuffer == NULL)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_9998);
				return -1;
			}
			else
			{
				g_MemLeak++;
			}
		}
	}

	return 0;
}

/*
function:   busi_do_FreeMem()
description:
            释放内存空间
Input:      
Output:
Return:
others: 
*/
int busi_do_FreeMem(void)
{
	int iThreadLoop;
	int iLoop;

	for(iThreadLoop = 0; iThreadLoop < g_AppData.iThreadNumber; iThreadLoop++)
	{
		if(g_AppThread[iThreadLoop].stOldRecordList != NULL)
		{
			for(iLoop = 0; iLoop < g_AppData.iMaxRecordNumber; iLoop++)
			{
				if(g_AppThread[iThreadLoop].stOldRecordList[iLoop].pchBuffer != NULL)
				{
					free(g_AppThread[iThreadLoop].stOldRecordList[iLoop].pchBuffer);
					g_AppThread[iThreadLoop].stOldRecordList[iLoop].pchBuffer = NULL;
					g_MemLeak--;
				}
			}		

			free(g_AppThread[iThreadLoop].stOldRecordList);
			g_AppThread[iThreadLoop].stOldRecordList = NULL;
			g_MemLeak--;
		}

		if(g_AppThread[iThreadLoop].stNewRecordList != NULL)
		{
			for(iLoop = 0; iLoop < g_AppData.iMaxRecordNumber; iLoop++)
			{
				if(g_AppThread[iThreadLoop].stNewRecordList[iLoop].pchBuffer != NULL)
				{
					free(g_AppThread[iThreadLoop].stNewRecordList[iLoop].pchBuffer);
					g_AppThread[iThreadLoop].stNewRecordList[iLoop].pchBuffer = NULL;
					g_MemLeak--;
				}
			}		

			free(g_AppThread[iThreadLoop].stNewRecordList);
			g_AppThread[iThreadLoop].stNewRecordList = NULL;
			g_MemLeak--;
		}
	}
	return 0;
}

/*
function:   busi_atexit()
description:
			系统注册退出挂钩函数，用于业务流程中途退出时，系统的收尾操作
Input:      
Output:
Return:     
others: 
*/
void busi_atexit(void)
{
	int iRcode;
	int iLoop;

	g_AppData.iThreadExitFlag = 1;

#ifdef DEBUG	
	printf("等待所有线程结束\n");
#endif

	for(iLoop = 0; iLoop < g_AppData.iThreadNumber; iLoop++)
	{	
		if(g_AppThread[iLoop].iThreadId != 0)
		{
			iRcode = pthread_join(g_AppThread[iLoop].iThreadId, NULL);
			if(iRcode)
			{
				BUSI_WRITE_RUNLOG(ERROR_APP_0033);
				/*return -1;*/
			}
		}
	}

#ifdef DEBUG	
	printf("所有线程结束\n");
#endif

	pthread_mutex_destroy(&g_AppData.iThreadMutex);	
	
	busi_do_FreeMem();

	if(g_MemLeak != 0)
	{
		sprintf(g_RunLog.LogBuffer, "APP:MSG:内存泄漏句柄数=%d", g_MemLeak);
		BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);		
	}	
}

/*
function:   busi_do_PthreadTask()
description:
		线程处理任务
Input:      
		pArgv:入口数据
Output:
Return:
		返回数据
others: 
*/
void *busi_do_PthreadTask(void *pArgv)
{
	int iRcode;
	int iLoop;
	int iThreadIndex;
	struct stApp_Thread *pstObj;
	char chMoveSrcFile[512+1];
	char chMoveDesFile[512+1];	

	iThreadIndex = atoi((char *)pArgv);

#ifdef DEBUG
	printf("进程号[%d]线程号[%d]编号[%d]激活\n", getpid(), pthread_self(), iThreadIndex);
#endif

	pstObj = &(g_AppThread[iThreadIndex]);

	while(g_AppData.iThreadExitFlag != 1)
	{
		if(pstObj->iBusyFlag != 1)
		{
			sleep(APP_THREAD_BUSY_SLEEP);
			continue;
		}

#ifdef DEBUG
		printf("进程号[%d]线程号[%d]编号[%d]新任务[%s]开始\n", getpid(), pthread_self(), iThreadIndex, pstObj->chRelationFlag);	
#endif
	//printf("进程号[%d]线程号[%d]编号[%d]新任务[%s]开始\n", getpid(), pthread_self(), iThreadIndex, pstObj->chRelationFlag);	
		/*打开核对输出文件*/
		iRcode=busi_do_OpenFile_MatchRecord(pstObj)
			|| busi_do_OpenFile_NewNoExist(pstObj)
			|| busi_do_OpenFile_OldNoExist(pstObj)
			|| busi_do_OpenFile_NoMatchFee(pstObj)
		;
		if(iRcode)
		{
			g_AppData.iThreadExitFlag = 1;
			pthread_exit(NULL);
		}

		pstObj->iOldNoExistCmpCount = 0;
		pstObj->iNewNoExistCmpCount = 0;
		pstObj->iNewFileRecordCount = 0;
		pstObj->iOldFileRecordCount = 0;
		pstObj->iMathchRecordCount  = 0;

		for(iLoop = 0; iLoop < g_AppData.iFeeFieldNumber; iLoop++)
		{
			pstObj->stFeeField[iLoop].iNoCmpCount = 0;
		}

		/*核对文件*/
		iRcode = busi_do_File(pstObj);
		if(iRcode)
		{
			g_AppData.iThreadExitFlag = 1;
			pthread_exit(NULL);
		}
				
		/*关闭核对输对文件*/
		busi_do_CloseFile_MatchRecord(pstObj);
		busi_do_CloseFile_NewNoExist(pstObj);
		busi_do_CloseFile_OldNoExist(pstObj);
		busi_do_CloseFile_NoMatchFee(pstObj);
				
		/***********************************************/

		/*统计所有核对文件相关信息*/
		iRcode = pthread_mutex_lock(&(g_AppData.iThreadMutex));
		if(iRcode)
		{
			g_AppData.iThreadExitFlag = 1;
			pthread_exit(NULL);
		}
		
		g_AppData.iAllFileOldNoExistCmpCount += pstObj->iOldNoExistCmpCount;
		g_AppData.iAllFileNewNoExistCmpCount += pstObj->iNewNoExistCmpCount;
		g_AppData.iAllFileNewFileRecordCount += pstObj->iNewFileRecordCount;
		g_AppData.iAllFileOldFileRecordCount += pstObj->iOldFileRecordCount;
		g_AppData.iAllFileMathchRecordCount  += pstObj->iMathchRecordCount;

		for(iLoop = 0; iLoop < g_AppData.iFeeFieldNumber; iLoop++)
		{
			g_AppData.stFeeField[iLoop].iAllFileNoCmpCount += pstObj->stFeeField[iLoop].iNoCmpCount;
		}

		pthread_mutex_unlock(&(g_AppData.iThreadMutex));	


		/*将处理完后的文件(有匹配)移到出口目录*/
		sprintf(chMoveSrcFile, "%s/%s", g_AppData.chPath_OldIn, pstObj->chOldFile);
		sprintf(chMoveDesFile, "%s/%s", g_AppData.chPath_OldOut, pstObj->chOldFile);

/*
#ifdef DEBUG
		printf("%s,%s\n", chMoveSrcFile, chMoveDesFile);
#endif
*/

		iRcode = cf_file_move(chMoveSrcFile, chMoveDesFile);
		if(iRcode)
		{
			BUSI_WRITE_RUNLOG(ERROR_APP_9999);
			g_AppData.iThreadExitFlag = 1;
			pthread_exit(NULL);
		}

		sprintf(chMoveSrcFile, "%s/%s", g_AppData.chPath_NewIn, pstObj->chNewFile);
		sprintf(chMoveDesFile, "%s/%s", g_AppData.chPath_NewOut, pstObj->chNewFile);
		
		iRcode = cf_file_move(chMoveSrcFile, chMoveDesFile);
		if(iRcode)
		{
			BUSI_WRITE_RUNLOG(ERROR_APP_9999);
			g_AppData.iThreadExitFlag = 1;
			pthread_exit(NULL);
		}

#ifdef DEBUG
		printf("进程号[%d]线程号[%d]编号[%d]新任务结束\n", getpid(), pthread_self(), iThreadIndex);		
#endif	
		pstObj->iBusyFlag = 0;	
	}

#ifdef DEBUG
		printf("进程号[%d]线程号[%d]编号[%d]退出\n", getpid(), pthread_self(), iThreadIndex);		
#endif	
	
	pthread_exit(NULL);
}

/*
Copyright (C), 1995-2004, Si-Tech Information Technology Ltd. 
file_name:	business.c
author:		caoshun
version:	1.02
date:		2004-11-23
description:
			�����˶�-ҵ����-�����ļ�
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

/* ����ȫ�ֱ��� */

/* ϵͳ����*/
struct stApp_Data g_AppData;
/* �����в��� */
struct stApp_Argv g_AppArgv;
/* ��־���� */
struct stErrLog g_RunLog;
/* �����ļ����� */
struct stIniFile_File g_IniFile;
/*�̶߳���*/
struct stApp_Thread g_AppThread[MAX_THREAD_NUMBER];
/* �ڴ�й©����� */
int g_MemLeak;

/*
function:   busi_init()
description:
            ��ʼ��Ӧ��ϵͳ����ȫ������
Input:      
Output:
Return:     
others: 
*/
void busi_init(void)
{
	/* ��ʼ��ȫ�ֱ��� */
	memset(&g_AppData, 0, sizeof(g_AppData));
	memset(&g_AppArgv, 0, sizeof(g_AppArgv));
	memset(g_AppThread, 0, sizeof(g_AppThread));
	
	/* ȡ��ǰϵͳ������ʱ�� */
	cf_time_getcurrent(g_AppData.chDealDate);

	/* ��־�����ʼ�� */
	ErrLog_Init(&g_RunLog);

	/* �����ļ������ʼ�� */
	IniFile_Init(&g_IniFile);

	g_MemLeak = 0;

	memset(&g_AppThread, 0, sizeof(g_AppThread));
}

/*
function:   busi_write_runlog()
description:
            ������п�����Ϣ����Ļ����־�ļ�
Input:      
            errmsg:�޻س��������Ϣ
            file:__FILE__
            line:__LINE__
Output:
Return:     
			0:�ɹ���other:���ɹ�
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
			��ʾ����İ汾��Ϣ��ʹ��˵��
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
			����������־�ļ�
Input:      
Output:
            0:�ɹ���other:���ɹ�
Return:     
others: 
*/
int busi_set_runlog(void)
{
	int iRcode;
	char chFileName[512+1];

	/* ����־�ļ� */
	memset(chFileName, 0, sizeof(chFileName));

	sprintf(chFileName, "%s/%s.%s.log", g_AppData.chPath_Log, APPLICATION_NAME, g_AppData.chDealDate);
	iRcode=ErrLog_SetLogFile(&g_RunLog, chFileName);
if(iRcode)
	{
		fprintf(stderr, "%s\n", ERROR_APP_0020);
		return -1;
	}
	
	/* ��־�ļ���ʽ���� */
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
            ���������в��������������Ƿ�Ϸ���������ʾ������Ϣ��
Input:      
            argc:�����в�������
            argv:�����в����б�
Output:
Return:     0:�ɹ���other:���ɹ�
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
            �������ļ��е�����
Input:      
            argc:�����в�������
            argv:�����в����б�
Output:
Return:     0:�ɹ���other:���ɹ�
others: 
*/
int busi_read_cfgfile(int argc, char *argv[])
{
	int iRcode;
	/* �������ļ� */
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
            ���������ļ������ݵĺϷ��ԣ�������������ݡ�
Input:      
Output:
Return:     0:�ɹ���other:���ɹ�
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

	/* ��ȡ�����ļ���GROUP */
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

	/* system GROUP����Ԫ�ط��� */
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
printf("/system GROUP����Ԫ�ط���/-->end\n");
	/* path_info GROUP����Ԫ�ط��� */
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
printf("/ path_info GROUP����Ԫ�ط���/-->end\n");
	/* record_info GROUP����Ԫ�ط��� */
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
printf("/ record_info GROUP����Ԫ�ط���/-->end\n");	
	/* new_index_field GROUP����Ԫ�ط��� */
	iCount = IniFile_Group_CountKey(new_index_field_group);
	if(iCount > MAX_INDEX_FIELD_NUMBER)
	{
		/* ҵ��֧�ָ�������ϵͳ���ֵ */
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
printf("/new_index_field GROUP����Ԫ�ط���/-->end\n");	
	/* old_index_field GROUP����Ԫ�ط��� */
	iCount = IniFile_Group_CountKey(old_index_field_group);
	if(iCount > MAX_INDEX_FIELD_NUMBER)
	{
		/* ҵ��֧�ָ�������ϵͳ���ֵ */
		BUSI_WRITE_RUNLOG(ERROR_APP_0011);
		return -1;
	}
	else
	{
		if(iCount != g_AppData.iIndexFieldNumber)
		{
			/* �������һ�� */
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
printf("/ old_index_field GROUP����Ԫ�ط���/-->end\n");	
	/* fee_field_list GROUP����Ԫ�ط��� */
	iCount = IniFile_Group_CountKey(fee_field_list_group);

	if(iCount > MAX_FEE_FIELD_NUMBER)
	{
		/* ҵ��֧�ָ�������ϵͳ���ֵ */
		BUSI_WRITE_RUNLOG(ERROR_APP_0011);
		
		return -1;
	}
	else
	{		
		for(iLoop = 0; iLoop < iCount; iLoop++)
		{
printf("/fee==[%d]����Ԫ�ط���/-->begin\n",iLoop);				
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
printf("/fee==[%d]����Ԫ�ط���/-->end\n",iLoop);		
		}
	
		g_AppData.iFeeFieldNumber = iCount;
	}
printf("/fee_field_list GROUP����Ԫ�ط���/-->end\n");	
	return 0;
}

/*
function:   busi_do_prefix()
description:
            ������ǰ��һЩ������
Input:      
Output:
Return:     0:�ɹ���other:���ɹ�
others: 
*/
int  busi_do_prefix(void)
{
	int iRcode;
	int iLoop;

	/* ������־�ļ� */
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

	/*�����߳�*/
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
            �������һЩ������
Input:      
Output:
Return:     0:�ɹ���other:���ɹ�
others: 
*/
int  busi_do_postfix(void)
{
	return 0;
}

/*
function:   busi_do_File()
description:
            �˶������ļ�
Input:      
Output:
Return:     0:�ɹ���other:���ɹ�
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

	/*��ʼ������������*/
	for(iLoop = 0; iLoop < g_AppData.iMaxRecordNumber; iLoop++)
	{
		pstObj->stNewRecordList[iLoop].iFlag = 0;
		pstObj->stOldRecordList[iLoop].iFlag = 0;

		/*
		memset(pstObj->stNewRecordList[iLoop].pchBuffer, 0, g_AppData.iNewRecordSize+1);
		memset(pstObj->stOldRecordList[iLoop].pchBuffer, 0, g_AppData.iOldRecordSize+1);
		*/
	}
	//printf("busi_do_File���̺�[%d]�̺߳�[%d]������[%s]��ʼ\n", getpid(), pthread_self(), pstObj->chRelationFlag);	
	/*���¾ɻ����ļ�*/
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

	/*��ȡ�»����ļ���ȫ����¼��������*/
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

	/*��ȡ�ɻ����ļ���ȫ����¼��������*/
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
	
	/*�ر��¾ɻ����ļ�*/
	fclose(fpOldFile);
	fclose(fpNewFile);

	/*********************************************************/

	/*���ȷ����¾����л����е�������Ϣ*/
	/*��ߺ˶�����*/
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
			/*�Ѿ������������»����ļ��еļ�¼*/
			break;
		}

		/*�����»�����¼�б�����һ��δ�����¼*/	
			
		/*�õ�ǰ�»�����¼��ɻ�����¼�б���������Ч��¼�Ա�*/
		iOldCurrentRecord = busi_do_FindMacthRecord(pstObj, iNewCurrentRecord-1);
		if(iOldCurrentRecord > 0)
		{
			/*�ҵ�ƥ���¼*/

			/*�Ա����еķ����ֶ���*/
			iRcode = busi_do_MatchRecordFee(
				pstObj,
				pstObj->stNewRecordList[iNewCurrentRecord-1].pchBuffer,
				pstObj->stOldRecordList[iOldCurrentRecord-1].pchBuffer
			);
			if(iRcode < 0)
			{
				return -1;
			}
			
			/*ɾ���»����б��д�ƥ���¼*/
			pstObj->stNewRecordList[iNewCurrentRecord-1].iFlag = 0;

			/*ɾ���ɻ����б��д�ƥ���¼*/
			pstObj->stOldRecordList[iOldCurrentRecord-1].iFlag = 0;
		}
		else
		{
		}

		iNewCurrentRecord++;
	}
	
	/*���»�����¼������δ����ļ�¼д���ļ�(�»����оɻ���û��)*/
	iRcode = busi_do_WriteFile_OldNoExist(pstObj);
	if(iRcode)
	{
		return -1;
	}
		
	/*���ɻ�����¼������δ����ļ�¼д���ļ�(�»���û�оɻ�����)*/
	iRcode = busi_do_WriteFile_NewNoExist(pstObj);
	if(iRcode)
	{
		return -1;
	}
		
	/*��ʾ�˶�ͳ����Ϣ*/
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
            ��ʾ�˶�����ͳ�ƽ��
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
	sprintf(g_RunLog.LogBuffer, "APP:MSG:������ʶ{%s}", pstObj->chRelationFlag);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:�»����ļ�{%s}", pstObj->chNewFile);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:�ɻ����ļ�{%s}", pstObj->chOldFile);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);	

	sprintf(g_RunLog.LogBuffer, "APP:MSG:�»����ļ���¼����{%d}", pstObj->iNewFileRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:�ɻ����ļ���¼����{%d}", pstObj->iOldFileRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:�¾ɻ����ļ���¼ƥ������{%d}", pstObj->iMathchRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:�ɻ���û���»����еĻ�����¼����{%d}", pstObj->iOldNoExistCmpCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:�»���û�оɻ����еĻ�����¼����{%d}", pstObj->iNewNoExistCmpCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);

	for(iLoop = 0; iLoop < g_AppData.iFeeFieldNumber; iLoop++)
	{
		sprintf(g_RunLog.LogBuffer, "APP:MSG:�¾ɻ���������{%d}��ƥ��ļ�¼����{%d}", iLoop+1, pstObj->stFeeField[iLoop].iNoCmpCount);
		BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	}

	pthread_mutex_unlock(&(g_AppData.iThreadMutex));

	return 0;
}

/*
function:   busi_do_AllFilePrintResult()
description:
            ��ʾ�����ļ��˶�����ͳ�ƽ��
Input:      
Output:
Return:
others: 
*/
void  busi_do_AllFilePrintResult(void)
{
	int iLoop;

	BUSI_WRITE_RUNLOG("APP:MSG:++++++++++++++++++++++++++++++++++");
	sprintf(g_RunLog.LogBuffer, "APP:MSG:���к˶��ļ��»����ļ���¼����{%ld}", g_AppData.iAllFileNewFileRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:���к˶��ļ��ɻ����ļ���¼����{%ld}", g_AppData.iAllFileOldFileRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:���к˶��ļ��¾ɻ����ļ���¼ƥ������{%ld}", g_AppData.iAllFileMathchRecordCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:���к˶��ļ��ɻ���û���»����еĻ�����¼����{%ld}", g_AppData.iAllFileOldNoExistCmpCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	sprintf(g_RunLog.LogBuffer, "APP:MSG:���к˶��ļ��»���û�оɻ����еĻ�����¼����{%ld}", g_AppData.iAllFileNewNoExistCmpCount);
	BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);

	for(iLoop = 0; iLoop < g_AppData.iFeeFieldNumber; iLoop++)
	{
		sprintf(g_RunLog.LogBuffer, "APP:MSG:���к˶��ļ��¾ɻ���������{%d}��ƥ��ļ�¼����{%ld}", iLoop+1, g_AppData.stFeeField[iLoop].iAllFileNoCmpCount);
		BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	}
}

/*
function:   busi_do_PreProcessNewIndexData()
description:
			���������»�����¼�������ֶ�ֵ
Input:
Output:
Return:
others:
*/
void busi_do_PreProcessNewIndexData(struct stApp_Thread *pstObj)
{
	int iLoop, iLoop1;

	/*���������»�����¼�������ֶ�ֵ*/
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
			�������оɻ�����¼�������ֶ�ֵ
Input:
Output:
Return:
others:
*/
void busi_do_PreProcessOldIndexData(struct stApp_Thread *pstObj)
{
	int iLoop, iLoop1;

	/*�������оɻ�����¼�������ֶ�ֵ*/
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
			�õ�ǰ�»�����¼��ɻ�����¼�б���������Ч��¼�Ա�
Input:
			pchNewData �»�����¼
Output:		<= 0 û���ҵ�ƥ���¼, > 0, ƥ���¼��(��1��ʼ)
Return:
others:
*/
int  busi_do_FindMacthRecord(struct stApp_Thread *pstObj, int iNewLoop)
{
	int iLoop, iLoop1;
		
	/*���»�����¼�������ֶ������оɻ����ļ��м�¼�Ա�*/
	for(iLoop = 0; iLoop < pstObj->iOldFileRecordCount; iLoop++)
	{
		/*�ü�¼����Ƿ���Ч*/
		if(pstObj->stOldRecordList[iLoop].iFlag == 0)
		{
			continue;
		}

		/*�����������ֶζԱ�*/
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
			/*���������ֶζ�ƥ��ɹ�*/
			return iLoop+1;
		}
	}

	return 0;
}

/*
function:   busi_do_MatchRecordFee()
description:
			�Ա��¾ɻ�����¼�з����ֶ�
Input:
			pNewData �»�����¼
			pOldData �ɻ�����¼
Output:		<0 �ڲ�����, =0 �����, >0 ���
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

		/*ȡ�»�����¼�������*/
		memcpy(
			pstObj->stFeeField[iLoop].stNew.chData,
			pchNewData+(g_AppData.stFeeField[iLoop].stNew.iBeginPosition-1),
			g_AppData.stFeeField[iLoop].stNew.iSeekSize
		);

		/*ȡ�ɻ�����¼�������*/
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
			/*��ǰ�����ֶ���ԱȲ��ɹ�*/

			/*ͳ��-��ǰ���öԱȲ���ͬ��¼����1*/
			pstObj->stFeeField[iLoop].iNoCmpCount ++;

			/*���¾ɻ�����¼����д���ļ�(���ò���ͬ)*/
			iRcode = busi_do_WriteFile_NoMatchFee(pstObj, iLoop, pchNewData, pchOldData);
			if(iRcode)
			{
				return -1;
			}

			return 0;
		}
		
	}

	/*ͳ��-ƥ���¼����1*/
	pstObj->iMathchRecordCount ++;

	/*���¾ɻ�����¼����д���ļ�(������ͬ)*/
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
            �˶�����Ŀ¼�������ļ�
Input:      
Output:
Return:     0:�ɹ���other:���ɹ�
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

		/*����ļ���û�й�����Ϣ�����������ļ�*/
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

			/*ƥ��������ļ�*/
			if(strstr(pNewDirent->d_name, chRelationFlag) == NULL)
			{
				continue;
			}
			else
			{
				strcpy(chNewFile, pNewDirent->d_name);
			}

			/*����Ч�Ŀձ��߳�*/
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
					sprintf(g_RunLog.LogBuffer, "APP:STA:�޿��н����߳�");
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
			
			/*ֻ�����һ����֮ƥ����ļ�*/
			break;
		}
		
		closedir(pNewDir);
	}

	closedir(pOldDir);

	/*****************************************************/
#ifdef DEBUG	
	printf("�ȴ������߳����������\n");
#endif
	
	/*�ȴ������߳����񶼽���*/
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
	printf("�����߳����������\n");
#endif

	if(g_AppData.iThreadExitFlag == 1)
	{
printf("====zhaoro");
		BUSI_WRITE_RUNLOG(ERROR_APP_0035);
		return -1;
	}

	/*****************************************************/
	/*��ʾû��ƥ�䴦����ļ�*/
	
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

		sprintf(g_RunLog.LogBuffer, "APP:MSG:{%s/%s}�ļ�û�д���", g_AppData.chPath_OldIn, pOldDirent->d_name);
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
	
		sprintf(g_RunLog.LogBuffer, "APP:MSG:{%s/%s}�ļ�û�д���", g_AppData.chPath_NewIn, pNewDirent->d_name);
		BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);
	}

	closedir(pNewDir);
	/*****************************************************/

	/*������к˶��ļ���ͳ����Ϣ*/
	busi_do_AllFilePrintResult();
	
	return 0;
}

/*
function:   busi_do_GetRelationFlag()
description:
            ȡ�ɻ����ļ����еĹ�����Ϣ
Input:      
Output:
Return:     0:�ɹ���other:���ɹ�
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
            �����ڴ�ռ�
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
            �ͷ��ڴ�ռ�
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
			ϵͳע���˳��ҹ�����������ҵ��������;�˳�ʱ��ϵͳ����β����
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
	printf("�ȴ������߳̽���\n");
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
	printf("�����߳̽���\n");
#endif

	pthread_mutex_destroy(&g_AppData.iThreadMutex);	
	
	busi_do_FreeMem();

	if(g_MemLeak != 0)
	{
		sprintf(g_RunLog.LogBuffer, "APP:MSG:�ڴ�й©�����=%d", g_MemLeak);
		BUSI_WRITE_RUNLOG(g_RunLog.LogBuffer);		
	}	
}

/*
function:   busi_do_PthreadTask()
description:
		�̴߳�������
Input:      
		pArgv:�������
Output:
Return:
		��������
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
	printf("���̺�[%d]�̺߳�[%d]���[%d]����\n", getpid(), pthread_self(), iThreadIndex);
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
		printf("���̺�[%d]�̺߳�[%d]���[%d]������[%s]��ʼ\n", getpid(), pthread_self(), iThreadIndex, pstObj->chRelationFlag);	
#endif
	//printf("���̺�[%d]�̺߳�[%d]���[%d]������[%s]��ʼ\n", getpid(), pthread_self(), iThreadIndex, pstObj->chRelationFlag);	
		/*�򿪺˶�����ļ�*/
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

		/*�˶��ļ�*/
		iRcode = busi_do_File(pstObj);
		if(iRcode)
		{
			g_AppData.iThreadExitFlag = 1;
			pthread_exit(NULL);
		}
				
		/*�رպ˶�����ļ�*/
		busi_do_CloseFile_MatchRecord(pstObj);
		busi_do_CloseFile_NewNoExist(pstObj);
		busi_do_CloseFile_OldNoExist(pstObj);
		busi_do_CloseFile_NoMatchFee(pstObj);
				
		/***********************************************/

		/*ͳ�����к˶��ļ������Ϣ*/
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


		/*�����������ļ�(��ƥ��)�Ƶ�����Ŀ¼*/
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
		printf("���̺�[%d]�̺߳�[%d]���[%d]���������\n", getpid(), pthread_self(), iThreadIndex);		
#endif	
		pstObj->iBusyFlag = 0;	
	}

#ifdef DEBUG
		printf("���̺�[%d]�̺߳�[%d]���[%d]�˳�\n", getpid(), pthread_self(), iThreadIndex);		
#endif	
	
	pthread_exit(NULL);
}

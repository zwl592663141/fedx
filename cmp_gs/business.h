/*
Copyright (C), 1995-2004, Si-Tech Information Technology Ltd. 
file_name:	business.h
author:		caoshun
version:	1.02
date:		2004-11-23
description:
			话单核对-业务函数-H文件
others:
history:
*/

#ifndef _BUSINESS_H_
#define _BUSINESS_H_

#define BUSI_WRITE_RUNLOG(aaa) busi_write_runlog(aaa, __FILE__, __LINE__)

int  busi_usage(int argc, char *argv[]);
int  busi_read_cfgfile(int argc, char *argv[]);
int  busi_check_cfgfile(void);
void busi_init(void);
void busi_version(void);
void busi_exit(void);
int  busi_set_runlog(void);
int  busi_write_runlog(char *errmsg, char *file, int line);
int  busi_do_prefix(void);
int  busi_do_postfix(void);
void busi_atexit(void);

int busi_do_MallocMem(void);
int busi_do_FreeMem(void);
int busi_do_Path(void);
int  busi_do_File(struct stApp_Thread *pstObj);
int busi_do_PrintResult(struct stApp_Thread *pstObj);
void busi_do_AllFilePrintResult(void);
int  busi_do_FindMacthRecord(struct stApp_Thread *pstObj, int iNewLoop);
int  busi_do_MatchRecordFee(struct stApp_Thread *pstObj, char *pchNewData, char *pchOldData);
int busi_do_GetRelationFlag(char *chOldFile, char *chRelationFlag);
void busi_do_PreProcessNewIndexData(struct stApp_Thread *pstObj);
void busi_do_PreProcessOldIndexData(struct stApp_Thread *pstObj);

void *busi_do_PthreadTask(void *pArgv);

int  busi_do_OpenFile_MatchRecord(struct stApp_Thread *pstObj);
int  busi_do_OpenFile_NewNoExist(struct stApp_Thread *pstObj);
int  busi_do_OpenFile_OldNoExist(struct stApp_Thread *pstObj);
int  busi_do_OpenFile_NoMatchFee(struct stApp_Thread *pstObj);
int  busi_do_CloseFile_MatchRecord(struct stApp_Thread *pstObj);
int  busi_do_CloseFile_NewNoExist(struct stApp_Thread *pstObj);
int  busi_do_CloseFile_OldNoExist(struct stApp_Thread *pstObj);
int  busi_do_CloseFile_NoMatchFee(struct stApp_Thread *pstObj);
int  busi_do_WriteFile_NoMatchFee(struct stApp_Thread *pstObj, int iIndex, char *pchNewData, char *pchOldData);
int  busi_do_WriteFile_MatchRecord(struct stApp_Thread *pstObj, char *pchNewData, char *pchOldData);
int  busi_do_WriteFile_OldNoExist(struct stApp_Thread *pstObj);
int  busi_do_WriteFile_NewNoExist(struct stApp_Thread *pstObj);

#endif

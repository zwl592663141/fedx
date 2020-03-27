/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	o2n_cmp.h
author:		caoshun
version:		2.00
date:		2006-01-07
description:
			话单核对-宏与结构定义-H文件
others:
history:
			20060107 caoshun 增加线程处理
*/

#ifndef _O2N_CMP_H_
#define _O2N_CMP_H_

#include <stdio.h>
#include <pthread.h>

/* 应用程序名称 */
#define APPLICATION_NAME				"o2n_cmp"
/* 应用程序版本号 */
#define APPLICATION_VERSION			"2.00"

#define MAX_THREAD_NUMBER				15			/*最大子线程数*/
#define MAX_INDEX_FIELD_NUMBER	      10		/*索引对比字段最大处理数*/
#define MAX_FEE_FIELD_NUMBER			25			/*费用对比字段最大处理数*/
#define MAX_FILE_RECORD_NUMBER	      2000000		/*话单文件最大记录数*/
#define MAX_FILE_WRITEBUFFER			1048576		/*文件缓冲空间大小*/
#define MAX_RECORD_LEN					4096		/*话单每行记录最大长度*/
#define MAX_FIELD_LEN					128			/*索引字段或比较字段最大长度*/

/*输出结果分隔线*/
#define APP_SPLIT_LINE	"--------------------------------------\n"

#define APP_FIELDTYPE_NUMBER		1
#define APP_FIELDTYPE_STRING		2

#define APP_THREAD_BUSY_SLEEP		3

/* 程序内部错误信息 */
#define ERROR_APP_0010 "APP:ERR:配置文件操作错误"
#define ERROR_APP_0011 "APP:ERR:配置文件内容错误"
#define ERROR_APP_0012 "APP:ERR:配置文件中指定文件或目录错误"
#define ERROR_APP_0020 "APP:ERR:运行日志文件操作错误"
#define ERROR_APP_0030 "APP:ERR:数据文件记录数太大"
#define ERROR_APP_0031 "APP:ERR:线程个数太大"
#define ERROR_APP_0032 "APP:ERR:建立线程失败"
#define ERROR_APP_0033 "APP:ERR:合并线程失败"
#define ERROR_APP_0034 "APP:ERR:线程加减锁失败"
#define ERROR_APP_0035 "APP:ERR:线程内部操作失败"
#define ERROR_APP_9997 "APP:ERR:打开文件错误"
#define ERROR_APP_9998 "APP:ERR:内存分配错误"
#define ERROR_APP_9999 "APP:ERR:程序内部错误"
#define ERROR_SYS_9999 "SYS:ERR:系统内部错误"

/*话单索引对比字段结构*/
struct stData_FieldInfo
{
	int iBeginPosition;				/*开始位置*/
	int iSeekSize;					/*操作位移长度*/
	char chData[MAX_FIELD_LEN+1];	/*内容(字符)*/
	long lData;						/*内容(数字)*/
	long iUnit;						/*费用操作单位*/
};

/*所有话单费用对比字段结构*/
struct stData_FeeField
{
	char chPath[128+1];			/*费用不同输出目录*/
	int iFieldType;				/*字段类型*/	
	struct stData_FieldInfo stNew;	/*新话单记录费用字段信息*/
	struct stData_FieldInfo stOld;	/*旧话单记录费用字段信息*/	
	int iNoCmpCount;				/*此费用字段对比不相同记录统计数*/	
	int iAllFileNoCmpCount;		/*所有核对文件-此费用字段对比不相同记录统计数*/
};

/*数据对比集*/
struct stData_CmpRecord
{
	char *pchBuffer;
	int iFlag;

	char chIndexData[MAX_INDEX_FIELD_NUMBER][MAX_FIELD_LEN+1];
};

/*线程全局数据*/
struct stApp_Thread
{
	pthread_t iThreadId;						/*内部编号*/
	char chThreadId[16+1];						/*内部编号*/	
	int iBusyFlag;								/*是否空间标记*/
	char chOldFile[512+1];						/*旧数据文件名*/
	char chNewFile[512+1];						/*新数据文件名*/
	char chRelationFlag[32+1];					/*文件匹配标记*/
	int iOldNoExistCmpCount;						/*旧话单没有新话单有的话单记录统计数*/
	int iNewNoExistCmpCount;					/*新话单没有旧话单有的话单记录统计数*/
	int iNewFileRecordCount;						/*新话单文件记录总数*/
	int iOldFileRecordCount;						/*旧话单文件记录总数*/
	int iMathchRecordCount;						/*新旧话单文件记录匹配总数*/	
	FILE *fpMatchRecord;							/*新旧话单中费用相同的话单记录备份文件*/
	FILE *fpNewNoExist;							/*旧话单有新话单没有的话单记录备份文件*/
	FILE *fpOldNoExist;							/*旧话单没有新话单有的话单记录备份文件*/
	FILE *fpNoMatchFee[MAX_FEE_FIELD_NUMBER];	/*新旧话单记录费用对比不同备份文件*/

	struct stData_CmpRecord *stNewRecordList;		/*新话单数据对比集*/
	struct stData_CmpRecord *stOldRecordList;		/*旧话单数据对比集*/	

	struct stData_FeeField stFeeField[MAX_FEE_FIELD_NUMBER];	/*新旧话单记录费用对比字段数据集*/			

	/*文件写缓冲*/
	char chFileBuffer_MatchRecord[MAX_FILE_WRITEBUFFER+1];
	char chFileBuffer_NewNoExist[MAX_FILE_WRITEBUFFER+1];
	char chFileBuffer_OldNoExist[MAX_FILE_WRITEBUFFER+1];
	char chFileBuffer_NoMatchFee[MAX_FEE_FIELD_NUMBER][MAX_FILE_WRITEBUFFER+1];	

};

/* 全局数据结构 */
struct stApp_Data
{
	char chDealDate[14+1];			/*程序开始处理时间*/
	char chPath_Log[128+1];			/*日志文件输出目录*/
	char chPath_OldNoExist[128+1];	/*旧话单没有新话单有的话单记录存放目录*/
	char chPath_NewNoExist[128+1];	/*新话单没有旧话单有的话单记录存放目录*/
	char chPath_FeeCmp[128+1];		/*新旧话单中费用相同的话单记录存放目录*/
	char chPath_NewIn[128+1];		/*新版话单核对入口目录*/
	char chPath_NewOut[128+1];		/*新版话单核对出口目录*/
	char chPath_OldIn[128+1];		/*旧版话单核对入口目录*/
	char chPath_OldOut[128+1];		/*旧版话单核对出口目录*/

	long iAllFileOldNoExistCmpCount;				/*所有核对文件-旧话单没有新话单有的话单记录统计数*/
	long iAllFileNewNoExistCmpCount;				/*所有核对文件-新话单没有旧话单有的话单记录统计数*/
	long iAllFileNewFileRecordCount;				/*所有核对文件-新话单文件记录总数*/
	long iAllFileOldFileRecordCount;				/*所有核对文件-旧话单文件记录总数*/
	long iAllFileMathchRecordCount;				/*所有核对文件-新旧话单文件记录匹配总数*/
	
	int iIndexFieldNumber;						/*话单索引比对定位字段数*/
	int iFeeFieldNumber;							/*话单费用对比字段数*/
	
	struct stData_FieldInfo stOldIndexField[MAX_INDEX_FIELD_NUMBER];	/*旧话单记录索引对比字段数据集*/
	struct stData_FieldInfo stNewIndexField[MAX_INDEX_FIELD_NUMBER];	/*新话单记录索引对比字段数据集*/

	struct stData_FeeField stFeeField[MAX_FEE_FIELD_NUMBER];			/*新旧话单记录费用对比字段数据集*/		

	int iNewRecordSize;							/*新话单记录长度*/
	int iOldRecordSize;							/*旧话单记录长度*/

	int iMaxRecordNumber;						/*最大文件记录数*/
	int iThreadNumber;							/*最大子线程数*/
	pthread_mutex_t iThreadMutex;				/*线程锁*/
	int iThreadExitFlag;							/*线程退出标识*/
};

/*命令行参数结构*/
struct stApp_Argv
{
	char chConfigFile[128+1];						/*配置文件*/
};

#endif

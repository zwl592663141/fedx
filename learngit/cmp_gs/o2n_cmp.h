/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	o2n_cmp.h
author:		caoshun
version:		2.00
date:		2006-01-07
description:
			�����˶�-����ṹ����-H�ļ�
others:
history:
			20060107 caoshun �����̴߳���
*/

#ifndef _O2N_CMP_H_
#define _O2N_CMP_H_

#include <stdio.h>
#include <pthread.h>

/* Ӧ�ó������� */
#define APPLICATION_NAME				"o2n_cmp"
/* Ӧ�ó���汾�� */
#define APPLICATION_VERSION			"2.00"

#define MAX_THREAD_NUMBER				15			/*������߳���*/
#define MAX_INDEX_FIELD_NUMBER	      10		/*�����Ա��ֶ��������*/
#define MAX_FEE_FIELD_NUMBER			25			/*���öԱ��ֶ��������*/
#define MAX_FILE_RECORD_NUMBER	      2000000		/*�����ļ�����¼��*/
#define MAX_FILE_WRITEBUFFER			1048576		/*�ļ�����ռ��С*/
#define MAX_RECORD_LEN					4096		/*����ÿ�м�¼��󳤶�*/
#define MAX_FIELD_LEN					128			/*�����ֶλ�Ƚ��ֶ���󳤶�*/

/*�������ָ���*/
#define APP_SPLIT_LINE	"--------------------------------------\n"

#define APP_FIELDTYPE_NUMBER		1
#define APP_FIELDTYPE_STRING		2

#define APP_THREAD_BUSY_SLEEP		3

/* �����ڲ�������Ϣ */
#define ERROR_APP_0010 "APP:ERR:�����ļ���������"
#define ERROR_APP_0011 "APP:ERR:�����ļ����ݴ���"
#define ERROR_APP_0012 "APP:ERR:�����ļ���ָ���ļ���Ŀ¼����"
#define ERROR_APP_0020 "APP:ERR:������־�ļ���������"
#define ERROR_APP_0030 "APP:ERR:�����ļ���¼��̫��"
#define ERROR_APP_0031 "APP:ERR:�̸߳���̫��"
#define ERROR_APP_0032 "APP:ERR:�����߳�ʧ��"
#define ERROR_APP_0033 "APP:ERR:�ϲ��߳�ʧ��"
#define ERROR_APP_0034 "APP:ERR:�̼߳Ӽ���ʧ��"
#define ERROR_APP_0035 "APP:ERR:�߳��ڲ�����ʧ��"
#define ERROR_APP_9997 "APP:ERR:���ļ�����"
#define ERROR_APP_9998 "APP:ERR:�ڴ�������"
#define ERROR_APP_9999 "APP:ERR:�����ڲ�����"
#define ERROR_SYS_9999 "SYS:ERR:ϵͳ�ڲ�����"

/*���������Ա��ֶνṹ*/
struct stData_FieldInfo
{
	int iBeginPosition;				/*��ʼλ��*/
	int iSeekSize;					/*����λ�Ƴ���*/
	char chData[MAX_FIELD_LEN+1];	/*����(�ַ�)*/
	long lData;						/*����(����)*/
	long iUnit;						/*���ò�����λ*/
};

/*���л������öԱ��ֶνṹ*/
struct stData_FeeField
{
	char chPath[128+1];			/*���ò�ͬ���Ŀ¼*/
	int iFieldType;				/*�ֶ�����*/	
	struct stData_FieldInfo stNew;	/*�»�����¼�����ֶ���Ϣ*/
	struct stData_FieldInfo stOld;	/*�ɻ�����¼�����ֶ���Ϣ*/	
	int iNoCmpCount;				/*�˷����ֶζԱȲ���ͬ��¼ͳ����*/	
	int iAllFileNoCmpCount;		/*���к˶��ļ�-�˷����ֶζԱȲ���ͬ��¼ͳ����*/
};

/*���ݶԱȼ�*/
struct stData_CmpRecord
{
	char *pchBuffer;
	int iFlag;

	char chIndexData[MAX_INDEX_FIELD_NUMBER][MAX_FIELD_LEN+1];
};

/*�߳�ȫ������*/
struct stApp_Thread
{
	pthread_t iThreadId;						/*�ڲ����*/
	char chThreadId[16+1];						/*�ڲ����*/	
	int iBusyFlag;								/*�Ƿ�ռ���*/
	char chOldFile[512+1];						/*�������ļ���*/
	char chNewFile[512+1];						/*�������ļ���*/
	char chRelationFlag[32+1];					/*�ļ�ƥ����*/
	int iOldNoExistCmpCount;						/*�ɻ���û���»����еĻ�����¼ͳ����*/
	int iNewNoExistCmpCount;					/*�»���û�оɻ����еĻ�����¼ͳ����*/
	int iNewFileRecordCount;						/*�»����ļ���¼����*/
	int iOldFileRecordCount;						/*�ɻ����ļ���¼����*/
	int iMathchRecordCount;						/*�¾ɻ����ļ���¼ƥ������*/	
	FILE *fpMatchRecord;							/*�¾ɻ����з�����ͬ�Ļ�����¼�����ļ�*/
	FILE *fpNewNoExist;							/*�ɻ������»���û�еĻ�����¼�����ļ�*/
	FILE *fpOldNoExist;							/*�ɻ���û���»����еĻ�����¼�����ļ�*/
	FILE *fpNoMatchFee[MAX_FEE_FIELD_NUMBER];	/*�¾ɻ�����¼���öԱȲ�ͬ�����ļ�*/

	struct stData_CmpRecord *stNewRecordList;		/*�»������ݶԱȼ�*/
	struct stData_CmpRecord *stOldRecordList;		/*�ɻ������ݶԱȼ�*/	

	struct stData_FeeField stFeeField[MAX_FEE_FIELD_NUMBER];	/*�¾ɻ�����¼���öԱ��ֶ����ݼ�*/			

	/*�ļ�д����*/
	char chFileBuffer_MatchRecord[MAX_FILE_WRITEBUFFER+1];
	char chFileBuffer_NewNoExist[MAX_FILE_WRITEBUFFER+1];
	char chFileBuffer_OldNoExist[MAX_FILE_WRITEBUFFER+1];
	char chFileBuffer_NoMatchFee[MAX_FEE_FIELD_NUMBER][MAX_FILE_WRITEBUFFER+1];	

};

/* ȫ�����ݽṹ */
struct stApp_Data
{
	char chDealDate[14+1];			/*����ʼ����ʱ��*/
	char chPath_Log[128+1];			/*��־�ļ����Ŀ¼*/
	char chPath_OldNoExist[128+1];	/*�ɻ���û���»����еĻ�����¼���Ŀ¼*/
	char chPath_NewNoExist[128+1];	/*�»���û�оɻ����еĻ�����¼���Ŀ¼*/
	char chPath_FeeCmp[128+1];		/*�¾ɻ����з�����ͬ�Ļ�����¼���Ŀ¼*/
	char chPath_NewIn[128+1];		/*�°滰���˶����Ŀ¼*/
	char chPath_NewOut[128+1];		/*�°滰���˶Գ���Ŀ¼*/
	char chPath_OldIn[128+1];		/*�ɰ滰���˶����Ŀ¼*/
	char chPath_OldOut[128+1];		/*�ɰ滰���˶Գ���Ŀ¼*/

	long iAllFileOldNoExistCmpCount;				/*���к˶��ļ�-�ɻ���û���»����еĻ�����¼ͳ����*/
	long iAllFileNewNoExistCmpCount;				/*���к˶��ļ�-�»���û�оɻ����еĻ�����¼ͳ����*/
	long iAllFileNewFileRecordCount;				/*���к˶��ļ�-�»����ļ���¼����*/
	long iAllFileOldFileRecordCount;				/*���к˶��ļ�-�ɻ����ļ���¼����*/
	long iAllFileMathchRecordCount;				/*���к˶��ļ�-�¾ɻ����ļ���¼ƥ������*/
	
	int iIndexFieldNumber;						/*���������ȶԶ�λ�ֶ���*/
	int iFeeFieldNumber;							/*�������öԱ��ֶ���*/
	
	struct stData_FieldInfo stOldIndexField[MAX_INDEX_FIELD_NUMBER];	/*�ɻ�����¼�����Ա��ֶ����ݼ�*/
	struct stData_FieldInfo stNewIndexField[MAX_INDEX_FIELD_NUMBER];	/*�»�����¼�����Ա��ֶ����ݼ�*/

	struct stData_FeeField stFeeField[MAX_FEE_FIELD_NUMBER];			/*�¾ɻ�����¼���öԱ��ֶ����ݼ�*/		

	int iNewRecordSize;							/*�»�����¼����*/
	int iOldRecordSize;							/*�ɻ�����¼����*/

	int iMaxRecordNumber;						/*����ļ���¼��*/
	int iThreadNumber;							/*������߳���*/
	pthread_mutex_t iThreadMutex;				/*�߳���*/
	int iThreadExitFlag;							/*�߳��˳���ʶ*/
};

/*�����в����ṹ*/
struct stApp_Argv
{
	char chConfigFile[128+1];						/*�����ļ�*/
};

#endif

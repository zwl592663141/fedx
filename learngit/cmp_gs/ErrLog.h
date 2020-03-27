/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	CookieFunc.c
author:		caoshun
version:	1.15
date:		2005-03-04
description:
			����һ����־������-H�ļ���
			���Ǽǳ�������ʱ������Ϣ����־��Ϣ��
			�û��������Ѷ�����־�ĵȼ��������ʽ��
others:
			��־��Ϣ���������ʽΪ��
			FORMAT[pid][serial][time][level][filename][lineno][data]
			
			pid		:������־�Ľ��̺�
			serial	:��־�ڲ���ˮ��
			time	:��־��¼ʱ��
			level	:��־��¼�ȼ�(debug, log, warning, error, fail)
			filename:��־��Ϣ�������ڵ�Դ�ļ���
			lineno	:��־��Ϣ�������ڵ�Դ�ļ��к�
			data	:�û��Զ������־��Ϣ
history:
			2004.10.22 ������ErrLog_CloseFileʱ����Ա����LogFileû�����
			2004.09.20 �����ڲ����ڸ�ʽ ��ʽ(YYYY:MM:DD HH:MM:SS) ��ʽ(YYYY/MM/DD HH:MM:SS)
			2004.08.31 ����ErrLog_OpenFile(), ErrLog_CloseFile()��
			           �ļ�����ʽ�����֣�
			           [ERRLOG_FILE_ALWAYOPEN]
			           ��������ʱ��־�ļ�һֱ�򿪣���ָ�����ļ���ر��ļ���
			           [ERRLOG_FILE_ALWAYCLOSE]
			           ����֧��ʱ��־�ļ����򿪣�ÿ��д��־���Զ�����ر��ļ���
			2004.07.28 ����__MYFUNC__�궨�塣
			2004.07.20 ��ɡ�
*/

#ifndef	_ERRLOG_H_
#define	_ERRLOG_H_

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*��־��Ϣ��ʽ���Ʊ�־*/
#define ERRLOG_FORMAT_FILE     0x00000001
#define ERRLOG_FORMAT_LINE     0x00000010
#define ERRLOG_FORMAT_TIME     0x00000100
#define ERRLOG_FORMAT_LEVEL    0x00001000
#define ERRLOG_FORMAT_PID      0x00010000
#define ERRLOG_FORMAT_SERIAL   0x00100000

/*��־�ȼ�������־*/
#define ERRLOG_LEVEL_DEBUG     1
#define ERRLOG_LEVEL_LOG       2
#define ERRLOG_LEVEL_WARNING   3
#define ERRLOG_LEVEL_ERROR     4
#define ERRLOG_LEVLE_FAIL      5

#define ERRLOG_FILE_ALWAYOPEN  1		
#define ERRLOG_FILE_ALWAYCLOSE 2

#define ERRLOG_MAX_BUFFER_LEN  8192

#ifdef _INCLUDE_HPUX_SOURCE 
#define __MYFUNC__ __FUNCTION__
#else
#ifdef _AIX
#define __MYFUNC__ __FUNCTION__
#else
#define __MYFUNC__ __func__
#endif
#endif


#define ERRLOG(log, data) ErrLog_Log(&log, data, __FILE__, __LINE__)
#define ERRLOG_BUFFER(log) ErrLog_Log(&log, log.LogBuffer, __FILE__, __LINE__)
#define ERRLOG_LEVEL(log, level, data) ErrLog_LogLevel(&log, level, data, __FILE__, __LINE__)

struct stErrLog
{
	char LogFile[ERRLOG_MAX_BUFFER_LEN+1];
	char LogBuffer[ERRLOG_MAX_BUFFER_LEN+1];
	int  LogLevel;
	int  LogFormat;
	long LogSerial;
	int  FileAlwayFlag;
	FILE *fpLogFile;
};

void  ErrLog_Init        (struct stErrLog *obj);
int   ErrLog_SetLogFile  (struct stErrLog *obj, char *data);
char *ErrLog_GetLogFile  (struct stErrLog *obj);
int   ErrLog_OpenFile    (struct stErrLog *obj);
int   ErrLog_CloseFile   (struct stErrLog *obj);
int   ErrLog_ifLogFile   (struct stErrLog *obj);
void  ErrLog_SetLogLevel (struct stErrLog *obj, int data);
int   ErrLog_GetLogLevel (struct stErrLog *obj);
void  ErrLog_SetLogFormat(struct stErrLog *obj, int data);
int   ErrLog_GetLogFormat(struct stErrLog *obj);
int   ErrLog_ClearLog    (struct stErrLog *obj);
int   ErrLog_Log         (struct stErrLog *obj, char *data, char *filename, int lineno);
int   ErrLog_LogLevel    (struct stErrLog *obj, int level, char *data, char *filename, int lineno);

int   ErrLog__Log        (struct stErrLog *obj, char *filename, int lineno, int level, char *data);

#if defined(__cplusplus)
}
#endif

#endif /* _ERRLOG_H_ */

/*
	void ErrLog__GetTime(char *data)
	

	int  ErrLog__Log(struct stErrLog *obj, char *filename, int lineno, int errorno, int level, char *data)
	�ڲ����ã���־��¼�ײ㺯��

	void ErrLog_Init(struct stErrLog *obj)
	
	
	int   ErrLog_OpenFile(struct stErrLog *obj)
	����־�ļ�һֱ�򿪡�
	
	int   ErrLog_CloseFile(struct stErrLog *obj)
	�ر�һֱ�򿪵���־�ļ���

	int ErrLog_SetLogFile(struct stErrLog *obj, char *data)
	

	char *ErrLog_GetLogFile(struct stErrLog *obj)

	int ErrLog_ifLogFile(struct stErrLog *obj)
	�Ƿ�Log������ļ� 0:�ļ� -1:��Ļ

	void ErrLog_SetLogLevel(struct stErrLog *obj, int data)
	

	int ErrLog_GetLogLevel(struct stErrLog *obj, )
	

	void ErrLog_SetLogFormat(struct stErrLog *obj, int data)
	

	int ErrLog_GetLogFormat(struct stErrLog *obj, )
	

	int ErrLog_ Log(struct stErrLog *obj, char *data, char *filename, int lineno)
	

	int ErrLog_LogLevel(struct stErrLog *obj, int level, char *data, char *filename, int lineno)
	��¼��־��Ϣ(����ǰ��־�����ȼ�)
	------------------------------
*/

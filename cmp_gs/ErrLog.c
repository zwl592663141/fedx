/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	CookieFunc.c
author:		caoshun
version:	1.15
date:		2005-03-04
description: 
			����һ����־������
others:
history:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "ErrLog.h"

/*
int ErrLog(char	*program_file, unsigned	int program_line, char *format,	...)
{
	va_list	args;
	char buffer[BUFFER_MAX_LEN];

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);
	return(_ErrLog(program_file, program_line , buffer));
}
*/

/*
function:	ErrLog_Init()
description:��ĳ�ʼ������
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		
others:
*/
void ErrLog_Init(struct stErrLog *obj)
{
	memset(obj->LogFile,  0, sizeof(obj->LogFile));
	memset(obj->LogBuffer,0, sizeof(obj->LogBuffer));
	obj->LogLevel  = 1;
	obj->LogFormat = 0;
	obj->LogSerial = 0;
	obj->FileAlwayFlag = ERRLOG_FILE_ALWAYCLOSE;
	obj->fpLogFile = NULL;

	obj->LogFormat = obj->LogFormat
		| ERRLOG_FORMAT_FILE
		| ERRLOG_FORMAT_LINE
	;
}

/*
function:	ErrLog_SetLogFile()
description:������־�ļ���
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		0:�ɹ�,!0:ʧ��
others:
			���data=""����־���stderr��Ļ���ڲ�����־�ļ��ɷ������жϡ�
*/
int ErrLog_SetLogFile(struct stErrLog *obj, char *data)
{
	FILE *fp;

	strncpy(obj->LogFile, data, ERRLOG_MAX_BUFFER_LEN);
	if(strcmp(obj->LogFile, "") == 0)
	{
		return 0;
	}

	fp = fopen(obj->LogFile, "a");
	if(fp == NULL)
	{
	printf("sss\n");
		fprintf(stderr, "ERRLOG LIBRARY:open logfile[%s] error!\n", obj->LogFile);
		return -1;
	}
	else
	{
		if(fclose(fp) == EOF)
		{
			fprintf(stderr, "ERRLOG LIBRARY:close logfile[%s] error!\n", obj->LogFile);
			return -1;
		}
	}

	return 0;
}

/*
function:	ErrLog_GetLogFile()
description:������־�ļ���
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		��־�ļ���
others:
*/
char* ErrLog_GetLogFile(struct stErrLog *obj)
{
	return obj->LogFile;
}

/*
function:	ErrLog_SetLogLevel()
description:������־�����ȼ�
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		
others:
*/
void ErrLog_SetLogLevel(struct stErrLog *obj, int data)
{
	obj->LogLevel = data;
}

/*
function:	ErrLog_GetLogLevel()
description:������־�����ȼ�
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		��־�ȼ�
others:
*/
int ErrLog_GetLogLevel(struct stErrLog *obj)
{
	return obj->LogLevel;
}

/*
function:	ErrLog_SetLogFormat()
description:������־�����ʽ��־
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		
others:
*/
void ErrLog_SetLogFormat(struct stErrLog *obj, int data)
{
	obj->LogFormat = data;
}

/*
function:	ErrLog_GetLogFormat()
description:������־�����ʽ��־
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		��־��־��
others:
*/
int ErrLog_GetLogFormat(struct stErrLog *obj)
{
	return obj->LogFormat;
}

/*
function:	ErrLog_ClearLog()
description:����־�ļ����
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		0:�ɹ�,!0ʧ��
others:
*/
int ErrLog_ClearLog(struct stErrLog *obj)
{
	FILE *fp;

	if(strcmp(obj->LogFile, "") != 0)
	{
		if(obj->FileAlwayFlag == ERRLOG_FILE_ALWAYOPEN)
		{
			fprintf(stderr, "ERRLOG LIBRARY:logfile[%s] already open!\n", obj->LogFile);
			return -1;
		}

		fp = fopen(obj->LogFile, "w");
		if(fp == NULL)
		{
			fprintf(stderr, "ERRLOG LIBRARY:open logfile[%s] error!\n", obj->LogFile);
			return -1;
		}
		else
		{
			if(fclose(fp) == EOF)
			{
				fprintf(stderr, "ERRLOG LIBRARY:close logfile[%s] error!\n", obj->LogFile);
				return -1;
			}
		}
	}

	fprintf(stderr, "ERRLOG LIBRARY:no set logfile!\n");
	return -1;
}

/*
function:	ErrLog_Log()
description:��¼��־��Ϣ
Input:		obj:�ڲ����ݽṹ
			data:��־��Ϣ
			filename:Դ�����ļ���
			lineno:Դ�����к�
Output:		
Return:		0:�ɹ�,!0ʧ��
others:
*/
int ErrLog_Log(struct stErrLog *obj, char *data, char *filename, int lineno)
{
	return ErrLog__Log(obj, filename, lineno, obj->LogLevel, data);
}

/*
function:	ErrLog_LogLevel()
description:��¼��־��Ϣ(����ǰ��־�����ȼ�)
Input:		obj:�ڲ����ݽṹ
			level:ָ����ǰ��־�ȼ�
			data:��־��Ϣ
			filename:Դ�����ļ���
			lineno:Դ�����к�
Output:		
Return:		0:�ɹ�,!0ʧ��
others:
*/
int ErrLog_LogLevel(struct stErrLog *obj, int level, char *data, char *filename, int lineno)
{
	return ErrLog__Log(obj, filename, lineno, level, data);
}

/*
function:	ErrLog__GetTime()
description:�ڲ����ã��õ���ǰʱ��
Input:		
Output:		data:��ǰʱ���ַ���
Return:		
others:
*/
void ErrLog__GetTime(char *data)
{
	struct tm *Time;
	time_t Time_t;

	time(&Time_t);
	Time = localtime(&Time_t);
	sprintf
	(
		data,
		"%d/%02d/%02d %02d:%02d:%02d",
		Time->tm_year+1900,
		Time->tm_mon+1,
		Time->tm_mday,
		Time->tm_hour,
		Time->tm_min,
		Time->tm_sec
	);
}

/*
function:	ErrLog__Log()
description:�ڲ����ã���־��¼�ײ㺯��
Input:		obj:�ڲ����ݽṹ
			level:ָ����ǰ��־�ȼ�
			data:��־��Ϣ
			filename:Դ�����ļ���
			lineno:Դ�����к�
Output:		
Return:		
others:
*/
int ErrLog__Log(struct stErrLog *obj, char *filename, int lineno, int level, char *data)
{
	char buffer[ERRLOG_MAX_BUFFER_LEN+1];
	char log_buffer[ERRLOG_MAX_BUFFER_LEN+1];
	FILE *fp;

	memset(buffer, 0, sizeof(buffer));
	memset(log_buffer, 0, sizeof(log_buffer));

	if(obj->LogFormat & ERRLOG_FORMAT_PID)
	{
		snprintf(buffer, ERRLOG_MAX_BUFFER_LEN, "[%ld]", getpid());
		strncat(log_buffer, buffer, ERRLOG_MAX_BUFFER_LEN);
	}

	if(obj->LogFormat & ERRLOG_FORMAT_SERIAL)
	{
		obj->LogSerial ++;
		snprintf(buffer, ERRLOG_MAX_BUFFER_LEN, "[%ld]", obj->LogSerial);
		strncat(log_buffer, buffer, ERRLOG_MAX_BUFFER_LEN);
	}

	if(obj->LogFormat & ERRLOG_FORMAT_TIME)
	{
		ErrLog__GetTime(buffer);
		strncat(log_buffer, "[", ERRLOG_MAX_BUFFER_LEN);
		strncat(log_buffer, buffer, ERRLOG_MAX_BUFFER_LEN);
		strncat(log_buffer, "]", ERRLOG_MAX_BUFFER_LEN);
	}

	if(obj->LogFormat & ERRLOG_FORMAT_LEVEL)
	{
		switch(level)
		{
			case 1 :
					snprintf(buffer, ERRLOG_MAX_BUFFER_LEN, "[debug]"); break;
			case 2 :
					snprintf(buffer, ERRLOG_MAX_BUFFER_LEN, "[log]"); break;
			case 3 :
					snprintf(buffer, ERRLOG_MAX_BUFFER_LEN, "[warning]"); break;
			case 4 :
					snprintf(buffer, ERRLOG_MAX_BUFFER_LEN, "[error]"); break;
			case 5 :
					snprintf(buffer, ERRLOG_MAX_BUFFER_LEN, "[fail]"); break;
		}
		strncat(log_buffer, buffer, ERRLOG_MAX_BUFFER_LEN);
	}

	if(obj->LogFormat & ERRLOG_FORMAT_FILE)
	{
		snprintf(buffer, ERRLOG_MAX_BUFFER_LEN, "[%s]", filename);
		strncat(log_buffer, buffer, ERRLOG_MAX_BUFFER_LEN);
	}

	if(obj->LogFormat & ERRLOG_FORMAT_LINE)
	{
		snprintf(buffer, ERRLOG_MAX_BUFFER_LEN, "[%d]", lineno);
		strncat(log_buffer, buffer, ERRLOG_MAX_BUFFER_LEN);
	}

	if(strcmp(obj->LogFile, "") == 0)
	{
		fprintf(stderr, "%s[%s]\n", log_buffer, data);
		return 0;
	}

	if(obj->FileAlwayFlag == ERRLOG_FILE_ALWAYOPEN)
	{
		fp = obj->fpLogFile;
	}
	else
	{
		fp = fopen(obj->LogFile, "a");
		if(fp == NULL)
		{
			fprintf(stderr, "ERRLOG LIBRARY:open logfile[%s] error!\n", obj->LogFile);
			return -1;
		}
	}

	if(fprintf(fp, "%s[%s]\n", log_buffer, data) < 0)
	{
		fclose(fp);
		fprintf(stderr, "ERRLOG LIBRARY:write logfile[%s] error!\n", obj->LogFile);
		return -1;
	}

	if(fflush(fp) == EOF)
	{
		fclose(fp);
		fprintf(stderr, "ERRLOG LIBRARY:write logfile[%s] error!\n", obj->LogFile);
		return -1;
	}

	if(obj->FileAlwayFlag == ERRLOG_FILE_ALWAYCLOSE)
	{
		if(fclose(fp) == EOF)
		{
			fprintf(stderr, "ERRLOG LIBRARY:close logfile[%s] error!\n", obj->LogFile);
			return -1;
		}
	}

	return 0;
}

/*
function:	ErrLog_ifLogFile()
description:�Ƿ�Log������ļ� 
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		0:�ļ� -1:��Ļ
others:
*/
int ErrLog_ifLogFile(struct stErrLog *obj)
{
	if(strcmp(obj->LogFile, "") == 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/*
function:	ErrLog_OpenFile()
description:����־�ļ�һֱ��
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		0:�ɹ�,!0:ʧ��
others:
*/
int   ErrLog_OpenFile(struct stErrLog *obj)
{
	obj->fpLogFile = fopen(obj->LogFile, "a");
	if(obj->fpLogFile == NULL)
	{
		fprintf(stderr, "ERRLOG LIBRARY:open logfile[%s] error!\n", obj->LogFile);
		return -1;
	}
	
	obj->FileAlwayFlag = ERRLOG_FILE_ALWAYOPEN;
	return 0;
}

/*
function:	ErrLog_CloseFile()
description:�ر�һֱ�򿪵���־�ļ�
Input:		obj:�ڲ����ݽṹ
Output:		
Return:		0:�ɹ�,!0:ʧ��
others:
*/
int   ErrLog_CloseFile   (struct stErrLog *obj)
{
	if(obj->FileAlwayFlag == ERRLOG_FILE_ALWAYCLOSE)
	{
		return 0;
	}

	if(obj->fpLogFile == NULL)
	{
		return 0;
	}

	if(fclose(obj->fpLogFile) == EOF)
	{
		fprintf(stderr, "ERRLOG LIBRARY:close logfile[%s] error!\n", obj->LogFile);
		return -1;
	}

	obj->fpLogFile = NULL;
	obj->FileAlwayFlag = ERRLOG_FILE_ALWAYCLOSE;
	memset(obj->LogFile, 0, sizeof(obj->LogFile));


	return 0;
}

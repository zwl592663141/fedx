

/********************************************************************
    Copyright (C), 1995-2004, Si-Tech Information Technology Ltd.
    file_name: common.h
    author: ��ʤѧ version: ��V1.00�� date: ��2009��05��20��
    description: ���幫�����ݡ�����
    others:
    history: 1.date: �޸�ʱ�� version: �汾��V1.00��author: ����
            modification: �޸�����
         2. ...
********************************************************************/


#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <strings.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <malloc.h>

#define MY_TRUE             1               /* �Զ����߼���ֵ */
#define MY_FALSE            0               /* �Զ����߼���ֵ */
#define MY_SUCCEED          0               /* �Զ�����ȷ״ֵ̬ */
#define MY_FAIL             1               /* �Զ������״ֵ̬ */

#define DATE_YEAR_LEN       4               /* ���׼���� */
#define DATE_MON_LEN        2               /* �±�׼���� */
#define DATE_DAY_LEN        2               /* �ձ�׼���� */
#define DATE_HOUR_LEN       2               /* Сʱ��׼���� */
#define DATE_MIN_LEN        2               /* ���ӱ�׼���� */
#define DATE_SEC_LEN        2               /* ���׼���� */
#define DATE_STD_LEN        14              /* ���ڱ�׼���� */

#define INFO_SOURCE_APP     "app"           /* ���������������Ϣ */
#define INFO_SOURCE_DB      "db"            /* ���ݿ��������Ϣ */
#define INFO_SOURCE_SYS     "sys"           /* �������������ݿ��������Ϣ, ������̵��ȵ�����ԭ���������Ϣ */

#define INFO_LEVEL_GEN      "gen"           /* ��ͨ��Ϣ */
#define INFO_LEVEL_WARN     "warn"          /* �澯��Ϣ */
#define INFO_LEVEL_EXIT     "exit"          /* ���ظ澯 */

#define ERROR_BASE          500             /* ��������ַ */

#define FILE_NOTE_FLAG      "#"             /* �ļ���¼ע�ͱ�־ */
#define CHAR_NEW_LINE       "\n"            /* ���з�  */
#define FILE_FIELD_SEP      "\t "           /* ��¼�����ķָ��� */

#define MAX_RECORD_LEN      2048            /* ��¼��󳤶� */
#define MAX_PATH_LEN        1024            /* ·����󳤶� */
#define MAX_REGEX_LEN       512
#define MAX_RULE_LEN        512
#ifndef max
    #define max( a, b )         ( (a) < (b) ) ? (b) : (a)
#endif
#ifndef min
    #define min( a, b )         ( (a) > (b) ) ? (b) : (a)
#endif

typedef struct _TIME_DATA
{
    char szTime[DATE_STD_LEN + 1];                              /* ʱ�� */

    char szYear[DATE_YEAR_LEN + 1];                             /* �� */
    char szMon[DATE_MON_LEN + 1];                               /* �� */
    char szDay[DATE_DAY_LEN + 1];                               /* �� */
    char szHour[DATE_HOUR_LEN + 1];                             /* ʱ */
    char szMin[DATE_MIN_LEN + 1];                               /* �� */
    char szSec[DATE_SEC_LEN + 1];                               /* �� */
} TIME_DATA;
typedef TIME_DATA *PTIME_DATA;

char szLogPath[512];                        /* ��־�ļ����·�� */
char szLogPrefix[128];                      /* ��־�ļ�ǰ׺ */
char szSrcDir[512];                         /* ԴĿ¼�� */
char szBinName[128];                        /* ���г����� */

time_t tmStartTime;                         /*  */
time_t tmEndTime;                           /*  */


int StrSplit(char *split_str, char *src_str, const char split_char);
int MatchField(const char *src_str,const char *regex_field, const char split_flag);
int CharDelete(char *fstr, const char target);
int CharPos(const char *source,const char target);
int MyLog( char *szSource, char *szLevel, char *szFmt, ... );


#endif

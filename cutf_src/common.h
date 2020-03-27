

/********************************************************************
    Copyright (C), 1995-2004, Si-Tech Information Technology Ltd.
    file_name: common.h
    author: 吉胜学 version: （V1.00） date: （2009－05－20）
    description: 定义公用数据、函数
    others:
    history: 1.date: 修改时间 version: 版本（V1.00）author: 作者
            modification: 修改内容
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

#define MY_TRUE             1               /* 自定义逻辑真值 */
#define MY_FALSE            0               /* 自定义逻辑假值 */
#define MY_SUCCEED          0               /* 自定义正确状态值 */
#define MY_FAIL             1               /* 自定义错误状态值 */

#define DATE_YEAR_LEN       4               /* 年标准长度 */
#define DATE_MON_LEN        2               /* 月标准长度 */
#define DATE_DAY_LEN        2               /* 日标准长度 */
#define DATE_HOUR_LEN       2               /* 小时标准长度 */
#define DATE_MIN_LEN        2               /* 分钟标准长度 */
#define DATE_SEC_LEN        2               /* 秒标准长度 */
#define DATE_STD_LEN        14              /* 日期标准长度 */

#define INFO_SOURCE_APP     "app"           /* 进程自身产生的信息 */
#define INFO_SOURCE_DB      "db"            /* 数据库产生的信息 */
#define INFO_SOURCE_SYS     "sys"           /* 除进程自身、数据库以外的信息, 如因进程调度等其他原因产生的信息 */

#define INFO_LEVEL_GEN      "gen"           /* 普通信息 */
#define INFO_LEVEL_WARN     "warn"          /* 告警信息 */
#define INFO_LEVEL_EXIT     "exit"          /* 严重告警 */

#define ERROR_BASE          500             /* 错误代码基址 */

#define FILE_NOTE_FLAG      "#"             /* 文件记录注释标志 */
#define CHAR_NEW_LINE       "\n"            /* 换行符  */
#define FILE_FIELD_SEP      "\t "           /* 记录各域间的分隔符 */

#define MAX_RECORD_LEN      2048            /* 记录最大长度 */
#define MAX_PATH_LEN        1024            /* 路径最大长度 */
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
    char szTime[DATE_STD_LEN + 1];                              /* 时间 */

    char szYear[DATE_YEAR_LEN + 1];                             /* 年 */
    char szMon[DATE_MON_LEN + 1];                               /* 月 */
    char szDay[DATE_DAY_LEN + 1];                               /* 日 */
    char szHour[DATE_HOUR_LEN + 1];                             /* 时 */
    char szMin[DATE_MIN_LEN + 1];                               /* 分 */
    char szSec[DATE_SEC_LEN + 1];                               /* 秒 */
} TIME_DATA;
typedef TIME_DATA *PTIME_DATA;

char szLogPath[512];                        /* 日志文件存放路径 */
char szLogPrefix[128];                      /* 日志文件前缀 */
char szSrcDir[512];                         /* 源目录名 */
char szBinName[128];                        /* 运行程序名 */

time_t tmStartTime;                         /*  */
time_t tmEndTime;                           /*  */


int StrSplit(char *split_str, char *src_str, const char split_char);
int MatchField(const char *src_str,const char *regex_field, const char split_flag);
int CharDelete(char *fstr, const char target);
int CharPos(const char *source,const char target);
int MyLog( char *szSource, char *szLevel, char *szFmt, ... );


#endif

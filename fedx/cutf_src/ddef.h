/*
    需要 CUTF_FORMAT_PATH,默认为FEDX_CONFIG_PATH
    fedx统计、提取工具
    1)提取指定字段：
        cutf -f(fields) msisdn,system_type -m fix aa.fix*
        cutf -f(fields) error_code,msisdn,start_datetime aa.fedx*
        cutf -c(column) 1-2,3-4 aa.fedx*(提取指定列)
    2)提取符合条件的话单到fedx格式记录
        cutf  -k(key) "system_type==* && error_code==Edwd001"  aa.fedx bb.fedx out.fedx
        cutf  -t(control) ./cutf.ctl  aa.fedx bb.fedx out.fedx
        ++++++  ctl格式 +++++
        system_type == vc && msisdn == 13079995712
        system_type == ss && ( msisdn == 13079995711 || other_party == 13079995711 )
        +++++++++++++++++++++
    参数：
        -f fields   segnames
        -m mode     fix/fdex, fedx as default
        -c column   1-2,1-,1
        -k key      logic expression

*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include "common.h"

#ifdef FEDX
    #include "fedx.h"
    fedx_param s_fedx_param;        /*fedx变量信息*/
#endif

#define MAX_READ_FILE           40960

#define CUTF_FORMAT_PATH        "CUTF_FORMAT_PATH"
#define FEDX_CONFIG_PATH        "FEDX_CONFIG_PATH"
#define FEDX_CONFIG_NAME        "FEDX_CONFIG_NAME"

#define BINFILE                 "cutf"
#define VERSION                 "V1.0"

#define OPT_K_CHAR              "-k"
#define OPT_K_STR               "-key"

#define OPT_F_CHAR              "-f"
#define OPT_F_STR               "-field"

#define OPT_M_CHAR              "-m"
#define OPT_M_STR               "-mode"

#define OPT_T_CHAR              "-t"
#define OPT_T_STR               "-control"

#define OPT_C_CHAR              "-c"
#define OPT_C_STR               "-column"

#define OPT_V_CHAR              "-v"
#define OPT_V_STR               "-version"

#define OPT_H_CHAR              "-h"
#define OPT_H_STR               "-help"

#define FIELD_SEP_CHAR          ","
#define COLUMN_SEP_CHAR         '-'
#define FMT_SEP_CHAR            "\t "
#define OUT_RECORD_SEP_CHAR     "|"

/*格式描述记录*/
typedef struct  _FORMAT_RECORD        /*文件记录格式*/
{
    int nFieldNo;                     /*字段序号：字段在配置文件中的序号，从0开始，依次增1*/
    char szFieldName[128 + 1];        /*字段名称：字段的名称*/
    char szPath[256 + 1];

    int nStart;                       /*字段开始位置：字段在详单中的开始位置，从0开始*/
    int nEnd;                         /*字段结束位置：字段在详单中的结束位置，从0开始*/
    int nLength;                      /*字段长度：字段在详单记录中的长度*/
    int nType;                        /*字段数据类型：0：字符，1：整数，10：缺省字段*/

    char szDefaultVal[256 + 1];       /*字段缺省值：当字段类型为10时，字段取值为字段缺省值，仅对批价有效*/
    char szMapCode[8 + 1];            /*字段内容映射文件序号：详单字段内容映射文件在详单字段内容映射控制文件中的序号。仅对详单查询有效*/
    char szFieldDesc[1024 + 1];       /*字段说明：字段的描述信息*/

} FORMAT_RECORD;

typedef FORMAT_RECORD *PFORMAT_RECORD;

/* 格式描述文件*/
typedef struct _FORMAT_FILES
{
    PFORMAT_RECORD  pFormatRecord;
    char            szSystemType[8 + 1];
    int             nCount;
    struct _FORMAT_FILES    *next;

}   FORMAT_FILES;

typedef struct FORMAT_FILES *PFORMAT_FILES;

/* 抽取的域名*/
typedef struct _FIELDS
{
    int  nOffset;
    int  nLen;
    char szFieldName[128 + 1];
	char groupName[10+1];
    struct _FIELDS *next;

}   FIELDS;

typedef FIELDS  *PFIELDS;


/* 条件域*/
typedef struct _KEY_FIELD
{
    char szLeft[128 + 1];
    char szRight[128 + 1];
    char szRelation[8 + 1];
    int  nResult;/*1-true,0-flase*/
}   KEY_FIELD;

typedef struct KEY_FIELD    *PKEY_FIELD;

/* 入口文件列表*/
typedef struct _FILES
{
    char szFile[512];
    struct _FILES   *next;
} FILES;

typedef struct FILES    *PFILES;

/* 全局数据*/
typedef struct _CDATA
{
    char            szBinFile[256];
    bool            fFlag;  /* field(1),field(2)...field(n)*/
    bool            cFlag;  /* column(eg.1-2,1,1-*/
    bool            mFlag;  /* true-fix,false-fedx*/
    bool            kFlag;  /* key,logic expression*/
    bool            tFlag;  /* key,batch mode ,control file*/
    bool            vFlag;  /* version information*/
    bool            hFlag;  /* help information*/
    FORMAT_FILES    *psFmtFiles;
    FIELDS          *psFields;
    FIELDS          *psColums;
    FILES           *psFiles;
    char            szOutFile[512];
} CDATA;

CDATA   cData;

int CompareByFileNo( void *left, void *right );
FORMAT_FILES *SearchFmtGroup( FORMAT_FILES *psFmtGroup, char *szSystemType );
void ExitProgram( int nCode );
void Version();
void Help() ;
int ParseField( FIELDS **pHead, char *szStr, char cMode );


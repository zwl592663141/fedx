/*
    ��Ҫ CUTF_FORMAT_PATH,Ĭ��ΪFEDX_CONFIG_PATH
    fedxͳ�ơ���ȡ����
    1)��ȡָ���ֶΣ�
        cutf -f(fields) msisdn,system_type -m fix aa.fix*
        cutf -f(fields) error_code,msisdn,start_datetime aa.fedx*
        cutf -c(column) 1-2,3-4 aa.fedx*(��ȡָ����)
    2)��ȡ���������Ļ�����fedx��ʽ��¼
        cutf  -k(key) "system_type==* && error_code==Edwd001"  aa.fedx bb.fedx out.fedx
        cutf  -t(control) ./cutf.ctl  aa.fedx bb.fedx out.fedx
        ++++++  ctl��ʽ +++++
        system_type == vc && msisdn == 13079995712
        system_type == ss && ( msisdn == 13079995711 || other_party == 13079995711 )
        +++++++++++++++++++++
    ������
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
    fedx_param s_fedx_param;        /*fedx������Ϣ*/
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

/*��ʽ������¼*/
typedef struct  _FORMAT_RECORD        /*�ļ���¼��ʽ*/
{
    int nFieldNo;                     /*�ֶ���ţ��ֶ��������ļ��е���ţ���0��ʼ��������1*/
    char szFieldName[128 + 1];        /*�ֶ����ƣ��ֶε�����*/
    char szPath[256 + 1];

    int nStart;                       /*�ֶο�ʼλ�ã��ֶ����굥�еĿ�ʼλ�ã���0��ʼ*/
    int nEnd;                         /*�ֶν���λ�ã��ֶ����굥�еĽ���λ�ã���0��ʼ*/
    int nLength;                      /*�ֶγ��ȣ��ֶ����굥��¼�еĳ���*/
    int nType;                        /*�ֶ��������ͣ�0���ַ���1��������10��ȱʡ�ֶ�*/

    char szDefaultVal[256 + 1];       /*�ֶ�ȱʡֵ�����ֶ�����Ϊ10ʱ���ֶ�ȡֵΪ�ֶ�ȱʡֵ������������Ч*/
    char szMapCode[8 + 1];            /*�ֶ�����ӳ���ļ���ţ��굥�ֶ�����ӳ���ļ����굥�ֶ�����ӳ������ļ��е���š������굥��ѯ��Ч*/
    char szFieldDesc[1024 + 1];       /*�ֶ�˵�����ֶε�������Ϣ*/

} FORMAT_RECORD;

typedef FORMAT_RECORD *PFORMAT_RECORD;

/* ��ʽ�����ļ�*/
typedef struct _FORMAT_FILES
{
    PFORMAT_RECORD  pFormatRecord;
    char            szSystemType[8 + 1];
    int             nCount;
    struct _FORMAT_FILES    *next;

}   FORMAT_FILES;

typedef struct FORMAT_FILES *PFORMAT_FILES;

/* ��ȡ������*/
typedef struct _FIELDS
{
    int  nOffset;
    int  nLen;
    char szFieldName[128 + 1];
	char groupName[10+1];
    struct _FIELDS *next;

}   FIELDS;

typedef FIELDS  *PFIELDS;


/* ������*/
typedef struct _KEY_FIELD
{
    char szLeft[128 + 1];
    char szRight[128 + 1];
    char szRelation[8 + 1];
    int  nResult;/*1-true,0-flase*/
}   KEY_FIELD;

typedef struct KEY_FIELD    *PKEY_FIELD;

/* ����ļ��б�*/
typedef struct _FILES
{
    char szFile[512];
    struct _FILES   *next;
} FILES;

typedef struct FILES    *PFILES;

/* ȫ������*/
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


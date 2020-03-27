#ifndef _MAIN_H_
#define _MAIN_H_

#include "os.h"
#include "database.h"
#include "ora_dpapi.h"
#include "pub_func.h"
#include "sdfs.h"

SDFS strSdfs; 

#define  DEBUG_INFO          /*打印调试信息*/

#define    SOFT_VERSION        "5.3"       /*版本号*/
#define    LOG_INTERVAL          900       /*日志输出时间间隔*/

#define    MAX_REC_STR_LEN      4000       /*定义最大的话单长度*/
#define    MAX_FILE_NAME_LEN     300       /*定义最大的文件名长度*/
#define    MAX_TABLE_NAME_LEN    300       /*表名的最大长度*/
#define    MAX_LOAD_ROW_LEN      400       /*数据区的最大行数 insert方式使用*/
#define    MAX_LOAD_FIELD_LEN    200       /*每个字段的最大长度 insert方式使用*/

#define    MAX_FILE_NUM          300       /*文件列表的最大文件数*/
#define    MAX_FLIST_LEN         100       /*文件列表中每个文件名的长度*/

#define    TABLE_PARAM    "b_load_param"
#define    TABLE_FIELD    "b_load_field"
#define    TABLE_FILE     "b_load_file"

typedef struct
{
	char chCluster[32+1];  //归属集群
	char chAttr[16+1];			//节点属性
	char chService[16+1];		//节点服务
	char chProcessName[64+1]; //进程启动bin文件名
	char chBuf[2048+1];			//返回字符串 ,查过这个长度 解析不了，如果必须扩长，还需要修改getTextField_long的长度
	char chInport[32+1];		//端口号
	char chProcEndFlag[32+1];			//目录id,用于区分进程的,也用于作为文件名唯一标识
	char sClusterInfo[256+1];		//集群信息
}stAppCfg_Zookeeper;


/*在入口参数表中配置的参数*/
typedef struct _param_table_
{
    char  inPath[MAX_FILE_NAME_LEN];      /*输入路径*/
    char  bakPath[MAX_FILE_NAME_LEN];     /*备份路径*/
    char  outPath[MAX_FILE_NAME_LEN];     /*输出路径*/
    char  errPath[MAX_FILE_NAME_LEN];     /*错单路径*/
    char  dupPath[MAX_FILE_NAME_LEN];     /*重单路径*/
    char  outPreFix[50];                  /*输出文件名前缀*/
    char  outPostFix[50];                 /*输出文件名后缀*/
    char  proclogPath[MAX_FILE_NAME_LEN]; /*处理日志路径*/
    char  runlogPath[MAX_FILE_NAME_LEN];  /*运行日志路径*/
    char  runFlag;                        /*是否需要循环处理 1是 0否*/
    char  sysTable[100];                  /*调度表表名*/
    char  procID[100];                    /*进程标识*/
    char  bLoadFile[100];                 /*读取配置b_load_file表*/
    char  bLoadField[100];                 /*读取配置b_load_field表*/
    stAppCfg_Zookeeper stZookeeper;
} PARAM_TABLE;

/*字段定义的数据结构*/
typedef struct _field_
{
    int seq;               /*序号*/
    char field_name[40];   /*字段名*/
    int pos;               /*位置*/
    int len;               /*长度*/
    char data_type[20];    /*类型*/
    char format[128];       /*格式*/
    char funcFlag;         /*是否为函数，1额外是，0否, 2 直接引用函数*/
} FIELD;

/*文件类型定义*/
typedef struct _file_info_
{
    char proc_id[40];           /*进程名标识*/
    char file_head[64];         /*文件名标识*/
    int  fieldNum;              /*字段总数*/
    char date_format[30];       /*日期格式*/
    char table[128];            /*表名格式*/
    int  table_fileFlag;        /*表名是否和文件名有关*/
    FIELD  field[500];          /*字段格式*/
    struct col_def column[500]; /*列定义 direct方式*/
    struct fld_def dfield[500]; /*字段定义 direct方式*/
    struct tab_def table_def;   /*表定义 direct方式*/
    int  flagDirect;            /*1 direct方式 0 insert方式*/
    int  headNum;               /*文件头记录行数*/
    int  tailNum;               /*文件尾记录行数*/
    char segChar[16];           /*分隔符*/
} FILE_INFO;

/*数据库信息*/
typedef struct _db_info_
{
    char  dbServer[50];                     /*数据库名*/
    char  dbUser[50];                       /*用户名*/
    char  dbPass[50];                       /*用户密码*/
    char  dbFile[MAX_FILE_NAME_LEN];        /*存储用户密码的文件*/
    DB_SESSION dbSession;                   /*数据库事务*/
} DB_INFO;

/**********全局变量**********************************************/
DB_INFO         gs_dbInfo;             /*数据库信息*/
struct loadctl  gs_dpctl;              /*DP API句柄*/
PARAM_TABLE     gs_paramTable;         /*在入口参数表中配置的参数*/
FILE_INFO       *gs_FileInfo;          /*各类文件对应的字段格式等信息*/
int             gi_FileNum;            /*文件类型的个数*/
char            *gc_FileList;          /*待处理的文件名列表*/

extern short GetArgParam(char *exePath, char *exeName, char *lockFile, int argc, char *argv[]);
extern short GetAllParam(char *outStr, DB_SESSION *DBSession, char *procID);
extern short processFile(char *outStr, PARAM_TABLE *PT, FILE_INFO *FI,
                  DB_SESSION *DBSession, struct loadctl  *lctl, char *fileName);
extern int GetSysStatus(char *outStr, DB_SESSION *DBSession, char *inTableName, short stopFlag, char *procId);

#endif

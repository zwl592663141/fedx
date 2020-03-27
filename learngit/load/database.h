/***********************************************************
 * Copyright (c) 1999,2002 .
 * Descriptions:
 *      database.h
 *      用于ORACLE数据库的连接及对数据库的各种操作.
 * Known bugs:
 *
 * Change log:
 *        who          MM/DD/YYYY        why
 *        wjs          12/05/2002        create
 **********************************************************/

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <oci.h>

#define ORACLE

#define DB_SUCCESS        0
#define DB_FAILURE        -1
#define DB_NO_DATA_FOUND  -100

#define MAX_FIELD_LEN  1024   /*字段最大长度*/
#define MAX_COL_LEN  50       /*最多列数*/

/*ORACLE环境变量*/
typedef struct _db_session
{
    OCIEnv      *envhp;
    OCIServer   *srvhp;
    OCIError    *errhp;
    OCISvcCtx   *svchp;
    OCIStmt     *stmthp;
    OCISession  *authp;
}DB_SESSION;

/*字段记录*/
typedef  struct _table_record
{
   char field[MAX_FIELD_LEN];
}TABLE_RECORD;

/*登录数据库*/
extern short ConnectDBMS(char *outStr, DB_SESSION *DBSession, char *user, char *password, char *server);

/*断开数据库*/
extern void DisconnectDBMS(DB_SESSION *DBSession);

/*获取数据库错误代码*/
extern sb4 GetDBErrCode( OCIError *errhp );

/*执行oracle语句*/
extern short ExecuteSql(char *outStr, DB_SESSION *DBSession, char *inSql);

/*从表中取一个字符串*/
extern short GetValueFromTable(char *outStr, char *outValue, DB_SESSION *DBSession, char *inSql);

/*取表的行数*/
extern int GetCountOfTable(char *outStr, DB_SESSION *DBSession, char *in_TableName);

/*取表的下一条数据*/
extern short FetchData(char *outStr, DB_SESSION *DBSession);

/*取一个SQL语句有多少列*/
int GetColNumFromSql(char *inSql);

/*取表的首记录*/
extern short GetRowFromTable(char *outStr, DB_SESSION *DBSession,
                      TABLE_RECORD *dataRec, int colNum, char *inSql,
                      TABLE_RECORD *vaRec, int vaCount);

/*批量执行语句*/
extern short ExeMultiSql(char *outStr, DB_SESSION *DBSession, char *inSql,
               char *recData, int inFieldLen, int inRowNum, int inColNum, int inMaxBindNum);
#endif

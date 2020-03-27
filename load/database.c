/***********************************************************
 * Copyright (c) 1999,2002 .
 * Descriptions:
 *      database.c
 *      用于ORACLE数据库的连接及对数据库的各种操作.
 * Known bugs:
 *
 * Change log:
 *   who   MM/DD/YYYY        why
 *   wjs   12/05/2002        create
 **********************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "database.h"

/***********************************************************
 * Function: 将输入字符串变成大写.
 * Parameter:
 *      [I]str                 :输入字符串
 * Returns:
 *      大写字符串
 **********************************************************/
char *UpCaseString(char *str)
{
    int i,l;

    l = (int)strlen(str);
    for(i=0;i<l;i++)
    {
        if(islower((int)str[i]))
            str[i] = (char)toupper((int)str[i]);
        else
            str[i] = str[i];
    }
    str[l] = '\0';

    return str;
}

/***********************************************************
 * Function: 获取数据库错误代码.
 * Parameter:
 *      [I]errhp               :错误句柄
 * Returns:
 *      错误代码
 **********************************************************/
sb4 GetDBErrCode( OCIError *errhp )
{
    char errBuf[512];
    sb4 errcode = 0;

    (void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode,
               (text *)errBuf, (ub4) sizeof(errBuf), OCI_HTYPE_ERROR);

    return errcode;
}

/***********************************************************
 * Function: 判断数据库错误.
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [I]errhp               :错误句柄
 *      [I]status              :错误状态
 * Returns:
 *      0                      :无错误
 *      -1                     :出错
 **********************************************************/
short CheckErr( char *outStr, OCIError *errhp, sword status )
{
    char errBuf[512];
    sb4 errcode = 0;
    int i;

    memset( errBuf, '\0', sizeof( errBuf ) );

    switch (status)
    {
        case OCI_SUCCESS:  /*0*/
            sprintf(outStr, "SUCCESS");
            break;
        case OCI_SUCCESS_WITH_INFO:  /*1*/
            sprintf(outStr, "OCI_SUCCESS_WITH_INFO");
            break;
        case OCI_NEED_DATA:   /*99*/
            sprintf(outStr, "OCI_NEED_DATA");
            break;
        case OCI_NO_DATA:   /*100*/
            sprintf(outStr, "OCI_NO_DATA");
            break;
        case OCI_ERROR:   /*-1*/
            (void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode,
                                (text *)errBuf, (ub4) sizeof(errBuf), OCI_HTYPE_ERROR);
            errBuf[100]='\0';
            for(i=0;i<(int)strlen(errBuf);i++)
            {
                if(errBuf[i]=='\n')
                {
                    errBuf[i]='\0';
                    break;
                }
            }
            sprintf(outStr, errBuf);
            break;
        case OCI_INVALID_HANDLE:   /*-2*/
            sprintf(outStr, "OCI_INVALID_HANDLE");
            break;
        case OCI_STILL_EXECUTING:   /*-3123*/
            strcpy(outStr, "OCI_STILL_EXECUTING");
            break;
        case OCI_CONTINUE:   /*-24200*/
            sprintf(outStr, "OCI_CONTINUE");
            break;
        default:
            sprintf(outStr, "DEFAULT");
    }

    if(status == OCI_SUCCESS)
        return DB_SUCCESS;
    else
        return DB_FAILURE;
}

/***********************************************************
 * Function: 登录数据库.
 * Parameter:
 *      [O]outStr              :执行结果描述
 *      [I]DBSession           :ORACLE事务指针
 *      [I]user                :用户名
 *      [I]password            :密码
 *      [I]server              :数据库连接串
 * Returns:
 *      0                      :登录成功
 *      -1                     :登录失败
 **********************************************************/
short ConnectDBMS(char *outStr, DB_SESSION *DBSession,
                  char *user, char *password, char *server)
{
         /*创建OCI环境*/
    if( OCIInitialize((ub4) OCI_OBJECT, (dvoid *)0,
                    (dvoid * (*)(dvoid *, size_t)) 0,
                    (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                    (void (*)(dvoid *, dvoid *)) 0 ))
    {
        sprintf(outStr, "FAILED:OCIInitialize");
        return OCI_ERROR;
    }

    if( OCIEnvInit( (OCIEnv **) &(DBSession->envhp), OCI_DEFAULT, (size_t) 0, (dvoid **) 0 ))
    {
        sprintf(outStr, "FAILED:OCIEnvInit");
        return OCI_ERROR;
    }
         /*申请错误句柄*/
    if( OCIHandleAlloc( (dvoid *) (DBSession->envhp), (dvoid **) &(DBSession->errhp),
              OCI_HTYPE_ERROR,     (size_t) 0, (dvoid **) 0))
    {
        sprintf(outStr, "FAILED:OCIHandleAlloc On Error Handle");
        return OCI_ERROR;
    }
         /*申请服务器句柄*/
    if( OCIHandleAlloc( (dvoid *) (DBSession->envhp), (dvoid **) &(DBSession->srvhp),
              OCI_HTYPE_SERVER,    (size_t) 0, (dvoid **) 0))
    {
        sprintf(outStr, "FAILED:OCIHandleAlloc On Srv Handle");
        return OCI_ERROR;
    }
         /*申请服务环境句柄*/
    if( OCIHandleAlloc( (dvoid *) (DBSession->envhp), (dvoid **) &(DBSession->svchp),
              OCI_HTYPE_SVCCTX,    (size_t) 0, (dvoid **) 0))
    {
        sprintf(outStr, "FAILED:OCIHandleAlloc On Service Context Handle");
        return OCI_ERROR;
    }
        /*连接server数据库*/
    if( OCIServerAttach( DBSession->srvhp, DBSession->errhp, (text *)server, strlen(server), 0))
    {
        sprintf(outStr, "FAILED:OCIServerAttach" );
        return OCI_ERROR;
    }
        /*设置服务环境的服务器属性*/
    if( OCIAttrSet( (dvoid *) (DBSession->svchp), OCI_HTYPE_SVCCTX, (dvoid *)(DBSession->srvhp),
               (ub4) 0,    OCI_ATTR_SERVER, (OCIError *) (DBSession->errhp)))
    {
        sprintf(outStr, "FAILED:OCIAttrSet" );
        return OCI_ERROR;
    }
        /*申请会话句柄*/
    if( OCIHandleAlloc((dvoid *) (DBSession->envhp), (dvoid **)&(DBSession->authp),
               (ub4) OCI_HTYPE_SESSION,    (size_t) 0, (dvoid **) 0))
    {
        sprintf(outStr, "FAILED:OCIHandleAlloc On Authentication Handle" );
        return OCI_ERROR;
    }
        /*设置会话所使用的用户帐户*/
    if( OCIAttrSet((dvoid *) (DBSession->authp), (ub4) OCI_HTYPE_SESSION,
                    (dvoid *) user, (ub4) strlen(user),
                    (ub4) OCI_ATTR_USERNAME, DBSession->errhp))
    {
        sprintf(outStr, "FAILED:OCIAttrSet on USERID" );
        return OCI_ERROR;
    }
        /*设置会话所使用的用户密码*/
    if( OCIAttrSet((dvoid *) (DBSession->authp), (ub4) OCI_HTYPE_SESSION,
                    (dvoid *)password, (ub4) strlen(password),
                    (ub4) OCI_ATTR_PASSWORD, DBSession->errhp))
    {
        sprintf(outStr, "FAILED:OCIAttrSet on PASSWD");
        return OCI_ERROR;
    }
       /*认证用户并建立会话*/
    if( CheckErr(outStr, DBSession->errhp, OCISessionBegin ( DBSession->svchp,
               DBSession->errhp, DBSession->authp, OCI_CRED_RDBMS, (ub4) OCI_DEFAULT))<0)
    {
        return OCI_ERROR;
    }
       /*设置会话服务环境*/
    if( OCIAttrSet((dvoid *) (DBSession->svchp), (ub4) OCI_HTYPE_SVCCTX,
                    (dvoid *) (DBSession->authp), (ub4) 0,
                    (ub4) OCI_ATTR_SESSION, (DBSession->errhp)))
    {
        sprintf(outStr, "FAILED:OCIAttrSet on session" );
        return OCI_ERROR;
    }
       /*申请语句句柄*/
    if( CheckErr(outStr, (DBSession->errhp), OCIHandleAlloc( (dvoid *) (DBSession->envhp),
               (dvoid **) &(DBSession->stmthp),    OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0))<0)
    {
        return OCI_ERROR;
    }

    return OCI_SUCCESS;
}

/***********************************************************
 * Function: 断开数据库.
 * Parameter:
 *        [I]DBSession           :ORACLE事务指针
 * Returns:
 *        none
 **********************************************************/
void DisconnectDBMS(DB_SESSION *DBSession)
{
    /* Release SQL statement handler */
    if (DBSession->stmthp != NULL)
    {
        OCIHandleFree((dvoid *)(DBSession->stmthp), OCI_HTYPE_STMT);
        DBSession->stmthp = NULL;
    }
    /* Delete the user session */
    if (DBSession->authp != NULL)
    {
        OCISessionEnd(DBSession->svchp, DBSession->errhp, DBSession->authp, (ub4)OCI_DEFAULT);
        DBSession->authp = NULL;
    }
    /* Delete access to the data source */
    if (DBSession->errhp)
    {
        OCIServerDetach(DBSession->srvhp, DBSession->errhp, OCI_DEFAULT);
    }
    /* Explicitly deallocate all handles */
    if (DBSession->srvhp != NULL)
    {
        OCIHandleFree((dvoid *)(DBSession->srvhp), OCI_HTYPE_SERVER);
        DBSession->srvhp = NULL;
    }
    if (DBSession->svchp != NULL)
    {
        OCIHandleFree((dvoid *)(DBSession->svchp), OCI_HTYPE_SVCCTX);
        DBSession->svchp = NULL;
    }
    if (DBSession->errhp != NULL)
    {
        OCIHandleFree((dvoid *)(DBSession->errhp), OCI_HTYPE_ERROR);
        DBSession->errhp = NULL;
    }

    return;
}

/***********************************************************
 * Function: 从表中取一个字符串.
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [O]outValue            :返回取得的字符串
 *      [I]DBSession           :ORACLE事务指针
 *      [I]inSql               :SQL语句
 * Returns:
 *      0                      :正常
 *      -1                     :出错
 **********************************************************/
short GetValueFromTable(char *outStr, char *outValue, DB_SESSION *DBSession, char *inSql)
{
    OCIDefine *defn1p = (OCIDefine *) 0;
    char tmpStr[1024];

    if( CheckErr( outStr, DBSession->errhp, OCIStmtPrepare( DBSession->stmthp,
                  DBSession->errhp, (text *)inSql, (ub4)strlen( (char *)inSql ),
                  (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT)) < 0 )
    {
        return DB_FAILURE;
    }

    if( CheckErr( outStr, DBSession->errhp, OCIDefineByPos( DBSession->stmthp,
                  &defn1p, DBSession->errhp,  1, (dvoid *) tmpStr,
                  sizeof(tmpStr), SQLT_STR, (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT) ) < 0 )
    {
        return DB_FAILURE;
    }

    if( CheckErr( outStr, DBSession->errhp, OCIStmtExecute( DBSession->svchp,
                  DBSession->stmthp, DBSession->errhp, (ub4)1, (ub4) 0,
                  (CONST OCISnapshot *)NULL, (OCISnapshot *)NULL, OCI_DEFAULT) ) < 0 )
         return DB_FAILURE;

    sprintf(outValue, "%s", tmpStr);

    return DB_SUCCESS;

}

/***********************************************************
 * Function: 取表的行数.
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [I]DBSession           :ORACLE事务指针
 *      [I]inTableName         :表名
 * Returns:
 *      >=0                    :表行数
 *      -1                     :出错
 **********************************************************/
int GetCountOfTable(char *outStr, DB_SESSION *DBSession, char *inTableName)
{
    char sqlCmd[1024], tmpStr[100];
    int tmpCount, len;
    char *p;

    tmpCount=0;
    memset( sqlCmd, '\0', sizeof(sqlCmd) );
    sprintf( sqlCmd, "select count(*) from (%s)",inTableName);
    p=strstr(UpCaseString(sqlCmd), "ORDER BY ");
    sprintf( sqlCmd, "select count(*) from (%s)",inTableName);
    if(p!=NULL)
    {
        len=(int)strlen(sqlCmd);
        sprintf(sqlCmd+len-(int)strlen(p), ")");
    }

    if( GetValueFromTable(outStr, tmpStr, DBSession, sqlCmd)<0)
        return DB_FAILURE;

    tmpCount = atoi(tmpStr);
    return tmpCount;
}

/***********************************************************
 * Function: 取表的下一条数据.
 * Parameter:
 *      [I]DBSession           :ORACLE事务指针
 * Returns:
 *      0                      :成功
 *      100                    :到达末尾
 *      其他                   :失败
 **********************************************************/
short FetchData(char *outStr, DB_SESSION *DBSession)
{
    short flag;

    flag = OCIStmtFetch( DBSession->stmthp, DBSession->errhp, (ub4)1,
                   (ub4)OCI_FETCH_NEXT, (ub4) OCI_DEFAULT );
    return flag;
}

/***********************************************************
 * Function: 取一个SQL语句有多少列.
 * Parameter:
 *      [I]inSql               :SQL语句
 * Returns:
 *      列数，如果出错，则返回0
 **********************************************************/
int GetColNumFromSql(char *inSql)
{
    char tmpSql[1024];
    int colNum, flag,i,j;
    char *ptr;

    colNum=1;
    sprintf(tmpSql, "%s", inSql);
    ptr=strstr(UpCaseString(tmpSql), "FROM");
    if(ptr == NULL)
        return 0;

    j = strlen(inSql) - strlen(ptr);
    sprintf(ptr, "%s", inSql + j);
    flag = 0;
    for(i=0;i<j;i++)
    {
        if(inSql[i] == '(')
            flag++;
        else if (inSql[i] == ')')
            flag--;
        else if( (inSql[i] == ',')&&(flag == 0) )
            colNum++;
    }
    return colNum;
}

/***********************************************************
 * Function: 取表的首记录.
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [I]DBSession           :ORACLE事务指针
 *      [O]dataRec             :表数据指针
 *      [I]colNum              :列数
 *      [I]inSql               :SQL语句
 *      [I]vaRec               :绑定变量的指针
 *      [I]vaCount             :绑定变量的个数 0表示使用静态SQL
 * Returns:
 *      -100                   :没有数据
 *      0                      :成功
 *      -1                     :出错
 **********************************************************/
short GetRowFromTable(char *outStr, DB_SESSION *DBSession,
                      TABLE_RECORD *dataRec, int colNum, char *inSql,
                      TABLE_RECORD *vaRec, int vaCount)
{
    OCIDefine *defnp[500];
    OCIBind *bindHandle[500];    /* 最多支持50个参数) */

    int i;
    char tmpStr[1000], fieldName[50];

    /*分析SQL*/
    if( CheckErr( tmpStr, DBSession->errhp, OCIStmtPrepare( DBSession->stmthp,
                  DBSession->errhp, (text *)inSql, (ub4)strlen( (char *)inSql ),
                  (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT)) < 0 )
    {
        sprintf(outStr, "%s", tmpStr);
        return DB_FAILURE;
    }

    /*绑定变量*/
    for(i=0;i<vaCount;i++)
    {
        sprintf(fieldName, ":v%d", i+1);
        if( CheckErr( outStr, DBSession->errhp, OCIBindByName(DBSession->stmthp,
                 &bindHandle[i], DBSession->errhp, (text *) fieldName,
                 -1, (dvoid *) (vaRec+i)->field,
                 (sword) MAX_FIELD_LEN, SQLT_STR, (dvoid *) 0,
                 (ub2 *) 0, (ub2) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT)) < 0 )
            return DB_FAILURE;
    }

    /*取数据*/
    for(i=0;i<colNum;i++)
    {
        defnp[i] = (OCIDefine *) 0;
        memset((dataRec+i)->field, '\0', sizeof((dataRec+i)->field) );
        if( CheckErr( tmpStr, DBSession->errhp, OCIDefineByPos( DBSession->stmthp,
                  &defnp[i], DBSession->errhp,  i+1, (dvoid *) (dataRec+i)->field,
                  sizeof((dataRec+i)->field), SQLT_STR, (dvoid *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT) ) < 0 )
        {
            sprintf(outStr, "%s", tmpStr);
            return DB_FAILURE;
        }
    }

    if( CheckErr( tmpStr, DBSession->errhp, OCIStmtExecute( DBSession->svchp, DBSession->stmthp,
                  DBSession->errhp, (ub4)1, (ub4) 0, (CONST OCISnapshot *)NULL,
                  (OCISnapshot *)NULL, OCI_DEFAULT) ) < 0 )
    {
        sprintf(outStr, "%s", tmpStr);
        if(strcmp(tmpStr,"OCI_NO_DATA")==0)
            return DB_NO_DATA_FOUND;
        else
            return DB_FAILURE;
    }

    return DB_SUCCESS;
}

/***********************************************************
 * Function: 执行oracle语句.
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [I]DBSession           :ORACLE事务指针
 *      [I]inSql               :SQL语句
 * Returns:
 *      0                      :无错误
 *      -1                     :出错
 **********************************************************/
short ExecuteSql(char *outStr, DB_SESSION *DBSession, char *inSql)
{
    if( CheckErr( outStr, DBSession->errhp, OCIStmtPrepare( DBSession->stmthp,
                  DBSession->errhp, (text *)inSql, (ub4)strlen( (char *)inSql ),
                  (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT)) < 0 )
    {
        return DB_FAILURE;
    }

    if (CheckErr(outStr, DBSession->errhp, OCIStmtExecute(DBSession->svchp,
                 DBSession->stmthp, DBSession->errhp, (ub4) 1, (ub4) 0,
                 (OCISnapshot *) NULL, (OCISnapshot *) NULL, (ub4) OCI_DEFAULT))<0 )
    {
        return DB_FAILURE;
    }

    return OCI_SUCCESS;   /* 0 */
}

/***********************************************************
 * Function: 批量执行语句
 * Parameter:
 *      [O]outStr              :错误描述字符串
 *      [I]DBSession           :ORACLE事务指针
 *      [I]inSql               :SQL语句
 *      [IO]recData            :待入库数据
 *      [I]inFieldLen          :字段最大长度数
 *      [I]inRowNum            :实际需处理的数据行数
 *      [I]inColNum            :变量参数的个数（即列数+1)
 *      [I]inMaxBindNum        :最大处理的数据行数
 * Returns:
 *      0                      :无错误
 *      -1                     :出错
 **********************************************************/
short ExeMultiSql(char *outStr, DB_SESSION *DBSession, char *inSql,
                            char *recData, int inFieldLen,
                            int inRowNum, int inColNum, int inMaxBindNum)
{
    OCIBind *bindHandle[500];    /* 用来进行数组绑定(最多支持100列) */

    char sqlCmd[4000];
    char fieldName[20];
    int i,j;

    for (i = 0; i < inColNum + 1; i++)
        bindHandle[i] = (OCIBind *) 0;

    /* 拼SQL语句 */
    sprintf(fieldName, ":f%d", inColNum);
    memset(sqlCmd, '\0', sizeof(sqlCmd));
    j = 0;
    j += sprintf( sqlCmd + j, "BEGIN\n%s;\n", inSql );
    j += sprintf( sqlCmd + j, "%s := '0';\n", fieldName);  /*正常*/
    j += sprintf( sqlCmd + j, "EXCEPTION WHEN OTHERS THEN\n");
    j += sprintf( sqlCmd + j, "IF SQLCODE = -1 THEN\n");
    j += sprintf( sqlCmd + j, "%s := '1';\n", fieldName);  /*重单*/
    j += sprintf( sqlCmd + j, "ELSE\n");
    j += sprintf( sqlCmd + j, "%s := '2'||SUBSTRB(SQLERRM,1,%d);\n", fieldName, inFieldLen-2);/*错误*/
    j += sprintf( sqlCmd + j, "END IF;\n");
    j += sprintf( sqlCmd + j, "NULL;\n");
    j += sprintf( sqlCmd + j, "END;");

    if( CheckErr( outStr, DBSession->errhp, OCIStmtPrepare( DBSession->stmthp,
                  DBSession->errhp, (text *)sqlCmd, (ub4)strlen( (char *)sqlCmd ),
                  (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT)) < 0 )
        return DB_FAILURE;

    /*绑定名字************************************************************************/
    for(i=0;i<inColNum;i++)
    {
        sprintf(fieldName, ":f%d", i+1);
        if( CheckErr( outStr, DBSession->errhp, OCIBindByName(DBSession->stmthp,
                 &bindHandle[i], DBSession->errhp, (text *) fieldName,
                 -1, (dvoid *) (recData+i*inFieldLen),
                 (sword) inFieldLen, SQLT_STR, (dvoid *) 0,
                 (ub2 *) 0, (ub2) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT)) < 0 )
            return DB_FAILURE;
    }

    /*绑定数组************************************************************************/
    for(i=0;i<inColNum;i++)
    {
        if( CheckErr( outStr, DBSession->errhp, OCIBindArrayOfStruct(bindHandle[i],
            DBSession->errhp, sizeof(char)*inColNum*inFieldLen, 0, 0, 0)) < 0 )
            return DB_FAILURE;
    }

    /*********************************************************************************/
    if (CheckErr(outStr, DBSession->errhp, OCIStmtExecute(DBSession->svchp,
            DBSession->stmthp, DBSession->errhp, (ub4) inRowNum, (ub4) 0,
            (OCISnapshot *) NULL, (OCISnapshot *) NULL, (ub4) OCI_DEFAULT))<0 )
        return DB_FAILURE;

    return DB_SUCCESS;
}

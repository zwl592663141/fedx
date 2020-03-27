

/********************************************************************
    copyright (C), 1995-2004, Si-Tech Information Technology Ltd.
    file_name: common.c
    author: 吉胜学 version: （V1.00） date: （2009－05－20）
    description: 实现公用函数
    others:
    history: 1.date: 修改时间 version: 版本（V1.00）author: 作者
            modification: 修改内容
         2. ...
********************************************************************/



#include "common.h"


/**********************************************************
    function: MyLog
    description: 写日志信息
    Input: szSource: 信息来源
       szLevel: 信息级别
       szFmt: 信息格式
    Output:
    Return: MY_SUCCEED: 成功, 失败退出
    others:
***********************************************************/

int MyLog( char *szSource, char *szLevel, char *szFmt, ... )
{
    va_list vaList;
    char szBuf[MAX_RECORD_LEN + 1];
    char szTime[14 + 1];
    FILE *fp;
    /* 0-打印到终端;1-记录日志文件,*/
    int nToFile = ( szLogPath[0] == '\0' || szLogPrefix[0] == '\0' ) ? 0 : 1;
    memset( szTime, 0, sizeof( szTime ) );
    GetCurrTime( szTime );
    if( nToFile )
    {
        sprintf( szBuf, "%s/%s%8.8s.runlog", szLogPath, szLogPrefix, szTime );
        fp = fopen( szBuf, "a" );
        if( NULL == fp )
        {
            fprintf( stderr, "%4.4s/%2.2s/%2.2s %2.2s:%2.2s:%2.2s\t%s\t%s\t%s\n",
                     szTime,
                     szTime + 4,
                     szTime + 6,
                     szTime + 8,
                     szTime + 10,
                     szTime + 12,
                     "sys",
                     "exit",
                     "不能打开日志文件" );
            fclose( fp );
            exit(1);
        }
    }
    sprintf( szBuf, "%4.4s/%2.2s/%2.2s %2.2s:%2.2s:%2.2s\t%s\t%s\t",
             szTime,
             szTime + 4,
             szTime + 6,
             szTime + 8,
             szTime + 10,
             szTime + 12,
             szSource,
             szLevel );
    if( nToFile )
    {
        fprintf( fp, "%s", szBuf );
    }
    else
    {
        fprintf( stdout, "%s", szBuf );
    }
#ifdef __DEBUG__
    if( nToFile )
    {
        printf( szBuf );
    }
#endif
    va_start( vaList, szFmt );
    if( nToFile )
    {
        vfprintf( fp, szFmt, vaList );
    }
    else
    {
        vsprintf( szBuf, szFmt, vaList );
        printf( szBuf );
    }
#ifdef __DEBUG__
    if( nToFile )
    {
        printf( szBuf );
    }
#endif
    va_end( vaList );
    if( 0 == strcmp( szLevel, INFO_LEVEL_EXIT ) )
    {
        sprintf( szBuf, "%4.4s/%2.2s/%2.2s %2.2s:%2.2s:%2.2s\t%s\t%s\t%s\n",
                 szTime,
                 szTime + 4,
                 szTime + 6,
                 szTime + 8,
                 szTime + 10,
                 szTime + 12,
                 "sys",
                 "exit",
                 "发生异常情况, 程序停止运行, 退出!\n\n" );
        if( nToFile )
        {
            fprintf( fp, "%s", szBuf );
        }
        else
        {
            printf( szBuf );
        }
#ifdef __DEBUG__
        if( nToFile )
        {
            printf( szBuf );
        }
#endif
        fclose( fp );
        exit(1);
    }
    fclose( fp );
    return MY_SUCCEED;
}

/**********************************************************
    function: TrimLeft
    description: 去字符串左端空格
    Input: szTrim: 待处理字符串
    Output:
    Return: szTrim: 处理后的字符串
    others:
***********************************************************/

char *TrimLeft( char *szTrim )
{
    char *pLoc;
    pLoc = szTrim;
    while( isspace( (int)*pLoc ) ) { pLoc++; }
    strcpy( szTrim, pLoc );
    return szTrim;
}

/**********************************************************
    function: TrimRight
    description: 去字符串右端空格
    Input: szTrim: 待处理字符串
    Output:
    Return: szTrim: 处理后的字符串
    others:
***********************************************************/

char *TrimRight( char *szTrim )
{
    int nLen;
    nLen = strlen( szTrim );
    if ( nLen <= 0 )
    {
        return szTrim;
    }
    while( nLen > 0 && isspace( (int)szTrim[ nLen - 1 ] ) ) { nLen--; }
    szTrim[nLen] = '\0';
    return szTrim;
}

/**********************************************************
    function: TrimLZero
    description: 去字符串左端0字符
    Input: szTrim: 待处理字符串
    Output:
    Return: szTrim: 处理后的字符串
    others:
***********************************************************/

char *TrimLZero( char *szTrim )
{
    char *pLoc;
    pLoc = szTrim;
    while( (*pLoc) == '0' ) { pLoc++; }
    if( 0 == *(pLoc) || 0 == strlen( pLoc ) )
    {
        strcpy( szTrim, "0" );
    }
    else
    {
        strcpy( szTrim, pLoc );
    }
    return szTrim;
}

/**********************************************************
    function: GetIniInfo
    description: 获取配置文件某一字段取值
    Input: szFileName: 配置文件名
       szLValue: 字段名
       szRValue: 字段值
    Output:
    Return: MY_SUCCEED: 成功 MY_FAIL: 失败
    others:
***********************************************************/

int GetIniInfo( char *szFileName, char *szLValue, char *szRValue )
{
    FILE *fp;
    char szRec[MAX_RECORD_LEN + 1], szVal[MAX_RECORD_LEN + 1];
    char *szPos;
    fp = fopen( szFileName, "r" );
    if( NULL == fp )
    {
        fprintf( stderr, "Can't Open Ini File [%s]\n", szFileName );
        return MY_FAIL;
    }
    while( NULL != fgets( szRec, sizeof( szRec ), fp ) )
    {
        szPos = NULL;
        if( NULL != ( szPos = ( char * ) strchr( szRec, '=' ) ) )
        {
            memcpy( szVal, szRec, szPos - szRec );
            szVal[szPos - szRec] = 0;
            TrimLeft( TrimRight( szVal ) );
            if( strcmp( szVal, szLValue ) == 0 )
            {
                strcpy( szRValue, szPos + 1 );
                TrimLeft( TrimRight( szRValue ) );
                fclose( fp );
                return MY_SUCCEED;
            }
            else
            {
                continue;
            }
        }
    }
    fclose( fp );
    return MY_FAIL;
}

/**********************************************************
    function: IsFloat
    description: 判断字符串值是否为浮点数
    Input: szNum: 字符串
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 不是
    others:
***********************************************************/

int IsFloat( char *szNum )
{
    int nFlag;
    nFlag = 0;
    if ( 0 == *(szNum) || 0 == strlen( szNum ) )
    {
        return MY_FALSE;
    }
    while ( *szNum )
    {
        if ( !isdigit( (int)(*szNum) ) )
        {
            if( '.' != (*szNum) || nFlag || 0 == *(szNum + 1) )
            {
                return MY_FALSE;
            }
            nFlag++;
        }
        szNum++;
    }
    return MY_TRUE;
}

/**********************************************************
    function: IsNum
    description: 判断字符串值是否为数值
    Input: szNum: 字符串
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 不是
    others:
***********************************************************/

int IsNum( char *szNum )
{
    if ( 0 == *(szNum) || 0 == strlen( szNum ) )
    {
        return MY_FALSE;
    }
    while ( *szNum )
    {
        if ( isdigit( (int)(*szNum) ) )
        {
            szNum++;
        }
        else
        {
            return MY_FALSE;
        }
    }
    return MY_TRUE;
}

/**********************************************************
    function: IsStr
    description: 判断字符串值是否合法
    Input: szNum: 字符串
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 不是
    others:
***********************************************************/

int IsStr( char *szStr )
{
    int nOffset;
    nOffset = 0;
    if( 0 == strlen( szStr ) || 0 == *(szStr) )
    {
        return MY_FALSE;
    }
    while( nOffset < strlen( szStr ) )
    {
        if( ( szStr[nOffset] >= 'a' &&  szStr[nOffset] <= 'z' )
                || ( szStr[nOffset] >= 'A' &&  szStr[nOffset] <= 'Z' )
                || ( szStr[nOffset] >= '0' &&  szStr[nOffset] <= '9' )
                || '_' == szStr[nOffset]
          )
        {
            nOffset++;
        }
        else
        {
            return MY_FALSE;
        }
    }
    return MY_TRUE;
}

/**********************************************************
    function: IsLeapYear
    description: 判断是否为闰年
    Input: nYear: 年
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 不是
    others:
***********************************************************/

int IsLeapYear( int nYear )
{
    if( ( nYear % 4 == 0 && nYear % 100 != 0 ) || nYear % 400 == 0 )
    {
        return MY_TRUE;
    }
    else
    {
        return MY_FALSE;
    }
}

/**********************************************************
    function: GetFileCount
    description: 获取文件记录数
    Input: szPath: 文件路径
    Output:
    Return: lCount: 记录数
    others:
***********************************************************/

long GetFileCount( char *szPath )
{
    FILE *fp;
    int lCount;
    char szBuf[MAX_RECORD_LEN + 1];
    fp = fopen( szPath, "r" );
    if( NULL == fp )
    {
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "文件 %s 打开失败!\n", szPath );
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "ErrorInfo: %d : %s\n", errno, strerror( errno ) );
        return -1;
    }
    lCount = 0;
    while( NULL != fgets( szBuf, sizeof( szBuf ), fp ) )
    {
        if( 0 != strncmp( szBuf, FILE_NOTE_FLAG, strlen( FILE_NOTE_FLAG ) ) )
        {
            lCount++;
        }
    }
    fclose( fp );
    return lCount;
}

/**********************************************************
    function: GetCurrTime
    description: 获取当前系统时间
    Input: szStr: 保存时间的字符串
    Output:
    Return: MY_SUCCEED: 成功
    others:
***********************************************************/

int GetCurrTime( char *szStr )
{
    time_t tTime;
    struct tm *tmTime;
    struct timeval mTime;
    time(&tTime);
    tmTime = localtime(&tTime);
    gettimeofday(&mTime, NULL);
    sprintf(szStr, "%04d%02d%02d%02d%02d%02d",
            tmTime->tm_year + 1900, tmTime->tm_mon + 1,
            tmTime->tm_mday, tmTime->tm_hour,
            tmTime->tm_min, tmTime->tm_sec
           );
    return MY_SUCCEED;
}

/**********************************************************
    function: GetLastDay
    description: 获取参照时间的上一天的时间
    Input: szLastTime: 上一天
       szCurrTime: 参照时间
    Output:
    Return: MY_SUCCEED: 成功
    others:
***********************************************************/

int GetLastDay( char *szLastTime, char *szCurrTime )
{
    time_t tTime;
    struct tm *tmTime;
    struct tm otmTime;
    char szBuf[128];
    char szTime[14 + 1];
    int nLen;
    int nYear, nMon, nDay, nHour, nMin, nSec;
    memset( &otmTime, 0, sizeof( struct tm ) );
    nLen = strlen( szCurrTime );
    if( nLen > 6 )
    {
        memcpy( szBuf, szCurrTime + 6, 2 );
        szBuf[2] = 0;
        otmTime.tm_mday = atoi( szBuf );
    }
    else
    {
        otmTime.tm_mday = 1;
    }
    if( nLen > 4 )
    {
        memcpy( szBuf, szCurrTime + 4, 2 );
        szBuf[2] = 0;
        otmTime.tm_mon = atoi( szBuf ) - 1;
    }
    memcpy( szBuf, szCurrTime, 4 );
    szBuf[4] = 0;
    otmTime.tm_year = atoi( szBuf ) - 1900;
    tTime =  mktime( &otmTime );
    tTime -= 86400;
    tmTime = localtime( &tTime );
    sprintf( szTime, "%04d%02d%02d%02d%02d%02d",
             tmTime->tm_year + 1900,
             tmTime->tm_mon + 1,
             tmTime->tm_mday,
             tmTime->tm_hour,
             tmTime->tm_min,
             tmTime->tm_sec);
    strcpy( szLastTime, szTime );
    return MY_SUCCEED;
}

/**********************************************************
    function: GetNextDay
    description: 获取参照时间的下一天的时间
    Input: szNextDay: 下一天
       szCurrTime: 参照时间
    Output:
    Return: MY_SUCCEED: 成功
    others:
***********************************************************/

int GetNextDay( char *szNextDay, char *szCurrDate )
{
    time_t tTime;
    struct tm *tmTime;
    struct tm otmTime;
    char szBuf[14 + 1];
    /*
        otmTime.tm_sec  = 0;
        otmTime.tm_min  = 0;
        otmTime.tm_hour = 1;
    */
    memset( &otmTime, 0, sizeof( struct tm ) );
    strncpy( szBuf, szCurrDate + 6, 2 );
    szBuf[2] = 0;
    otmTime.tm_mday = atoi( szBuf );
    strncpy( szBuf, szCurrDate + 4, 2 );
    szBuf[2] = 0;
    otmTime.tm_mon = atoi( szBuf ) - 1;
    strncpy( szBuf, szCurrDate, 4 );
    szBuf[4] = 0;
    otmTime.tm_year = atoi( szBuf ) - 1900;
    tTime =  mktime( &otmTime );
    tTime += 86400;
    tmTime = localtime( &tTime );
    sprintf( szNextDay, "%04d%02d%02d",
             tmTime->tm_year + 1900,
             tmTime->tm_mon + 1,
             tmTime->tm_mday );
    return MY_SUCCEED;
}

/**********************************************************
    function: GetNextMonth
    description: 获取参照时间的下一天的时间
    Input: szNextMonth: 下一天
       szCurrTime: 参照时间
    Output:
    Return: MY_SUCCEED: 成功
    others:
***********************************************************/

int GetNextMonth( char *szNextMonth, char *szCurrDate )
{
    int nYear, nMon, nLastDay;
    char szYear[DATE_YEAR_LEN + 1];
    char szMonth[DATE_MON_LEN + 1];
    strncpy( szYear, szCurrDate, DATE_YEAR_LEN );
    szYear[DATE_YEAR_LEN] = 0;
    nYear = atoi( szYear );
    strncpy( szMonth, szCurrDate + DATE_YEAR_LEN, DATE_MON_LEN );
    szMonth[DATE_MON_LEN] = 0;
    nMon = atoi( szMonth );
    nMon++;
    if( nMon > 12 )
    {
        nMon = 1;
        nYear++;
    }
    sprintf( szNextMonth, "%04d%02d", nYear, nMon );
    return MY_SUCCEED;
}

/**********************************************************
    function: GetMonthLastDay
    description: 获取某一月的最后一天
    Input: szLastDay: 最后一天
       szTime: 参照时间
    Output:
    Return: MY_SUCCEED: 成功
    others:
***********************************************************/

int GetMonthLastDay( char *szLastDay, char *szTime )
{
    int nYear, nMon, nLastDay;
    char szYear[DATE_YEAR_LEN + 1];
    char szMonth[DATE_MON_LEN + 1];
    strncpy( szYear, szTime, DATE_YEAR_LEN );
    szYear[DATE_YEAR_LEN] = 0;
    nYear = atoi( szYear );
    strncpy( szMonth, szTime + DATE_YEAR_LEN, DATE_MON_LEN );
    szMonth[DATE_MON_LEN] = 0;
    nMon = atoi( szMonth );
    if( nMon == 4 || nMon == 6 || nMon == 9 || nMon == 11 )
    {
        nLastDay = 30;
    }
    else if( nMon == 2 )
    {
        if( IsLeapYear( nYear ) )
        {
            nLastDay = 29;
        }
        else
        {
            nLastDay = 28;
        }
    }
    else
    {
        nLastDay = 31;
    }
    sprintf( szLastDay, "%04d%02d%02d", nYear, nMon, nLastDay );
    return MY_SUCCEED;
}

/**********************************************************
    function: FileIsExist
    description: 判断文件是否存在
    Input: szFile: 文件名
    Output:
    Return: MY_TRUE: 存在 MY_FALSE: 不存在
    others:
***********************************************************/

int FileIsExist( char *szFile )
{
    if( 0 == access( szFile, F_OK ) )
    {
        return MY_TRUE;
    }
    else
    {
        return MY_FALSE;
    }
}

/**********************************************************
    function: GetFileSize
    description: 获取文件的大小
    Input: szFile: 文件名
    Output:
    Return: 文件大小
    others:
***********************************************************/

long GetFileSize( char *szFile )
{
    struct stat statBuf;
    statBuf.st_size = -1;
    lstat( szFile, &statBuf );
    return ( statBuf.st_size );
}

/**********************************************************
    function: IsDir
    description: 判断文件是否为目录文件
    Input: szDirName: 文件名
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 不是
    others:
***********************************************************/

int IsDir( char *szDirName )
{
    struct stat statMode;
    if ( (lstat( szDirName, &statMode ) == 0) && (statMode.st_mode & S_IFDIR) )
    {
        return (MY_TRUE);
    }
    else
    {
        return (MY_FALSE);
    }
}

/**********************************************************
    function: ChkLogPath
    description: 检验日志文件路径有效性
    Input: szPath: 日志文件目录
    Output:
    Return: MY_SUCCEED: 有效 MY_FAIL: 无效
    others:
***********************************************************/

int ChkLogPath( char *szPath )
{
    FILE *fp;
    char szBuf[MAX_RECORD_LEN + 1];
    char szTime[14 + 1];
    if( !IsDir( szPath ) )
    {
        return MY_FAIL;
    }
    memset( szTime, 0, sizeof( szTime ) );
    GetCurrTime( szTime );
    sprintf( szBuf, "%s/%s%8.8s.%s.runlog", szLogPrefix, szLogPath, szTime, szSrcDir );
    fp = fopen( szBuf, "a" );
    if( NULL == fp )
    {
        return MY_FAIL;
    }
    fclose( fp );
    return MY_SUCCEED;
}

/**********************************************************
    function: BackGround
    description: 程序转入后台运行
    Input:
    Output:
    Return: SUCCEED: 成功, 失败则程序退出
    others:
***********************************************************/

int BackGround( void )
{
    pid_t pid;
    pid = fork();
    if ( 0 > pid )
    {
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_EXIT, "程序转入后台失败\n\n" );
        exit( 1 );
    }
    else if ( pid > 0 )
    {
        exit( 0 );
    }
    else
    {
        setsid();
        chdir( "/" );
        umask( 0 );
    }
    return MY_SUCCEED;
}

/**********************************************************
    function: IsDate
    description: 判断是否为标准日期
    Input: szDate: 日期
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 不是
    others:
***********************************************************/

int IsDate( char *szDate )
{
    char szBuf[128];
    int nYear, nMon, nDay, nHour, nMin, nSec;
    int nLen;
    if( !IsNum( szDate ) )
    {
        return MY_FALSE;
    }
    nLen = strlen( szDate );
    if( DATE_STD_LEN != nLen )
    {
        return MY_FALSE;
    }
    memcpy( szBuf, szDate     , 4 );
    szBuf[4] = 0;
    nYear = atoi( szBuf );
    memcpy( szBuf, szDate +  4, 2 );
    szBuf[2] = 0;
    nMon  = atoi( szBuf );
    memcpy( szBuf, szDate +  6, 2 );
    szBuf[2] = 0;
    nDay  = atoi( szBuf );
    memcpy( szBuf, szDate +  8, 2 );
    szBuf[2] = 0;
    nHour = atoi( szBuf );
    memcpy( szBuf, szDate + 10, 2 );
    szBuf[2] = 0;
    nMin  = atoi( szBuf );
    memcpy( szBuf, szDate + 12, 2 );
    szBuf[2] = 0;
    nSec  = atoi( szBuf );
    if( nMon > 12 || nDay > 31 || nHour > 23 || nMin > 59 || nSec > 59 )
    {
        return MY_FALSE;
    }
    if( ( nMon == 4 || nMon == 6 || nMon == 9 || nMon == 11 ) && nDay > 30 )
    {
        return MY_FALSE;
    }
    if( nMon == 2 )
    {
        if( IsLeapYear( nYear ) )
        {
            return (nDay > 29) ? MY_FALSE : MY_TRUE;
        }
        else
        {
            return (nDay > 28) ? MY_FALSE : MY_TRUE;
        }
    }
    return MY_TRUE;
}

/**********************************************************
    function: IsDate
    description: 判断是否为精确到小时的日期
    Input: szDate: 日期
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 不是
    others:
***********************************************************/

int IsHourDate( char *szDate )
{
    char szBuf[128];
    int nYear, nMon, nDay, nHour, nMin, nSec;
    int nLen;
    if( !IsNum( szDate ) )
    {
        return MY_FALSE;
    }
    nLen = strlen( szDate );
    if( ( DATE_YEAR_LEN + DATE_MON_LEN + DATE_DAY_LEN + DATE_HOUR_LEN ) != nLen )
    {
        return MY_FALSE;
    }
    memcpy( szBuf, szDate     , 4 );
    szBuf[4] = 0;
    nYear = atoi( szBuf );
    memcpy( szBuf, szDate +  4, 2 );
    szBuf[2] = 0;
    nMon  = atoi( szBuf );
    memcpy( szBuf, szDate +  6, 2 );
    szBuf[2] = 0;
    nDay  = atoi( szBuf );
    memcpy( szBuf, szDate +  8, 2 );
    szBuf[2] = 0;
    nHour = atoi( szBuf );
    if( nMon > 12 || nDay > 31 || nHour > 23 )
    {
        return MY_FALSE;
    }
    if( ( nMon == 4 || nMon == 6 || nMon == 9 || nMon == 11 ) && nDay > 30 )
    {
        return MY_FALSE;
    }
    if( nMon == 2 )
    {
        if( IsLeapYear( nYear ) )
        {
            return (nDay > 29) ? MY_FALSE : MY_TRUE;
        }
        else
        {
            return (nDay > 28) ? MY_FALSE : MY_TRUE;
        }
    }
    return MY_TRUE;
}

/**********************************************************
    function: IsDate
    description: 判断是否为精确到日的日期
    Input: szDate: 日期
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 不是
    others:
***********************************************************/

int IsDayDate( char *szDayDate )
{
    char szBuf[128];
    int nYear, nMon, nDay;
    int nLen;
    if( !IsNum( szDayDate ) )
    {
        printf("%d\n",__LINE__);
        return MY_FALSE;
    }
    nLen = strlen( szDayDate );
    if( ( DATE_YEAR_LEN + DATE_MON_LEN + DATE_DAY_LEN ) != nLen )
    {
        printf("%d\n",__LINE__);
        return MY_FALSE;
    }
    memcpy( szBuf, szDayDate     , 4 );
    szBuf[4] = 0;
    nYear = atoi( szBuf );
    memcpy( szBuf, szDayDate +  4, 2 );
    szBuf[2] = 0;
    nMon  = atoi( szBuf );
    memcpy( szBuf, szDayDate +  6, 2 );
    szBuf[2] = 0;
    nDay  = atoi( szBuf );
    if( nMon > 12 || nDay > 31 )
    {
        printf("%d\n",__LINE__);
        return MY_FALSE;
    }
    if( ( nMon == 4 || nMon == 6 || nMon == 9 || nMon == 11 ) && nDay > 30 )
    {
        printf("%d\n",__LINE__);
        return MY_FALSE;
    }
    if( nMon == 2 )
    {
        if( IsLeapYear( nYear ) )
        {
            return (nDay > 29) ? MY_FALSE : MY_TRUE;
        }
        else
        {
            return (nDay > 28) ? MY_FALSE : MY_TRUE;
        }
    }
    return MY_TRUE;
}

/**********************************************************
    function: IsDate
    description: 判断是否为精确到月的日期
    Input: szDate: 日期
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 不是
    others:
***********************************************************/

int IsMonthDate( char *IsMonthDate )
{
    char szBuf[128];
    int nYear, nMon;
    int nLen;
    if( !IsNum( IsMonthDate ) )
    {
        return MY_FALSE;
    }
    nLen = strlen( IsMonthDate );
    if( ( DATE_YEAR_LEN + DATE_MON_LEN ) != nLen )
    {
        return MY_FALSE;
    }
    memcpy( szBuf, IsMonthDate     , 4 );
    szBuf[4] = 0;
    nYear = atoi( szBuf );
    memcpy( szBuf, IsMonthDate +  4, 2 );
    szBuf[2] = 0;
    nMon  = atoi( szBuf );
    if( nMon > 12 )
    {
        return MY_FALSE;
    }
    return MY_TRUE;
}

/**********************************************************
    function: GetInDirName
    description: 获取路径的 BaseName
    Input: szFullDir: 路径
    Output: szDirName: BaseName
    Return: MY_SUCCEED: 成功 MY_FAIL: 失败
    others:
***********************************************************/

int GetInDirName( char *szDirName, char *szFullDir )
{
    char szDir[MAX_PATH_LEN + 1];
    int nLen;
    strcpy( szDir, szFullDir );
    nLen = strlen( szDir );
    while( nLen )
    {
        if( '/' == szDir[nLen - 1] )
        {
            szDir[nLen - 1] = 0;
        }
        else
        {
            break;
        }
        nLen--;
    }
    strcpy( szDirName, strrchr( szDir, '/' ) + 1 );
    if( 0 == *( szDirName ) )
    {
        strcpy( szDirName, szFullDir );
    }
    return MY_SUCCEED ;
}

/**********************************************************
    function: GetLoginInfo
    description: 获取登陆口令文件中的用户名、密码
    Input: szLoginFile: 登陆口令文件
    Output: szUser: 用户名
        szPwD: 密码
    Return: MY_SUCCEED: 成功 MY_FAIL: 失败
    others:
***********************************************************/

int GetLoginInfo( char *szLoginFile, char *szUser, char *szPwd )
{
    FILE *fp;
    char szKey[MAX_RECORD_LEN + 1];
    char szStr[MAX_RECORD_LEN + 1];
    int i;
    if( NULL == ( fp = fopen( szLoginFile, "r" ) ) )
    {
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "口令文件 %s 打开失败!\n", szLoginFile );
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "ErrorInfo: %d : %s\n", errno, strerror( errno ) );
        return MY_FAIL;
    }
    strcpy( szKey, "DFJLT" );
    fgets( szUser, MAX_RECORD_LEN, fp );
    fgets( szUser, MAX_RECORD_LEN, fp );
    fgets( szPwd,  MAX_RECORD_LEN, fp );
    fgets( szPwd,  MAX_RECORD_LEN, fp );
    szUser[strlen(szUser) - 1] = 0;
    szPwd[strlen(szPwd) - 1] = 0;
    fclose( fp );
    strcpy( szStr, szUser );
    for( i = 0; i < strlen( szStr ); i++ )
    {
        szStr[i] = szStr[i] ^ szKey[i%strlen(szKey)];
    }
    strcpy( szUser, szStr );
    strcpy( szStr, szPwd );
    for( i = 0; i < strlen( szStr ); i++ )
    {
        szStr[i] = szStr[i] ^ szKey[i%strlen(szKey)];
    }
    strcpy( szPwd, szStr );
    return MY_SUCCEED;
}

/**********************************************************
    function: MoveFile
    description: 移动文件
    Input: szSrcFile: 源文件
       szDestFile: 目标文件
    Output:
    Return: MY_SUCCEED: 成功 MY_FAIL: 失败
    others:
***********************************************************/


int MoveFile( char *szSrcFile, char *szDstFile )
{
    if( -1 == rename( szSrcFile, szDstFile ) )
    {
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "移动文件 %s 至 %s 失败\n", szSrcFile, szDstFile );
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "ErrorInfo: %d : %s\n", errno, strerror( errno ) );
        return MY_FAIL;
    }
    return MY_SUCCEED;
}

/**********************************************************
    function: DeleteFile
    description: 移动文件
    Input: szSrcFile: 源文件

    Output:
    Return: MY_SUCCEED: 成功 MY_FAIL: 失败
    others:
***********************************************************/


int DeleteFile( char *szSrcFile )
{
    if( -1 == remove( szSrcFile) )
    {
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "删除文件 %s失败\n", szSrcFile );
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "ErrorInfo: %d : %s\n", errno, strerror( errno ) );
        return MY_FAIL;
    }
    return MY_SUCCEED;
}

/**********************************************************
    function: IsSameFileSystem
    description: 判断两个目录是否在同一个文件系统
    Input: szDir1: 目录1
       szDir2: 目录2
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 否
    others:
***********************************************************/

int IsSameFileSystem( char *szDir1, char *szDir2 )
{
    struct stat statMode1;
    struct stat statMode2;
    if( 0 != lstat( szDir1, &statMode1 ) || 0 != lstat( szDir2, &statMode2 ) )
    {
        return MY_FALSE;
    }
    if( statMode1.st_dev != statMode2.st_dev )
    {
        return MY_FALSE;
    }
    return MY_TRUE;
}

/**********************************************************
    function: IsRecordLine
    description: 判断记录是否为有效记录
    Input: szRecord: 记录内容
    Output:
    Return: MY_TRUE: 是 MY_FALSE: 否
    others:
***********************************************************/

int IsRecordLine( char *szRecord )
{
    if( 0 == strncmp( szRecord, FILE_NOTE_FLAG, strlen( FILE_NOTE_FLAG ) ) )
    {
        return MY_FALSE;;
    }
    TrimLeft( TrimRight( szRecord ) );
    if( 0 == *( szRecord ) )
    {
        return MY_FALSE;;
    }
    return MY_TRUE;
}

/**********************************************************
    function: GetTimeData
    description: 分解时间字符串各子项, 保存在指定的时间结构体中
    Input: szTime: 时间字符串
       ptData; 时间结构体
    Output:
    Return: MY_SUCCEED: 成功
    others:
***********************************************************/

int GetTimeData( char *szTime, PTIME_DATA ptData )
{
    strncpy( ptData->szYear, szTime     , DATE_YEAR_LEN );
    ptData->szYear[DATE_YEAR_LEN] = 0;
    strncpy( ptData->szMon , szTime +  4, DATE_MON_LEN  );
    ptData->szMon[DATE_MON_LEN]   = 0;
    strncpy( ptData->szDay , szTime +  6, DATE_DAY_LEN  );
    ptData->szDay[DATE_DAY_LEN]   = 0;
    strncpy( ptData->szHour, szTime +  8, DATE_HOUR_LEN );
    ptData->szHour[DATE_HOUR_LEN] = 0;
    strncpy( ptData->szMin , szTime + 10, DATE_MIN_LEN  );
    ptData->szMin[DATE_MIN_LEN]   = 0;
    strncpy( ptData->szSec , szTime + 12, DATE_SEC_LEN  );
    ptData->szSec[DATE_SEC_LEN]   = 0;
    return MY_SUCCEED;
}
char *BaseName(char *src)
{
#ifdef _DEBUG_
    printf("[%s][%d] BaseName=[%s]\n",__FILE__,__LINE__,src);
#endif
    return src;
}


void *GetFilename(const char *fullpathname)
{
    char *save_name, *pos;
    int name_len;
    name_len = strlen(fullpathname);
    pos = fullpathname + name_len;
    while(*pos != '\\' && pos != fullpathname)
    {
        pos --;
    }
    if(pos == fullpathname)
    {
        save_name = fullpathname+1;
        return save_name;
    }
    name_len = name_len-(pos-fullpathname);
    save_name = (char *) malloc(name_len+1);
    memcpy(save_name,pos+1,name_len);
    return save_name;
}


/***********************************************************
    Function: 替换字符串.
    Parameter:
        [IO]inStr              :替换前的输入字符串
        [I]oldStr              :需要替换的子串
        [I]newStr              :替换的新子串
    Revurns:
        none
 **********************************************************/
void strRep( char *inStr, char *oldStr, char *newStr )
{
    int len, i=0, j=0;
    char tmpStr[4000]= {0};
    len = strlen(oldStr);
    while( inStr[i]!='\0' )
    {
        if(strncmp(inStr+i, oldStr, len)==0)
        {
            j = j + sprintf(tmpStr+j, "%s", newStr);
            i = i + len;
        }
        else
        {
            tmpStr[j++] = inStr[i];
            i++;
        }
    }
    tmpStr[j] = '\0';
    sprintf(inStr, "%s", tmpStr);
    return;
}

/***********************************************************
    Function: 替换字符串.
    Parameter:
        [IO]inStr              :替换前的输入字符串
        [I]oldStr              :需要替换的子串
        [I]newStr              :替换的新子串
    Revurns:
        none
 **********************************************************/
long datetimetosec(const char *timeStr)
{
    long tmp_sec = -1;
    char tmp[16];
    struct tm time_tm;
    if(strlen(timeStr) != 14)
    { return -1; }
    memcpy(tmp, timeStr, 4);
    tmp[4] = 0;
    time_tm.tm_year = atoi(tmp) - 1900;
    if(time_tm.tm_year<0)
    { return -1; }
    memcpy(tmp, timeStr + 4, 2);
    tmp[2] = 0;
    time_tm.tm_mon = atoi(tmp) - 1;
    memcpy(tmp, timeStr + 6, 2);
    tmp[2] = 0;
    time_tm.tm_mday = atoi(tmp);
    memcpy(tmp, timeStr + 8, 2);
    tmp[2] = 0;
    time_tm.tm_hour = atoi(tmp);
    memcpy(tmp, timeStr + 10, 2);
    tmp[2] = 0;
    time_tm.tm_min = atoi(tmp);
    memcpy(tmp, timeStr + 12, 2);
    tmp[2] = 0;
    time_tm.tm_sec = atoi(tmp);
    tmp_sec = mktime(&time_tm);
    return tmp_sec;
}


/***********************************************************
    Function: 替换字符串.
    Parameter:
        [IO]inStr              :替换前的输入字符串
        [I]oldStr              :需要替换的子串
        [I]newStr              :替换的新子串
    Revurns:
        none
 **********************************************************/
char *TimeToChar( time_t tt, char *szStr )
{
    struct tm *tmTime;
    tmTime = localtime( &tt );
    memset( szStr, 0, sizeof(szStr) );
    sprintf(szStr, "%04d%02d%02d%02d%02d%02d",
            tmTime->tm_year + 1900, tmTime->tm_mon + 1,
            tmTime->tm_mday, tmTime->tm_hour,
            tmTime->tm_min, tmTime->tm_sec
           );
    return szStr;
}

/***********************************************************
    Function: 替换字符串.
    Parameter:
        [IO]inStr              :替换前的输入字符串
        [I]oldStr              :需要替换的子串
        [I]newStr              :替换的新子串
    Revurns:
        none
 **********************************************************/
int GetConfigValue( char *szValue, char *szBeginGroup, char *szEndGroup, char *szKeyStr, FILE *fp )
{
    char szTmpBuf[2048],*p,szKey[30+1];
    memset( szTmpBuf, 0, sizeof(szTmpBuf) ) ;
    while( fgets(szTmpBuf, sizeof(szTmpBuf), fp) )
    {
        szTmpBuf[strlen(szTmpBuf)-1] = 0 ;
        TrimLeft( TrimRight( szTmpBuf ) );
        if( ! IsRecordLine(szTmpBuf) )
        { continue; }
        if( strcmp(szTmpBuf, szBeginGroup ) != 0 )
        { continue; }
        memset( szTmpBuf, 0, sizeof(szTmpBuf) ) ;
        while( fgets(szTmpBuf, sizeof(szTmpBuf), fp) )
        {
            szTmpBuf[strlen(szTmpBuf)-1] = 0 ;
            TrimLeft( TrimRight( szTmpBuf ) ) ;
            if( strncmp(szTmpBuf, szEndGroup,strlen(szEndGroup)) == 0 )
            { return -1; }
            if( ! IsRecordLine(szTmpBuf) )
            { continue; }
            if( NULL == (p = strchr( szTmpBuf, '=' )) )
            { continue; }
            memset( szKey, 0, sizeof(szKey) );
            memset( szValue, 0, sizeof(szValue) );
            strncpy( szKey, szTmpBuf, strlen(szTmpBuf) - strlen(p) );
            strcpy( szValue, p+1 );
            TrimLeft( TrimRight( szKey ) );
            TrimLeft( TrimRight( szValue ) );
            if( strcmp( szKey, szKeyStr ) != 0 )
            { continue; }
            if( NULL != (p = strchr( szValue, '#' )) )
            {
                *p = 0;
            }
            return 0;
        }
    }
    return -1;
}

/******************************************************************
    Function:       int MatchField(const char* src_str,const char* regex_field, const char split_flag)
    Description:    简易正则表达式域的规则匹配函数
    Input:          const char* src_str 需要检测的字符串
                const char* regex_field  多个简易正则表达式的组合，支持!、*
                const char split_flag，多个简易正则表达式的分割字符
    Return:         int  1表示匹配，0表示不匹配
    Others:         配置表简易正则表达式域, regex_field的最大长度为MAX_REGEX_LEN -1
********************************************************************/
int MatchField(const char *src_str,const char *regex_field, const char split_flag)
{
    char tmp_regex[MAX_REGEX_LEN], sp_flag, sp_tmp[MAX_REGEX_LEN];
    int  res_flag = 1;
    if(strlen(src_str) < 1)
    { return 0; }
    if(strlen(regex_field) > MAX_REGEX_LEN - 1)
    { return 0; }
    StrTrim(tmp_regex, regex_field);
    /*第一个字符为'*'，直接匹配*/
    if(tmp_regex[0]=='*')
    { return 1; }
    if(tmp_regex[0]=='!')
    {
        res_flag = 0;
        /*删除'!'*/
        CharDelete(tmp_regex, '!');
    }
    /*如果分割标志没有指定，默认为','*/
    //sp_flag=(split_flag==0?',':split_flag);
    sp_flag=',';
    while(tmp_regex[0] != 0)
    {
        if(StrSplit(sp_tmp, tmp_regex, sp_flag) < 0)
        {
            return 0;
        }
        if(MatchRule(src_str, sp_tmp))
        {
            return res_flag;
        }
    }
    return !res_flag;
}

/******************************************************************
    Function:       int StrSplit(char* split_str, char* src_str, const char split_char)
    Description:    将字符串str_str从split_char的地方分割成两部分，第一部分放在split_str，
                第二部分放回src_str中。
    Input:          const char* src_str 需要分割的字符串
                const char* split_str  分割后的第一部分
                const char split_char，分割字符
    Return:         int  -1表示失败，0表示成功
    Others:         如果src_str不包含split_char，则把整个src_str放到split_str。
                src_str的最大长度为MAX_REGEX_LEN - 1
********************************************************************/
int StrSplit(char *split_str, char *src_str, const char split_char)
{
    int slen = 0, pos;
    char tmp[MAX_REGEX_LEN];
    if((slen = strlen(src_str)) < 1 || slen > MAX_REGEX_LEN - 1)
    { return -1; }
    if(split_char == 0)
    { return -1; }
    /*去掉结尾的split_char*/
    if(src_str[slen-1] == split_char)
    {
        src_str[slen-1] = 0;
    }
    pos = CharPos(src_str, split_char);
    if(pos < 0)
    {
        sprintf(split_str, "%s", src_str);
        src_str[0] = 0;
        return 0;
    }
    if(pos > 0)
    {
        sprintf(tmp, "%s", src_str);
        snprintf(split_str, pos + 1, "%s", tmp);
    }
    else
    { 
    split_str[0] = 0; 
	}
    sprintf(src_str, "%s", tmp + pos + 1);
    return 0;
}
/******************************************************************
    Function:       int CharPos（const char* source,const char target）
    Description:    找出字符在字符串中首次出现的位置
    Input:          const char* source  需要进行搜索的字符串
                const char target   需要搜索的字符
    Return:         int   字符出现的位置，返回-1表示没有找到
    Others:
********************************************************************/
int CharPos(const char *source,const char target)
{
    int len,i;
    if (source==NULL||!(len=strlen(source)))
    { return -1; }
    for(i=0; i<len; i++)
    {
        if(source[i]==target)
        { return i; }
    }
    return -1;
}


/******************************************************************
    Function:       int IsNumAlpha（const char str）
    Description:    判断字符是否是数字和字母
    Input:          const char str  需要进行检测的字符
    Return:         int 非0表示不成立 0表示成立
    Others:
********************************************************************/
int IsNumAlpha(const char str)
{
    if(!isdigit((int)str)&&!isalpha((int)str))
    { return 1; }
    return 0;
}
/******************************************************************
    Function:       int CharDelete(char* fstr, const char target)
    Description:    把字符串fstr中的target字符删除掉
    Input:          const char* fstr  需要进行删除的字符串
                const char sstr   需要删除的字符
    Return:         int   非0表示字符串太长或太短， 0表示成功
    Others:
********************************************************************/
int CharDelete(char *fstr, const char target)
{
    int len = strlen(fstr);
    char tmp[256];
    int i,k=0;
    /*
        if(target==NULL)
    */
    if(target==0)
    { return 0; }
    if(len<1||len>255)
    { return 1; }
    memset(tmp, 0, sizeof(tmp));
    for(i=0; i<len; i++)
    {
        if(fstr[i]!=target)
        { tmp[i-k]=fstr[i]; }
        else
        { k++; }
    }
    sprintf(fstr, "%s", tmp);
    return 0;
}

/******************************************************************
    Function:       int CharCount(const char* str, const char ch)
    Description:    统计字符ch在字符串src中的数量
    Input:          const char* str  需要进行字符统计的字符串
                const char  ch   需要统计数量的字符
    Return:         int  统计的结果
    Others:
********************************************************************/
int CharCount(const char *str, const char ch)
{
    int co = 0,i;
    int len = strlen(str);
    if(len<1)
    { return 0; }
    for(i=0; i<len; i++)
    {
        if(str[i]==ch)
        { co++; }
    }
    return co;
}
/******************************************************************
    Function:       int StrPos（const char* fstr, const char* sstr）
    Description:    找出字符串sstr在字符串fstr中首次出现的位置
    Input:          const char* fstr  需要进行搜索的字符串
                const char* sstr  需要搜索的字符串
    Return:         int   字符串出现的位置，返回-1表示没有找到
    Others:
********************************************************************/
int StrPos(const char *fstr, const char *sstr)
{
    int slen = strlen(sstr);
    int flen = strlen(fstr);
    int pos;
    if(slen==0)
    { return -1; }
    if(slen > flen)
    { return -1; }
    for(pos=0; pos<(flen-slen+1); pos++)
    {
        if(strncmp(sstr,fstr+pos,slen)==0)
        { return pos; }
    }
    return -1;
}
/******************************************************************
    Function:       int CharReplace(char* fstr, const char target, const char rp)
    Description:    把字符串fstr中的target字符替换为rp字符
    Input:          char* fstr         需要进行字符替换的字符串
                const char target  需要替换的字符
                const char rp      替换的字符
    Return:         int
    Others:         如果rp为NULL，等同于CharDelete
********************************************************************/
int CharReplace(char *fstr, const char target, const char rp)
{
    int len = strlen(fstr);
    int i;
    if(len<1)
    { return 1; }
    if(rp==0)
    {
        CharDelete(fstr,target);
        return 0;
    }
    for(i=0; i<len; i++)
    {
        if(fstr[i]==target)
        { fstr[i]=rp; }
    }
    return 0;
}



/*检查方括号的匹配情况*/
int CheckFK(const char *rule_str)
{
    int a,b,c,d,i;
    char tmp[MAX_RULE_LEN];
    int len = strlen(rule_str);
    if((a=CharCount(rule_str,'['))!=(b=CharCount(rule_str,']')))
    { return 1; }
    if(a==0)
    { return 0; }
    c = StrPos(rule_str,"[");
    d = StrPos(rule_str,"]");
    if(d-c<3)
    { return 1; }
    else if(d-c==3)
    {
        if(IsNumAlpha(rule_str[c+1])||IsNumAlpha(rule_str[c+2]))
        { return 1; }
    }
    else if(d-c==4)
    {
        if(rule_str[c+2]=='-')
        {
            if(IsNumAlpha(rule_str[c+1])||IsNumAlpha(rule_str[c+3]))
            { return 1; }
        }
        else
        {
            if(IsNumAlpha(rule_str[c+1])||IsNumAlpha(rule_str[c+2])||IsNumAlpha(rule_str[c+3]))
            { return 1; }
        }
    }
    else
    {
        for(i=c+1; i<d; i++)
        {
            if(IsNumAlpha(rule_str[i]))
            { return 1; }
        }
    }
    if(a>1&&(len-1-d)>3)
    {
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp,"%s",rule_str+d+1);
        if(CheckFK(tmp))
        { return 1; }
    }
    return 0;
}
/******************************************************************
    Function:       int SyntaxCheck(const char* rule_str)
    Description:    简易正则表达式的语法检测函数
    Input:          const char* rule_str  规则字符串
    Return:         int  非0表示非法 0表示合法
    Others:         rule_str的长度必须大于1
********************************************************************/
int SyntaxCheck(const char *rule_str)
{
    int len;
    int kc;
    char tmp_rule[MAX_RULE_LEN];
    memset(tmp_rule, 0, sizeof(tmp_rule));
    StrTrim(tmp_rule,rule_str);
    len = strlen(tmp_rule);
    if(len<1)
    { return 1; }
    if((kc=CharCount(tmp_rule,'^'))>1)
    { return 1; }
    else if(kc==1&&tmp_rule[0]!='^')
    { return 1; }
    if((kc=CharCount(tmp_rule,'$'))>1)
    { return 1; }
    else if(kc==1&&tmp_rule[len-1]!='$')
    { return 1; }
    if(CheckFK(tmp_rule))
    { return 1; }
    return 0;
}

/*获取规则的最小有效长度*/
int RuleMinLen(const char *rule_str)
{
    int len = 0,i;
    int rlen=strlen(rule_str);
    char rule_tmp[MAX_RULE_LEN];
    sprintf(rule_tmp, "%s", rule_str);
    for(i=0; i<rlen; i++)
    {
        if(rule_tmp[i]=='^'||rule_tmp[i]=='$')
        { continue; }
        if(rule_tmp[i]=='[')
        {
            len += 1;
            rule_tmp[i] = '0';
            i = StrPos(rule_tmp,"]");
            rule_tmp[i] = '0';
        }
        else
        { len += 1; }
    }
    return len;
}

/*检测字符是否在规则表示的范围内*/
int CheckScale(const char t_char, const char *tmp_ptr)
{
    char big_v, small_v;
    int len = strlen(tmp_ptr),i;
    if(tmp_ptr[1]=='-')
    {
        if(tmp_ptr[0]>tmp_ptr[2])
        {
            big_v = tmp_ptr[0];
            small_v = tmp_ptr[2];
        }
        else if(tmp_ptr[0]<tmp_ptr[2])
        {
            big_v = tmp_ptr[2];
            small_v = tmp_ptr[0];
        }
        else
        {
            if(t_char!=tmp_ptr[0])
            { return 1; }
        }
        if(t_char<small_v||t_char>big_v)
        { return 1; }
    }
    else
    {
        for(i=0; i<len; i++)
        {
            if(t_char==tmp_ptr[i])
            { break; }
        }
        if(i==len)
        { return 1; }
    }
    return 0;
}

/******************************************************************
    Function:       int MatchRule(const char* other_party, const char* rule_str)
    Description:    简易正则表达式的规则匹配函数
    Input:          const char* other_party 需要检测的字符串
                const char* rule_str    规则字符串
    Return:         int  非0表示匹配 0表示不匹配
    Others:         函数内调用了SyntaxCheck函数。rule_str必须小于MAX_RULE_LEN个字符
********************************************************************/
int MatchRule(const char *other_party, const char *rule_str)
{
    int olen = strlen(other_party);
    char tmp_ptr[16], tmp_rule[MAX_RULE_LEN];
    int rlen = strlen(rule_str);
    int rindex = 0;
    int i, pos,slen;
    if( 0 == strcmp(rule_str,"*") )
    { return 1; }
    if(rlen < 1 || rlen > MAX_RULE_LEN - 1)
    { return 0; }
    if((slen=RuleMinLen(rule_str))>olen)
    { return 0; }
    if(SyntaxCheck(rule_str))
    { return 0; }
    sprintf(tmp_rule, "%s", rule_str);
    for(i=0; i<olen; i++)
    {
        if(rindex>rlen-1)
        { break; }
        if(strlen(other_party+i)< RuleMinLen(&tmp_rule[rindex]))
        { return 0; }
        if(tmp_rule[0]=='^')
        {
            if(rindex==0)
            { rindex = 1; }
            if(tmp_rule[rindex]=='[')
            {
                pos = StrPos(tmp_rule, "]");
                memset(tmp_ptr, 0, sizeof(tmp_ptr));
                strncpy(tmp_ptr,&tmp_rule[rindex+1],pos - rindex - 1);
                if(CheckScale(other_party[i],tmp_ptr))
                { return 0; }
                rindex = pos + 1;
                if(rindex>(rlen-1))
                { return 1; }
                if(tmp_rule[rindex]=='$')
                { return (i==olen-1); }
                tmp_rule[pos]='0';
                continue;
            }
            else
            {
                if(tmp_rule[rindex]!='?'&&other_party[i]!=tmp_rule[rindex])
                { return 0; }
                rindex++;
                if(rindex>(rlen-1))
                { return 1; }
                if(tmp_rule[rindex]=='$')
                { return (i==olen-1); }
                continue;
            }/*end if(tmp_rule[rindex]=='[')*/
        }
        else
        {
            if(tmp_rule[rindex]=='[')
            {
                pos = StrPos(tmp_rule, "]");
                memset(tmp_ptr, 0, sizeof(tmp_ptr));
                strncpy(tmp_ptr,&tmp_rule[rindex+1],pos - rindex - 1);
                if(CheckScale(other_party[i],tmp_ptr))
                {
                    if(rindex>0)
                    {
                        rindex = 0;
                        i--;
                        CharReplace(tmp_rule, '#', ']');
                    }
                    continue;
                }
                rindex = pos + 1;
                if(rindex>(rlen-1))
                { return 1; }
                if(tmp_rule[rindex]=='$')
                { return (i==olen-1); }
                tmp_rule[pos]='#';
                continue;
            }
            else
            {
                if(other_party[i]!=tmp_rule[rindex]&&tmp_rule[rindex]!='?')
                {
                    if(rindex>0)
                    {
                        rindex = 0;
                        i--;
                    }
                    continue;
                }
                rindex++;
                if(rindex>(rlen-1))
                { return 1; }
                if(tmp_rule[rindex]=='$')
                { return (i==olen-1); }
                continue;
            }/*if(tmp_rule[rindex]=='[')*/
        }/*end if(tmp_rule[0]=='^')*/
    }/*end for(i=0; i<olen; i++)*/
    return 0;
}

/******************************************************************
    Function:       int StrTrim（char* target,const char* src）
    Description:    将字符串source的头、尾空格去掉后放入字符串target中
    Input:          char* target     存放处理后串的字符数组指针
                const char* src  需要去空格的字符串
    Return:         int 有效字符串的长度
    Others:         target 必须是已经分配了足够存储空间的字符数组
********************************************************************/
int StrTrim(char *target,const char *source)
{
    int i=0, size=strlen(source);
    char tmp[1024];
    if(source==NULL||size<1||size>1024)
    { return 0; }
    memset(tmp, 0, sizeof(tmp));
    for(i=0; i<size; i++)
    {
        if(!isspace((int)source[i]))
        { break; }
    }
    strncpy(tmp,&source[i],size-i);
    for(i=(strlen(tmp)-1); i>=0; i--)
    {
        if(!isspace((int)tmp[i]))
        { break; }
    }
    snprintf(target,i+2,"%s",tmp);
    return strlen(target);
}


/*删除字符串中的空格*/
void DeleteBlank(char *s_buff)
{
    int i=0,j=0;
    char buff[2048]= {0};
    while(s_buff[i]!='\0')
    {
        if(s_buff[i]!=' '&&s_buff[i]!='\t')
        {
            buff[j++]=s_buff[i];
        }
        i++;
    }
    strcpy(s_buff,buff);
}

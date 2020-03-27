

/********************************************************************
    copyright (C), 1995-2004, Si-Tech Information Technology Ltd.
    file_name: common.c
    author: ��ʤѧ version: ��V1.00�� date: ��2009��05��20��
    description: ʵ�ֹ��ú���
    others:
    history: 1.date: �޸�ʱ�� version: �汾��V1.00��author: ����
            modification: �޸�����
         2. ...
********************************************************************/



#include "common.h"


/**********************************************************
    function: MyLog
    description: д��־��Ϣ
    Input: szSource: ��Ϣ��Դ
       szLevel: ��Ϣ����
       szFmt: ��Ϣ��ʽ
    Output:
    Return: MY_SUCCEED: �ɹ�, ʧ���˳�
    others:
***********************************************************/

int MyLog( char *szSource, char *szLevel, char *szFmt, ... )
{
    va_list vaList;
    char szBuf[MAX_RECORD_LEN + 1];
    char szTime[14 + 1];
    FILE *fp;
    /* 0-��ӡ���ն�;1-��¼��־�ļ�,*/
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
                     "���ܴ���־�ļ�" );
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
                 "�����쳣���, ����ֹͣ����, �˳�!\n\n" );
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
    description: ȥ�ַ�����˿ո�
    Input: szTrim: �������ַ���
    Output:
    Return: szTrim: �������ַ���
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
    description: ȥ�ַ����Ҷ˿ո�
    Input: szTrim: �������ַ���
    Output:
    Return: szTrim: �������ַ���
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
    description: ȥ�ַ������0�ַ�
    Input: szTrim: �������ַ���
    Output:
    Return: szTrim: �������ַ���
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
    description: ��ȡ�����ļ�ĳһ�ֶ�ȡֵ
    Input: szFileName: �����ļ���
       szLValue: �ֶ���
       szRValue: �ֶ�ֵ
    Output:
    Return: MY_SUCCEED: �ɹ� MY_FAIL: ʧ��
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
    description: �ж��ַ���ֵ�Ƿ�Ϊ������
    Input: szNum: �ַ���
    Output:
    Return: MY_TRUE: �� MY_FALSE: ����
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
    description: �ж��ַ���ֵ�Ƿ�Ϊ��ֵ
    Input: szNum: �ַ���
    Output:
    Return: MY_TRUE: �� MY_FALSE: ����
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
    description: �ж��ַ���ֵ�Ƿ�Ϸ�
    Input: szNum: �ַ���
    Output:
    Return: MY_TRUE: �� MY_FALSE: ����
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
    description: �ж��Ƿ�Ϊ����
    Input: nYear: ��
    Output:
    Return: MY_TRUE: �� MY_FALSE: ����
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
    description: ��ȡ�ļ���¼��
    Input: szPath: �ļ�·��
    Output:
    Return: lCount: ��¼��
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
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "�ļ� %s ��ʧ��!\n", szPath );
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
    description: ��ȡ��ǰϵͳʱ��
    Input: szStr: ����ʱ����ַ���
    Output:
    Return: MY_SUCCEED: �ɹ�
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
    description: ��ȡ����ʱ�����һ���ʱ��
    Input: szLastTime: ��һ��
       szCurrTime: ����ʱ��
    Output:
    Return: MY_SUCCEED: �ɹ�
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
    description: ��ȡ����ʱ�����һ���ʱ��
    Input: szNextDay: ��һ��
       szCurrTime: ����ʱ��
    Output:
    Return: MY_SUCCEED: �ɹ�
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
    description: ��ȡ����ʱ�����һ���ʱ��
    Input: szNextMonth: ��һ��
       szCurrTime: ����ʱ��
    Output:
    Return: MY_SUCCEED: �ɹ�
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
    description: ��ȡĳһ�µ����һ��
    Input: szLastDay: ���һ��
       szTime: ����ʱ��
    Output:
    Return: MY_SUCCEED: �ɹ�
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
    description: �ж��ļ��Ƿ����
    Input: szFile: �ļ���
    Output:
    Return: MY_TRUE: ���� MY_FALSE: ������
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
    description: ��ȡ�ļ��Ĵ�С
    Input: szFile: �ļ���
    Output:
    Return: �ļ���С
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
    description: �ж��ļ��Ƿ�ΪĿ¼�ļ�
    Input: szDirName: �ļ���
    Output:
    Return: MY_TRUE: �� MY_FALSE: ����
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
    description: ������־�ļ�·����Ч��
    Input: szPath: ��־�ļ�Ŀ¼
    Output:
    Return: MY_SUCCEED: ��Ч MY_FAIL: ��Ч
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
    description: ����ת���̨����
    Input:
    Output:
    Return: SUCCEED: �ɹ�, ʧ��������˳�
    others:
***********************************************************/

int BackGround( void )
{
    pid_t pid;
    pid = fork();
    if ( 0 > pid )
    {
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_EXIT, "����ת���̨ʧ��\n\n" );
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
    description: �ж��Ƿ�Ϊ��׼����
    Input: szDate: ����
    Output:
    Return: MY_TRUE: �� MY_FALSE: ����
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
    description: �ж��Ƿ�Ϊ��ȷ��Сʱ������
    Input: szDate: ����
    Output:
    Return: MY_TRUE: �� MY_FALSE: ����
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
    description: �ж��Ƿ�Ϊ��ȷ���յ�����
    Input: szDate: ����
    Output:
    Return: MY_TRUE: �� MY_FALSE: ����
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
    description: �ж��Ƿ�Ϊ��ȷ���µ�����
    Input: szDate: ����
    Output:
    Return: MY_TRUE: �� MY_FALSE: ����
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
    description: ��ȡ·���� BaseName
    Input: szFullDir: ·��
    Output: szDirName: BaseName
    Return: MY_SUCCEED: �ɹ� MY_FAIL: ʧ��
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
    description: ��ȡ��½�����ļ��е��û���������
    Input: szLoginFile: ��½�����ļ�
    Output: szUser: �û���
        szPwD: ����
    Return: MY_SUCCEED: �ɹ� MY_FAIL: ʧ��
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
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "�����ļ� %s ��ʧ��!\n", szLoginFile );
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
    description: �ƶ��ļ�
    Input: szSrcFile: Դ�ļ�
       szDestFile: Ŀ���ļ�
    Output:
    Return: MY_SUCCEED: �ɹ� MY_FAIL: ʧ��
    others:
***********************************************************/


int MoveFile( char *szSrcFile, char *szDstFile )
{
    if( -1 == rename( szSrcFile, szDstFile ) )
    {
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "�ƶ��ļ� %s �� %s ʧ��\n", szSrcFile, szDstFile );
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "ErrorInfo: %d : %s\n", errno, strerror( errno ) );
        return MY_FAIL;
    }
    return MY_SUCCEED;
}

/**********************************************************
    function: DeleteFile
    description: �ƶ��ļ�
    Input: szSrcFile: Դ�ļ�

    Output:
    Return: MY_SUCCEED: �ɹ� MY_FAIL: ʧ��
    others:
***********************************************************/


int DeleteFile( char *szSrcFile )
{
    if( -1 == remove( szSrcFile) )
    {
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "ɾ���ļ� %sʧ��\n", szSrcFile );
        MyLog( INFO_SOURCE_APP, INFO_LEVEL_WARN, "ErrorInfo: %d : %s\n", errno, strerror( errno ) );
        return MY_FAIL;
    }
    return MY_SUCCEED;
}

/**********************************************************
    function: IsSameFileSystem
    description: �ж�����Ŀ¼�Ƿ���ͬһ���ļ�ϵͳ
    Input: szDir1: Ŀ¼1
       szDir2: Ŀ¼2
    Output:
    Return: MY_TRUE: �� MY_FALSE: ��
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
    description: �жϼ�¼�Ƿ�Ϊ��Ч��¼
    Input: szRecord: ��¼����
    Output:
    Return: MY_TRUE: �� MY_FALSE: ��
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
    description: �ֽ�ʱ���ַ���������, ������ָ����ʱ��ṹ����
    Input: szTime: ʱ���ַ���
       ptData; ʱ��ṹ��
    Output:
    Return: MY_SUCCEED: �ɹ�
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
    Function: �滻�ַ���.
    Parameter:
        [IO]inStr              :�滻ǰ�������ַ���
        [I]oldStr              :��Ҫ�滻���Ӵ�
        [I]newStr              :�滻�����Ӵ�
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
    Function: �滻�ַ���.
    Parameter:
        [IO]inStr              :�滻ǰ�������ַ���
        [I]oldStr              :��Ҫ�滻���Ӵ�
        [I]newStr              :�滻�����Ӵ�
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
    Function: �滻�ַ���.
    Parameter:
        [IO]inStr              :�滻ǰ�������ַ���
        [I]oldStr              :��Ҫ�滻���Ӵ�
        [I]newStr              :�滻�����Ӵ�
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
    Function: �滻�ַ���.
    Parameter:
        [IO]inStr              :�滻ǰ�������ַ���
        [I]oldStr              :��Ҫ�滻���Ӵ�
        [I]newStr              :�滻�����Ӵ�
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
    Description:    ����������ʽ��Ĺ���ƥ�亯��
    Input:          const char* src_str ��Ҫ�����ַ���
                const char* regex_field  �������������ʽ����ϣ�֧��!��*
                const char split_flag���������������ʽ�ķָ��ַ�
    Return:         int  1��ʾƥ�䣬0��ʾ��ƥ��
    Others:         ���ñ����������ʽ��, regex_field����󳤶�ΪMAX_REGEX_LEN -1
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
    /*��һ���ַ�Ϊ'*'��ֱ��ƥ��*/
    if(tmp_regex[0]=='*')
    { return 1; }
    if(tmp_regex[0]=='!')
    {
        res_flag = 0;
        /*ɾ��'!'*/
        CharDelete(tmp_regex, '!');
    }
    /*����ָ��־û��ָ����Ĭ��Ϊ','*/
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
    Description:    ���ַ���str_str��split_char�ĵط��ָ�������֣���һ���ַ���split_str��
                �ڶ����ַŻ�src_str�С�
    Input:          const char* src_str ��Ҫ�ָ���ַ���
                const char* split_str  �ָ��ĵ�һ����
                const char split_char���ָ��ַ�
    Return:         int  -1��ʾʧ�ܣ�0��ʾ�ɹ�
    Others:         ���src_str������split_char���������src_str�ŵ�split_str��
                src_str����󳤶�ΪMAX_REGEX_LEN - 1
********************************************************************/
int StrSplit(char *split_str, char *src_str, const char split_char)
{
    int slen = 0, pos;
    char tmp[MAX_REGEX_LEN];
    if((slen = strlen(src_str)) < 1 || slen > MAX_REGEX_LEN - 1)
    { return -1; }
    if(split_char == 0)
    { return -1; }
    /*ȥ����β��split_char*/
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
    Function:       int CharPos��const char* source,const char target��
    Description:    �ҳ��ַ����ַ������״γ��ֵ�λ��
    Input:          const char* source  ��Ҫ�����������ַ���
                const char target   ��Ҫ�������ַ�
    Return:         int   �ַ����ֵ�λ�ã�����-1��ʾû���ҵ�
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
    Function:       int IsNumAlpha��const char str��
    Description:    �ж��ַ��Ƿ������ֺ���ĸ
    Input:          const char str  ��Ҫ���м����ַ�
    Return:         int ��0��ʾ������ 0��ʾ����
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
    Description:    ���ַ���fstr�е�target�ַ�ɾ����
    Input:          const char* fstr  ��Ҫ����ɾ�����ַ���
                const char sstr   ��Ҫɾ�����ַ�
    Return:         int   ��0��ʾ�ַ���̫����̫�̣� 0��ʾ�ɹ�
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
    Description:    ͳ���ַ�ch���ַ���src�е�����
    Input:          const char* str  ��Ҫ�����ַ�ͳ�Ƶ��ַ���
                const char  ch   ��Ҫͳ���������ַ�
    Return:         int  ͳ�ƵĽ��
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
    Function:       int StrPos��const char* fstr, const char* sstr��
    Description:    �ҳ��ַ���sstr���ַ���fstr���״γ��ֵ�λ��
    Input:          const char* fstr  ��Ҫ�����������ַ���
                const char* sstr  ��Ҫ�������ַ���
    Return:         int   �ַ������ֵ�λ�ã�����-1��ʾû���ҵ�
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
    Description:    ���ַ���fstr�е�target�ַ��滻Ϊrp�ַ�
    Input:          char* fstr         ��Ҫ�����ַ��滻���ַ���
                const char target  ��Ҫ�滻���ַ�
                const char rp      �滻���ַ�
    Return:         int
    Others:         ���rpΪNULL����ͬ��CharDelete
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



/*��鷽���ŵ�ƥ�����*/
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
    Description:    ����������ʽ���﷨��⺯��
    Input:          const char* rule_str  �����ַ���
    Return:         int  ��0��ʾ�Ƿ� 0��ʾ�Ϸ�
    Others:         rule_str�ĳ��ȱ������1
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

/*��ȡ�������С��Ч����*/
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

/*����ַ��Ƿ��ڹ����ʾ�ķ�Χ��*/
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
    Description:    ����������ʽ�Ĺ���ƥ�亯��
    Input:          const char* other_party ��Ҫ�����ַ���
                const char* rule_str    �����ַ���
    Return:         int  ��0��ʾƥ�� 0��ʾ��ƥ��
    Others:         �����ڵ�����SyntaxCheck������rule_str����С��MAX_RULE_LEN���ַ�
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
    Function:       int StrTrim��char* target,const char* src��
    Description:    ���ַ���source��ͷ��β�ո�ȥ��������ַ���target��
    Input:          char* target     ��Ŵ���󴮵��ַ�����ָ��
                const char* src  ��Ҫȥ�ո���ַ���
    Return:         int ��Ч�ַ����ĳ���
    Others:         target �������Ѿ��������㹻�洢�ռ���ַ�����
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


/*ɾ���ַ����еĿո�*/
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

#include "ddef.h"

int FreeGlobleMemory( )
{
    FreeFields( cData.psFields );
    FreeFiles( cData.psFiles );
    FreeColumns( cData.psColums );
    FreeFmtInfo( cData.psFmtFiles );
    fedx_freeGlobalMem( );
    return 0;
}

void ExitProgram( int nCode )
{
    FreeGlobleMemory( );
    exit( nCode );
}


int DealFixRecord( CDATA *psData )
{
    FILE *fp = NULL;
    char szRecord[2048],szOutRecord[2048],szSystemType[2+1];
    FILES   *file = psData->psFiles;
    FIELDS  *field = NULL ;
    int nOffset = 0;
    FORMAT_FILES *psLastFmt = NULL;
    /*field*/
    if( psData->fFlag )
    {
        while( file != NULL )
        {
            fp = fopen( file->szFile, "r" );
            if( fp == NULL )
            {
                fprintf( stderr, "WARNNING:Cannot open src file %s.(%s)\n", file->szFile, strerror(errno) );
                file = file->next;
                continue;
            }
            memset( szRecord, 0, sizeof(szRecord) );
            while( fgets( szRecord, sizeof(szRecord), fp ) )
            {
                memset( szSystemType, 0, sizeof(szSystemType) );
                strncpy( szSystemType, szRecord, 2 );
                if( NULL == psLastFmt || strcmp( szSystemType, psLastFmt->szSystemType ) != 0 )
                {
                    psLastFmt = NULL;
                    psLastFmt = SearchFmtGroup( psData->psFmtFiles, szSystemType );
                    if( NULL == psLastFmt )
                    {
                        fprintf( stderr, "WARNNING:unkown system type [%s]\n", szSystemType );
                        continue;
                    }
                    field = psData->psFields ;
                    if( ! UpdateSelectFields( field, psLastFmt ) )
                    {
                        fprintf( stderr,"WARNNING:update select fields failed.\n" );
                        continue;
                    }
                }
                field = psData->psFields ;
                memset( szOutRecord, 0, sizeof(szOutRecord) );
                nOffset = 0;
                while( field != NULL )
                {
                    sprintf( szOutRecord+nOffset, "%-*.*s%-*.*s",
                             field->nLen, field->nLen, szRecord+field->nOffset,
                             strlen(OUT_RECORD_SEP_CHAR), strlen(OUT_RECORD_SEP_CHAR), OUT_RECORD_SEP_CHAR );
                    nOffset = strlen( szOutRecord );
                    field = field->next;
                }
                szOutRecord[ strlen(szOutRecord) -strlen(OUT_RECORD_SEP_CHAR) ] = 0;
                printf( "%s\n", szOutRecord );
                memset( szRecord, 0, sizeof(szRecord) );
            }
            fclose( fp );
            file = file->next;
        }
    };
    /* column*/
    if( psData->cFlag )
    {
        while( file != NULL )
        {
            fp = fopen( file->szFile, "r" );
            if( fp == NULL )
            {
                fprintf( stderr, "WARNNING:Cannot open src file %s.(%s)\n", file->szFile, strerror(errno) );
                file = file->next;
                continue;
            }
            memset( szRecord, 0, sizeof(szRecord) );
            while( fgets( szRecord, sizeof(szRecord), fp ) )
            {
                if( szRecord[ strlen( szRecord ) - 1 ] == '\n' )
                { szRecord[ strlen( szRecord ) - 1 ] = 0; }
                if( szRecord[ strlen( szRecord ) - 1 ] == '\r' )
                { szRecord[ strlen( szRecord ) - 1 ] = 0; }
                field = psData->psColums ;
                memset( szOutRecord, 0, sizeof(szOutRecord) );
                nOffset = 0;
                while( field != NULL )
                {
                    if( field->nLen >= 0 )
                    {
                        field->nLen = min( field->nLen, strlen(szOutRecord+nOffset) );
                        sprintf( szOutRecord+nOffset, "%-*.*s%-*.*s",
                                 field->nLen, field->nLen, szRecord+field->nOffset,
                                 strlen(OUT_RECORD_SEP_CHAR), strlen(OUT_RECORD_SEP_CHAR), OUT_RECORD_SEP_CHAR );
                    }
                    else
                    {
                        sprintf( szOutRecord+nOffset, "%-s%-*.*s",
                                 szRecord+field->nOffset,
                                 strlen(OUT_RECORD_SEP_CHAR), strlen(OUT_RECORD_SEP_CHAR), OUT_RECORD_SEP_CHAR );
                    }
                    nOffset = strlen( szOutRecord );
                    field = field->next;
                }
                szOutRecord[ strlen(szOutRecord) -strlen(OUT_RECORD_SEP_CHAR) ] = 0;
                printf( "%s\n", szOutRecord );
                memset( szRecord, 0, sizeof(szRecord) );
            }
            fclose( fp );
            file = file->next;
        }
    };
    return 0;
}

#ifdef FEDX
int DealFedxRecord( CDATA *psData )
{
    FILE *fp = NULL;
    char szRecord[2048],szOutRecord[2048];
    FILES   *file = psData->psFiles;
    FIELDS  *field = NULL ;
    int nOffset = 0;
    FORMAT_FILES *psLastFmt = NULL;
    char  groupName[10+1];
    /*field*/
    if( psData->fFlag )
    {
        while( file != NULL )
        {
            fedx_paramInit( &s_fedx_param );
            if( fedx_getCurrVer( &s_fedx_param, "20080101" ) == NULL)
            {
                fprintf( stderr, "WARNNING:call fedx_getCurrVer error at [%d]!\n", __LINE__ );
                ExitProgram( 1 );
            }
            fp = fopen( file->szFile, "rb" );
            if( fp == NULL )
            {
                fprintf( stderr, "WARNNING:Cannot open src file %s.(%s)\n", file->szFile, strerror(errno) );
                file = file->next;
                continue;
            }
            while( fedx_loadMem( &s_fedx_param, fp ) != 0 )
            {
                fedx_paramReInit( &s_fedx_param );
                if( fedx_getHeadNode( &s_fedx_param, NULL ) != 0 )
                {
                    fprintf( stderr, "WARNNING:read file %s cause [ pCurrNode wrong ] error at [%d]!\n ", file->szFile, __LINE__);
                    fedx_freeRecord( &s_fedx_param );
                    break;
                }
                if( fedx_read_mem_record( &s_fedx_param ) != 0 )
                {
                    fprintf( stderr, "WARNNING:read file %s cause [ fedx_read_mem_record ] error at [%d]!\n ", file->szFile, __LINE__ );
                    fedx_freeRecord( &s_fedx_param );
                    break;
                }
                memset( szOutRecord, 0, sizeof(szOutRecord) );
                nOffset = 0;
                field = psData->psFields ;
                while( field != NULL )
                {
                    memset( szRecord, 0, sizeof(szRecord) );
                    if( fedx_getFieldValueByName( &s_fedx_param, szRecord, field->szFieldName,field->groupName) != 0 )
                    {
                        fprintf( stderr, "WARNNING:Get fedx record's field [%s] failed!\n",field->szFieldName);
                        continue;
                    }
                    sprintf( szOutRecord+nOffset, "%-*.*s%-*.*s",strlen( szRecord ), strlen( szRecord ), szRecord,strlen(OUT_RECORD_SEP_CHAR), strlen(OUT_RECORD_SEP_CHAR), OUT_RECORD_SEP_CHAR );
                    nOffset = strlen( szOutRecord );
                    field = field->next;
                }
                szOutRecord[ strlen(szOutRecord) -strlen(OUT_RECORD_SEP_CHAR) ] = 0;
                printf( "%s\n", szOutRecord );
            }
            fclose( fp );
            file = file->next;
        }
    }
    /*column*/
    if( psData->cFlag )
    {
        while( file != NULL )
        {
            fedx_paramInit( &s_fedx_param );
            if( fedx_getCurrVer( &s_fedx_param, "20080101" ) == NULL)
            {
                fprintf( stderr, "WARNNING:call fedx_getCurrVer error at [%d]!\n", __LINE__ );
                ExitProgram( 1 );
            }
            fp = fopen( file->szFile, "rb" );
            if( fp == NULL )
            {
                fprintf( stderr, "WARNNING:Cannot open src file %s.(%s)\n", file->szFile, strerror(errno) );
                file = file->next;
                continue;
            }
            while( fedx_loadMem( &s_fedx_param, fp ) != 0 )
            {
                fedx_paramReInit( &s_fedx_param );
                if( fedx_getHeadNode( &s_fedx_param, NULL ) != 0 )
                {
                    fprintf( stderr, "WARNNING:read file %s cause [ pCurrNode wrong ] error at [%d]!\n ", file->szFile, __LINE__);
                    fedx_freeRecord( &s_fedx_param );
                    break;
                }
                if( fedx_read_mem_record( &s_fedx_param ) != 0 )
                {
                    fprintf( stderr, "WARNNING:read file %s cause [ fedx_read_mem_record ] error at [%d]!\n ", file->szFile, __LINE__ );
                    fedx_freeRecord( &s_fedx_param );
                    break;
                }
                memset( szRecord, 0, sizeof(szRecord) );
                if( fedx_getFieldValueByName( &s_fedx_param, szRecord, "system_type", NULL ) != 0 )
                {
                    fprintf( stderr, "WARNNING:Get fedx record's field [ system_type ] failed!\n" );
                    continue;
                }
                if( NULL == psLastFmt || strcmp( szRecord, psLastFmt->szSystemType ) != 0 )
                {
                    psLastFmt = NULL;
                    psLastFmt = SearchFmtGroup( psData->psFmtFiles, szRecord );
                    if( NULL == psLastFmt )
                    {
                        fprintf( stderr, "WARNNING:unkown system type [%s]\n", szRecord );
                        continue;
                    }
                }
                memset( szRecord, 0, sizeof(szRecord) );
                if( ! GetFixRecord( &s_fedx_param, psLastFmt, szRecord ) )
                {
                    fprintf( stderr, "WARNNING:GetFixRecord failed!\n" );
                    continue;
                }
                nOffset = 0;
                field = psData->psColums ;
                memset( szOutRecord, 0, sizeof(szOutRecord) );
                while( field != NULL )
                {
                    if( field->nLen >= 0 )
                    {
                        field->nLen = min( field->nLen, strlen(szOutRecord+nOffset) );
                        sprintf( szOutRecord+nOffset, "%-*.*s%-*.*s",
                                 field->nLen, field->nLen, szRecord+field->nOffset,
                                 strlen(OUT_RECORD_SEP_CHAR), strlen(OUT_RECORD_SEP_CHAR), OUT_RECORD_SEP_CHAR );
                    }
                    else
                    {
                        sprintf( szOutRecord+nOffset, "%-s%-*.*s",
                                 szRecord+field->nOffset,
                                 strlen(OUT_RECORD_SEP_CHAR), strlen(OUT_RECORD_SEP_CHAR), OUT_RECORD_SEP_CHAR );
                    }
                    nOffset = strlen( szOutRecord );
                    field = field->next;
                }
                szOutRecord[ strlen(szOutRecord) -strlen(OUT_RECORD_SEP_CHAR) ] = 0;
                printf( "%s\n", szOutRecord );
            }
            fclose( fp );
            file = file->next;
        }
    }
    return 0;
}

int GetFixRecord( fedx_param *psFedxStru, FORMAT_FILES *pfFile, char *szRecord )
{
    PFORMAT_RECORD pfRecord;
    int nLoop = 0,nOffset = 0;
    char szFieldValue[MAX_RECORD_LEN + 1];
    pfRecord    = pfFile->pFormatRecord;
    memset( szRecord, 0, sizeof(szRecord) );
    for( nLoop = 0; nLoop < pfFile->nCount; nLoop++, pfRecord++ )
    {
        if( strcasecmp( pfRecord->szFieldName, "newline" ) == 0 )
        {
            break;
        }
        memset( szFieldValue, 0, sizeof(szFieldValue) );
        if( fedx_getFieldValueByName( psFedxStru, szFieldValue, pfRecord->szFieldName, NULL ) != 0 )
        {
            fprintf( stderr, "WARNNING:Get fedx record's field [ system_type ] failed!\n" );
            return 0;
        }
        TrimLeft( TrimRight( szFieldValue ) );
        switch( pfRecord->nType )
        {
        /* ÊýÖµÐÍ*/
        case 1:
            sprintf( szRecord+nOffset, "%0*d.0*d", pfRecord->nLength, pfRecord->nLength, atoi(szFieldValue) );
            break;
        case 0:
        case 10:
        default:
            sprintf( szRecord+nOffset, "%-*.*s", pfRecord->nLength, pfRecord->nLength, szFieldValue );
            break;
        }
        nOffset += pfRecord->nLength ;
    }
    if( szRecord[ strlen( szRecord ) - 1 ] != '\n' )
    { strcat( szRecord+strlen(szRecord), "\n" ); }
    return 1;
}

#endif


FORMAT_FILES *SearchFmtGroup( FORMAT_FILES *psFmtGroup, char *szSystemType )
{
    FORMAT_FILES    *t = psFmtGroup;
    while( t != NULL )
    {
        if( strcmp( t->szSystemType, szSystemType ) == 0 )
        { return t; }
        t = t->next;
    }
    return NULL;
}

int GetFmtInfo( FORMAT_FILES **psFmtFiles )
{
    DIR *dir;
    struct dirent *dp;
    char *szFmtPath = NULL;
    char *szFedxName = NULL;
    FORMAT_FILES  *psCurrFile;
    FORMAT_RECORD *pfRec;
    FILE *fp = NULL;
    char szRecord[ 2048 ];
    char *szPos = NULL;
    int  nFieldNo = 0;
    char szFmtFile[512];
    szFmtPath = getenv( CUTF_FORMAT_PATH );
    if( ! IsDir( szFmtPath ) )
    {
        fprintf( stderr, "ERROR:env [%s] may do not be set.\n", CUTF_FORMAT_PATH );
        ExitProgram( 1 );
    }
    dir = opendir( szFmtPath );
    while( dp = readdir( dir ) )
    {
        if( fnmatch( "bill.??", dp->d_name, 0 ) != 0 )
        {
            continue;
        }
        if( *psFmtFiles == NULL )
        {
            *psFmtFiles = (FORMAT_FILES *) malloc( sizeof( FORMAT_FILES ) );
            if( NULL == *psFmtFiles )
            {
                fprintf( stderr, "ERROR:malloc for psFmtFiles failed.(%s)\n", strerror(errno) );
                ExitProgram( 1 );
            }
            psCurrFile = *psFmtFiles;
        }
        else
        {
            psCurrFile = *psFmtFiles;
            while( NULL != psCurrFile->next )
            {
                psCurrFile = psCurrFile->next;
            }
            psCurrFile->next = (FORMAT_FILES *) malloc( sizeof( FORMAT_FILES ) );
            if( NULL == psCurrFile->next )
            {
                fprintf( stderr, "ERROR:malloc for psFmtFiles failed.(%s)\n", strerror(errno) );
                ExitProgram( 1 );
            }
            psCurrFile = psCurrFile->next;
        }
        memset( psCurrFile, 0, sizeof(FORMAT_FILES) );
        psCurrFile->next = NULL;
        sprintf( psCurrFile->szSystemType, "%-2.2s", dp->d_name + 5 );
        memset( szFmtFile, 0, sizeof(szFmtFile) );
        sprintf( szFmtFile, "%s", szFmtPath );
        if( szFmtFile[strlen(szFmtFile)-1] != '/' )
        { strcat( szFmtFile, "/" ); }
        strcat( szFmtFile, dp->d_name );
        psCurrFile->nCount = GetFileCount( szFmtFile );
        if( 0 >= psCurrFile->nCount )
        {
            fprintf( stderr, "ERROR:format file can't be null !(%s)", szFmtFile );
            ExitProgram( 1 );
        }
        psCurrFile->pFormatRecord = (FORMAT_RECORD *) malloc( sizeof(FORMAT_RECORD) * psCurrFile->nCount );
        if( NULL ==  psCurrFile->pFormatRecord )
        {
            fprintf( stderr, "ERROR:malloc for format file failed!(%s)\n", strerror(errno) );
            ExitProgram( 1 );
        }
        memset( psCurrFile->pFormatRecord , 0, sizeof(FORMAT_RECORD) * psCurrFile->nCount );
        pfRec = psCurrFile->pFormatRecord;
        fp = fopen( szFmtFile, "r" );
        if( NULL == fp )
        {
            fprintf( stderr, "ERROR:open format file %s failed(%s).\n", strerror(errno) );
            ExitProgram( 1 );
        }
        memset( szRecord, 0, sizeof(szRecord) );
        while( fgets( szRecord, sizeof(szRecord), fp ) )
        {
            if( ! IsRecordLine( szRecord ) )
            { continue; }
            if( szRecord[strlen(szRecord)-1] == '\n' )
            { szRecord[strlen(szRecord)-1] = 0; }
            szPos = NULL;
            szPos = strtok( szRecord, FMT_SEP_CHAR );
            nFieldNo = 0;
            while( NULL != szPos )
            {
                switch( nFieldNo )
                {
                case 0:
                    pfRec->nFieldNo = atoi( szPos );
                    break;
                case 1:
                    strcpy( pfRec->szFieldName, szPos );
                    break;
                case 2:
                    pfRec->nStart = atoi( szPos );
                    break;
                case 3:
                    pfRec->nEnd = atoi( szPos );
                    break;
                case 4:
                    pfRec->nLength = atoi( szPos );
                    break;
                case 5:
                    pfRec->nType = atoi( szPos );
                    break;
                case 6:
                case 7:
                case 8:
                default:
                    break;
                }
                nFieldNo ++;
                szPos = strtok( NULL, FMT_SEP_CHAR );
            }
            pfRec ++;
            memset( szRecord, 0, sizeof(szRecord) );
        }
        fclose( fp );
        pfRec = psCurrFile->pFormatRecord ;
        qsort( pfRec, psCurrFile->nCount, sizeof(FORMAT_RECORD), CompareByFileNo );
    }
    closedir( dir );
#ifdef FEDX
    memset( szFmtFile, 0, sizeof(szFmtFile) );
    sprintf( szFmtFile, "%s", getenv( FEDX_CONFIG_PATH ) );
    if( szFmtFile[ strlen(szFmtFile) - 1 ] != '/' )
    { strcat( szFmtFile, "/" ); }
    strcat( szFmtFile, getenv( FEDX_CONFIG_NAME ) );
    if( 0 != fedx_getConf( szFmtFile ) )
    {
        fprintf( stderr, "ERROR:fedx_getConf %s failed!\n", szFmtFile );
        ExitProgram( 1 );
    }
#endif
    return 0;
}


int DebugOutFmtFiles( FORMAT_FILES *psFmtFiles )
{
    FORMAT_FILES    *t = psFmtFiles ;
    FORMAT_RECORD   *r;
    int nLoop = 0;
    while( t != NULL )
    {
        printf( "\t\t==== desc %-2.2s ====\n", t->szSystemType );
        r = t->pFormatRecord;
        for( nLoop = 0; nLoop < t->nCount ; nLoop ++, r ++ )
        {
            printf( "[%3d] [%-15.15s] [%3d] [%3d] [%3d] [%3d]\n", r->nFieldNo, r->szFieldName, r->nStart, r->nEnd, r->nLength, r->nType );
        }
        printf( "\t\t====  end  ====\n\n", t->szSystemType );
        t = t->next;
    }
    return 0;
}

int FreeFmtInfo( FORMAT_FILES *psFmtFiles )
{
    FORMAT_FILES    *t;
    FORMAT_RECORD   *r;
    while( psFmtFiles != NULL )
    {
        t = NULL;
        t = (psFmtFiles)->next;
        r = (psFmtFiles)->pFormatRecord;
        free( r );
        r = NULL;
        free( psFmtFiles );
        psFmtFiles = NULL;
        psFmtFiles = t;
    }
    return 0;
}

int GetCmdInfo( CDATA *psData, int argc, char *argv[] )
{
#ifdef _DEBUG_
    printf("[%s][%d] argc=[%d] argv=[%s]\n",__FILE__,__LINE__,argc,argv[0]);
#endif
    char szTmpBuf[2048];
    char szStack[2048];
    int nLoop = 0,nOptBegin = 0, nListFlag = 0,nDupFlag = 0,nNum = 0;
    char *szFiles = NULL;
    char *szPos = NULL;
    FILES *psFiles;
    //sprintf( psData->szBinFile, "%s",GetFilename(argv[0]));
    sprintf( psData->szBinFile, "%s","cutf");
    nLoop ++;
    nOptBegin = 0;
    nListFlag = 0;
    szFiles   = NULL;
#ifdef _DEBUG_
    printf("[%s][%d] szBinFile=[%s]\n",__FILE__,__LINE__,psData->szBinFile);
#endif
    memset( szStack, 0, sizeof(szStack) );
    for( ; nLoop < argc ; nLoop ++ )
    {
        memset( szTmpBuf, 0, sizeof(szTmpBuf) );
        strcpy( szTmpBuf, *(argv+nLoop) );
        if( ! nOptBegin )
        {
            if( szTmpBuf[0] == '-' )
            {
                nOptBegin  = 1;
            }
        };
        if( nOptBegin )
        {
            if( strlen( szTmpBuf ) != 2 )
            {
                strcat( szStack, szTmpBuf + 2 );
            }
            else
            {
                nLoop ++;
                strcat( szStack, *(argv+nLoop) );
            };
            if( strncmp( szTmpBuf, OPT_K_CHAR, strlen(OPT_K_CHAR) ) == 0 )
            {
                psData->kFlag = true;
            }
            else if( strncmp( szTmpBuf, OPT_F_CHAR, strlen(OPT_F_CHAR) ) == 0 )
            {
                psData->fFlag = true;
                ParseField( &(psData->psFields), szStack, 'f' );
            }
            else if( strncmp( szTmpBuf, OPT_C_CHAR, strlen(OPT_C_CHAR) ) == 0 )
            {
                psData->cFlag = true;
                ParseField( &(psData->psColums), szStack, 'c' );
            }
            else if( strncmp( szTmpBuf, OPT_M_CHAR, strlen(OPT_M_CHAR) ) == 0 )
            {
                if( strcasecmp( szStack, "fix" ) == 0 )
                {
                    psData->mFlag = true;
                }
                else
                {
                    psData->mFlag = false;
                }
            }
            else if( strncmp( szTmpBuf, OPT_T_CHAR, strlen(OPT_T_CHAR) ) == 0 )
            {
                psData->tFlag = true;
            }
            else if( strcmp( szTmpBuf, OPT_V_CHAR ) == 0 )
            {
                psData->vFlag = true;
            }
            else if( strcmp( szTmpBuf, OPT_H_CHAR ) == 0 )
            {
                psData->hFlag = true;
            }
            else
            {
                fprintf( stderr, "ERROR:--unsupported option [%s]\n", szTmpBuf );
                ExitProgram( 1 );
            }
            nOptBegin = 0;
            memset( szTmpBuf, 0, sizeof(szTmpBuf) );
            memset( szStack, 0, sizeof(szStack) );
            continue;
        }
        break;
    }
    if( ! CheckOptionGroup( psData ) )
    {
        fprintf( stderr, "ERROR:option used error.\n" );
        ExitProgram( 1 );
    }
    nNum = 0;
    for( ; nLoop < argc ; nLoop ++ )
    {
        if( IsDir( argv[nLoop] ) )
        {
            fprintf( stderr, "ERROR:arguments cannot be directory.(%s)\n", argv[nLoop] );
            ExitProgram( 1 );
        }
        if( argv[nLoop][0] == '-' )
        {
            fprintf( stderr, "ERROR:Cannot appear '-' in file name.(%s)!\n", argv[nLoop] );
            ExitProgram( 1 );
        }
        if( psData->kFlag || psData->tFlag )
        {
            if( nLoop >= argc -1 )
            {
                strcpy( psData->szOutFile, argv[nLoop] );
                break;
            }
        }
        if( psData->psFiles == NULL )
        {
            psData->psFiles = ( FILES * ) malloc( sizeof(FILES) );
            if( NULL == psData->psFiles )
            {
                fprintf( stderr, "ERROR:malloc for list files failed.(%s)\n", strerror(errno) );
                ExitProgram( 1 );
            }
            psFiles = psData->psFiles;
        }
        else
        {
            psFiles = psData->psFiles;
            nDupFlag = 0;
            while( psFiles->next != NULL )
            {
                /*  if( strcmp( argv[nLoop], psFiles->szFile ) == 0 )
                    {
                    nDupFlag = 1;
                    break;
                    }*/
                psFiles = psFiles->next;
            }
            /*  if( nDupFlag )
                continue;
            */
            psFiles->next = ( FILES * ) malloc( sizeof(FILES) );
            if( NULL == psFiles->next )
            {
                fprintf( stderr, "ERROR:malloc for list files failed.(%s)\n", strerror(errno) );
                ExitProgram( 1 );
            }
            psFiles = psFiles->next;
        }
        if( nNum ++ >= MAX_READ_FILE )
        {
            fprintf( stderr, "ERROR:In file num cannot be greater than %d\n", MAX_READ_FILE );
            ExitProgram( 1 );
        }
        memset( psFiles, 0, sizeof(FILES) );
        strcpy( psFiles->szFile, argv[nLoop] );
        psFiles->next = NULL;
    }
    if( psData->vFlag )
    {
        if( psData->psFiles != NULL )
        {
            fprintf( stderr, "Use -h or -v to view help or version information.\n" );
            ExitProgram( 1 );
        }
        else
        {
            Version( );
        }
    };
    if( psData->hFlag )
    {
        if( psData->psFiles != NULL )
        {
            fprintf( stderr, "Use -h or -v to view help or version information.\n" );
            ExitProgram( 1 );
        }
        else
        {
            Help( );
        }
    };
    if( psData->fFlag )
    {
        if( psData->psFields  == NULL )
        {
            fprintf( stderr, "ERROR:field(s) is needed when \"-f\" is used\n" );
            ExitProgram( 1 );
        };
        if( psData->psFiles == NULL )
        {
            fprintf( stderr, "ERROR:file(s) is needed when \"-f\" is used.\nUse -h or -v to view help or version information.\n" );
            ExitProgram( 1 );
        };
        /*if( ! CheckSelectFields( psData ) )
        {
            fprintf( stderr, "ERROR:check select fields error.\n" );
            ExitProgram( 1 );
        };*/
    };
    if( psData->cFlag )
    {
        if( psData->psColums  == NULL )
        {
            fprintf( stderr, "ERROR:column(s) is needed when \"-c\" is used\n" );
            ExitProgram( 1 );
        };
        if( psData->psFiles == NULL )
        {
            fprintf( stderr, "ERROR:file(s) is needed when \"-c\" is used.\nUse -h or -v to view help or version information.\n" );
            ExitProgram( 1 );
        };
    };
    if( ! psData->vFlag && ! psData->hFlag &&
            ! psData->fFlag && ! psData->cFlag )
    {
        Help( );
    }
    return 0;
}

void Version( )
{
    printf("***************************************************************\n");
    printf("*                         SI-TECH                             *\n");
    printf("************************ %s  %s ***************************\n", BINFILE, VERSION );
    printf("*                         SI-TECH                             *\n");
    printf("***************************************************************\n");
    ExitProgram( 0 ) ;
}

void Help( )
{
    printf("***************************************************************\n");
    printf("*                         SI-TECH                             *\n");
    printf("***************************************************************\n");
    printf( "*[%s]\n", BINFILE );
    printf( "*Version :  %s\n", VERSION );
    printf( "*Usage   :  %s -m fix -f msisdn,system_type aa.fix* bb.fix*  \n", BINFILE );
	printf( "*Usage   :  %s -m fix -f system_tpye,call_duration#1 aa.fix* bb.fix*  \n", BINFILE );
    printf( "*           %s [-m fedx] -f error_code,msisdn,start_datetime aa.fedx* bb.fedx*\n", BINFILE );
    printf( "*           %s -c 1-2,3-4 -c 5- aa.fedx [a,b].fedx\n", BINFILE );
    printf( "*\n" );
    printf("***************************************************************\n");
    printf("*                         SI-TECH                             *\n");
    printf("***************************************************************\n");
    ExitProgram( 0 ) ;
}

int FreeFiles( FILES *psFiles )
{
    FILES *f ;
    while( psFiles != NULL )
    {
        f = psFiles->next;
        free( psFiles );
        psFiles = NULL;
        psFiles = f;
    }
    return 0;
}

int DebugOutFiles( FILES *psFiles )
{
    FILES *f = psFiles ;
    while( f != NULL )
    {
        printf( "[%s]\n", f->szFile );
        f = f->next;
    }
    return 0;
}

int CheckOptionGroup( CDATA *psData )
{
    /* version*/
    if( psData->vFlag )
    {
        psData->vFlag = true;
        psData->fFlag = false;
        psData->cFlag = false;
        psData->mFlag = false;
        psData->kFlag = false;
        psData->tFlag = false;
        psData->hFlag = false;
    };
    /* version*/
    if( psData->hFlag )
    {
        psData->hFlag = true;
        psData->vFlag = false;
        psData->fFlag = false;
        psData->cFlag = false;
        psData->mFlag = false;
        psData->kFlag = false;
        psData->tFlag = false;
    };
    /* field mode*/
    if( psData->fFlag )
    {
        psData->fFlag = true;
        if( psData->cFlag )
        {
            return 0;
        }
        else
        {
            psData->cFlag = false;
            return 1;
        }
    };
    /* column mode*/
    if( psData->cFlag )
    {
        psData->cFlag = true;
        if( psData->fFlag )
        {
            return 0;
        }
        else
        {
            psData->fFlag = false;
            return 1;
        }
    };
    /* key mode*/
    if( psData->kFlag )
    {
        psData->kFlag = true;
        if( psData->tFlag )
        {
            return 0;
        }
        else
        {
            psData->tFlag = false;
            return 1;
        }
    };
    /* control mode*/
    if( psData->tFlag )
    {
        psData->tFlag = true;
        if( psData->kFlag )
        {
            return 0;
        }
        else
        {
            psData->kFlag = false;
            return 1;
        }
    };
    return 1;
}

int CheckSelectFields( CDATA *psData )
{
    FIELDS       *field  = psData->psFields;
    FORMAT_FILES *f   = NULL;
    FORMAT_RECORD *r;
    int nLoop = 0;
    bool bResult ;
    while( field != NULL )
    {
        bResult = false;
        f   = psData->psFmtFiles ;
        while( f != NULL )
        {
            r = f->pFormatRecord;
            for( nLoop = 0; nLoop < f->nCount; nLoop ++, r ++ )
            {
                if( strcasecmp( field->szFieldName, r->szFieldName ) == 0 )
                {
                    bResult = true;
                }
            }
            f = f->next;
        }
        if( ! bResult )
        {
            fprintf( stderr, "ERROR:field [%s] not found.\n", field->szFieldName );
            return 0;
        }
        field = field->next;
    }
    return 1;
}

int UpdateSelectFields( FIELDS *field, FORMAT_FILES *f )
{
    FORMAT_RECORD *r;
    int nLoop = 0;
    bool bResult ;
    while( field != NULL )
    {
        bResult = false;
        r = f->pFormatRecord;
        for( nLoop = 0; nLoop < f->nCount; nLoop ++, r ++ )
        {
            if( strcasecmp( field->szFieldName, r->szFieldName ) == 0 )
            {
                field->nOffset = r->nStart;
                field->nLen    = r->nLength;
                bResult = true;
            }
        }
        if( ! bResult )
        {
            fprintf( stderr, "ERROR:field [%s] not found in [%s].\n", field->szFieldName, f->szSystemType );
            return 0;
        }
        field = field->next;
    }
    return 1;
}

int InitGlobleData( CDATA *psData )
{
    memset( psData, 0, sizeof(CDATA) );
    psData->fFlag = false;
    psData->cFlag = false;
    psData->mFlag = false;
    psData->kFlag = false;
    psData->tFlag = false;
    psData->vFlag = false;
    psData->hFlag = false;
    psData->psFields = NULL;
    psData->psFiles = NULL;
    psData->psColums = NULL;
    return 0;
}

int CompareByFileNo( void *left, void *right )
{
    FORMAT_RECORD *leftRec  = (FORMAT_RECORD *) left;
    FORMAT_RECORD *rightRec = (FORMAT_RECORD *) right;
    return leftRec->nFieldNo < rightRec->nFieldNo ?  -1 : 1 ;
}

int ParseField( FIELDS **pHead, char *szStr, char cMode )
{
    FIELDS  *psField;
    char    *szPos = NULL;
    szPos = NULL;
    szPos = strtok( szStr, FIELD_SEP_CHAR );
    while( NULL != szPos )
    {
        if( *pHead == NULL )
        {
            *pHead = ( FIELDS * ) malloc( sizeof(FIELDS) );
            if( NULL == *pHead )
            {
                fprintf( stderr, "ERROR:malloc for fields failed(%s)\n", strerror(errno) );
                ExitProgram( 1 );
            }
            psField = *pHead ;
        }
        else
        {
            psField = *pHead;
            while( NULL != psField->next )
            {
                psField = psField->next;
            }
            psField->next = ( FIELDS * ) malloc( sizeof(FIELDS) );
            if( NULL == psField->next )
            {
                fprintf( stderr, "ERROR:malloc for fields failed(%s)\n", strerror(errno) );
                ExitProgram( 1 );
            }
            psField = psField->next;
        }
        memset( psField, 0, sizeof( FIELDS ) );
        psField->next = NULL;
        if( cMode == 'f' )
        {
            if(strchr(szPos,'#') > 0)
            {
                StrSplit(psField->szFieldName,szPos,'#');
                strcpy(psField->groupName,"GGROUP");
                strcat(psField->groupName,szPos);
            }
            else
            {
                strcpy(psField->szFieldName, szPos );
                strcpy(psField->groupName,"");
            }
#ifdef _DEBUG_
            printf("[%s][%d]szFieldName=[%s]groupName=[%s] \n",__FILE__,__LINE__,psField->szFieldName,psField->groupName);
#endif
        }
        else if( cMode == 'c' )
        {
            ParseColumn( & psField->nOffset, & psField->nLen, szPos );
        };
        szPos = strtok( NULL, FIELD_SEP_CHAR );
    }
    return 0;
}


int ParseColumn( int *nBegin, int *nEnd, char *szStr )
{
    char szLeft[64],szRight[64],*szPos = NULL;
    *nBegin  = -1;
    *nEnd    = -1;
    memset( szLeft, 0, sizeof(szLeft) );
    memset( szRight, 0, sizeof(szRight) );
    szPos = strchr( szStr, COLUMN_SEP_CHAR );
    if( NULL == szPos )
    {
        if( ! IsNum( szStr ) )
        {
            fprintf( stderr, "ERROR:The list arguments following the c option are not correct.[%s]\n", szStr );
            ExitProgram( 1 );
        }
        *nBegin = atol( szStr ) - 1 <= 0 ? 0 : atol( szStr ) - 1 ;
        *nEnd   = *nBegin;
    }
    else
    {
        strncpy( szLeft, szStr, szPos - szStr );
        strcpy(  szRight, szPos + 1 );
        if( ! IsNum( szLeft ) )
        {
            fprintf( stderr, "ERROR:The list arguments following the c option are not correct.[%s]\n", szStr );
            ExitProgram( 1 );
        }
        *nBegin  = atol( szLeft ) - 1 <= 0 ? 0 : atol( szLeft ) - 1 ;
        if( *szRight == NULL )
        {
            *nEnd = -9 ;
        }
        else
        {
            if( ! IsNum( szRight ) )
            {
                fprintf( stderr, "ERROR:The list arguments following the c option are not correct[2].\n" );
                ExitProgram( 1 );
            }
            *nEnd = atol( szRight ) - 1 <= 0 ? 0 : atol( szRight ) - 1 ;
        }
    }
    if( *nEnd >= 0 )
    {
        *nEnd = *nEnd - *nBegin + 1;
    }
    return 0;
}


int FreeFields(  FIELDS *pHead )
{
    FIELDS *psField;
    while( NULL != pHead )
    {
        psField = (pHead)->next ;
        free( pHead );
        pHead = NULL;
        pHead = psField;
    }
    return 0;
}

int FreeColumns(  FIELDS *pHead )
{
    FIELDS *psField;
    while( NULL != pHead )
    {
        psField = (pHead)->next ;
        free( pHead );
        pHead = NULL;
        pHead = psField;
    }
    return 0;
}

int DebugOutFields( FIELDS  *psFields, char cMode )
{
    FIELDS  *t;
    t = psFields;
    printf("===== field info =====\n");
    while( t != NULL )
    {
        if( cMode == 'c' )
        {
            printf( "[%d]\t", t->nOffset );
            printf( "[%d]\n", t->nLen );
        }
        else if( cMode == 'f' )
        {
            printf( "[%s]\n", t->szFieldName );
        }
        t = t->next;
    }
    printf("===== field info =====\n\n");
    return 0;
}

int main( int argc, char *argv[] )
{
    InitGlobleData( &cData );
    GetFmtInfo( &( cData.psFmtFiles) );
    /*DebugOutFmtFiles( cData.psFmtFiles );*/
    GetCmdInfo( &cData, argc, argv );
    /*  DebugOutFields( cData.psFields, 'f' );
        DebugOutFields( cData.psColums, 'c' );  */
    if( cData.mFlag )
    {
        DealFixRecord( & cData );
    }
    else
    {
        DealFedxRecord( & cData );
    }
    ExitProgram( 0 ) ;
}

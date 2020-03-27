
/***********************************************************
 * Copyright (c) 1999,2002 .
 * Descriptions:
 *      _pub.c
 *      一些公用函数.
 * Known bugs:
 *
 * Change log:
 *        who          MM/DD/YYYY        why
 *        wangjs       03/20/2005        create
 **********************************************************/
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "os.h"
#include "load_main.h"

#ifdef UNIX_OS
  #include <dirent.h>
  #include <fnmatch.h>
  #include <unistd.h>
  #include <errno.h>
#else
  #include <io.h>
  #include <process.h>
  #include <sys/locking.h>
  #include <share.h>
#endif


/* 日志文件名前缀 */
static char LOG_SYS_PREFIX[100] = {0};
static char LOG_SYS_POSTFIX[] = ".00.runlog";

/* 进程名称 */
static char PROC_NAME[100] = {0};

/*文件排序*/
typedef struct _sort_file_
{
    char file_name[100];    /*文件名*/
    time_t   mtime;         /*文件最后修改时间时间*/
}SORT_FILE;

/***********************************************************
 * Function: 获取系统时间.
 * Parameter:
 *      [O]outDateStr     :输出时间字符串,'yyyymmddhhmiss'
 * Returns:
 *      none
 **********************************************************/
void GetSysDateTime( char *outDateStr )
{
    time_t     timer;
    time(&timer);
    strftime(outDateStr, 15, "%Y%m%d%H%M%S", localtime(&timer));
    return;
}

/***********************************************************
 * Function: 获取系统时间的总秒数.
 * Parameter:
 *      none
 * Returns:
 *      long           :系统时间总秒数
 **********************************************************/
long GetTimeSec( void )
{
    time_t tmpTime;
    time( &tmpTime );
    return (long)tmpTime;
}

/***********************************************************
 * Function: 初始日志文件，得到日志文件名前半部分
 * Parameter:
 *      [I]logPath             :日志路径
 *      [I]exeName             :进程名
 * Revurns:
 *      none
 **********************************************************/
void Initlog(char *logPath, char *exeName)
{
    sprintf(LOG_SYS_PREFIX, "%s%s.", logPath, exeName);
    sprintf(PROC_NAME, "%s", exeName);
    return;
}

/***********************************************************
 * Function: 写运行日志
 * Parameter:
 *      [I]level               :信息级别
 *      [I]mesg                :要写入日志的字符串
 * Revurns:
 *      none
 **********************************************************/
void Writelog(char *level, char *mesg, ...)
{
    FILE *fp;
    time_t  timer;
    va_list va;
    char timestamp[20]={0};
    char logFile[200]={0};
    char str[1000];

    va_start(va, mesg);
    vsprintf(str, mesg, va);
    va_end(va);

    if(LOG_SYS_PREFIX[0] == '\0')
    {
        printf("%s\n", str);
        return;
    }

    time(&timer);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", localtime(&timer));

    sprintf(logFile, LOG_SYS_PREFIX);
    strncat(logFile, timestamp, 8);
    strcat(logFile, LOG_SYS_POSTFIX);
    fp = fopen(logFile, "a+");
    if(fp == NULL)
        return;

    fprintf(fp, "%s:%-*s:%-*s:%s\n", timestamp, 20, PROC_NAME, 6, level, str);
    fclose(fp);

    return;
}

/***********************************************************
 * Function: 替换字符串.
 * Parameter:
 *      [IO]inStr              :替换前的输入字符串
 *      [I]oldStr              :需要替换的子串
 *      [I]newStr              :替换的新子串
 * Revurns:
 *      none
 **********************************************************/
void strRep( char *inStr, char *oldStr, char *newStr )
{
    int len, i=0, j=0;
    char tmpStr[2000]={0};

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
 * Function: 分离路径和文件名.
 * Parameter:
 *      [O]outPath              :输出路径
 *      [O]outFile              :输出文件名
 *      [I]inStr                :输入字符串(带路径的文件名)
 * Returns:
 *      none
 **********************************************************/
void SeparatePathFile( char *outPath, char *outFile, char *inStr)
{
    char *ptrChar;
    char tmpStr[300]={0},tmpStr2[300]={0};

    sprintf( tmpStr, "%s", inStr );
    ptrChar=strrchr( tmpStr, PATH_CHAR );
    if( ptrChar != NULL )
    {
        sprintf( outFile, "%s", ptrChar + 1 );
        sprintf( outPath, "%s", tmpStr );
        outPath[strlen(tmpStr) - strlen(ptrChar+1)] = '\0';
    }
    else
    {
        sprintf( outFile, "%s", tmpStr);
        sprintf( outPath, ".%c", PATH_CHAR );
    }

    if(outFile[0]=='\0')
        sprintf( outPath, "%s", outFile );

    #ifdef UNIX_OS
    if(outPath[0] == '.')
    {
        getcwd(tmpStr, 300);
        if(strlen(outPath)>1)
        {
            sprintf( tmpStr2, "%s", outPath+1 );
            sprintf(outPath, "%s%s", tmpStr, tmpStr2);
        }
        else
            sprintf(outPath, "%s%c", tmpStr, PATH_CHAR);
    }
    #endif

    return;
}

/***********************************************************
 * Function: 替换环境变量
 * Parameter:
 *      [IO]inDir               :输入字符串
 * Revurns:
 *      none
 **********************************************************/
void RepEnv(char *inDir)
{
    char tmpStr[2000]={0};
    char envStr[100], envStr1[100], *envValue;
    char *p, *p1;

    sprintf(tmpStr, inDir);
    /*查找环境变量*/
    while( (p=strstr(tmpStr, "${"))!=NULL )
    {
        if( (p1=strstr(p, "}"))==NULL)
        {
            return;
        }
        memset(envStr, '\0', sizeof(envStr));
        strncpy(envStr, p+2, strlen(p)-strlen(p1)-2);
        envValue = getenv(envStr);
        sprintf(envStr1, "${%s}", envStr);
        strRep(tmpStr, envStr1, envValue);
    }
    sprintf(inDir, tmpStr);
    return;
}

/***********************************************************
 * Function: 通过对指定文件加锁来确认此进程是否已在运行.
 * Parameter:
 *      [I]inFileName     :锁文件名(带路径)
 * Returns:
 *      0                 :加锁成功，此进程没有在运行
 *      -1                :加锁失败，此进程已经在运行
 **********************************************************/
short CheckUniqueCopy( const char *inFileName )
{
    int pid_file_fd;
    int flag;
    char line[1048];

    #ifdef UNIX_OS
       struct flock lock;
    #endif

    #ifdef UNIX_OS
       /* open the PID file, create if nonexistent */
       pid_file_fd = open(inFileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
       if(pid_file_fd == -1)
       {
           return -1;
       }

       lock.l_type   = F_WRLCK;
       lock.l_start  = 0;
       lock.l_whence = SEEK_SET;
       lock.l_len    = 0;
       flag = fcntl(pid_file_fd, F_SETLK, &lock);
       if(flag < 0)
       {
          if(errno == EACCES || errno == EAGAIN)
          {
             close(pid_file_fd);
             return -1;
          }
          else
          {
             close(pid_file_fd);
             return -1;
          }
       }

       ftruncate(pid_file_fd, 0);
       sprintf(line, "%ld\n", (long)getpid());
       write(pid_file_fd, line, strlen(line));
    #else
       pid_file_fd = _sopen( inFileName, _O_RDWR | O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE );
       if( pid_file_fd == -1 )
       {
           return -1;
       }

       flag = _locking( pid_file_fd, LK_NBLCK, 30L );
       if( flag == -1 )
       {
           close(pid_file_fd);
           return -1;
       }
       sprintf(line, "%ld\n", _getpid());
       _write(pid_file_fd, line, strlen(line));
    #endif

    return 0;
}

/**
*    读取参数文件
*/
int get_config_val(FILE *cfg_fp,char *seg_name,char *val_name,char *ret_val)
{
    char p_buff[1024*2];
    char p_line_info[1024*2];
    char p_seg_name[1024*2];
    char p_ret_val[1024*2];
    char *tmp;
    int tmp_i;
    char err_affix[1000]={0};

    rewind(cfg_fp);
    do
    {
        memset(p_buff,0,1024*2);
        clearerr(cfg_fp);
        if(NULL==fgets(p_buff,1024*2,cfg_fp))
        {
            if(ferror(cfg_fp))
            {
                return 0;
            }
        }
        tmp=p_buff;
        while('\t'==tmp[0] || ' '==tmp[0])
        {
            tmp++;
        }
        strcpy(p_line_info,tmp);
        if(NULL!=(tmp=strchr(p_line_info,'#')))
        {
            tmp[0]=0;
        }
        if(NULL!=(tmp=strchr(p_line_info,'\r')))
        {
            tmp[0]=0;
        }
        if(NULL!=(tmp=strchr(p_line_info,'\n')))
        {
            tmp[0]=0;
        }
        if(strlen(p_line_info))
        {
            if('['==p_line_info[0])
            {
                tmp=strchr(p_line_info,']');
                if(NULL==tmp)
                {
                    sprintf(err_affix,"associated with [%s]",p_line_info);
                    return 0;
                }
                tmp[0]=0;
                strcpy(p_seg_name,p_line_info+1);
            }
            if((size_t)0!=strlen(seg_name) && 0==strcmp(p_seg_name,seg_name))
            {
                tmp_i=strlen(val_name);
                if(0==strncmp(p_line_info,val_name,tmp_i))
                {
                    if(' '==p_line_info[tmp_i] || '\t'==p_line_info[tmp_i] || '='==p_line_info[tmp_i])
                    {
                        tmp=strchr(p_line_info,'\"');
                        if(NULL==tmp)
                        {
                            sprintf(err_affix,"associated with [%s]",p_line_info);
                            return 0;
                        }
                        strcpy(p_ret_val,tmp+1);
                        tmp=strchr(p_ret_val,'\"');
                        if(NULL==tmp)
                        {
                            sprintf(err_affix,"associated with [%s]",p_ret_val);
                            return 0;
                        }
                        tmp[0]=0;
                        strcpy(ret_val,p_ret_val);
                        return 1;
                    }
                }
            }
        }
    }while(!feof(cfg_fp));

    return 0;
}

/***********************************************************
 * Function: 解密Login文件(山西BOSS使用)
 * Parameter:
 *      [I]inPassFile           :密码文件
 *      [O]outUser              :输出用户名
 *      [O]outPass              :输出密码
 * Returns:
 *      0                       :成功
 *      -1                      :打开密码文件出错
 **********************************************************/
short GetLoginInfo(char *inPassFile, char *outUser, char *outPass)
{
    FILE *fp;
    char user[20];
    char pwd[20];
    int    i, k, pointer = 0;
    char key[5];

    sprintf(key, "DFJLT");
    fp = fopen (inPassFile, "r");
    if(fp == NULL)
        return -1;

    fgets (user, 20, fp);
    fgets (user, 20, fp);
    fgets (pwd, 20, fp);
    fgets (pwd, 20, fp);
    fclose (fp);

    pointer = 0;
    i = strlen (user);
    user[i - 1] = 0x00;

    for (k = 0; k < i - 1; k++)
    {
        user[k] = user[k] ^ key[pointer];
        pointer++;
        if (pointer == 5)
            pointer = 0;
    }

    pointer = 0;
    i = strlen(pwd);
    pwd[i-1] = 0x00;
    for (k = 0; k < i - 1; k++)
    {
        pwd[k] = pwd[k] ^ key[pointer];
        pointer++;
        if (pointer == 5)
            pointer = 0;
    }

    sprintf(outUser, "%s", user);
    sprintf(outPass, "%s", pwd);
    return 0;
}

/***********************************************************
 * Function: 根据密码文件得到登陆用户和口令(山西BOSS使用)
 * Parameter:
 *      [I]inPassFile           :密码文件
 *      [I]inDbType             :数据库类别
 *      [O]outUser              :输出用户名
 *      [O]outPass              :输出密码
 *      [O]outServer            :输出服务名
 * Returns:
 *      0                       :成功
 *      -1                      :打开密码文件出错
 **********************************************************/
short DecodePass_Login(char *inPassFile, char *inDbType, char *outUser, char *outPass, char *outServer)
{
    FILE *fp;
    char tmpFile[100]={0};

    if( (fp = fopen (inPassFile, "r")) == NULL)
        return -1;

    if( (0==get_config_val(fp,inDbType,"DB_STRING",outServer)) ||
        (0==get_config_val(fp,inDbType,"DB_FILE",tmpFile)) )
    {
        fclose (fp);
        return -1;
    }
    fclose (fp);

    RepEnv(outServer);
    RepEnv(tmpFile);
    if(GetLoginInfo(tmpFile, outUser, outPass)<0)
        return -1;

    return 0;
}

/***********************************************************
 * Function: 删除字符串中两端的所有指定字符.
 * Parameter:
 *      [IO]inStr              :输入字符串
 *      [I]beCutedChar         :要删除的字符
 *      [I]alignMode           :对齐模式 R 右对齐,C 居中,L 左对齐
 * Returns:
 *      none
 **********************************************************/
void CutChar( char *inStr, char beCutedChar, char alignMode )
{
    int   i, len;

    /*删除右边字符*/
    if((alignMode=='L')||(alignMode=='C'))
    {
        len = strlen(inStr);
        for(i=len-1; i>=0; i--)
        {
            if(inStr[i] != beCutedChar)
                break;
        }
        if(i < len-1)
            inStr[i+1] = '\0';
    }

    /*删除左边字符*/
    if((alignMode=='R')||(alignMode=='C'))
    {
        len = strlen(inStr);
        for(i=0; i<len; i++)
        {
            if(inStr[i] != beCutedChar)
                break;
        }
        if(i>0)
        {
            memmove(inStr, inStr+i, len-i);
            inStr[len-i] = '\0';
        }
    }
    return;
}

/*排序*/
int mtime_cmp( const void *arg1, const void *arg2 )
{
    SORT_FILE* sf1 = (SORT_FILE*)arg1;
    SORT_FILE* sf2 = (SORT_FILE*)arg2;
    return memcmp( &sf1->mtime, &sf2->mtime, sizeof(sf1->mtime) );
}

/***********************************************************
 * Function: 获取指定路径下的文件名列表.
 * Parameter:
 *      [I]fileList            :存放文件名列表的指针
 *      [I]inDir               :路径
 *      [I]inMatchStr          :文件名通配符
 *      [I]nameLen             :每个文件名的最大长度
 *      [I]maxNum              :本次要取得的最多文件数
 * Returns:
 *      none
 **********************************************************/
void GetFileList(char *fileList, char *inDir, char *inMatchStr,
                  int nameLen, int maxNum )
{
    char tmpDir[300], tmpStr[300];
    int file_num=0;    

    /*UNIX处理*/
#ifdef UNIX_OS   
    /*DIR  *dirPtr;
    struct dirent *direntPtr=NULL;
    struct stat file_st;*/
    
    SDIR *  dirPtr=NULL;
    sdirent * direntPtr;
    struct sstat  file_st;
            
    int i;
    SORT_FILE *fSort;   /*排序指针*/

    if( (dirPtr = sopendir(inDir, &strSdfs) ) == NULL )
        return;
    if( (direntPtr = sreaddir(dirPtr, &strSdfs)) == NULL )
    {
        sclosedir(dirPtr, &strSdfs);
        return;
    }
  
    fSort = (SORT_FILE *)malloc( maxNum * sizeof(SORT_FILE) );
    do
    {
    	 
        sprintf(tmpStr, "%s", direntPtr->d_name);
			
				if(strncmp(direntPtr->d_name,".",1)==0 || strcmp(direntPtr->d_name,"..")==0 )
			 	{
			     continue;
			 	}
				
			 	/*if(fnmatch(inDir,direntPtr->d_name,0) != 0)
       	{
        	continue;
       	}*/
                        

        sprintf(tmpDir,"%s%s", inDir, direntPtr->d_name);
        sstat(tmpDir,&file_st,&strSdfs);

        /*if( !(S_ISREG(file_st.st_mode)))
            continue;*/

        sprintf( (fSort+file_num)->file_name, "%s", tmpStr);
       /* (fSort+file_num)->mtime = file_st.st_mtime;*/
        file_num++;
        if(file_num >= maxNum)
            break;
    }while ((direntPtr = sreaddir( dirPtr,&strSdfs)) != NULL);
    sclosedir(dirPtr,&strSdfs);

    /*按时间排序*/
    if(file_num>0)
    {
        qsort (fSort, file_num, sizeof(SORT_FILE), mtime_cmp); 
    }
    for(i=0; i<file_num; i++)
        strcpy(fileList + i*nameLen, (fSort+i)->file_name);
    /*释放内存*/
    free(fSort);

#else    /*WINDOWS处理*/
    long handle;
    int done;
    struct _finddata_t tmpFile;
       
    sprintf(tmpDir,"%s%s", inDir, inMatchStr);
    if( (handle = _findfirst(tmpDir, &tmpFile)) <= 0 )
        return;
    do
    {
        sprintf(tmpStr, "%s", tmpFile.name);
        if( (strcmp(tmpStr,".")==0)||(strcmp(tmpStr,"..")==0) )
            continue;
   
        strcpy(fileList + file_num*nameLen, tmpStr);
        file_num++;
        if(file_num >= maxNum)
            break;
    }while ((done = _findnext(handle,&tmpFile)) == 0);
    _findclose(handle);
#endif

    return;
}

/***********************************************************
 * Function: 替换环境变量，并检查目录有效性
 * Parameter:
 *      [I]outStr               :信息描述
 *      [IO]inDir               :目录名
 * Revurns:
 *      0                       :合法目录
 *      -1                      :非法目录
 **********************************************************/
short CheckValidPath(char *outStr, char *inDir)
{
    #ifdef UNIX_OS
    /*DIR  *dirPtr;

    RepEnv(inDir);
    dirPtr = sopendir( inDir,&strSdfs );
    if( dirPtr == NULL)
    {
        sprintf(outStr, "%s is a invalid path.", inDir);
        return -1;
    }
    sclosedir(dirPtr,&strSdfs);*/
    #endif

    if( inDir[strlen(inDir)-1] != PATH_CHAR )
        sprintf(inDir, "%s%c", inDir, PATH_CHAR);

    return 0;
}

/*将指定文件中的文件名列表改名，然后删掉指定文件*/
void RenameTempFile(char *path, char *fileName)
{
    FILE *fp;
    char tmpStr[300];
    char str[1000];
    char oldFile[300], newFile[300];

    sprintf(tmpStr, "%s%s", path, fileName);
    if( (fp=fopen(tmpStr, "r")) == NULL )
        return;

    while( !feof(fp) )
    {
        memset( str, 0, sizeof(str) );
        fgets( (char *)str, sizeof(str), fp );
        if( strlen(str)<5)
            continue;
        sscanf(str, "%s\t%s\n", oldFile, newFile);
        if(rename(oldFile, newFile)!=0)
        {
            sprintf(str, "mv %s %s", oldFile, newFile);
            system(str);
        }
    }
    fclose(fp);
    remove(tmpStr);

    return;
}

/*删除指定路径下的临时文件*/
void RemoveTempFile(char *inDir, char *fileName)
{
    #ifdef UNIX_OS
    char matchStr[300], tmpStr[300];
    DIR  *dirPtr;
    struct dirent *direntPtr=NULL;
    struct stat file_st;

    sprintf(matchStr, "%s*", fileName);
    if( (dirPtr = opendir(inDir) ) == NULL )
        return;
    if( (direntPtr = readdir(dirPtr)) == NULL )
    {
        closedir(dirPtr);
        return;
    }
    do
    {
        sprintf(tmpStr, "%s", direntPtr->d_name);
        if( (direntPtr->d_ino == 0) || (strcmp(tmpStr, ".") == 0) ||
            (strcmp(tmpStr, "..") == 0) || (fnmatch(matchStr, tmpStr,0) != 0) )
            continue;
        sprintf(tmpStr,"%s%s", inDir, direntPtr->d_name);
        stat(tmpStr,&file_st);
        if( !(S_ISREG(file_st.st_mode)))
            continue;
        remove(tmpStr);
    }while ((direntPtr = readdir( dirPtr)) != NULL);
    closedir(dirPtr);
    #endif

    return;
}

/***********************************************************
 * Function: 分割字符串
 * Parameter:
 *        [I]str              :字符串
 *        [I]delimiter        :分割符
 *        [O]outArray         :输出数组
 *        [O]outLen           :数组长度
 * Returns:
 *        none
 **********************************************************/
void SplitString(char *str, char delimiter, char outArray[][100], int *outLen)
{
    int i, j=0, k=0, len;
    size_t s;

    s = sizeof(outArray[0]);
    len = strlen(str);
    memset(outArray[0], '\0', s);
    for(i=0; i<len; i++)
    {
        if(str[i] != delimiter)
            outArray[j][k++] = str[i];
        else
        {
            memset(outArray[++j], '\0', s);
            k = 0;
        }
    }
    *outLen = j + 1;
}

/*****************************************************************/
/*!
    * @brief 分割字符串,多个连续的分割符视为多个
    * @param [in]  inStr        输入字符串
    * @param [in]  cc           分割符
    * @param [out] outLen       数组长度
    * @param [in]  maxLen       最大输出长度
    * @param [out] iArr         输出每个字符串的偏移位置
    * @return 无
    */
void SplitStrM(char *inStr, char *cc, int *outLen, int maxLen, int iArr[])
{
    int j;
    char *p, *p1;

    iArr[0] = 0;
    j = 1;

    p1 = inStr;
    if(0 == cc[1])
    {
        while(NULL != (p = strchr(p1, cc[0])) )
        {
            p1 = p + 1;
            p[0] = 0;
            iArr[j] = p1 - inStr;
            j++;        
        }
        *outLen = j;
        return;
    }
    else
    {
        int len = strlen(cc);
        while(NULL != (p = strstr(p1, cc)) )
        {
            p1 = p + len;
            p[0] = 0;
            iArr[j] = p1 - inStr;
            j++;        
        }
        *outLen = j;
        return;
    }
}



/*****************************************************************/
/*!
    * @brief 分割字符串,
    * @param [in]  sstr        输入字符串
    * @param [in]  t           分割符
    * @param [in] dstr         取出sstr的第fn个字段并将这个字段赋给dstr
    * @param [in]  fn          第几个分割字段
    * @return 无
    */
int getTextField(char * dstr,char * sstr,char t,int fn)
{
	int i;
	char *tps , *tpe;
	char lstr[256+1];
	
	tps = sstr;
	
	for ( i = 1;i < fn;i ++ )
	{
		tps = strchr( tps , t );
		if ( tps == NULL ) return -1;
		tps ++ ;
	}
	
	tpe = strchr( tps , t );
	if( tpe == NULL ) 
		i = strlen( tps );
	else              
		i = tpe - tps;
		
	memcpy( lstr , tps , i );
	lstr[i] = '\0';
	
	while ( lstr[strlen( lstr ) - 1] == '\n' ) 
		lstr[strlen( lstr ) - 1] = '\0';
		
	strcpy( dstr , lstr );
	
	return 1;
}

/* 取系统时间，格式为YYYYMMDDHHMISS */
char * getSysDate(char * sysDate)
{
time_t sec;
struct tm t;

sec = time(NULL);
localtime_r(&sec,&t);
sprintf(sysDate,"%04d%02d%02d%02d%02d%02d",\
	t.tm_year+1900,t.tm_mon+1,t.tm_mday,\
	t.tm_hour,t.tm_min,t.tm_sec);
sysDate[14]='\0';
return sysDate;
}

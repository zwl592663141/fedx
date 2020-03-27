#include "fedx.h"      /* 引用头文件 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "fedx_tool_public.h"

#define MAX_PATH_LEN                            1024                                            /* 路径最大长度 */
#define MAX_FIELDS_LEN                 100                                                    /* 字段名最大长度 */
#define MAX_VALUE_LEN                         200                                                     /* 字段值最大长度 */

void help()
{
	printf("使用方法：setf 入口文件名 出口文件名 字段值\n\n");
	printf("eg：setf in_file out_file field1=value1  field2=value2\n\n");
}

/**********************************************************************************************/
/*
* 函数介绍：判断文件是否存在
* 输入参数：szFile: 文件名
* 输出参数： 无
* 返回值  ：0:正常结束;其它:错误退出
*/
int FileIsExist( char * szFile )
{
	if( 0 == access( szFile, F_OK ) )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
* 函数介绍：判断文件是否为目录文件
* 输入参数：szDirName: 文件名
* 输出参数： 无
* 返回值  ：0:正常结束;其它:错误退出
*/
int IsDir( char * szDirName )
{
	struct stat statMode;

	if ( (lstat( szDirName, &statMode ) == 0) && (statMode.st_mode & S_IFDIR) )
	{
		return (0);
	}
	else
	{
		return (-1);
 	}
}



/**********************************************************************************************/
/*
* 函数介绍：主函数
* 输入参数：int argc: 输入参数个数, char *argv[]:输入参数值
* 输出参数：无
* 返回值  ：0:正常结束;其它:错误退出
*/
int main(int argc, char *argv[])
{
	if (argc <=2)
	{
		help();
		exit(0);
	}


  char fieldsName[MAX_FIELDS_LEN];
  char fieldsValue[MAX_VALUE_LEN];
  char fileName[MAX_PATH_LEN];
  char outFileName[MAX_PATH_LEN];
  
  char tmp[MAX_FIELDS_LEN+MAX_VALUE_LEN+2];
  int i=0;
  fedx_param s_fedx_param;   
  FILE *fp=NULL;
  FILE *outFp=NULL;
  char *sp_value=NULL ;
  char *szFmtPath=NULL;
  char szFmtFile[512+1];

  szFmtPath = getenv("FEDX_CONFIG_PATH");

  sprintf(fileName,"%s",argv[1]);
  sprintf(outFileName,"%s",argv[2]);
  if((IsDir(szFmtPath)!=0))
  {
   	printf("dir is not %s\n", szFmtPath);
   	exit( -1);

  }
  else
  {
    if(szFmtPath[strlen(szFmtPath) -1] !='/')
    strcat(szFmtPath,"/");
    memset(szFmtFile, 0, sizeof(szFmtFile));
    sprintf(szFmtFile, "%s", szFmtPath);
    strcat(szFmtFile,  getenv("FEDX_CONFIG_NAME"));
  }
  
  if(FileIsExist(fileName))
  {
    printf("打开 [%s] 文件失败 \n",fileName);
    exit( -1);
  }
  
  
  /*if (fedx_getConf("/bill_pkg/config/detail/format/v3.00/config.xml")!=0)*/
  if (fedx_getConf(szFmtFile)!=0)/*全局配置变量初始化*/
  {
    printf("fedx_getConf失败 \n\n");
    exit(-1);
  }

  fedx_paramInit(&s_fedx_param);  
  
  fp=fopen(fileName,"r");
  if(fp==NULL)
  {
    printf("fopen[%s]失败 \n\n",fileName);
    exit(-1);
  }

  outFp=fopen(outFileName,"aw+");
  if(fp==NULL)
  {
    printf("fopen outFileName[%s] 失败 \n\n",outFileName);
    exit(-1);
  }

  while( fedx_loadFile(&s_fedx_param,fp)!=0)
  {

		for(i=3; i<argc;i++)
		{       

    	memset(fieldsName, 0, sizeof(fieldsName));
    	memset(fieldsValue, 0, sizeof(fieldsValue));
    	memset(tmp, 0, sizeof(tmp));
    	sprintf(tmp,"%s",argv[i]);
    	
    	sp_value=strtok(tmp, "=");
    	 sprintf(fieldsName,"%s",sp_value); 
    	
    	while(sp_value)
    	{
    		/*memcpy(fieldsValue, sp_value, sizeof(sp_value));*/
				sprintf(fieldsValue,"%s", sp_value);
    	        
				sp_value=strtok(NULL, "=");
    	}
    	/*printf("argc=%s, i=%d, fieldsName=%s, fieldsValue=%s\n", tmp, i, fieldsName, fieldsValue);*/
    	
    	if(fedx_setFieldValueByName(&s_fedx_param, fieldsValue,fieldsName, NULL))
    	{
				printf("fedx_setFieldValueByName[fieldsName=%s][fieldsValue=%s]失败 \n\n",fieldsName, fieldsValue); 
				break;
    	}

		}

		if(fedx_write_record(&s_fedx_param,outFp)!=0)
		{
			printf("fedx_write_recordwrong\n");
 			break;
		}
                  
		fflush(outFp);
		fedx_freeRecord(&s_fedx_param);

  }

  fclose(fp);
  fclose(outFp);

  fedx_freeGlobalMem();
  
  return 0;
}

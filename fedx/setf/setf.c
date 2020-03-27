#include "fedx.h"      /* ����ͷ�ļ� */
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

#define MAX_PATH_LEN                            1024                                            /* ·����󳤶� */
#define MAX_FIELDS_LEN                 100                                                    /* �ֶ�����󳤶� */
#define MAX_VALUE_LEN                         200                                                     /* �ֶ�ֵ��󳤶� */

void help()
{
	printf("ʹ�÷�����setf ����ļ��� �����ļ��� �ֶ�ֵ\n\n");
	printf("eg��setf in_file out_file field1=value1  field2=value2\n\n");
}

/**********************************************************************************************/
/*
* �������ܣ��ж��ļ��Ƿ����
* ���������szFile: �ļ���
* ��������� ��
* ����ֵ  ��0:��������;����:�����˳�
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
* �������ܣ��ж��ļ��Ƿ�ΪĿ¼�ļ�
* ���������szDirName: �ļ���
* ��������� ��
* ����ֵ  ��0:��������;����:�����˳�
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
* �������ܣ�������
* ���������int argc: �����������, char *argv[]:�������ֵ
* �����������
* ����ֵ  ��0:��������;����:�����˳�
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
    printf("�� [%s] �ļ�ʧ�� \n",fileName);
    exit( -1);
  }
  
  
  /*if (fedx_getConf("/bill_pkg/config/detail/format/v3.00/config.xml")!=0)*/
  if (fedx_getConf(szFmtFile)!=0)/*ȫ�����ñ�����ʼ��*/
  {
    printf("fedx_getConfʧ�� \n\n");
    exit(-1);
  }

  fedx_paramInit(&s_fedx_param);  
  
  fp=fopen(fileName,"r");
  if(fp==NULL)
  {
    printf("fopen[%s]ʧ�� \n\n",fileName);
    exit(-1);
  }

  outFp=fopen(outFileName,"aw+");
  if(fp==NULL)
  {
    printf("fopen outFileName[%s] ʧ�� \n\n",outFileName);
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
				printf("fedx_setFieldValueByName[fieldsName=%s][fieldsValue=%s]ʧ�� \n\n",fieldsName, fieldsValue); 
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

/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	function.c
author:		caoshun
version:	1.20
date:		2006-01-09
description:
			��ҵ���޹صĹ��ܺ���-�����ļ�
others:
history:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "function.h"

#ifdef DEBUG
#define AUTOCALL_DEBUG printf("DEBUG->FILE[%s]LINE[%d]ERRNO[%d]ERRMSG[%s]\n", __FILE__, __LINE__, errno, strerror(errno));fflush(stdout);
#else
#define AUTOCALL_DEBUG ;
#endif

/*
function:   func_get_db_login()
description:
            �����ݿ��¼�����ļ���ȡ�����е��û���������
Input:      
            filename:�����ļ���
            key:���ܵ���Կ
Output:		
			usernmae:�������û���
			password:����������
Return:     0:�ɹ���other:���ɹ�
others: 
*/
int func_get_db_login(char* filename, char *key, char* username,char* password)
{
	FILE *fp;
	
	fp = fopen(filename, "r");
	if(fp == NULL)
	{
		AUTOCALL_DEBUG;
		return -1;
	}

	fgets(username,32,fp);
	fgets(username,32,fp);
	fgets(password,32,fp);
	fgets(password,32,fp);
	
	fclose(fp);
    
    /* �����Ľ��н��� */
	func_decode_password(key, username);
	func_decode_password(key, password);

	return 0;
}

/*
function:   func_decode_password()
description:
            �����Ľ��н���
Input:      
            key:���ܵ���Կ
            encode:��������
Output:
			encode:��������
Return:
others: 
*/
void func_decode_password(char* key, char* encode)
{
	int i, len, pointer;

	/* �Զ�������㷨 */
	pointer = 0;
	len = strlen(encode);
	encode[len-1]='\0';

	for (i=0; i<len-1; i++)
	{
		encode[i] = encode[i] ^ key[pointer];
		pointer++;
		if (pointer == 5)
		{
			pointer = 0;
		}
	}
}

/*
function:   func_chmod_rwx()
description:
            Ϊ���ڵ��ļ��Ӷ�дִ������
Input:      
            filename:��������ļ���
Output:
Return:
			0:�ɹ���other:���ɹ�
others: 
*/
int func_chmod_rwx(char *filename)
{
	if(access(filename, F_OK)!=0)
	{
		AUTOCALL_DEBUG;
		return -1;
	}

	if(chmod(filename, S_IRWXU|S_IRWXG|S_IROTH)!=0)
	{
		AUTOCALL_DEBUG;
		return -1;
	}

	return 0;
}

/*
function:   func_get_module()
description:
			��ȡģ�������Ϣ
Input:
			obj:ȡģ�����ڲ����ݽṹ
			number:�����ȡģ��������ֵ
Output:
Return:
			-1:����
			>=0:����ȡģ�����Ӧ������
others:
*/
int func_get_module(struct func_stModule *obj, int number)
{
	int i;
	int module_value;

	module_value = number % obj->module_number;

	for(i = 0; i <obj->count; i++)
	{
		if(module_value == obj->data[i].number)
		{
			return i;
		}
	}

	AUTOCALL_DEBUG;
	return -1;
}

/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	function.c
author:		caoshun
version:	1.20
date:		2006-01-09
description:
			与业务无关的功能函数-代码文件
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
            打开数据库登录加密文件，取出其中的用户名与密码
Input:      
            filename:加密文件名
            key:解密的密钥
Output:		
			usernmae:解码后的用户名
			password:解码后的密码
Return:     0:成功，other:不成功
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
    
    /* 对密文进行解码 */
	func_decode_password(key, username);
	func_decode_password(key, password);

	return 0;
}

/*
function:   func_decode_password()
description:
            对密文进行解码
Input:      
            key:解密的密钥
            encode:编码数据
Output:
			encode:解码数据
Return:
others: 
*/
void func_decode_password(char* key, char* encode)
{
	int i, len, pointer;

	/* 自定义解码算法 */
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
            为存在的文件加读写执行属性
Input:      
            filename:需操作的文件名
Output:
Return:
			0:成功，other:不成功
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
			求取模后相关信息
Input:
			obj:取模操作内部数据结构
			number:需进行取模操作的数值
Output:
Return:
			-1:错误
			>=0:返回取模结果对应的内容
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

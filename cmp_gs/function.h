/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	function.h
author:		caoshun
version:	1.20
date:		2006-01-09
description:
			与业务无关的功能函数-H代码文件
others:
history:
			20051202 caoshun 删除func_get_datetime()。
			20060109 caoshun 删除大量其它函数。
*/

#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#if defined(__cplusplus)
extern "C" {
#endif

/*func_get_module 函数内部数据结构*/
struct func_stModuleList
{
	int number;				/*取模结果*/
	char content[256+1];	/*取模结果对应的内容*/
};

/*func_get_module 函数内部数据结构*/
struct func_stModule
{
	int count;							/*取模配置表记录总数*/
	int module_number;					/*取模操作系数*/
	struct func_stModuleList data[32];	/*取模配置表*/
};

int func_get_db_login(char* filename, char *key, char* username,char* password);
int func_chmod_rwx(char *filename);
void func_decode_password(char* key, char* encode);
int func_get_module(struct func_stModule *obj, int number);

#if defined(__cplusplus)
}
#endif

#endif


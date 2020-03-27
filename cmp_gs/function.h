/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	function.h
author:		caoshun
version:	1.20
date:		2006-01-09
description:
			��ҵ���޹صĹ��ܺ���-H�����ļ�
others:
history:
			20051202 caoshun ɾ��func_get_datetime()��
			20060109 caoshun ɾ����������������
*/

#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#if defined(__cplusplus)
extern "C" {
#endif

/*func_get_module �����ڲ����ݽṹ*/
struct func_stModuleList
{
	int number;				/*ȡģ���*/
	char content[256+1];	/*ȡģ�����Ӧ������*/
};

/*func_get_module �����ڲ����ݽṹ*/
struct func_stModule
{
	int count;							/*ȡģ���ñ��¼����*/
	int module_number;					/*ȡģ����ϵ��*/
	struct func_stModuleList data[32];	/*ȡģ���ñ�*/
};

int func_get_db_login(char* filename, char *key, char* username,char* password);
int func_chmod_rwx(char *filename);
void func_decode_password(char* key, char* encode);
int func_get_module(struct func_stModule *obj, int number);

#if defined(__cplusplus)
}
#endif

#endif


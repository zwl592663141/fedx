/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	IniFile.c
author:		caoshun
version:	1.04
date:		2005-11-15
description:
			ר�����ڴ���INI�����ļ�
others:
history:
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "IniFile.h"

#ifdef DEBUG
#define AUTOCALL_DEBUG printf("DEBUG->FILE[%s]LINE[%d]ERRNO[%d]ERRMSG[%s]\n", __FILE__, __LINE__, errno, strerror(errno));fflush(stdout);
#else
#define AUTOCALL_DEBUG ;
#endif

/*
function:   IniFile__RTrim()
description:
            �ڲ�������ɾ���ַ��������Ч�ַ�
Input:      
            buffer:�ַ���
Output:		
Return:     ��������ַ���
others: 
*/
char *IniFile__RTrim(char *buffer)
{
	char *p_buffer;

	if(*buffer == '\0')
	{
		return buffer;
	}

	p_buffer = buffer;
	while(*p_buffer == ' ' || *p_buffer == '\t' || *p_buffer == '\r' || *p_buffer == '\n')
	{
		p_buffer++;
	}
	strcpy(buffer, p_buffer);
	return buffer;
}

/*
function:   IniFile__LTrim()
description:
            �ڲ�������ɾ���ַ����ұ���Ч�ַ�
Input:      
            buffer:�ַ���
Output:		
Return:     ��������ַ���
others: 
*/
char* IniFile__LTrim(char *buffer)
{
	int len;

	if(*buffer == '\0')
	{
		return buffer;
	}

	len = strlen(buffer);
	while(buffer[len-1] == ' ' || buffer[len-1] == '\t' || buffer[len-1] == '\r' || buffer[len-1] == '\n')
	{
		len--;
	}
	buffer[len] = '\0';
	return buffer;
}

/*
function:   IniFile__Trim()
description:
            �ڲ�������ɾ���ַ���������Ч�ַ�
Input:      
            buffer:�ַ���
Output:		
Return:     ��������ַ���
others: 
*/
char* IniFile__Trim(char *buffer)
{
	IniFile__LTrim(buffer);
	return IniFile__RTrim(buffer);
}

/*
function:   IniFile__Analyze1()
description:
            �ڲ�����������GROUP����
Input:      
            buffer:�ַ���
Output:		
Return:     GROUP's name
others: 
*/
int IniFile__Analyze1(char *buffer, char *result)
{
	char temp[INIFILE_BUFFER_MAX_LEN+1];
	char *p_buffer;
	int  len;
	
	if(*buffer != '[')
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	if(strlen(buffer) < 3)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	strcpy(temp, buffer+1);
	IniFile__Trim(temp);
	if(strlen(temp) == 0)
	{
		AUTOCALL_DEBUG;
		return -1;
	}

	p_buffer = temp + strlen(temp) - 1;
	if(*p_buffer != ']')
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	*p_buffer = '\0';
	IniFile__Trim(temp);
	
	len = strlen(temp);
	if(len < 1 || len > INIFILE_NAME_MAX_LEN)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	if(strchr(temp, '[') != NULL)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	if(strchr(temp, ']') != NULL)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	strcpy(result, temp);
	
	return 0;
}

/*
function:   IniFile__Analyze2()
description:
            �ڲ�����������KEY����
Input:      
            buffer:�ַ���
Output:		
Return:     KEY's name
others: 
*/
int IniFile__Analyze2(char *buffer, char *result1, char *result2)
{
	int len;
	char *p_buffer;
	char temp[INIFILE_BUFFER_MAX_LEN];
	
	strcpy(temp, buffer);
	p_buffer = strchr(temp, '=');
	if(p_buffer == NULL)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	*p_buffer = '\0';
	IniFile__Trim(temp);
	
	len = strlen(temp);
	if(len < 1 || len > INIFILE_NAME_MAX_LEN)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	strcpy(result1, temp);
	
	p_buffer = strchr(buffer, '=');
	p_buffer ++;
	strcpy(temp, p_buffer);
	IniFile__Trim(temp);
	
	len = strlen(temp);
	if(len > INIFILE_VALUE_MAX_LEN)
	{
		AUTOCALL_DEBUG;
		return -1;
	}	
	strcpy(result2, temp);
	
	return 0;
}

/*
function:   IniFile_Key_SetName()
description:
            ����KEY������
Input:      
            obj:key����
            data:key's name
Output:		
Return:     
others: 
*/
void IniFile_Key_SetName(struct stIniFile_Key *obj, char *data)
{
	strcpy(obj->name, data);
}

/*
function:   IniFile_Key_SetValue()
description:
            ����KEY��ֵ
Input:      
            obj:key����
            data:key's value
Output:		
Return:     
others: 
*/
void  IniFile_Key_SetValue(struct stIniFile_Key *obj, char *data)
{
	strcpy(obj->value, data);
}

/*
function:   IniFile_Key_GetName()
description:
            ȡKEY������
Input:      
            obj:key����
Output:		
Return:     key's name
others: 
*/
char *IniFile_Key_GetName(struct stIniFile_Key *obj)
{
	return obj->name;
}

/*
function:   IniFile_Key_GetValue()
description:
            ȡKEY��ֵ
Input:      
            obj:key����
Output:		
Return:     key's value
others: 
*/
char *IniFile_Key_GetValue(struct stIniFile_Key *obj)
{
	return obj->value;
}

/*
function:   IniFile_Group_AppendKey()
description:
            ��GROUP������KEY
Input:      
            group_obj:group����
            name:key's name
            value:key's value
Output:		
Return:     0:�ɹ�,!0:ʧ��
others: 
*/
int   IniFile_Group_AppendKey(struct stIniFile_Group *group_obj, char *name, char *value)
{
	struct stIniFile_Key *new_key;

	if(group_obj->key_number+1 >= INIFILE_KEY_MAX_NUMBER)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	new_key = &(group_obj->key_list[group_obj->key_number]);
	strcpy(new_key->name, name);
	strcpy(new_key->value, value);
	group_obj->key_number++;
	
	return 0;
}

/*
function:   IniFile_Group_SetName()
description:
            ����GROUP������
Input:      
            obj:group����
            name:group's name
Output:		
Return:     
others: 
*/
void  IniFile_Group_SetName(struct stIniFile_Group *obj, char *data)
{
	strcpy(obj->name, data);
}

/*
function:   IniFile_Group_GetName()
description:
            ȡGROUP������
Input:      
            obj:group����
Output:		
Return:     group's name 
others: 
*/
char *IniFile_Group_GetName(struct stIniFile_Group *obj)
{
	return obj->name;
}

/*
function:   IniFile_Group_CountKey()
description:
            ȡGROUP��KEY���ܺ�
Input:      
            obj:group����
Output:		
Return:     key�ĸ���
others: 
*/
int   IniFile_Group_CountKey(struct stIniFile_Group *obj)
{
	return obj->key_number;
}

/*
function:   IniFile_Group_GetKeyByIndex()
description:
            ����������GROUP��ȡKEY
Input:      
            group_obj:group����
       		n:����
Output:		key_obj:key���������
Return:     0:�ɹ�,!0ʧ��
others: 
*/
int   IniFile_Group_GetKeyByIndex(struct stIniFile_Group *group_obj, struct stIniFile_Key **key_obj, int n)
{
	if(n < 0)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	if(n > group_obj->key_number-1)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	*key_obj = &(group_obj->key_list[n]);
	
	return 0;
}

/*
function:   IniFile_Group_GetKeyByName()
description:
            ��������GROUP��ȡKEY
Input:      
            group_obj:group����
       		data:key's name
Output:		key_obj:key���������
Return:     0:�ɹ�,!0ʧ��
others: 
*/
int IniFile_Group_GetKeyByName(struct stIniFile_Group *group_obj, struct stIniFile_Key **key_obj, char *data)
{
	int n;
	struct stIniFile_Key *new_obj;
/*printf("group_obj->key_number=[%d]\n",group_obj->key_number);	*/
	for(n = 0; n < group_obj->key_number; n++)
	{
		new_obj = &(group_obj->key_list[n]);
/*printf("==[%s][%s]--[%d]\n",new_obj->name,data,n);*/
		if(strcmp(new_obj->name, data) == 0)
		{
			*key_obj = new_obj;
			return 0;
		}
	}
	
	return -1;
}

/*
function:   IniFile_File_AppendGroup()
description:
            ��FILE������GROUP
Input:      
            file_obj:file����
       		data:name's name
Output:		
Return:     0:�ɹ�,!0ʧ��
others: 
*/
int IniFile_File_AppendGroup(struct stIniFile_File *file_obj, char *name)
{
	struct stIniFile_Group *new_group;
	if(file_obj->group_number+1 >= INIFILE_GROUP_MAX_NUMBER)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	new_group = &(file_obj->group_list[file_obj->group_number]);
	strcpy(new_group->name, name);
	file_obj->group_number ++;

	return 0;
}

/*
function:   IniFile_ReadFile()
description:
            ��INI�ļ����ݵ�һ��FILE��
Input:      
            obj:file����
       		filename:�ļ���
Output:		
Return:     0:�ɹ�,!0ʧ��
others: 
*/
int IniFile_ReadFile(struct stIniFile_File *obj, char *filename)
{
	struct stIniFile_Group *current_group;
	int iRcode;
	FILE *fp;
	char buffer[INIFILE_BUFFER_MAX_LEN+1];
	char name[INIFILE_NAME_MAX_LEN+1];
	char value[INIFILE_VALUE_MAX_LEN+1];	
	
	current_group = NULL;


	fp = fopen(filename, "r");
	if(fp == NULL)
	{
		AUTOCALL_DEBUG;
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));
	fgets(buffer, INIFILE_BUFFER_MAX_LEN, fp);
	while(!feof(fp))
	{
		switch(*buffer)
		{
			/* ע�� */
			case '#' :
				break;
			/* ���� */
			case ' ' :
			case '\t':
			case '\r':
			case '\n':
				if(strcmp(IniFile__Trim(buffer), "") != 0)
				{
					AUTOCALL_DEBUG;
					return -1;
				}
				
				break;
			/* GROUPԪ�� */
			case '[' :
				iRcode = IniFile__Analyze1(buffer, name);
			
				if(iRcode)
				{
					AUTOCALL_DEBUG;
					return -1;
				}
				
				iRcode = IniFile_File_AppendGroup(obj, name);
				if(iRcode)
				{
					AUTOCALL_DEBUG;
					return -1;
				}
				iRcode = IniFile_File_GetGroupByIndex(obj, &current_group, obj->group_number-1);
				if(iRcode)
				{
					AUTOCALL_DEBUG;
					return -1;
				}
				break;
			/* KEYԪ�� */
			default:
				iRcode = IniFile__Analyze2(buffer, name, value);
				if(iRcode)
				{
					AUTOCALL_DEBUG;
					return -1;
				}
				if(current_group == NULL)
				{
					AUTOCALL_DEBUG;
					return -1;
				}
				iRcode = IniFile_Group_AppendKey(current_group, name, value);
				if(iRcode)
				{
					AUTOCALL_DEBUG;
					return -1;
				}
				break;
		}
		
		memset(buffer, 0, sizeof(buffer));
		fgets(buffer, INIFILE_BUFFER_MAX_LEN, fp);
	}
	
	if(fclose(fp) == EOF)
	{
		AUTOCALL_DEBUG;
		return -1;
	}

	return 0;
}

/*
function:   IniFile_WriteFile()
description:
            дFILE�е����ݵ�INI�ļ���
Input:      
            obj:file����
       		filename:�ļ���
Output:		
Return:     0:�ɹ�,!0ʧ��
others: 
*/
int IniFile_WriteFile(struct stIniFile_File *obj, char *filename)
{
	FILE *fp;
	struct stIniFile_Group *group_obj;
	struct stIniFile_Key *key_obj;
	int group_count, key_count, group_loop, key_loop;
	int iRcode;
	char buffer[INIFILE_BUFFER_MAX_LEN+1];

	fp = fopen(filename, "w");
	if(fp == NULL)
	{
		AUTOCALL_DEBUG;
		return -1;
	}

	group_count = IniFile_File_CountGroup(obj);
	for(group_loop = 0; group_loop < group_count; group_loop++)
	{
		iRcode = IniFile_File_GetGroupByIndex(obj, &group_obj, group_loop);
		if(iRcode)
		{
			AUTOCALL_DEBUG;
			fclose(fp);
			return -1;
		}

		sprintf(buffer, "[%s]\n", group_obj->name);
		if(fputs(buffer, fp) == EOF)
		{
			AUTOCALL_DEBUG;
			fclose(fp);
			return -1;
		}

		key_count = IniFile_Group_CountKey(group_obj);
		for(key_loop = 0; key_loop < key_count; key_loop++)
		{
			iRcode = IniFile_Group_GetKeyByIndex(group_obj, &key_obj, key_loop);
			if(iRcode)
			{
				AUTOCALL_DEBUG;
				fclose(fp);
				return -1;
			}

			sprintf(buffer, "%s=%s\n", key_obj->name, key_obj->value);
			if(fputs(buffer, fp) == EOF)
			{
				AUTOCALL_DEBUG;
				fclose(fp);
				return -1;
			}	
		}
	}

	if(fclose(fp) == EOF)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	return 0;
}

/*
function:   IniFile_File_CountGroup()
description:
            ȡFILE��GROUP���ܺ�
Input:      
            obj:file����
Output:		
Return:     group�ĸ���
others: 
*/
int IniFile_File_CountGroup(struct stIniFile_File *obj)
{
	return obj->group_number;
}

/*
function:   IniFile_File_GetGroupByIndex()
description:
            ����������FILE��ȡGROUP
Input:      
            file_obj:file����
            n:����
Output:		group_obj:group���������
Return:     0:�ɹ�,!0ʧ��
others: 
*/
int IniFile_File_GetGroupByIndex(struct stIniFile_File *file_obj, struct stIniFile_Group **group_obj, int n)
{
	if(n < 0)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	if(n > file_obj->group_number-1)
	{
		AUTOCALL_DEBUG;
		return -1;
	}
	
	*group_obj = &(file_obj->group_list[n]);
	return 0;
}

/*
function:   IniFile_File_GetGroupByName()
description:
            ��������FILE��ȡGROUP
Input:      
            file_obj:file����
            data:group's name
Output:		group_obj:group���������
Return:     0:�ɹ�,!0ʧ��
others: 
*/
int IniFile_File_GetGroupByName(struct stIniFile_File *file_obj, struct stIniFile_Group **group_obj, char *data)
{
	int n;
	struct stIniFile_Group *new_obj;
/*printf("==[%s]--[%d]\n",data,file_obj->group_number);*/
	for(n = 0; n < file_obj->group_number; n++)
	{
		new_obj = &(file_obj->group_list[n]);
/*printf("==[%s][%s]--[%d]\n",new_obj->name,data,n);*/
		if(strcmp(new_obj->name, data) == 0)
		{
			*group_obj = new_obj;
			return 0;
		}
	}
	
	return -1;
}

/*
function:   IniFile_Init()
description:
            ��ʼ��FILE�ڲ����ݽṹ�����裩
Input:      
            file_obj:file����
Output:		
Return:     
others: 
*/
void IniFile_Init(struct stIniFile_File *obj)
{
	memset(obj, 0, sizeof(struct stIniFile_File));
}

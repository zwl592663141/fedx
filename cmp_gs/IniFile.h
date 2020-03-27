/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	IniFile.h
author:		caoshun
version:	1.04
date:		2005-11-15
description:
			ר�����ڴ���INI�����ļ�-H�ļ�
others:
			[�ļ���ʽ]
			#this is ini file		//ע��
									//����
			[group1]				//�鶨��, ��������Ϊ��
			key1=value1				//"key1" ����(����Ϊ��), "=" �ָ���(����Ϊ��), "value1" ��ֵ(����Ϊ��)
			key2=value2				//���壬���ܵ������ڣ��������鶨�����ڡ�
			[group2]				
			key1=value1				
			[group3]
			------------------------------
			[�ڲ����ݽṹ��ϵ]
			IniFile_File|---IniFile_Group
			            |
				        |---IniFile_Group|---IniFile_Key
						                 |
							             |---IniFile_Key
history:
			2005.11.15 �����ڲ���غ��С�����ٶ��������ڴ�(Ŀǰ��1MB)
			2005.02.02 ȥ���ڲ�strncpy,snprintf������
			2004.09.21 �ڲ����������
			2004.07.20 ������ɡ�
*/

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef _INIFILE_H_
#define _INIFILE_H_

#define INIFILE_BUFFER_MAX_LEN				4096
#define INIFILE_NAME_MAX_LEN				64
#define INIFILE_VALUE_MAX_LEN				256
#define INIFILE_KEY_MAX_NUMBER				100
#define INIFILE_GROUP_MAX_NUMBER			50

/*INI�ļ�������ṹ*/
struct stIniFile_Key
{
	char name[INIFILE_NAME_MAX_LEN+1];
	char value[INIFILE_VALUE_MAX_LEN+1];
};

/*INI�ļ������ṹ*/
struct stIniFile_Group
{
	char name[INIFILE_NAME_MAX_LEN+1];
	int  key_number;
	struct stIniFile_Key key_list[INIFILE_KEY_MAX_NUMBER];
};

/*INI�ļ�����ṹ*/
struct stIniFile_File
{
	int  group_number;
	struct stIniFile_Group group_list[INIFILE_GROUP_MAX_NUMBER];
};

void  IniFile_Key_SetName (struct stIniFile_Key *obj, char *data);
void  IniFile_Key_SetValue(struct stIniFile_Key *obj, char *data);
char *IniFile_Key_GetName(struct stIniFile_Key *obj);
char *IniFile_Key_GetValue(struct stIniFile_Key *obj);

int   IniFile_Group_AppendKey(struct stIniFile_Group *group_obj, char *name, char *value);
void  IniFile_Group_SetName(struct stIniFile_Group *obj, char *data);
char *IniFile_Group_GetName(struct stIniFile_Group *obj);
int   IniFile_Group_CountKey(struct stIniFile_Group *obj);
int   IniFile_Group_GetKeyByIndex(struct stIniFile_Group *group_obj, struct stIniFile_Key **key_obj, int n);
int   IniFile_Group_GetKeyByName(struct stIniFile_Group *group_obj, struct stIniFile_Key **key_obj, char *data);

int IniFile_File_AppendGroup(struct stIniFile_File *file_obj, char *name);
int IniFile_File_CountGroup(struct stIniFile_File *obj);
int IniFile_File_GetGroupByIndex(struct stIniFile_File *file_obj, struct stIniFile_Group **group_obj, int n);
int IniFile_File_GetGroupByName(struct stIniFile_File *obj, struct stIniFile_Group **group_obj, char *data);

int  IniFile_ReadFile(struct stIniFile_File *obj, char *filename);
int  IniFile_WriteFile(struct stIniFile_File *obj, char *filename);
void IniFile_Init(struct stIniFile_File *obj);

#if defined(__cplusplus)
}
#endif

#endif /*ifndef _INIFILE_H_ */

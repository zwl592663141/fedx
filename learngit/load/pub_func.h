
/***********************************************************
 * Copyright (c) 1999,2002 .
 * Descriptions:
 *      _pub.h
 *      一些公用函数.
 * Known bugs:
 *
 * Change log:
 *        who          MM/DD/YYYY        why
 *        wangjs       03/20/2005        create
 **********************************************************/

#ifndef _FUNC_H_
#define _FUNC_H_

#define LEVEL_INFO      "info"   /* informational */
#define LEVEL_WARN      "warn"   /* warning conditions */
#define LEVEL_ERR       "error"  /* error conditions */

/*获取系统时间*/
extern void GetSysDateTime( char *outDateStr );

/*获取系统时间的总秒数*/
extern long GetTimeSec( void );

/*初始日志文件，得到日志文件名前半部分*/
extern void Initlog(char *logPath, char *exeName);

/*写系统运行日志*/
extern void Writelog(char *level, char *mesg, ...);

/*替换字符串*/
extern void strRep( char *inStr, char *oldStr, char *newStr );

/*通过对指定文件加锁来确认此进程是否已在运行*/
extern short CheckUniqueCopy(const char *inFileNamename);

/*分离路径和文件名*/
extern void SeparatePathFile(char *outPath, char *outFile, char *inStr);

/*替换环境变量*/
extern void RepEnv(char *inDir);

/* 解密Login文件*/
extern short GetLoginInfo(char *inPassFile, char *outUser, char *outPass);

/*根据密码文件得到登陆用户和口令*/
extern short DecodePass(char *inPassFile, char *inDbType, char *outUser, char *outPass, char *outServer);

/*根据密码文件得到登陆用户和口令(山西BOSS使用)*/
short DecodePass_Login(char *inPassFile, char *inDbType, char *outUser, char *outPass, char *outServer);

/*生成密码字符串*/
extern short GetEncryptStr(void);

/*删除字符串中两端的所有指定字符*/
extern void CutChar( char *inStr, char beCutedChar, char alignMode );

/*获取指定路径下的文件名列表*/
extern void GetFileList(char *fileList, char *inDir, char *inMatchStr, int nameLen, int maxNum );

/*检查目录有效性*/
extern short CheckValidPath(char *outStr, char *inDir);

/*将指定文件中的文件名列表改名，然后删掉指定文件*/
extern void RenameTempFile(char *path, char *fileName);

/*删除指定路径下的临时文件*/
extern void RemoveTempFile(char *inDir, char *fileName);

/*分割字符串*/
extern void SplitString(char *str, char delimiter, char outArray[][100], int *outLen);

extern void SplitStrM(char *inStr, char *cc, int *outLen, int maxLen, int iArr[]);

/*读取参数文件*/
extern int get_config_val(FILE *cfg_fp,char *seg_name,char *val_name,char *ret_val);

extern int getTextField(char * dstr,char * sstr,char t,int fn);
extern char * getSysDate(char * sysDate);

#endif

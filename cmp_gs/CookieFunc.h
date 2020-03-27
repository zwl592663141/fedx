/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	CookieFunc.h
author:		caoshun
version:	2.00
date:		2006-01-09
description:
			һЩ���õĺ�����-H�ļ�
others:
history:
			2006.01.09 ����cf_file_copy()cf_file_move()cf_file_delzero()cf_files_move()cf_files_delzero()cf_files_del()	��
			2005.12.02 ����cf_string_tt2str()������cf_time_getcurrent()��
			2005.11.09 �޸�cf_datastru_halfsearch����,�����ڲ����ܡ�
			2005.06.09 ��cf_time_getseconds��Ϊcf_time_getminutes
			2005.03.04 ����cf_sys_open()��cf_sys_bc()��cf_datastru_halfsearch(), cf_string_delchar()��
			2005.02.04 ����cf_file_settime()��
			2005.01.08 ����cf_time_getnextmonth(), cf_time_getforwmonth(), cf_time_getweekday()��cf_time_str2tt()��
			2005.01.05 ����cf_time_getmonthdays()��
			2004.12.30 ����cf_time_gethours, cf_time_getseconds()��
			2004.12.19 ����cf_string_isfloat(), cf_time_ifleapyear()��
			2004.08.31 ����ʵ��cf_file_iffile(), cf_file_ifdir(), ����cf_file_ifsamedisk()��
			2004,08.11 ����cf_string_ltrim1(), cf_string_rtrim1(), cf_string_trim1()��
			2004.07.26 ����cf_time_getcurrent()��
			2003.10.21 ��ʼ��	
*/

#ifndef _COOKIE_FUNC_
#define _COOKIE_FUNC_

#include <time.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
������ʵ����ȹ�ϵ
-----------------------------
     +  -  *  /  (  )  #  
-----------------------------
+ |  >  >  <  <  <  >  >  
- |  >  >  <  <  <  >  >  
* |  >  >  >  >  <  >  >  
/ |  >  >  >  >  <  >  >  
( |  <  <  <  <  <  =  ?  
) |  >  >  >  >  ?  >  >  
# |  <  <  <  <  <  ?  =  
------------------------------
*/ 

/*cf_bc �����ڲ����ݽṹ*/
struct cf_stBcStackOpt{
	int error_flag;				/*�����ɹ���־*/
	char data[128];				/*�ڲ����ݼ�*/
	int count;					/*�ڲ�������*/
};

/*cf_bc �����ڲ����ݽṹ*/
struct cf_stBcStackData{
	int error_flag;				/*�����ɹ���־*/
	int data[128];				/*�ڲ����ݼ�*/
	int count;					/*�ڲ�������*/
};

char *cf_string_upper(char *obj);
char *cf_string_lower(char *obj);
char *cf_string_ltrim(char *obj);
char *cf_string_rtrim(char *obj);
char *cf_string_trim(char *obj);
char *cf_string_ltrim1(char *obj);
char *cf_string_rtrim1(char *obj);
char *cf_string_trim1(char *obj);
int   cf_string_strcount(const char *obj1, const char *obj2);
int   cf_string_charcount(const char *obj1, char obj2);
int   cf_string_camel(const char *obj1, char *obj2);
int   cf_string_ifdigital(const char *obj);
int   cf_string_ifalpha(const char *obj);
int   cf_string_ifdate(const char *obj);
int   cf_string_iffloat(const char *obj);
char *cf_string_delchar(char *obj, char c);

int    cf_time_getdays(time_t *obj1, time_t *obj2);
int    cf_time_gethours(time_t *obj1, time_t *obj2);
int    cf_time_getminutes(time_t *obj1, time_t *obj2);
void cf_time_getcurrent(char *data14);
int    cf_time_ifleapyear(int yyyy);
int    cf_time_getmonthdays(int yyyy, int mm);
int    cf_time_getnextmonth(int mm);
int    cf_time_getforwmonth(int mm);
int    cf_time_ifweekend(int yyyymmdd);
int    cf_time_str2tt(const char *date14, time_t *tt);
void cf_time_tt2str(time_t *tt, char *date14);

int cf_amount_strto(const char *src, char *dst);
int cf_amount_tostr(const char *src, char *dst);
int cf_amount_strtohanzi(const char *src, char *dst);
int cf_amount_tohanzi(const char *src, char *dst);

int cf_file_iffile(const char *fn);
int cf_file_ifdir(const char *fn);
int cf_file_ifsamedisk(const char *fn1, const char *fn2);
int cf_file_settime(const char *fn, const char *date);
int cf_file_copy(char *src_file, char *des_file);
int cf_file_move(char *src_file, char *des_file);
int cf_file_delzero(char *filename);
int cf_files_move(char *src_path, char *des_path);
int cf_files_delzero(char *dirname);
int cf_files_del(char *dirname);

int cf_datastru_halfsearch(
	void *list,
	int count,
	void *src_data,
	int(*func)(void *src_data, void *list, int index),
	int *seek
);

int  cf_sys_bc(char *exp, long *result);
void cf_sys_bc__initstack(struct cf_stBcStackOpt *stack_opt, struct cf_stBcStackData *stack_data);
void cf_sys_bc__push_stackopt(struct cf_stBcStackOpt *object, char opt);
void cf_sys_bc__push_stackdata(struct cf_stBcStackData *object, long value);
char cf_sys_bc__gettop_stackopt(struct cf_stBcStackOpt *object);
long cf_sys_bc__gettop_stackdata(struct cf_stBcStackData *object);
char cf_sys_bc__pop_stackopt(struct cf_stBcStackOpt *object);
long cf_sys_bc__pop_stackdata(struct cf_stBcStackData *object);
int  cf_sys_bc__checkexp(char *exp);
void cf_sys_bc__readexp(char **exp, char *item);
int  cf_sys_bc__isopt(char opt);
int  cf_sys_bc__calculate(long data_a, char opt, long data_b, long *result);
char cf_sys_bc__cmpopt(char opt1, char opt2);

int cf_sys_popen(char *in_data, char *out_data);

#if defined(__cplusplus)
}
#endif

#endif /* _COOKIE_FUNC_H_ */

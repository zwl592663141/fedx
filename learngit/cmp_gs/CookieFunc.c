/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	CookieFunc.c
author:		caoshun
version:	2.00
date:		2006-01-09
description:
			һЩ���õĺ�����-H�ļ�
others:
history:	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <utime.h>

#include "CookieFunc.h"

static char	Amount_HanZia_str[10][4]=
{
	"��","Ҽ","��","��","��","��","½","��","��","��"
};

static char	Amount_HanZib_str[7][4]=
{
	""  ,"ʰ","��","Ǫ","��","��"
};

static char	Amount_HanZic_str[5][4]=
{
	"Ԫ","��","��","��","��"
};

/*
function:	cf_string_upper()
description:���ַ������ַ�ȫ��ת��Ϊ��д
Input:		obj:�����ַ���
Output:		ת������ַ���
Return:		ת������ַ���
others:
*/
char *cf_string_upper(char *obj)
{
	char *p;

	p = obj;
	while(*p != '\0')
	{
		*p = toupper(*p);
		p++;
	}

	return obj;
}

/*
function:   cf_string_lower()
description:���ַ������ַ�ȫ��ת��ΪСд
Input:		obj:�����ַ���
Output:		ת������ַ���
Return:		0:�ɹ�,!0:ʧ��
others:
*/
char *cf_string_lower(char *obj)
{
	char *p;

	p = obj;
	while(*p != '\0')
	{
		*p = tolower(*p);
		p++;
	}

	return obj;
}

/*
function:   cf_string_rtrim()
description:���ַ������ָ���Ŀո��ȥ
Input:		obj:�����ַ���
Output:		ת������ַ���
Return:		ת������ַ���
others:
*/
char* cf_string_rtrim(char *obj)
{
	char *p_buffer;

	if(*obj == '\0')
	{
		return obj;
	}

	p_buffer = obj;
	while(*p_buffer == ' ')
	{
		p_buffer++;
	}
	strcpy(obj, p_buffer);
	return obj;
}

/*
function:   cf_string_rtrim1()
description:���ַ������ָ����SPACE|TAB|CR|LF��ȥ
Input:		obj:�����ַ���
Output:		ת������ַ���
Return:		ת������ַ���
others:
*/
char* cf_string_rtrim1(char *obj)
{
	char *p_buffer;

	if(*obj == '\0')
	{
		return obj;
	}

	p_buffer = obj;
	while(*p_buffer == ' ' || *p_buffer == '\t' || *p_buffer == '\r' || *p_buffer == '\n')
	{
		p_buffer++;
	}
	strcpy(obj, p_buffer);
	return obj;
}

/*
function:   cf_string_ltrim()
description:���ַ����ұ�ָ���Ŀո��ȥ
Input:		obj:�����ַ���
Output:		ת������ַ���
Return:		ת������ַ���
others:
*/
char* cf_string_ltrim(char *obj)
{
	int len;

	if(*obj == '\0')
	{
		return obj;
	}

	len = strlen(obj);
	while(obj[len-1] == ' ')
	{
		len--;
	}
	obj[len] = '\0';
	return obj;
}

/*
function:   cf_string_ltrim1()
description:���ַ����ұ�ָ����SPACE|TAB|CR|LF��ȥ
Input:		buffer:�����ַ���
Output:		ת������ַ���
Return:		ת������ַ���
others:
*/
char* cf_string_ltrim1(char *obj)
{
	int len;

	if(*obj == '\0')
	{
		return obj;
	}

	len = strlen(obj);
	while(obj[len-1] == ' ' || obj[len-1] == '\t' || obj[len-1] == '\r' || obj[len-1] == '\n')
	{
		len--;
	}
	obj[len] = '\0';
	return obj;
}

/*
function:   cf_string_trim()
description:���ַ�������ָ���Ŀո��ȥ
Input:		buffer:�����ַ���
Output:		ת������ַ���
Return:		ת������ַ���
others:
*/
char* cf_string_trim(char *obj)
{
	cf_string_ltrim(obj);
	return cf_string_rtrim(obj);
}

/*
function:   cf_string_trim1()
description:���ַ�������ָ����SPACE|TAB|CR|LF��ȥ
Input:		buffer:�����ַ���
Output:		ת������ַ���
Return:		ת������ַ���
others:
*/
char* cf_string_trim1(char *obj)
{
	cf_string_ltrim1(obj);
	return cf_string_rtrim1(obj);
}

/*
function:   cf_string_strcount()
description:��ѯ�Ӵ���Ŀ�괮���ظ����ڵĸ���
Input:		obj1:Ŀ�괮,obj2:�Ӵ�
Output:		
Return:		�Ӵ��ظ����ڸ���
others:
*/
int cf_string_strcount(const char *obj1, const char *obj2)
{
	int i;
	char *p;

	i = 0;
	p = strstr(obj1, obj2);
	while(p != NULL)
	{
		i++;
		p = p + 1;
		p = strstr(p, obj2);
	}

	return i;
}

/*
function:   cf_string_charcount()
description:��ѯ�ַ���Ŀ�괮���ظ����ڵĸ���
Input:		obj1:Ŀ�괮,obj2:�ַ�
Output:		
Return:		�ַ��ظ����ڸ���
others:
*/
int cf_string_charcount(const char *obj1, char obj2)
{
	int i;
	char *p;

	i = 0;
	p = strchr(obj1, obj2);
	while(p != NULL)
	{
		i++;
		p = p + 1;
		p = strchr(p, obj2);
	}

	return i;
}

/*
function:   cf_string_camel()
description:���ִ�������������������ת��
Input:		obj1:Ŀ�괮,obj2:�ַ�
Output:		
Return:		�ַ��ظ����ڸ���
others:
			����aaa_bbb_ccc--->AaaBbbCcc
*/
int cf_string_camel(const char *obj1, char *obj2)
{
	char buffer[1024];
	char *p1, *p2;
	int  flag = 0;

	strcpy(buffer, obj1);
	cf_string_lower(buffer);
	p1 = buffer;
	p2 = obj2;

	while(*p1 != '\0')
	{
		if(*p1 == '_')
		{
			flag = 0;
			p1++;
		}
		if(flag == 0)
		{
			*p2 = toupper(*p1);
			flag = 1;
		}
		else
		{
			*p2 = *p1;
		}
		p1++;
		p2++;
	}
	*p2 = '\0';

	return 0;
}

/*
function:   cf_string_iffloat()
description:�ж�һ���ַ����Ƿ�Ϊ������
Input:		obj:�����ַ���
Output:		
Return:		0:�Ϸ�,!0:�Ƿ�
others:
*/
int cf_string_iffloat(const char *obj)
{
	int flag;
	const char *p_obj;
	
	flag = 0;
	
	if(obj == NULL || *obj == '\0')
	{
		return -1;
	}
	
	p_obj = obj;
	while(*p_obj != '\0')
	{
		if(isdigit(*p_obj) == 0)
		{
			if(*p_obj == '.')
			{
				if(flag == 0)
				{
					if(*(p_obj+1) == '\0') 
					{
						return -1;
					}
					else
					{
						flag = 1;
					}
				}
				else
				{
					return -1;
				}
			}
			else
			{
				return -1;
			}
		}
		p_obj++;
	}
	
	return 0;
}

/*
function:   cf_string_delchar()
description:ɾ��һ���ַ���ָ���ַ�
Input:		obj:�ַ���
			c:��ɾ�����ַ�
Output:		
Return:		��������ַ���
others:
*/
char *cf_string_delchar(char *obj, char c)
{
	char *p_obj;
	char *p_tmp;
	
	p_obj = obj;
	while(*p_obj != '\0')
	{
		if(*p_obj == c)
		{
			p_tmp = p_obj+1;
			while(*p_tmp == c)
			{
				p_tmp++;
				continue;
			}
		
			strcpy(p_obj, p_tmp);
			continue;
		}
		p_obj++;
	}
	
	return obj;
}


/*
function:   cf_string_ifdigital()
description:�ж�һ���ַ����е��ַ�ȫΪ����
Input:		obj:�ַ���
Output:		
Return:		0:�Ϸ�,!0:�Ƿ�
others:
*/
int   cf_string_ifdigital(const char *obj)
{
	char *p;

	if(*obj == '\0')
	{
		return -1;
	}

	p = (char *)obj;
	while(*p != '\0')
	{
		if(*p >= '0' && *p <= '9')
		{
			p++;
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

/*
function:   cf_string_ifalpha()
description:�ж�һ���ַ����е��ַ�ȫΪ��ĸ
Input:		obj:�ַ���
Output:		
Return:		0:�Ϸ�,!0:�Ƿ�
others:
*/
int   cf_string_ifalpha(const char *obj)
{
	char *p;
	
	p = (char *)obj;
	while(*p != '\0')
	{
		if
		(
			(*p >= 'a' && *p <= 'z')
			||
			(*p >= 'A' && *p <= 'Z')
		)
		{
			p++;
		}
		else
		{
			return -1;
		}
	}
	
	return 0;
}

/*
function:   cf_string_ifdate()
description:�ж�һ���ַ����Ƿ�Ϊ������(yyyymmdd)
Input:		obj:�ַ���
Output:		
Return:		0:�Ϸ�,!0:�Ƿ�
others:
*/
int cf_string_ifdate(const char *obj)
{
	char tmp[8+1];
	int nYear, nMon, nDay;

	if(obj == NULL || strlen(obj) != 8)
	{
		return -1;
	}

	if(cf_string_ifdigital(obj) != 0)
	{
		return -1;
	}
	
	memcpy( tmp, obj     , 4 ); tmp[4] = 0;	nYear = atoi( tmp );
	memcpy( tmp, obj +  4, 2 ); tmp[2] = 0;	nMon  = atoi( tmp );
	memcpy( tmp, obj +  6, 2 ); tmp[2] = 0;	nDay  = atoi( tmp );

	if( nMon > 12 || nDay > 31)
	{
		return -1;
	}
	
	if( ( nMon == 4 || nMon == 6 || nMon == 9 || nMon == 11 ) && nDay > 30 )
	{
		return -1;
	}

	if( nMon == 2 )
	{
		if( cf_time_ifleapyear( nYear ) == 0)
		{
			return (nDay > 29) ? -1 : 0; 
		}	
		else 
		{
			return (nDay > 28) ? -1 : 0;
		}	
	}
	
	return 0;	
}

/*
function:   cf_file_iffile()
description:�����Ƿ�Ϊ�ļ�
Input:		fn:�ļ���
Output:		
Return:		0:�Ϸ�,!0:�Ƿ�
others:
*/
int cf_file_iffile(const char *fn)
{
	struct stat filestat;

	if(stat(fn, &filestat) != 0)
	{
		return -1;
	}

	if(filestat.st_mode & S_IFREG)
	{
		return 0;
	}

	return -1;
}

/*
function:   cf_file_ifdir()
description:�����Ƿ�ΪĿ¼
Input:		fn:Ŀ¼��
Output:		
Return:		0:�Ϸ�,!0:�Ƿ�
others:
*/
int cf_file_ifdir(const char *fn)
{
	struct stat filestat;

	if(stat(fn, &filestat) != 0)
	{
		return -1;
	}

	if(filestat.st_mode & S_IFDIR)
	{
		return 0;
	}

	return -1;
}

/*
function:   cf_file_ifsamedisk()
description:�����ļ��Ƿ���ͬһ���ļ�ϵͳ�
Input:		fn1:�ļ�1,fn2:�ļ�2
Output:		
Return:		0:�ڣ�1:���ڣ�-1:����
others:
*/
int   cf_file_ifsamedisk(const char *fn1, const char *fn2)
{
	struct stat filestat1, filestat2;
	
	if(stat(fn1, &filestat1) != 0)
	{
		return -1;
	}
	
	if(stat(fn2, &filestat2) != 0)
	{
		return -1;
	}
	
	if(filestat1.st_dev == filestat2.st_dev)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/*
function:   cf_file_settime()
description:�����ļ��ĸ���ʱ�������ʱ��
Input:		fn:�ļ�,date:�ļ���ʱ��(8 byte)
Output:		
Return:		0:�ɹ�,!0:ʧ��
others:
*/
int   cf_file_settime(const char *fn, const char *date)
{
	struct utimbuf timebuf;
	time_t time_ttt;
	struct tm *time_mmm;
	char buffer[4+1];

	if(strlen(date) != 8)
	{
		return -1;
	}

	time(&time_ttt);
	time_mmm = localtime(&time_ttt);

	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, date, 4);
	time_mmm->tm_year = atoi(buffer)-1900;

	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, date+4, 2);
	time_mmm->tm_mon = atoi(buffer)-1;

	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, date+6, 2);
	time_mmm->tm_mday = atoi(buffer);

	time_mmm->tm_hour = 0;
	time_mmm->tm_min = 0;
	time_mmm->tm_sec = 0;

	time_ttt=mktime(time_mmm);

	timebuf.actime = time_ttt;
	timebuf.modtime = time_ttt;

	if(utime(fn, &timebuf)<0)
	{
		return -1;
	}

	return 0;
}

/*
function:   cf_time_getdays()
description:ȡ������֮���������
Input:		obj1:ʱ��1,obj2:ʱ��2
Output:		
Return:		������
others:
*/
int cf_time_getdays(time_t *obj1, time_t *obj2)
{
	struct tm *tm1, *tm2;
	time_t t1, t2;

	tm1 = localtime(obj1);
	t1 = mktime(tm1)/86400;
	tm2 = localtime(obj2);
	t2 = mktime(tm2)/86400;	

	return (int)(t2 - t1);
}

/*
function:   cf_time_gethours()
description:ȡ������֮���Сʱ��
Input:		obj1:ʱ��1,obj2:ʱ��2
Output:		
Return:		Сʱ��
others:
*/
int cf_time_gethours(time_t *obj1, time_t *obj2)
{
	struct tm *tm1, *tm2;
	time_t t1, t2;

	tm1 = localtime(obj1);
	t1 = mktime(tm1)/3600;
	tm2 = localtime(obj2);
	t2 = mktime(tm2)/3600;	

	return (int)(t2 - t1);
}

/*
function:   cf_time_getminutes()
description:ȡ������֮��ķ��Ӳ�
Input:		obj1:ʱ��1,obj2:ʱ��2
Output:		
Return:		���Ӳ�
others:
*/
int cf_time_getminutes(time_t *obj1, time_t *obj2)
{
	struct tm *tm1, *tm2;
	time_t t1, t2;

	tm1 = localtime(obj1);
	t1 = mktime(tm1)/60;
	tm2 = localtime(obj2);
	t2 = mktime(tm2)/60;	

	return (int)(t2 - t1);
}

/*
function:   cf_time_getcurrent()
description:��ϵͳ�������ַ���14λYYYYMMDDhhmmss����ʽ����
Input:	
		�����ַ���
Output:		
Return:		
others:
*/
void cf_time_getcurrent(char *date14)
{
	time_t time_ttt;
	struct tm *time_mmm;

	time(&time_ttt);
	time_mmm = localtime(&time_ttt);

	sprintf
	(
		date14,
		"%04d%02d%02d%02d%02d%02d",
		time_mmm->tm_year+1900,
		time_mmm->tm_mon+1,
		time_mmm->tm_mday,
		time_mmm->tm_hour,
		time_mmm->tm_min,
		time_mmm->tm_sec
	);
}

/*
function:   cf_time_ifleapyear()
description:�ж��Ƿ�Ϊ�������
Input:		yyyy:���
Output:		
Return:		0:�Ϸ�,!0:�Ƿ�
others:
*/
int cf_time_ifleapyear(int yyyy)
{
	if(yyyy == 0)
	{
		return -1;
	}

	if(
		(yyyy % 4 == 0 && yyyy % 100 != 0 )
		||
		yyyy % 400 == 0
	)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
function:   cf_time_getmonthdays()
description:��ָ���·ݵ�����
Input:		yyyy:���,mm:�·�
Output:		
Return:		����
others:
*/
int cf_time_getmonthdays(int yyyy, int mm)
{
    if(mm == 2)
    {
        if(cf_time_ifleapyear(yyyy) == 0)
        {
            return 29;
        }
        else
        {
            return 28;
        }
    }

    if(mm == 4 || mm == 6 || mm == 9 || mm == 11)
    {
        return 30;
    }
    else
    {
        return 31;
    }
}

/*
function:   cf_time_getnextmonth()
description:����һ�µ��º�
Input:		mm:�·�
Output:		
Return:		>0:�·�,-1:ʧ��
others:
*/
int   cf_time_getnextmonth(int mm)
{
	if(mm < 1 || mm >12)
	{
        return -1;
	}

    mm += 1;

    if(mm > 12)
	{
		mm = 1;
	}

    return mm;
}

/*
function:   cf_time_getforwmonth()
description:����һ�µ��º�
Input:		mm:�·�
Output:		
Return:		>0:�·�,-1:ʧ��
others:
*/
int   cf_time_getforwmonth(int mm)
{
    if(mm < 1 || mm >12)
    {
        return -1;
    }

    mm -= 1;

    if(mm < 1)
    {
        mm = 12;
    }

    return mm;
}

/*
function:   cf_time_getweekday()
description:�����������ڼ�
Input:		yyyymmdd:������
Output:		
Return:		>0:���ڼ�,-1:ʧ��
others:
*/
int   cf_time_getweekday(int yyyymmdd)
{
	int iRcode;
    char buffer[14+1];
    struct tm *time_tm;
    time_t time_tt;

	memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d000000", yyyymmdd);
    iRcode = cf_time_str2tt(buffer, &time_tt);
    if(iRcode)
	{
        return -1;
	}

    time_tm = localtime(&time_tt);

    if(time_tm->tm_wday == 0)
        return 7;

    return time_tm->tm_wday;
}

/*
function:   cf_time_str2tt()
description:���ַ���ת����time_t������
Input:		date14:�����ַ���(14 byte)
Output:		tt:time_t���͵�����
Return:		0:�ɹ�,!0:ʧ��
others:
*/
int cf_time_str2tt(const char *date14, time_t *tt)
{
	char tmp[14+1];

	struct tm time_tm;

	if(strlen(date14) != 14)
	{
		return -1;
	}

	memcpy(tmp, date14, 4);
	tmp[4] = 0;
	time_tm.tm_year = atoi(tmp) - 1900;

	if(time_tm.tm_year < 0)
	{
		return -1;
	}

	memcpy(tmp, date14+4, 2);
	tmp[2] = 0;
	time_tm.tm_mon = atoi(tmp) - 1;

	memcpy(tmp, date14+6, 2);
	tmp[2] = 0;
	time_tm.tm_mday = atoi(tmp);

	memcpy(tmp, date14+8, 2);
	tmp[2] = 0;
	time_tm.tm_hour = atoi(tmp);

	memcpy(tmp, date14+10, 2);
	tmp[2] = 0;
	time_tm.tm_min = atoi(tmp);

	memcpy(tmp, date14+12, 2);
	tmp[2] = 0;
	time_tm.tm_sec = atoi(tmp);

	*tt =  mktime(&time_tm);
	return 0;
}

/*
function:   cf_time_tt2str()
description:��time_t������ת�����ַ���
Input:		tt:time_t���͵�����
Output:		date14:�����ַ���(14 byte)
Return:		
others:
*/
void cf_time_tt2str(time_t *tt, char *date14)
{
	struct tm *time_mmm;

	time_mmm = localtime(tt);

	sprintf
	(
		date14,
		"%04d%02d%02d%02d%02d%02d",
		time_mmm->tm_year+1900,
		time_mmm->tm_mon+1,
		time_mmm->tm_mday,
		time_mmm->tm_hour,
		time_mmm->tm_min,
		time_mmm->tm_sec
	);
}

/*
function:   cf_amount_strto()
description:����ֵ�ַ���ת���ɽ����ַ���
Input:		src:��ֵ�ַ���
Output:		dst:�����ַ���
Return:		0:�ɹ�,!0:ʧ��
others:
			����1234--->1,234
*/
int cf_amount_strto(const char *src, char *dst)
{
	int  len, i, j;
	char *pDot;

	if((src == NULL) || (strlen(src) == 0))
	{
		return -1;
	}
	if((pDot = strchr(src, '.')) == NULL)
	{
		len = strlen(src);
	}
	else
	{
		len = strlen(src) - strlen(pDot);
	}

	j = 0;
	for(i = 0; i < len; i++)
	{
		if((((len - i)%3) == 0) && (i != 0))
		{
			*(dst+j) = ',';
			j++;
		}
		*(dst+j) = *(src+i);
		j++;
	}
	*(dst+j) = '\0';

	if(pDot != NULL)
	{
		strcat(dst, pDot);
	}

	return 0;
}

/*
function:   cf_amount_tostr()
description:�������ַ���ת������ֵ�ַ���
Input:		src:�����ַ���
Output:		dst:��ֵ�ַ���
Return:		0:�ɹ�,!0:ʧ��
others:
			����1,234--->1234
*/
int cf_amount_tostr(const char *src, char *dst)
{
	char *pSrc, *pDst;

	if((src == NULL) || (strlen(src) == 0))
	{
		return -1;
	}

	pSrc = (char *)src;
	pDst = dst;
	while(*pSrc != '\0')
	{
		if(*pSrc == ',')
		{
			pSrc ++;
		}
		else
		{
			*pDst = *pSrc;
			pSrc ++;
			pDst ++;
		}
	}
	*pDst = '\0';
	
	return 0;
}

/*
function:   cf_amount_strtohanzi()
description:����ֵ�ַ���ת���ɽ��ں����ַ�����
Input:		src:��ֵ�ַ���
Output:		dst:���ں����ַ���
Return:		0:�ɹ�,!0:ʧ��
others:
			����1234--->ҼǪ������ʰ��Ԫ��
*/
int cf_amount_strtohanzi(const char *src, char *dst)
{
	char *pSrc, *pDst, *pPoint, *pTmp;
	char tmp_buffer[1024];
	int  zhengsu_flag, danwei_flag, danwei1_flag;

	if((src == NULL) || (strlen(src) == 0))
	{
		return -1;
	}

	pSrc = (char *)src;
	pDst = dst;
	zhengsu_flag = 1;
	danwei_flag = 0;
	danwei1_flag = 0;
	*pDst = '\0';

	pPoint = strchr(src, '.');
	if(pPoint == NULL)
	{
		pTmp = pSrc + strlen(src) - 1;
	}
	else
	{
		pTmp = pPoint - 1;
	}

	while(pTmp >= pSrc)
	{
		strcpy(tmp_buffer, pDst);
		strcpy(pDst, Amount_HanZia_str[*pTmp - '0']);
		strcat(pDst, Amount_HanZib_str[danwei_flag]);
		strcat(pDst, tmp_buffer);

		pTmp --;
		switch(danwei_flag)
		{
			case  3:
					if(danwei1_flag == 0)
					{
						danwei_flag = 4;
					}
					else
					{
						danwei_flag = 5;
					}
					break;
			case  4:
					danwei_flag = 1;
					danwei1_flag = 1;
					break;
			case  5:
					danwei_flag = 1;
					danwei1_flag = 0;
					break;
			default:
					danwei_flag ++;
		}
	}

	strcat(pDst, Amount_HanZic_str[0]);

	if(pPoint != NULL)
	{
		if(*(pPoint+1) != '\0')
		{
			if((*(pPoint+1) - '0') != 0)
			{
				strcat(pDst, Amount_HanZia_str[*(pPoint+1) - '0']);
				strcat(pDst, Amount_HanZic_str[1]);
				zhengsu_flag = 0;
			}
			if(*(pPoint+2) != '\0')
			{
				if((*(pPoint+2) - '0') != 0)
				{
					strcat(pDst, Amount_HanZia_str[*(pPoint+2) - '0']);
					strcat(pDst, Amount_HanZic_str[2]);
					zhengsu_flag = 0;
				}
			}
		}
	}

	if(zhengsu_flag == 1)
	{
		strcat(pDst, Amount_HanZic_str[3]);
	}

	return 0;
}

/*
function:   cf_amount_tohanzi()
description:�������ַ���ת���ɽ��ں����ַ���
Input:		src:�����ַ���
Output:		dst:���ں����ַ���
Return:		0:�ɹ�,!0:ʧ��
others:
			����1,234--->ҼǪ������ʰ��Ԫ��
*/
int cf_amount_tohanzi(const char *src, char *dst)
{
	char tmp[1024];
	return cf_amount_tostr(src, tmp) || cf_amount_strtohanzi(tmp, dst);
}

/*
function:   cf_sys_popen()
description:
            ��Unix�ܵ��н���һ��Shell�����������
Input:      
            in_data:Shell����
            out_data:������
Output:
Return:
			0:�ɹ���other:���ɹ�
others: 
			ֻ�ܲ���һ������
*/
int  cf_sys_popen(char *in_data, char *out_data)
{
	FILE *fp;

	fp = popen(in_data, "r");
	if(fp == NULL)
	{
		return -1;
	}

	if(fgets(out_data, 1024, fp) == NULL)
	{
		pclose(fp);
		return -1;
	}

	if(pclose(fp))
	{
		return -1;
	}

	return 0;
}

/*
function:   cf_datastru_halfsearch()
description:
			���ֲ��ҷ�
Input:
			list:�ṹ����
			count:�ṹ������Ч��¼��
			src_data:��ȽϵĽṹ����
			func:�����ڲ��ȽϵĻص�����
Output:
			seek:�Ƚ϶�����²������鶨λ
Return:
			<0:û���ҵ�
			>=:�ҵ�,���������±�
others:
*/
int cf_datastru_halfsearch(
	void *list,
	int count,
	void *src_data,
	int(*func)(void *src_data, void *list, int index),
	int *seek
)
{
	int high, low, mid;
	int flag;

	*seek = 0;

	if(count == 0)
	{
		return -1;
	}

	high = count-1;
	low = 0;

	while(low <= high)
	{
		mid = (low + high) / 2;
		flag = func(src_data, list, mid);
		if(flag == 0)
		{
			*seek = mid;
			return mid;
		}
		else
		{
			if(flag<0)
			{
				high = mid - 1;
			}
			else
			{
				low = mid + 1;
			}
		}
	}

	*seek = low;

	return -1;
}

/*
function:   cf_sys_bc__initstack()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
void cf_sys_bc__initstack(struct cf_stBcStackOpt *stack_opt, struct cf_stBcStackData *stack_data)
{
	memset(stack_opt, 0, sizeof(struct cf_stBcStackOpt));
	memset(stack_data, 0, sizeof(struct cf_stBcStackData));
}

/*
function:   cf_sys_bc__push_stackopt()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
void cf_sys_bc__push_stackopt(struct cf_stBcStackOpt *object, char opt)
{
	object->data[object->count] = opt;
	object->count++;
}

/*
function:   cf_sys_bc__push_stackdata()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
void cf_sys_bc__push_stackdata(struct cf_stBcStackData *object, long value)
{
	object->data[object->count] = value;
	object->count++;
}

/*
function:   cf_sys_bc__gettop_stackopt()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
char cf_sys_bc__gettop_stackopt(struct cf_stBcStackOpt *object)
{
	if(object->count == 0)
	{
		object->error_flag = 1;
		return '\0';
	}
	
	return object->data[object->count-1];
}

/*
function:   cf_sys_bc__gettop_stackdata()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
long cf_sys_bc__gettop_stackdata(struct cf_stBcStackData *object)
{
	if(object->count == 0)
	{
		object->error_flag = 1;
		return 0;
	}
	
	return object->data[object->count-1];
}

/*
function:   cf_sys_bc__pop_stackopt()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
char cf_sys_bc__pop_stackopt(struct cf_stBcStackOpt *object)
{
	if(object->count == 0)
	{
		object->error_flag = 1;
		return '\0';
	}

	object->count--;

	return object->data[object->count];
}

/*
function:   cf_sys_bc__pop_stackdata()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
long cf_sys_bc__pop_stackdata(struct cf_stBcStackData *object)
{
	if(object->count == 0)
	{
		object->error_flag = 1;
		return 0;
	}
	
	object->count--;
	
	return object->data[object->count];
}

/*
function:   cf_sys_bc__checkexp()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
int cf_sys_bc__checkexp(char *exp)
{
	char *p_exp;

	p_exp = exp;
	while(*p_exp != '\0')
	{
		if(*p_exp >= '0' && *p_exp <= '9')
		{
			p_exp++;
			continue;
		}
		else
		{
			if(
				*p_exp == '+' ||
				*p_exp == '-' ||
				*p_exp == '*' ||
				*p_exp == '/' ||
				*p_exp == '(' ||
				*p_exp == ')'
			)
			{
				p_exp++;
				continue;
			}
			else
			{
				return -1;
			}
		}
	}

	return 0;
}

/*
function:   cf_sys_bc__readexp()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
void cf_sys_bc__readexp(char **exp, char *item)
{
	char cur_char;
	
	cur_char = (*exp)[0];

	if(
		cur_char == '+'
		||
		cur_char == '-'
		||
		cur_char == '*'
		||
		cur_char == '/'
		||
		cur_char == '('
		||
		cur_char == ')'
		||
		cur_char == '#'
	)
	{
		*item = cur_char;
		*(item+1) = '\0';
		(*exp)++;
		return;
	}

	for(;;)
	{
		*item = cur_char;
		item++;
		(*exp)++;

		cur_char = (*exp)[0];
		
		if(
			cur_char == '+'
			||
			cur_char == '-'
			||
			cur_char == '*'
			||
			cur_char == '/'
			||
			cur_char == '('
			||
			cur_char == ')'
			||
			cur_char == '#'
			)
		{
			*item = '\0';
			return;
		}
	}
}

/*
function:   cf_sys_bc__isopt()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
int cf_sys_bc__isopt(char opt)
{
	if(
		opt == '+' ||
		opt == '-' ||
		opt == '*' ||
		opt == '/' ||
		opt == '(' ||
		opt == ')' ||
		opt == '#'
	)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
function:   cf_sys_bc__calculate()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
int cf_sys_bc__calculate(long data_a, char opt, long data_b, long *result)
{
	switch(opt)
	{
		case '+':
			*result =  data_a + data_b;
			break;
		case '-':
			*result =  data_a - data_b;
			break;
		case '*':
			*result =  data_a * data_b;
			break;
		case '/':
			*result =  data_a / data_b;
			break;
	}

	return 0;
}

/*
function:   cf_sys_bc__cmpopt()
description:
			cf_sys_bc �ڲ�����
Input:
Output:
Return:
others:
*/
char cf_sys_bc__cmpopt(char opt1, char opt2)
{
	char result;

	switch(opt1)
	{
		case '+':
			switch(opt2)
			{
				case '+':
					result = '>';
					break;
				case '-':
					result = '>';
					break;
				case '*':
					result = '<';
					break;
				case '/':
					result = '<';
					break;
				case '(':
					result = '<';
					break;
				case ')':
					result = '>';
					break;
				case '#':
					result = '>';
					break;
			}
			break;
		case '-':
			switch(opt2)
			{
				case '+':
					result = '>';
					break;
				case '-':
					result = '>';
					break;
				case '*':
					result = '<';
					break;
				case '/':
					result = '<';
					break;
				case '(':
					result = '<';
					break;
				case ')':
					result = '>';
					break;
				case '#':
					result = '>';
					break;
			}
			break;
		case '*':
			switch(opt2)
			{
				case '+':
					result = '>';
					break;
				case '-':
					result = '>';
					break;
				case '*':
					result = '>';
					break;
				case '/':
					result = '>';
					break;
				case '(':
					result = '<';
					break;
				case ')':
					result = '>';
					break;
				case '#':
					result = '>';
					break;
			}
			break;
		case '/':
			switch(opt2)
			{
				case '+':
					result = '>';
					break;
				case '-':
					result = '>';
					break;
				case '*':
					result = '>';
					break;
				case '/':
					result = '>';
					break;
				case '(':
					result = '<';
					break;
				case ')':
					result = '>';
					break;
				case '#':
					result = '>';
					break;
			}
			break;
		case '(':
			switch(opt2)
			{
				case '+':
					result = '<';
					break;
				case '-':
					result = '<';
					break;
				case '*':
					result = '<';
					break;
				case '/':
					result = '<';
					break;
				case '(':
					result = '<';
					break;
				case ')':
					result = '=';
					break;
				case '#':
					result = '?';
					break;
			}
			break;
		case ')':
			switch(opt2)
			{
				case '+':
					result = '>';
					break;
				case '-':
					result = '>';
					break;
				case '*':
					result = '>';
					break;
				case '/':
					result = '>';
					break;
				case '(':
					result = '?';
					break;
				case ')':
					result = '>';
					break;
				case '#':
					result = '>';
					break;
			}
			break;
		case '#':
			switch(opt2)
			{
				case '+':
					result = '<';
					break;
				case '-':
					result = '<';
					break;
				case '*':
					result = '<';
					break;
				case '/':
					result = '<';
					break;
				case '(':
					result = '<';
					break;
				case ')':
					result = '?';
					break;
				case '#':
					result = '=';
					break;
			}
			break;
	}

	return result;
}

/*
function:   cf_sys_bc()
description:
			������ʽ
Input:
			exp:���ʽ
Output:
			result:������
Return:
			0:�ɹ�,!0:ʧ��
others:
			֧��+,-,*,/,(,)���������
			��֧��-1�ȱ��ʽ������ͨ��(0-1)������
			
			���㷨 ���� �廪��ѧ������ ���ݽṹ �������������ʵ�֡�
*/
int cf_sys_bc(char *exp, long *result)
{
	int iRcode;
	char new_exp[1024+1];
	char read_tmp[32+1];
	char current_opt;
	long current_value_a, current_value_b, current_value_c;
	char *p_exp;
	struct cf_stBcStackOpt stack_opt;
	struct cf_stBcStackData stack_data;
	
	/*ȥ�����ʽ�ڲ��Ŀղ�*/
	memset(new_exp, 0, sizeof(new_exp));
	strcpy(new_exp, exp);
	cf_string_delchar(new_exp, ' ');

	/*��֤���ʽ�����Ƿ�Ϸ�*/
	iRcode = cf_sys_bc__checkexp(new_exp);
	if(iRcode)
	{
		return -1;
	}
	
	/*�����ʽ�ӽ�����*/
	strcat(new_exp, "#");

	/*��ʼ�� ����� ������ ��ջ*/
	cf_sys_bc__initstack(&stack_opt, &stack_data);

	/*push��ʼ��=>����� ��ջ*/
	cf_sys_bc__push_stackopt(&stack_opt, '#');

	p_exp = new_exp;

	/*ȡ��һ�����ʽ����*/
	memset(read_tmp, 0, sizeof(read_tmp));
	cf_sys_bc__readexp(&p_exp, read_tmp);

	for(;;)
	{
		/*ȡ ����� ��ջ*/
		current_opt = cf_sys_bc__gettop_stackopt(&stack_opt);
		if(stack_opt.error_flag != 0)
		{
			return -1;
		}

		/*��û�д�������ʽ��������ʱ*/
		if( ! 
			(read_tmp[0] == '#' && current_opt == '#')
		)
		{
			/*������ǰ���ʽ������ ����� ���� ������*/
			if(cf_sys_bc__isopt(read_tmp[0]) != 0)
			{
				/*push��������=>������ ��ջ*/
				cf_sys_bc__push_stackdata(&stack_data, atol(read_tmp));
				
				/*ȡ��һ�����ʽ����*/
				memset(read_tmp, 0, sizeof(read_tmp));
				cf_sys_bc__readexp(&p_exp, read_tmp);
			}
			else
			{
				/*ȡ����� ��ջ*/
				current_opt = cf_sys_bc__gettop_stackopt(&stack_opt);
				if(stack_opt.error_flag != 0)
				{
					return -1;
				}

				/*�Ƚϵ�ǰ ����� �� ��һ������� �����ȼ�*/
				switch(
					cf_sys_bc__cmpopt(current_opt, read_tmp[0])
				)
				{
					case '<':
						/*push�����=>����� ��ջ*/
						cf_sys_bc__push_stackopt(&stack_opt, read_tmp[0]);

						/*ȡ��һ�����ʽ����*/
						memset(read_tmp, 0, sizeof(read_tmp));
						cf_sys_bc__readexp(&p_exp, read_tmp);
						break;
					
					case '=':
						/*pop�����=>����� ��ջ*/
						/*��֧��ǰ����� �� ��һ�����*/
						current_opt = cf_sys_bc__pop_stackopt(&stack_opt);
						if(stack_opt.error_flag != 0)
						{
							return -1;
						}

						/*ȡ��һ�����ʽ����*/
						memset(read_tmp, 0, sizeof(read_tmp));
						cf_sys_bc__readexp(&p_exp, read_tmp);
						break;

					case '>':
						/*pop�����=>����� ��ջ*/
						current_opt = cf_sys_bc__pop_stackopt(&stack_opt);
						if(stack_opt.error_flag != 0)
						{
							return -1;
						}
						/*pop������=>������ ��ջ*/
						current_value_b = cf_sys_bc__pop_stackdata(&stack_data);
						if(stack_data.error_flag != 0)
						{
							return -1;
						}
						/*pop������=>������ ��ջ*/
						current_value_a = cf_sys_bc__pop_stackdata(&stack_data);
						if(stack_data.error_flag != 0)
						{
							return -1;
						}

						/*���㵱ǰ����õ�һ���м�������*/
						iRcode = cf_sys_bc__calculate(current_value_a, current_opt, current_value_b, &current_value_c);
						if(iRcode != 0)
						{
							return -1;
						}

						/*push�м�������=>������ ��ջ*/
						cf_sys_bc__push_stackdata(&stack_data, current_value_c);
						break;	

					case '?':
						return -1;
				}
			}
		}
		else
		{
			break;
		}
	}

	/*ȡ������������*/
	*result = cf_sys_bc__gettop_stackdata(&stack_data);
	if(stack_data.error_flag != 0)
	{
		return -1;
	}

	return 0;
}

/*
function:   cf_file_copy()
description:
            �����ļ�
Input:      
            src_file:Դ�����ļ�
            des_file:Ŀ�ز����ļ�
Output:
Return:
			0:�ɹ���other:���ɹ�
others: 
*/
int cf_file_copy(char *src_file, char *des_file)
{
	FILE* des_f = NULL;
	FILE* src_f = NULL;
	char buffer[4096+1];
	size_t read_len, write_len; 

	if(strcmp(src_file, des_file)==0)
	{
		return 0;
	}

	if((src_f=fopen(src_file, "rb"))==NULL)
	{
		return -1;
	}
	if((des_f=fopen(des_file, "wb"))==NULL)
	{
		fclose(src_f);
		return -1;
	}

	while(!feof(src_f))
	{
		memset(buffer, 0, sizeof(buffer));
		read_len = fread(buffer, 1, 4096, src_f);
		if(read_len != 4096 && feof(src_f) == 0)
		{
			fclose(des_f);
			fclose(src_f);
			return -1;
		}
		write_len = fwrite(buffer, 1, read_len, des_f);
		if(read_len != write_len)
		{
			fclose(des_f);
			fclose(src_f);
			return -1;
		}
	}

	fclose(des_f);
	fclose(src_f);
	
	return 0;
}

/*
function:   cf_file_move()
description:
            �ƶ��ļ�
Input:      
            src_file:Դ�����ļ�
            des_file:Ŀ�ز����ļ�
Output:
Return:
			0:�ɹ���other:���ɹ�
others: 
*/
int  cf_file_move(char *src_file, char *des_file)
{
	if(strcmp(src_file, des_file)==0)
	{
		return 0;
	}

	if(rename(src_file, des_file) != 0)
	{		
		return -1;
	}
	
	return 0;
}

/*
function:   cf_files_move()
description:
            �ƶ�Ŀ¼�������ļ�
Input:      
            src_file:Դ����Ŀ¼
            des_file:Ŀ�ز���Ŀ¼
Output:
Return:
			0:�ɹ���other:���ɹ�
others: 
*/
int cf_files_move(char *src_path, char *des_path)
{
	DIR  *pDir;
	struct dirent *pDirent;
	char chSrcFilename[1024+1];
	char chDesFilename[1024+1];

	pDir = opendir(src_path);
	if(pDir == NULL)
	{
		return -1;
	}

	while((pDirent = readdir(pDir)) != NULL)
	{
		if(strcmp(pDirent->d_name, "." ) == 0)	continue;
		if(strcmp(pDirent->d_name, "..") == 0)	continue;
		
		memset(chSrcFilename, 0, sizeof(chSrcFilename));
		memset(chDesFilename, 0, sizeof(chDesFilename));
		sprintf(chSrcFilename, "%s/%s", src_path, pDirent->d_name);
		sprintf(chDesFilename, "%s/%s", des_path, pDirent->d_name);

		if(rename(chSrcFilename, chDesFilename) != 0)
		{
			closedir(pDir);
			return -1;
		}
	}

	closedir(pDir);
	
	return 0;
}

/*
function:   cf_file_zerofile()
description:
			ɾ�����ֽ��ļ�
Input:
			filename:�����ļ�
Output:
Return:
			-1:����
			0:��ȷ
			1:�ǿ��ֽ��ļ�
others:
*/
int cf_file_zerofile(char *filename)
{
	struct stat fp_stat;

	if(stat(filename, &fp_stat) != 0)
	{
		return -1;
	}

	if(fp_stat.st_size != 0)
	{
		return 1;
	}

	if(remove(filename) != 0)
	{
		return -1;
	}
	
	return 0;
}

/*
function:   cf_files_delzero()
description:
			ɾ��һ��Ŀ¼�����п��ֽ��ļ�
Input:
			dirname:����Ŀ¼
Output:
Return:
			-1:����
			0:��ȷ
others:
*/
int cf_files_delzero(char *dirname)
{
	DIR  *p_dir;
	struct dirent *p_dirent;
	char filename[1024+1];

	p_dir = opendir(dirname);
	if(p_dir == NULL)
	{
		return -1;
	}

	while((p_dirent = readdir(p_dir)) != NULL)
	{
		if(strcmp(p_dirent->d_name, "." ) == 0)	continue;
		if(strcmp(p_dirent->d_name, "..") == 0)	continue;

		memset(filename, 0, sizeof(filename));
		sprintf(filename, "%s/%s", dirname, p_dirent->d_name);
		
		if(remove(filename) != 0)
		{
			return -1;
		}
	}

	closedir(p_dir);
	
	return 0;
}

/*
function:   cf_files_del()
description:
			ɾ��һ��Ŀ¼�������ļ�
Input:
			dirname:����Ŀ¼
Output:
Return:
			-1:����
			0:��ȷ
others:
*/
int cf_files_del(char *dirname)
{
	DIR  *p_dir;
	struct dirent *p_dirent;
	char filename[1024+1];

	p_dir = opendir(dirname);
	if(p_dir == NULL)
	{
		return -1;
	}

	while((p_dirent = readdir(p_dir)) != NULL)
	{
		if(strcmp(p_dirent->d_name, "." ) == 0)	continue;
		if(strcmp(p_dirent->d_name, "..") == 0)	continue;

		memset(filename, 0, sizeof(filename));
		sprintf(filename, "%s/%s", dirname, p_dirent->d_name);

		if(remove(filename) != 0)
		{
			closedir(p_dir);
			return -1;
		}
	}

	closedir(p_dir);
	
	return 0;
}


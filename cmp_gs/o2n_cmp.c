/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	o2n_cmp.c
author:		caoshun
version:		2.00
date:		2006-01-07
description:
			�����˶�-������-�����ļ�
others:
history:
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "o2n_cmp.h"
#include "business.h"

/*
function:   main()
description:
			�Ʒ�-��������-���س���
Input:      
            argc:�����в�������
            argv:�����в����б�
Output:
Return:     0:�ɹ���other:���ɹ�
others: 
*/
int main(int argc, char *argv[])
{
	int iRcode;

	/* ȫ�����ݳ�ʼ�� */
	busi_init();

	/* ������������Ƿ�Ϸ� */
	iRcode = busi_usage(argc, argv);

	if(iRcode)
	{
		return -1;
	}

	/* �������ļ� */
	iRcode = busi_read_cfgfile(argc, argv);
	if(iRcode)
	{
		return -1;
	}

	/* ���������ļ��е������Ƿ�Ϸ� */
	iRcode = busi_check_cfgfile();
	if(iRcode)
	{
		return -1;
	}

	/* ע��ϵͳ�˳��ҹ����� */
	iRcode = atexit(busi_atexit);
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG(ERROR_SYS_9999); 
		return -1;
	}

	/* ������ǰ���� */
	iRcode = busi_do_prefix();
	if(iRcode)
	{
		return -1;
	}
    
	BUSI_WRITE_RUNLOG("APP:MSG:application startup......");

	/* �������̿��� */
	iRcode = busi_do_Path();
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG("APP:MSG:application shutdown......");
		return -1;
	}
	
	BUSI_WRITE_RUNLOG("APP:MSG:application shutdown......");

	/* ���������� */
	iRcode = busi_do_postfix();
	if(iRcode)
	{
		return -1;
	}
	
	return 0;
}

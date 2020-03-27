/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	o2n_cmp.c
author:		caoshun
version:		2.00
date:		2006-01-07
description:
			话单核对-主程序-代码文件
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
			计费-无主回收-主控程序
Input:      
            argc:命令行参数个数
            argv:命令行参数列表
Output:
Return:     0:成功，other:不成功
others: 
*/
int main(int argc, char *argv[])
{
	int iRcode;

	/* 全局数据初始化 */
	busi_init();

	/* 分析命令参数是否合法 */
	iRcode = busi_usage(argc, argv);

	if(iRcode)
	{
		return -1;
	}

	/* 读配置文件 */
	iRcode = busi_read_cfgfile(argc, argv);
	if(iRcode)
	{
		return -1;
	}

	/* 分析配置文件中的内容是否合法 */
	iRcode = busi_check_cfgfile();
	if(iRcode)
	{
		return -1;
	}

	/* 注册系统退出挂钩函数 */
	iRcode = atexit(busi_atexit);
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG(ERROR_SYS_9999); 
		return -1;
	}

	/* 任务处理前操作 */
	iRcode = busi_do_prefix();
	if(iRcode)
	{
		return -1;
	}
    
	BUSI_WRITE_RUNLOG("APP:MSG:application startup......");

	/* 任务流程控制 */
	iRcode = busi_do_Path();
	if(iRcode)
	{
		BUSI_WRITE_RUNLOG("APP:MSG:application shutdown......");
		return -1;
	}
	
	BUSI_WRITE_RUNLOG("APP:MSG:application shutdown......");

	/* 任务处理后操作 */
	iRcode = busi_do_postfix();
	if(iRcode)
	{
		return -1;
	}
	
	return 0;
}

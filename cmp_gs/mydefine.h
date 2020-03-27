/*
Copyright (C), 1995-2006, Si-Tech Information Technology Ltd. 
file_name:	mydefine.h
author:		caoshun
version:	1.03
date:		2005-03-04
description:
			与业务无关-内部底层宏与结构定义-H文件
others:
history:
*/

#ifndef _MYPUBLIC_H_
#define _MYPUBLIC_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* 打印内部调试信息 */
#ifdef DEBUG
#define AUTOCALL_DEBUG printf("DEBUG->FILE[%s]LINE[%d]ERRNO[%d]ERRMSG[%s]\n", __FILE__, __LINE__, errno, strerror(errno));fflush(stdout);
#else
#define AUTOCALL_DEBUG
#endif

/* 自定义函数宏*/
#define MEMSET(aaa)  memset(aaa, 0, sizeof(aaa))
#define MEMSET1(aaa) memset(&aaa, 0, sizeof(aaa))

#if defined(__cplusplus)
}
#endif

#endif

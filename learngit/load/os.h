/***********************************************************
 * Copyright (c) 1999,2002.
 * Descriptions:
 *      os.h
 *      定义操作系统.
 * Known bugs:
 *
 * Change log:
 *        who          MM/DD/YYYY        why
 *        wangjs       03/20/2005        create
 **********************************************************/

#ifndef _OS_H_
#define _OS_H_

#define  UNIX_OS
/*#define  WIN32_OS*/

#ifdef UNIX_OS
  #define  PATH_CHAR        '/'
#else
  #define  PATH_CHAR        '\\'
  #define  pid_t            int
  
  #define  fork()   (0)
  #define  wait(i)   (0)
  #define WEXITSTATUS(i)  (0)
#endif

#endif

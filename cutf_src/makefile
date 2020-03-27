#!/usr/sh
# FOR HP 64 BIT SYSTEM
#CFLAGS = -w -Os -O3 -Wall -pedantic-errors -ldl -lpthread
CFLAGS = -w -g -m64 -ldl -lpthread

# FOR HP 64 BIT SYSTEM
AR = ar -rcs

DEFINE=-DSS_64BIT_SERVER -D_REENTRANT -D FEDX #-D_DEBUG_
IFLAGS=-I. -I ${FEDX_HOME}/include 
#LFLAGS=-L. -L${LIBXML}/lib/libxml  -lxml2 -lm #-lpthread 
LIBXMLLFLAGS=  -L${FEDX_HOME}/lib/libxml  -lxml2
FEDXLFLAGS= -lm -L. -L${FEDX_HOME}/lib -lfedx  ${LIBXMLLFLAGS}

CC=cc
COMPILE = ${CC} ${CFLAGS} ${IFLAGS} ${DEFINE} 
MAKEEXE = ${COMPILE}

DECOBJ=cmain.o common.o

all:	cutf

cutf:${DECOBJ}
	${MAKEEXE} -o $@ ${DECOBJ} ${FEDXLFLAGS}

.c.o:
	${COMPILE} -c $<
clean:
	rm -f  *.o  cutf

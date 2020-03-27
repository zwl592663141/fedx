TARGET=setf
SOURCES:=$(wildcard ./*.c)
OBJECTS:=$(patsubst %.c,%.o,$(SOURCES))
IFLAGS=-I${FEDX_HOME}/include
LFLAGS=-L${FEDX_HOME}/lib -lfedx -L${FEDX_HOME}/lib/libxml -lxml2
CC=gcc
CFLAGS=-g -Wall ${IFLAGS}


all:${TARGET}

${OBJECTS}:${SOURCES}
	$(CC) $(CFLAGS) -c $<
${TARGET}:${OBJECTS}
	$(CC) -o $@ $< ${LFLAGS}

.PHONY:clean
clean:
	rm *.o
	rm setf
	
#gcc setf.c -o setf -I${FEDX_HOME}/include -L${FEDX_HOME}/lib -lfedx -L${FEDX_HOME}/lib/libxml -lxml2

CC=					cc
#C_FLAG=				-xarch=native64
C_FLAG=			-g	-q64 #-bmaxdata:0xFFFFFFFF
#C_FLAG=				+DA2.0W +DS2.0 -DSS_64BIT_SERVER -DDEBUG

OTHER_FILE=			o2n_cmp.cfg

C_FILE=				o2n_cmp.c\
					function.c\
					business.c\
					ErrLog.c\
					CookieFunc.c\
					IniFile.c\
					o2n_cmp_ck.c\
					o2n_cmp_fm.c

OBJ_FILE=			o2n_cmp.o\
					function.o\
					business.o\
					ErrLog.o\
					CookieFunc.o\
					IniFile.o

INCLUDE_FILE=		mydefine.h\
					o2n_cmp.h\
					business.h\
					function.h\
					ErrLog.h\
					CookieFunc.h\
					IniFile.h


EXE_FILE=			o2n_cmp\
					o2n_cmp_ck\
					o2n_cmp_fm

INCLUDES=			-I.

LIBS=				-L. -lpthread

all : $(EXE_FILE)

o2n_cmp_ck : o2n_cmp_ck.c
	$(CC) $(C_FLAG) -o $@ $? $(LIBS)
	
o2n_cmp_fm : o2n_cmp_fm.c
	$(CC) $(C_FLAG) -o $@ $? $(LIBS)	

o2n_cmp : $(OBJ_FILE) $(INCLUDE_FILE)
	$(CC) $(C_FLAG) -o $@ $(OBJ_FILE) $(LIBS)

o2n_cmp.o : o2n_cmp.c $(INCLUDE_FILE)
	$(CC) $(C_FLAG) -c o2n_cmp.c $(INCLUDES)

business.o : business.c $(INCLUDE_FILE)
	$(CC) $(C_FLAG) -c business.c $(INCLUDES)

function.o : function.c $(INCLUDE_FILE)
	$(CC) $(C_FLAG) -c function.c $(INCLUDES)

ErrLog.o : ErrLog.c $(INCLUDE_FILE)
	$(CC) $(C_FLAG) -c ErrLog.c $(INCLUDES)

CookieFunc.o : CookieFunc.c $(INCLUDE_FILE)
	$(CC) $(C_FLAG) -c CookieFunc.c $(INCLUDES)

IniFile.o : IniFile.c $(INCLUDE_FILE)
	$(CC) $(C_FLAG) -c IniFile.c $(INCLUDES)

clean:
	rm $(EXE_FILE) $(OBJ_FILE) $(LIB_FILE)

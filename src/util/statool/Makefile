# ------------------------------------------------------- #
#  util/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #
BBSHOME=../../../

# For FreeBSD
CC	= gcc
CFLAGS	= -O2 -DBSD44 -pipe -fomit-frame-pointer -I../../include -I../../WD -D_BBS_UTIL_C_
LDFLAGS	= -s -L../../lib -ldao -lcrypt

# For SunOS 5.6
#CC	= gcc
#CFLAGS	= -O2 -pipe -fomit-frame-pointer -I../include -I../WD -DSYSV -DSOLARIS
#LDFLAGS	= -ltermcap -lnsl -lsocket -ldl -L../lib -ldao

# For Linux
#CC	= gcc
#CFLAGS	= -O2 -pipe -fomit-frame-pointer -I../include -I../WD -DLINUX -D_BBS_UTIL_C_
#LDFLAGS	= -s -fforce-mem -funroll-loops -fstrict-prototype -Wunused -L../lib -ldao -lcrypt

.SUFFIXES: .o .c

.c:   ;   $(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

TOOL =	account poststat birth gnp horoscope topmsg voteboard\
	topquery yearsold toppost toptime toplogin\
	topmoney topboard topgold sexcount toplimit\
	admin bm_check toppersonal post_open topexp

#	toprpg topexp topfive
all: 	$(TOOL)
#	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)


install:$(TOOL)
	strip $(TOOL)
	chown bbs.bbs $(TOOL)
	cp -fp $(TOOL) $(BBSHOME)/bin

clean:
	rm -fr $(TOOL) *.o *.BAK *.log *~

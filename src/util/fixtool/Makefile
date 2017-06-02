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
CFLAGS	= -O2 -DBSD44 -pipe -fomit-frame-pointer -I$(BBSHOME)/src/include -I$(BBSHOME)/src/WD -D_BBS_UTIL_C_
LDFLAGS	= -s -L$(BBSHOME)/src/lib -ldao -lcrypt

# For SunOS 5.6
#CC	= gcc
#CFLAGS	= -O2 -pipe -fomit-frame-pointer -I$(BBSHOME)/src/include -I$(BBSHOME)/src/WD -DSYSV -DSOLARIS
#LDFLAGS	= -ltermcap -lnsl -lsocket -ldl -L$(BBSHOME)/src/lib -ldao

# For Linux
#CC	= gcc
#CFLAGS	= -O2 -pipe -fomit-frame-pointer -I$(BBSHOME)/src/include -I$(BBSHOME)/src/WD -DLINUX -D_BBS_UTIL_C_
#LDFLAGS	= -s -fforce-mem -funroll-loops -fstrict-prototype -Wunused -L$(BBSHOME)/src/lib -ldao -lcrypt

.SUFFIXES: .o .c

.c:   ;   $(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

TOOL =	buildir buildetc buildnames repasswd rebrd\
        transboard transb transvote transpal\
	transbpal transaloha repasswd_online\
	transpass

#	hashrpg transrpg
all: 	$(TOOL)
#	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)


install:$(TOOL)
	strip $(TOOL)
	chown bbs.bbs $(TOOL)
	cp -fp buildir $(BBSHOME)/bin

clean:
	rm -fr $(TOOL) *.o *.BAK *.log *~

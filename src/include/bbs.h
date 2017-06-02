/*-------------------------------------------------------*/
/* bbs.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : all header files			 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/

#ifndef	_BBS_H_
#define	_BBS_H_

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

/* wildcat 搬過來,免得一直重覆include */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <time.h>
#include <sys/time.h>

#if	defined(__linux__) && (__GLIBC__ != 2)
#include <bsd/sgtty.h>
#else
#include <sgtty.h>
#endif


#ifdef  SYSV

#ifndef LOCK_EX
#define LOCK_EX		F_LOCK
#define LOCK_UN		F_ULOCK
#endif

#define getdtablesize()         (64)

#define usleep(usec)            {               \
    struct timeval t;                           \
    t.tv_sec = usec / 1000000;                  \
    t.tv_usec = usec % 1000000;                 \
    select( 0, NULL, NULL, NULL, &t);           \
}

#endif				/* SYSV */

#define	BMIN(a,b)	((a<b)?a:b)
#define	BMAX(a,b)	((a>b)?a:b)

#define YEA (1)			/* Booleans  (Yep, for true and false) */
#define NA  (0)

#define NOECHO		0x0000		/* Flags to getdata input function */
#define DOECHO		0x0100
#define LCECHO		0x0200
#define GCARRY		0x0400
#define PASS		0x1000

#define GET_LIST        0x1000          /* 取得 Link List */
#define GET_USER        0x2000          /* 取得 user id */
#define GET_BRD         0x4000          /* 取得 board id */

#define I_TIMEOUT   (-2)	/* Used for the getchar routine select call */
#define I_OTHERDATA (-333)	/* interface, (-3) will conflict with chinese */

#include "config.h"		/* User-configurable stuff */
#include "perm.h"		/* user/board permission */
#include "struct.h"		/* data structure */
#include "global.h"		/* global variable & definition */
#include "modes.h"		/* The list of valid user modes */
#include "dao.h"
#include "attr.h"		/* Thor.990311: dynamic attribute database */

#endif				/* _BBS_H_ */


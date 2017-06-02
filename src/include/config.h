/*-------------------------------------------------------*/
/* config.h     ( WD-BBS Ver 2.3 )			 */
/*-------------------------------------------------------*/
/* target : site-configurable settings                   */
/* create : 95/03/29                                     */
/* update : 98/12/09                                     */
/*-------------------------------------------------------*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* ----------------------------------------------------- */
/* ©w¸q BBS ¯¸¦W¦ì§}                                     */
/* ------------------------------------------------------*/

#define BOARDNAME       "«Â¥§´µ©@°ØÀ]"			/* ¤¤¤å¯¸¦W */
#define BBSNAME         "Venice Cofe BBS"		/* ­^¤å¯¸¦W */
#define MYHOSTNAME      "infor.org"			/* ºô§} */
#define MYIP		"203.64.26.200"			/* IP */
#define MYVERSION       "WD-BBS 2.9 SNAP"		/* µ{¦¡ª©¥» */
#define MYBBSID         "WD"				/* Âà«H¥N½X */
#define BBSHOME         "/home/i89/hialan/MYBBS/bbs"	/* BBS ªº®a */
#define BBSUSER         "hialan"
#define BBSUID          2319
#define BBSGID          1005
#define TAG_VALID	"[WD BBS]To "	
			/* shakalaca.000814: §ï¥Î MagicKey »{ÃÒ
					     ¨ä¹ê´N¤£¥Î³o­Óª±·N¤F :p */

/* ----------------------------------------------------- */
/* ²ÕºA³W¹º                                              */
/* ------------------------------------------------------*/

#define HAVE_CHKLOAD		/* check cpu load */
#ifdef HAVE_CHKLOAD
  #define MAX_CPULOAD     (10)            /* CPU ³Ì°ªload */
  #define MAX_SWAPUSED    (10)            /* SWAP³Ì°ª¨Ï¥Î²v */
#endif

#define WITHOUT_CHROOT		/* ¤£»Ý­n root set-uid */
#define HAVE_MOVIE		/* Åã¥Ü°ÊºA§i¥ÜªO */
#define INTERNET_PRIVATE_EMAIL	/* ¥i¥H±H¨p¤H«H¥ó¨ì Internet */
#define BIT8			/* ¤ä´©¤¤¤å¨t²Î */
#define GB_SUPPORT		/* ¤ä´©Åã¥Ü ÁcÅé->Â²Åé¤¤¤å½X */
#define DEFAULTBOARD	"SYSOP"	/* ¹w³]¬ÝªO */
#define LOGINATTEMPTS	(3)	/* ³Ì¤j¶i¯¸¥¢»~¦¸¼Æ */
#define LOGINASNEW		/* ±Ä¥Î¤W¯¸¥Ó½Ð±b¸¹¨î«× */
#define INTERNET_EMAIL		/* ¤ä´© InterNet Email ¥\¯à(§t Forward) */
#define	REG_MAGICKEY		/* µo¥X MagicKey eMail ¨­¥÷»{ÃÒ«H¨ç */
#define	REG_FORM		/* shakalaca: ¶ñµù¥U³æ */
				/* shakalaca.000813: »{ÃÒ¤è¦¡½Ð¾Ü¤@ */
#undef  NEWUSER_LIMIT		/* ·s¤â¤W¸ôªº¤T¤Ñ­­¨î */
#undef  POSTNOTIFY		/* ·s¤å³¹³qª¾¥\¯à */
#undef  INVISIBLE		/* ÁôÂÃ BBS user ¨Ó¦Û¦ó³B */
#define MULTI_NUMS	(1)	/* ³Ì¦h­«½Æ¤W¯¸¤H¦¸ (guest°£¥~) */
#define INITIAL_SETUP		/* ­è¶}¯¸¡AÁÙ¨S«Ø¥ß¹w³]¬ÝªO[SYSOP] */
#define HAVE_MMAP		/* ±Ä¥Î mmap(): memory mapped I/O */
#define HAVE_ANONYMOUS		/* ´£¨Ñ Anonymous ªO */
#define HAVE_ORIGIN		/* Åã¥Ü author ¨Ó¦Û¦ó³B */
#define HAVE_MAILCLEAN		/* ²M²z©Ò¦³¨Ï¥ÎªÌ­Ó¤H«H½c */
#define WHERE			/* ¬O§_¦³¬G¶m¥\¯à */
#define HAVE_NOTE_2		/* wildcat:¤p¬ÝªO¥\¯à */
#define HAVE_GEM_GOPHER		/* shakalaca: ³s½uºëµØ°Ï */
#define FORM_REG		/* shakalaca: ¶ñµù¥U³æ */

/* ----------------------------------------------------- */
/* ÀH BBS ¯¸³W¼Ò¦¨ªø¦ÓÂX¼W                               */
/* ----------------------------------------------------- */

#define MAXUSERS        (65536)         /* ³Ì°ªµù¥U¤H¼Æ */
#define MAXBOARD        (512)           /* ³Ì¤j¶}ª©­Ó¼Æ */
#define MAXACTIVE       (384)           /* ³Ì¦h¦P®É¤W¯¸¤H¼Æ */
#define MAX_FRIEND      (128)           /* ¸ü¤J cache ¤§³Ì¦hªB¤Í¼Æ¥Ø */
#define MAX_REJECT      (16)            /* ¸ü¤J cache ¤§³Ì¦hÃa¤H¼Æ¥Ø */
#define MAX_MOVIE       (999)		/* ³Ì¦h°ÊºA¬Ýª©¼Æ */
#define MAX_FROM        (1024)		/* ³Ì¦h¬G¶m¼Æ */
#define MAX_REVIEW      (10)            /* ³Ì¦h¤ô²y¦^ÅU */

/* ----------------------------------------------------- */
/* ¨ä¥L¨t²Î¤W­­°Ñ¼Æ                                      */
/* ----------------------------------------------------- */

#define MAX_HISTORY     12		/* °ÊºA¬ÝªO«O«ù 12 µ§¾ú¥v°O¿ý */
#define MAXKEEPMAIL     (100)		/* ³Ì¦h«O¯d´X«Ê MAIL¡H */
#define MAXEXKEEPMAIL   (400)		/* ³Ì¦h«H½c¥[¤j¦h¤Ö«Ê */
#define MAX_NOTE        (32)            /* ³Ì¦h«O¯d´X½g¯d¨¥¡H */
#define MAXSIGLINES     (8)             /* Ã±¦WÀÉ¤Þ¤J³Ì¤j¦æ¼Æ */
#define MAXQUERYLINES   (16)            /* Åã¥Ü Query/Plan °T®§³Ì¤j¦æ¼Æ */
#define MAXPAGES        (999)           /* more.c ¤¤¤å³¹­¶¼Æ¤W­­ (lines/22) */
#define MOVIE_INT       (10)		/* ¤Á´«°Êµeªº¶g´Á®É¶¡ 12 ¬í */
#define	MAXTAGS		(256)		/* post/mail reader ¼ÐÅÒ¼Æ¥Ø¤§¤W­­ */

/* ----------------------------------------------------- */
/* µo§b¹L¤[¦Û°ÊÃ±°h                                      */
/* ------------------------------------------------------*/

#define LOGIN_TIMEOUT	(5*60)		/* login ®É¦h¤[¨S¦¨¥\Ã±¤J´NÂ_½u */

#define DOTIMEOUT

#ifdef  DOTIMEOUT
#define IDLE_TIMEOUT    (60*60*6)	/* ¤@¯ë±¡ªp¤§ timeout (¬í)*/
#define SHOW_IDLE_TIME                  /* Åã¥Ü¶¢¸m®É¶¡ */
#endif

/* ----------------------------------------------------- */
/* chat.c & xchatd.c ¤¤±Ä¥Îªº port ¤Î protocol           */
/* ------------------------------------------------------*/

#define CHATPORT	3838

#define MAXROOM         16              /* ³Ì¦h¦³´X¶¡¥]´[¡H */

#define EXIT_LOGOUT     0
#define EXIT_LOSTCONN   -1
#define EXIT_CLIERROR   -2
#define EXIT_TIMEDOUT   -3
#define EXIT_KICK       -4

#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"

#define BADCIDCHARS " *:/\,;.?"        /* Chat Room ¤¤¸T¥Î©ó nick ªº¦r¤¸ */


/* ----------------------------------------------------- */
/* ¨t²Î°Ñ¼Æ      cache                                   */
/* ----------------------------------------------------- */
#define MAGIC_KEY       2002   /* ¨­¤À»{ÃÒ«H¨ç½s½X */

#define SEM_ENTER      -1      /* enter semaphore */
#define SEM_LEAVE      1       /* leave semaphore */
#define SEM_RESET      0       /* reset semaphore */

#define BRDSHM_KEY      1415
#define UIDSHM_KEY      1417
#define UTMPSHM_KEY     1419
#define FILMSHM_KEY     1421    /* °ÊºA¬Ýª© , ¸`¤é */
#define FROMSHM_KEY     1423    /* whereis, ³Ì¦h¨Ï¥ÎªÌ */

#define BRDSEM_KEY      2305
#define FILMSEM_KEY     2300    /* semaphore key */
#define FROMSEM_KEY     2303    /* semaphore key */

/* ----------------------------------------------------- */
/* ¥Ó½Ð±b¸¹®É­n¨D¥Ó½ÐªÌ¯u¹ê¸ê®Æ                          */
/* ----------------------------------------------------- */

#define SHOWUID                 /* ¬Ý¨£¨Ï¥ÎªÌ UID */
#define SHOWTTY                 /* ¬Ý¨£¨Ï¥ÎªÌ TTY */
#define SHOWPID                 /* ¬Ý¨£¨Ï¥ÎªÌ PID */

#define REALINFO                /* ¯u¹ê©m¦W */

#ifdef  REALINFO
#undef  ACTS_REALNAMES          /* ¥D¥Ø¿ýªº (U)ser Åã¥Ü¯u¹ê©m¦W */
#undef  POST_REALNAMES          /* ¶K¤å¥ó®Éªþ¤W¯u¹ê©m¦W */
#undef  MAIL_REALNAMES          /* ±H¯¸¤º«H¥ó®Éªþ¤W¯u¹ê©m¦W */
#undef  QUERY_REALNAMES         /* ³Q Query ªº User §iª¾¯u¹ê©m¦W */
#endif

/* ----------------------------------------------------- */
/* http                                                  */
/* ----------------------------------------------------- */

#define USE_LYNX   	      /* ¨Ï¥Î¥~³¡lynx dump ? */
#undef USE_PROXY
#ifdef  USE_PROXY
#define PROXYSERVER "140.112.28.165"
#define PROXYPORT   3128
#endif

#define LOCAL_PROXY           /* ¬O§_¶}±Òlocal ªºproxy */
#ifdef  LOCAL_PROXY
#define HPROXYDAY   3         /* localªºproxy refresh¤Ñ¼Æ */
#endif

/* ----------------------------------------------------- */
/* ´£¨Ñ¥~±¾µ{¦¡                                          */
/* ----------------------------------------------------- */

#define HAVE_EXTERNAL

  #ifdef HAVE_EXTERNAL
    #undef HAVE_GOPHER     /* ´£¨Ñ gopher */
    #undef HAVE_WWW        /* ´£¨Ñ www browser */
    #define HAVE_BIG2       /* ´£¨Ñ big2 game */
    #define HAVE_MJ         /* ´£¨Ñ mj game */
    #define HAVE_BRIDGE     /* ´£¨Ñ Ok Bridge */
    #undef HAVE_ARCHIE     /* have arche */
    #undef BBSDOORS        /* ´£¨Ñ bbs doors */
    #define HAVE_GAME       /* ´£¨Ñºô¸ô³s½u¹CÀ¸ */
  #endif

#endif

/* ----------------------------------------------------- */
/* WD ¦Û­q©w¸qâ                                          */
/* ----------------------------------------------------- */

/*#define HYPER_BBS*/
#define ANNOUNCE_BRD	"Announce"
#define VOTEBOARD	"VoteBoard"
#define PERSONAL_ALL_BRD	"Personal.All"
#define HIDE_ALL_BRD	"Hide_All"

#define DEF_MAXP	2500	/* ¬ÝªO¤å³¹°ò¥»¤W­­¼Æ¶q */
#define DEF_MAXT	365	/* ¬ÝªO¤å³¹°ò¥»«O¯d¤Ñ¼Æ */

#define COLOR1	"[46;37m"
#define COLOR2	"[1m[44;33m"
#define COLOR3	"[1;44m"

/* ----------------------------------------------------- */
/* ¶®¼Ö¥§®¦ ¦Û­q©w¸qâ                                    */
/* ----------------------------------------------------- */
                                                                                
#define MAXPATHLEN      256     /* µ{¦¡¸ô®|ªº³Ì¤jªø«× */
#define MAXMONEY(cuser) ((cuser.totaltime*10) + (cuser.numlogins*100) + (cuser.numposts*1000))
  /* ª÷¿ú¤W­­ hialan:030131*/
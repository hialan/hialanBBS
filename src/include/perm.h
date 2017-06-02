/*-------------------------------------------------------*/
/* perm.h       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : permission levels of user & board            */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#ifndef _PERM_H_
#define _PERM_H_

/* ----------------------------------------------------- */
/* These are the 16 basic permission bits.               */
/* ----------------------------------------------------- */

#define PERM_BASIC        000000000001
#define PERM_CHAT         000000000002
#define PERM_PAGE         000000000004
#define PERM_POST         000000000010
#define PERM_LOGINOK      000000000020
#define PERM_MAILLIMIT    000000000040
#define PERM_CLOAK        000000000100
#define PERM_SEECLOAK     000000000200
#define PERM_XEMPT        000000000400
#define PERM_DENYPOST     000000001000
#define PERM_BM           000000002000
#define PERM_ACCOUNTS     000000004000
#define PERM_CHATROOM     000000010000
#define PERM_BOARD        000000020000
#define PERM_SYSOP        000000040000
#define PERM_POSTMASK     000000100000 
#define PERM_BBSADM       000000100000 
#define PERM_NOTOP        000000200000 
#define PERM_XFILE        000000400000 
#define PERM_RESEARCH	  000001000000 
#define PERM_FROM         000002000000 
#define PERM_GOOD         000004000000 
#define PERM_22           000010000000 
#define PERM_ANNOUNCE     000020000000 
#define PERM_MG           000040000000 
#define PERM_SMG          000100000000 
#define PERM_AD           000200000000 
#define PERM_SAD          000400000000 
#define PERM_PAINT        001000000000 
#define PERM_SPAINT       002000000000 
#define PERM_SECRETARY    004000000000 
#define PERM_LSYSOP       010000000000 
#define PERM_WILDCAT      020000000000 

/* ----------------------------------------------------- */
/* These permissions are bitwise ORs of the basic bits.  */
/* ----------------------------------------------------- */

 
/* 新使用者擁有的權限 */
/* wildcat :
   必要時 可把PERM_POST拿掉，改為必須email認證後方有 */
#define PERM_DEFAULT    (PERM_BASIC | PERM_POST)

#define PERM_ADMIN      (PERM_ACCOUNTS | PERM_SYSOP)
#define PERM_ALLBOARD   (PERM_SYSOP | PERM_BOARD)
#define PERM_LOGINCLOAK (PERM_SYSOP | PERM_ACCOUNTS)
#define PERM_SEEULEVELS PERM_SYSOP
#define PERM_SEEBLEVELS (PERM_SYSOP | PERM_BM)
#define PERM_NOTIMEOUT  PERM_SYSOP

#define PERM_READMAIL   PERM_BASIC
#define PERM_FORWARD    PERM_BASIC      /* to do the forwarding */

#define PERM_INTERNET   PERM_LOGINOK    /* 身份認證過關的才能寄信到 Internet */

#define HAS_PERM(x)     ((x)?cuser.userlevel&(x):1)
#define HAVE_PERM(x)    (cuser.userlevel&(x))
#define PERM_HIDE(u) ((u)->userlevel & PERM_SYSOP && (u)->userlevel & PERM_DENYPOST)


/* ----------------------------------------------------- */
/* 各種權限的中文意義                                    */
/* ----------------------------------------------------- */

#define NUMPERMS        32

#ifndef _ADMIN_C_
extern char *permstrings[];

#else
char *permstrings[] = {
  "基本權力",                   /* PERM_BASIC */
  "進入聊天室",                 /* PERM_CHAT */
  "找人聊天",                   /* PERM_PAGE */
  "發表文章",                   /* PERM_POST */
  "註冊程序認證",               /* PERM_LOGINOK */
  "信件無上限",                 /* PERM_MAILLIMIT */
  "隱身術",                     /* PERM_CLOAK */
  "看見忍者",                   /* PERM_SEECLOAK */
  "永久保留帳號",               /* PERM_XEMPT */
  "站長隱身術",                 /* PERM_DENYPOST */
  "板主",                       /* PERM_BM */
  "帳號總管",                   /* PERM_ACCOUNTS */
  "聊天室總管",                 /* PERM_CHATCLOAK */
  "看板總管",                   /* PERM_BOARD */
  "站長",                       /* PERM_SYSOP */
  "BBSADM",                     /* PERM_BBSADM & PERM_POSTMASK */
  "不列入排行榜",               /* PERM_NOTOP */
  "管理站上文件",               /* PERM_XFILE */
  "研發小組",                   /* PERM_RESEARCH */
  "修改故鄉",                   /* PERM_FROM */
  "文藝展裁判",                 /* PERM_GOOD */
  "沒想到",                     /* PERM_ */
  "精華區總長",                 /* PERM_Announce */
  "特務組",                     /* PERM_MG */
  "特務組長",                   /* PERM_SMG */
  "文宣組",                     /* PERM_AD */
  "文宣組長",                   /* PERM_SAD */
  "美工組",                     /* PERM_PAINT */
  "美工組長",                   /* PERM_SPAINT */
  "秘書",                       /* PERM_SECRETARY */
  "小站長",                     /* PERM_LSYSOP */
  "野貓老大"                    /* PERM_CAVE */  
};

#endif
#endif                          /* _PERM_H_ */

/*-------------------------------------------------------*/
/* habit.h      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : 使用者喜好設定                               */
/* create : 98/05/21                                     */
/*-------------------------------------------------------*/

#ifndef _HABIT_H_
#define _HABIT_H_

/* ----------------------------------------------------- */
/* these are habits in userec.habit                      */
/* ----------------------------------------------------- */

#define HABIT_MOVIE       000000000001    /* 開/關動態看板 */
#define HABIT_COLOR       000000000002    /* 彩色/黑白切換 */
#define HABIT_ALARM	  000000000004    /* 半點報時 */
#define HABIT_BELL	  000000000010	  /* 聲音 */
#define HABIT_BOARDLIST	  000000000020    /* 看板列表顯示文章數或是編號 */
#define HABIT_CYCLE	  000000000040    /* 循環式閱讀 */
#define HABIT_LIGHTBAR	  000000000100	  /* 光棒選單 */
#define HABIT_NOBRDHELP	  000000000200	  /* 不顯示看板說明 */

#define HAS_HABIT(x)     ((x)?cuser.habit&(x):1)
#define HAVE_HABIT(x)    (cuser.habit&(x))

#define HABIT_NEWUSER    (HABIT_MOVIE | HABIT_COLOR | HABIT_ALARM | HABIT_BELL | HABIT_LIGHTBAR)
#define HABIT_GUEST	 (HABIT_MOVIE | HABIT_COLOR | HABIT_LIGHTBAR)
/* ----------------------------------------------------- */
/* 各種喜好設定的中文意義                                */
/* ----------------------------------------------------- */


#ifndef _USER_C_
extern char *habitstrings[];

#else

#define NUMHABITS        8

char *habitstrings[] = {
/* HABIT_MOVIE */      "動態看板       ",
/* HABIT_COLOR */      "彩色顯示模式   ",
/* HABIT_ALARM */      "半點報時       ",
/* HABIT_BELL  */      "聲音           ",
/* HABIT_BOARDLIST */  "看板列表顯示   ",
/* HABIT_CYCLE */      "循環式閱\讀    ",
/* HABIT_LIGHTBAR */   "使用選單光棒   ",
/* HABIT_NOBRDHELP */  "不顯示看板說明"
};


char *habit_help_strings[] ={
/* HABIT_MOVIE */      "切換要不要顯示動態看板",
/* HABIT_COLOR */      "切換要不要顯示彩色的畫面，如果關閉的話所有畫面會變成單色顯示",
/* HABIT_ALARM */      "選擇是否要系統在整點發送訊息",
/* HABIT_BELL  */      "選擇是否要系統發出提醒聲",
/* HABIT_BOARDLIST */  "看板列表時，最前方顯示文章數或編號",
/* HABIT_CYCLE */      "在文章列表時，看到最後一篇會自動回到第一篇繼續閱\讀",
/* HABIT_LIGHTBAR */   "選擇是否使用選單光棒   ",
/* HABIT_NOBRDHELP */  "選擇看板時，不顯示看板說明  "
};

#endif

#endif                          /* _HABIT_H_ */

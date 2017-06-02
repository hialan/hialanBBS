/*-------------------------------------------------------*/
/* global.h     ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : global definitions & variables               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/* ----------------------------------------------------- */
/* GLOBAL DEFINITION                                     */
/* ----------------------------------------------------- */


/* ÀÉ¦W³]©w */

#define FN_PASSWD       ".PASSWDS"      /* User records */
#define FN_BOARD        ".BOARDS"       /* board list */
#define FN_GAME		".GAME"		/* RPG records */
#define FN_USIES        "usies"         /* BBS log */
#define FN_USSONG       "ussong"        /* ÂIºq */
#define FN_VOTE		".VCH"		/* §ë²¼ */
#define FN_LIST		".LIST"		/* ¦W³æ */
#define FN_POSTPREFIX	".POSTPREFIX"	/* ¼ÐÃDÃþ§O by hialan 3/21/2002*/
#define FN_POST_NOTE	".POSTNOTE"	/* ychia.021212:¬ÝªO¦Û­q¤å³¹µoªí­n»â */
#define FN_MAGICKEY	"MagicKey"	/* »{ÃÒ½XÀÉ®× */
#define FN_PAL		"pal"
#define FN_ALOHA	"aloha"

#define FN_TOPSONG      "log/topsong"
#define FN_GAMEMONEY    "game/money"
#define FN_MONEYLOG     "log/moneystat"
#define FN_OVERRIDES    "overrides"
#define FN_REJECT       "reject"
#define FN_CANVOTE      "can_vote"
#define FN_WATER        "water"
#define FN_APPLICATION  "application"
#define FN_VISABLE      "visable"
#define FN_MYFAVORITE   "favorite"

#define FN_TICKET_RECORD "game/ticket.data"
#define FN_TICKET_USER   "game/ticket.user"
#define FN_TICKET        "game/ticket.result"

#define FN_BMW          "bmw"           /* itoc.011104: for BMW */

#define DEFAULT_BOARD   str_sysop

/* Áä½L³]©w */

#ifndef EXTEND_KEY
#define EXTEND_KEY
#define KEY_TAB         9
#define KEY_ESC         27
#define KEY_UP          0x0101
#define KEY_DOWN        0x0102
#define KEY_RIGHT       0x0103
#define KEY_LEFT        0x0104
#define KEY_HOME        0x0201
#define KEY_INS         0x0202
#define KEY_DEL         0x0203
#define KEY_END         0x0204
#define KEY_PGUP        0x0205
#define KEY_PGDN        0x0206
#endif

#define Ctrl(c)         ( c & 037 )

#ifdef SYSV
#undef CTRL                     /* SVR4 CTRL macro is hokey */
#define CTRL(c) ('c'&037)       /* This gives ESIX a warning...ignore it! */
#endif

#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)
#define char_lower(c)  ((c >= 'A' && c <= 'Z') ? c|32 : c)

/* ----------------------------------------------------- */
/* External function declarations                        */
/* ----------------------------------------------------- */

#define TRACE   log_usies

/* ----------------------------------------------------- */
/* °T®§¦r¦ê¡G¿W¥ß¥X¨Ó¡A¥H§Q¤ä´©¦UºØ»y¨¥                  */
/* ----------------------------------------------------- */

#define STR_CURSOR      ">>"
#define STR_UNCUR       "  "

#define STR_AUTHOR1     "§@ªÌ:"
#define STR_AUTHOR2     "µo«H¤H:"
#define STR_POST1       "¬ÝªO:"
#define STR_POST2       "¯¸¤º:"

#define LEN_AUTHOR1     5
#define LEN_AUTHOR2     7

#define STR_GUEST       "guest"

#define MSG_SEPERATOR   "\
¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w"

#define MSG_CLOAKED     "¼K¼K,¸ú°_¨ÓÅo!"
#define MSG_UNCLOAK     "­«²{¦¿´ò¤F...."

#define MSG_WORKING     "³B²z¤¤¡A½Ðµy­Ô..."

#define MSG_CANCEL      "¨ú®ø¡C"
#define MSG_USR_LEFT    "User ¤w¸gÂ÷¶}¤F"
#define MSG_NOBODY      "¥Ø«eµL¤H¤W½u"
#define MSG_MY_FAVORITE "§Úªº³Ì·R¬ÝªO"

#define MSG_DEL_OK      "§R°£§¹²¦"
#define MSG_DEL_CANCEL  "¨ú®ø§R°£"
#define MSG_DEL_ERROR   "§R°£¿ù»~"
#define MSG_DEL_NY      "½Ð½T©w§R°£(Y/N)?[N] "

#define MSG_FWD_OK      "¤å³¹Âà±H§¹¦¨!"
#define MSG_FWD_ERR1    "Âà±H¥¢»~: ¨t²Îµo¥Í¿ù»~"
#define MSG_FWD_ERR2    "Âà±H¥¢»~: ¦a§}¿ù»~¡A¬dµL¦¹¤H"

#define MSG_SURE_NY     "½Ð±z½T©w(Y/N)¡H[N] "
#define MSG_SURE_YN     "½Ð±z½T©w(Y/N)¡H[Y] "

#define MSG_BID         "½Ð¿é¤J¬ÝªO¦WºÙ¡G"
#define MSG_UID         "½Ð¿é¤J¨Ï¥ÎªÌ¥N¸¹¡G"
#define MSG_PASSWD      "½Ð¿é¤J±zªº±K½X: "

#define MSG_BIG_BOY     "«Ó­ô"
#define MSG_BIG_GIRL    "¬ü¤k"
#define MSG_LITTLE_BOY  "©³­}"
#define MSG_LITTLE_GIRL "¬ü¬Ü"
#define MSG_MAN         "¨û¨û"
#define MSG_WOMAN       "ªü«¼"
#define MSG_PLANT       "´Óª«"
#define MSG_MIME        "Äqª«"

#define ERR_BOARD_OPEN  ".BOARD ¶}±Ò¿ù»~"
#define ERR_BOARD_UPDATE        ".BOARD §ó·s¦³»~"
#define ERR_PASSWD_OPEN ".PASSWDS ¶}±Ò¿ù»~"

#define ERR_BID         "§A·d¿ù¤F°Õ¡I¨S¦³³o­ÓªO³á¡I"
#define ERR_UID         "³o¸Ì¨S¦³³o­Ó¤H°Õ¡I"
#define ERR_PASSWD      "±K½X¤£¹ï³á¡I§A¦³¨S¦³«_¥Î¤H®aªº¦W¦r°Ú¡H"
#define ERR_FILENAME    "ÀÉ¦W¤£¦Xªk¡I"

#define MSG_SELECT_BOARD        \
        COLOR1"[1m¡i [37m¿ï¾Ü¬ÝªO[33m ¡j[0m\n½Ð¿é¤J¬ÝªO¦WºÙ(«öªÅ¥ÕÁä¦Û°Ê·j´M)¡G"

#define MSG_MAILER      \
COLOR2"  «H«H¬Û±¤  "\
COLOR1"[1m (R)[37m¦^«H [33m(x)[37mÂà¹F [33m(y)[37m¸s²Õ¦^«H [33m(D)[37m§R°£ [33m(c)[37m½Æ»s¤å³¹  [33m[G][37mÄ~Äò¡H    [m"

#define P_BOARD "¹ï¤£°_,¦¹ªO¥u­ã¬ÝªO¦n¤Í¶i¤J,½Ð¦VªO¥D¥Ó½Ð¤J¹Ò³\\¥i"


#ifdef  _MAIN_C_

/* ----------------------------------------------------- */
/* GLOBAL VARIABLE                                       */
/* ----------------------------------------------------- */

char trans_buffer[256];         /* ¤@¯ë¶Ç»¼ÅÜ¼Æ add by Ptt */

int usernum;
pid_t currpid;                  /* current process ID */
usint currstat;
int currmode = 0;
int curredit = 0;
int showansi = 1;
time_t login_start_time;
time_t update_time;
time_t schedule_time;
char schedule_string[100];
userec cuser;                   /* current user structure */
userec xuser;                   /* lookup user structure */
char quote_file[80] = "\0";
char quote_user[80] = "\0";
time_t paste_time;
char paste_title[STRLEN];
char paste_path[256];
int  paste_level;
char currtitle[TTLEN + 1] = "\0";
char vetitle[40] = "\0";
char currowner[STRLEN] = "\0";
char currauthor[IDLEN + 2] = "\0";
char currfile[FNLEN];           /* current file name @ bbs.c mail.c */
uschar currfmode;               /* current file mode */
char currmsg[100];
char currboard[IDLEN + 2];
usint currbrdattr;
char currBM[IDLEN * 3 + 10];
char reset_color[4] = "[m";
char margs[64] = "\0";           /*  main argv list*/
int inmore = 0;
/* global string variables */


/* filename */

char *fn_passwd         = FN_PASSWD;
char *fn_board          = FN_BOARD;
char *fn_note_ans       = "note.ans";
char *fn_register       = "register.new";
char *fn_plans          = "plans";
char *fn_writelog       = "writelog";
char *fn_talklog        = "talklog";
char *fn_magickey	= FN_MAGICKEY;
char *fn_overrides      = FN_OVERRIDES;
char *fn_reject         = FN_REJECT;
char *fn_canvote        = FN_CANVOTE;
char *fn_notes          = "notes";
char *fn_water          = FN_WATER;
char *fn_visable        = FN_VISABLE;
char *fn_myfavorite     = FN_MYFAVORITE;
char *fn_mandex         = "/.Names";

/* message */

char *msg_seperator     = MSG_SEPERATOR;
char *msg_mailer        = MSG_MAILER;

char *msg_cancel        = MSG_CANCEL;
char *msg_usr_left      = MSG_USR_LEFT;
char *msg_nobody        = MSG_NOBODY;

char *msg_sure_ny       = MSG_SURE_NY;
char *msg_sure_yn       = MSG_SURE_YN;

char *msg_bid           = MSG_BID;
char *msg_uid           = MSG_UID;

char *msg_del_ok        = MSG_DEL_OK;
char *msg_del_ny        = MSG_DEL_NY;

char *msg_fwd_ok        = MSG_FWD_OK;
char *msg_fwd_err1      = MSG_FWD_ERR1;
char *msg_fwd_err2      = MSG_FWD_ERR2;

char *err_board_update  = ERR_BOARD_UPDATE;
char *err_bid           = ERR_BID;
char *err_uid           = ERR_UID;
char *err_filename      = ERR_FILENAME;

char *str_mail_address  = "." BBSUSER "@" MYHOSTNAME;
char *str_new           = "new";
char *str_reply         = "Re: ";
char *str_space         = " \t\n\r";
char *str_sysop         = "SYSOP";
char *str_author1       = STR_AUTHOR1;
char *str_author2       = STR_AUTHOR2;
char *str_post1         = STR_POST1;
char *str_post2         = STR_POST2;
char *BoardName         = BOARDNAME;
char *str_dotdir        = ".DIR";
char tmpbuf[512];
#else                           /* _MAIN_C_ */


/* ----------------------------------------------------- */
/* GLOBAL VARIABLE                                       */
/* ----------------------------------------------------- */

extern char trans_buffer[256];         /* ¤@¯ë¶Ç»¼ÅÜ¼Æ add by Ptt */

extern int usernum;
extern pid_t currpid;
extern usint currstat;
extern int currmode;
extern int curredit;
extern int showansi;
extern int talkrequest;
extern time_t login_start_time;
extern time_t update_time;
extern time_t schedule_time;
extern char schedule_string[100];
extern userec cuser;            /* current user structure */
extern userec xuser;            /* lookup user structure */

extern char quote_file[80];
extern char quote_user[80];
extern time_t paste_time;
extern char paste_title[STRLEN];
extern char paste_path[256];
extern int  paste_level;
extern char currowner[STRLEN];
extern char currauthor[IDLEN + 2];
extern uschar currfmode;               /* current file mode */
extern char currtitle[TTLEN + 1];
extern char vetitle[44];
extern char currfile[FNLEN];
extern char currmsg[100];

extern char currboard[];        /* name of currently selected board */
extern usint currbrdattr;
extern char currBM[];           /* BM of currently selected board */
extern char reset_color[];
extern char margs[];
extern int inmore;
/* global string variable */

/* filename */

extern char *fn_passwd;
extern char *fn_board;
extern char *fn_note_ans;
extern char *fn_register;
extern char *fn_plans;
extern char *fn_writelog;
extern char *fn_talklog;
extern char *fn_magickey;
extern char *fn_overrides;
extern char *fn_reject;
extern char *fn_canvote;
extern char *fn_notes;
extern char *fn_water;
extern char *fn_visable;
extern char *fn_myfavorite;
extern char *fn_mandex;

/* message */

extern char *msg_seperator;
extern char *msg_mailer;

extern char *msg_cancel;
extern char *msg_usr_left;
extern char *msg_nobody;

extern char *msg_sure_ny;
extern char *msg_sure_yn;

extern char *msg_bid;
extern char *msg_uid;

extern char *msg_del_ok;
extern char *msg_del_ny;

extern char *msg_fwd_ok;
extern char *msg_fwd_err1;
extern char *msg_fwd_err2;

extern char *err_board_update;
extern char *err_bid;
extern char *err_uid;
extern char *err_filename;

extern char *str_mail_address;
extern char *str_new;
extern char *str_reply;
extern char *str_space;
extern char *str_sysop;
extern char *str_author1;
extern char *str_author2;
extern char *str_post1;
extern char *str_post2;
extern char *BoardName;
extern char *str_dotdit;
extern char tmpbuf[512];

#ifdef XINU
extern int errno;

#endif

#endif                          /* _MAIN_C_ */

extern int errno;
extern jmp_buf byebye;          /* for exception condition like I/O error */

extern user_info *currutmp;

extern int dumb_term;
extern int t_lines, t_columns;  /* Screen size / width */
extern int b_lines;             /* Screen bottom line number: t_lines-1 */
extern int p_lines;             /* a Page of Screen line numbers: tlines-4 */

extern char fromhost[];
extern char save_title[];       /* used by editor when inserting */

extern int KEY_ESC_arg;


void doent();   /* read,mail,announce list */

#endif                          /* _GLOBAL_H_ */

/*-------------------------------------------------------*/
/* stuff.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : utility routines                             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"
#include <varargs.h>

void
setuserfile(buf, fname)
  char *buf, *fname;
{
  sprintf(buf, "home/%s/%s", cuser.userid, fname);
}

void
setbdir(buf, boardname)
  char *buf, *boardname;
{
  sprintf(buf, "boards/%s/%s", boardname,
    currmode & MODE_DIGEST ? fn_mandex : ".DIR");
}

int
invalid_fname(str)
  char *str;
{
  char ch;

  if (strspn(str, ".") == strlen(str))
     return 1;

  while (ch = *str++)
  {
    if (not_alnum(ch) && !strchr("@[]-._", ch))
      return 1;
  }
  return 0;
}


int
invalid_pname(str)
  char *str;
{
  char *p1, *p2, *p3;

  p1 = str;
  while (*p1) 
  {
    if (!(p2 = strchr(p1, '/')))
      p2 = str + strlen(str);
    
    if (p1 + 1 > p2 || p1 + strspn(p1, ".") == p2)
      return 1;
    
    for (p3 = p1; p3 < p2; p3++)
      if (not_alnum(*p3) && !strchr("@[]-._", *p3))
        return 1;
    
    p1 = p2 + (*p2 ? 1 : 0);
  }
  return 0;
}



int
valid_ident(ident)
  char *ident;
{
  static char *invalid[] = {"unknown@", "root@", "gopher@", "bbs@",
  "@bbs", "guest@", "@ppp", "@slip", NULL};
  char buf[128];
  int i;

  str_lower(buf, ident);
  for (i = 0; invalid[i]; i++)
    if (strstr(buf, invalid[i]))
      return 0;
  return 1;
}


/*
woju
*/
int
userid_is_BM(userid, list)
  char *userid, *list;                  /* ªO¥D¡GBM list */
{
  register int ch, len;

  ch = list[0];
  if ((ch > ' ') && (ch < 128))
  {
    len = strlen(userid);
    do
    {
      if (!ci_strncmp(list, userid, len))
      {
        ch = list[len];
        if ((ch == 0) || (ch == '/') || (ch == ']'))
          return 1;
      }
      while (ch = *list++)
      {
        if (ch == '/')
          break;
      }
    } while (ch);
  }
  return 0;
}


int
is_BM(list)
  char *list;
{
  return userid_is_BM(cuser.userid, list);
}


/* ----------------------------------------------------- */
/* ÀÉ®×ÀË¬d¨ç¼Æ¡GÀÉ®×¡B¥Ø¿ý¡BÄÝ©ó                        */
/* ----------------------------------------------------- */


off_t
dashs(fname)
  char *fname;
{
  struct stat st;

  if (!stat(fname, &st))
        return (st.st_size);
  else
        return -1;
}


long
dasht(fname)
  char *fname;
{
  struct stat st;

  if (!stat(fname, &st))
        return (st.st_mtime);
  else
        return -1;
}


int
dashl(fname)
  char *fname;
{
  struct stat st;

  return (lstat(fname, &st) == 0 && S_ISLNK(st.st_mode));
}


dashf(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}


int
dashd(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}


int
belong(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[80], *ptr;

    while (fgets(buf, 80, fp))
    {
      if ((ptr = strtok(buf, str_space)) && !strcasecmp(ptr, key))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}


int
belong_spam(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[STRLEN], *ptr;

    while (fgets(buf, STRLEN, fp))
    {
      if(buf[0] == '#') continue;
      if ((ptr = strtok(buf, " \t\n\r")) && strstr(key, ptr))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}


char *
Cdatelite(clock)
  time_t *clock;
{
  static char foo[18];
  struct tm *mytm = localtime(clock);

  strftime(foo, 18, "%D %T", mytm);
  return (foo);
}


char *
Cdate(clock)
  time_t *clock;
{
  static char foo[18];
  struct tm *mytm = localtime(clock);

  strftime(foo, 18, "%D %T %a", mytm);
  return (foo);
}


void
pressanykey_old(va_alist)
  va_dcl
{
  va_list ap;
  char msg[128], *fmt;
  int ch;

  msg[0]=0;
  va_start(ap);
  fmt = va_arg(ap, char *);
  if(fmt) vsprintf(msg, fmt, ap);
  va_end(ap);
  if (msg[0])
  {
    move(b_lines, 0); clrtoeol();
    prints(COLOR1"[1m¡¹ [37m%-54s  "COLOR2"[ªÅ¥Õ]©Î ESC_c¼È¦s [m", msg);

  }
  else
  {
    outmsg(COLOR1"[1m                        ¡¹ ½Ð«ö [37m(Space/Return)"
    COLOR1" Ä~Äò ¡¹                         [m");
  }
  
  do
  {
    ch = igetkey();
    if (ch == KEY_ESC && KEY_ESC_arg == 'c')
      capture_screen();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  move(b_lines, 0);
  clrtoeol();
  refresh();
}

void
pressanykey(va_alist)
  va_dcl
{
  va_list ap;
  char msg[128], *fmt;
  int ch;
  screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

  msg[0]=0;
  va_start(ap);
  fmt = va_arg(ap, char *);
  if(fmt) vsprintf(msg, fmt, ap);
  va_end(ap);
  
  vs_save(screen);
  if (msg[0])
  {
    int len;
    
    len = strlen(msg) + 12;
    if(len > 80) len = 80;
    if(len < 28) len = 28;  /*title*/
    
    msgbox(0, len, "«ö¥ô·NÁäÄ~Äò....", msg, 1);
  }
  else
  {
    outmsg(COLOR1"[1m                        ¡¹ ½Ð«ö [37m(Space/Return)"
    COLOR1" Ä~Äò ¡¹                         [m");
  }
  
  do
  {
    ch = igetkey();
    if (ch == KEY_ESC && KEY_ESC_arg == 'c')
      capture_screen();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  vs_restore(screen);
  refresh();
}


void
bell()
{
  char sound[3], *ptr;

  ptr = sound;
  memset(ptr, Ctrl('G'), sizeof(sound));
  if (HAS_HABIT(HABIT_BELL))
    write(1, ptr, sizeof(sound));
}


int
search_num(ch, max)
{
  int clen = 1;
  int x, y;
  extern unsigned char scr_cols;
  char genbuf[10];

  outmsg("[7m ¸õ¦Ü²Ä´X¶µ¡G[0m");
  outc(ch);
  genbuf[0] = ch;
  getyx(&y, &x);
  x--;
  while ((ch = igetch()) != '\r')
  {
    if (ch == 'q' || ch == 'e')
      return -1;
    if (ch == '\n')
      break;
    if (ch == '\177' || ch == Ctrl('H'))
    {
      if (clen == 0)
      {
        bell();
        continue;
      }
      clen--;
      move(y, x + clen);
      outc(' ');
      move(y, x + clen);
      continue;
    }
    if (!isdigit(ch))
    {
      bell();
      continue;
    }
    if (x + clen >= scr_cols || clen >= 6)
    {
      bell();
      continue;
    }
    genbuf[clen++] = ch;
    outc(ch);
  }
  genbuf[clen] = '\0';
  move(b_lines, 0);
  clrtoeol();
  if (genbuf[0] == '\0')
    return -1;
  clen = atoi(genbuf);
  if (clen == 0)
    return 0;
  if (clen > max)
    return max;
  return clen - 1;
}


void
stand_title(title)
  char *title;
{
  clear();
  prints(COLOR1"[1m¡i %s ¡j[m\n", title);
}


/* opus : cursor position */
void
cursor_show(row, column)
  int row, column;
{
  int i;

  i = (rand()%(strlen(cuser.cursor)/2))*2;
  move(row, column);
  prints("%c%c", cuser.cursor[i], cuser.cursor[i+1]);
  move(row, column + 1);
}


void
cursor_clear(row, column)
  int row, column;
{
  move(row, column);
  outs(STR_UNCUR);
}


int
cursor_key(row, column)
  int row, column;
{
  int ch;

  cursor_show(row, column);
  ch = igetkey();
  cursor_clear(row, column);
  return ch;
}

void
printdash(mesg)
  char *mesg;
{
  int head = 0, tail;

  if (mesg)
    head = (strlen(mesg) + 1) >> 1;

  tail = head;

  while (head++ < 38)
    outch('-');

  if (tail)
  {
    outch(' ');
    outs(mesg);
    outch(' ');
  }

  while (tail++ < 38)
    outch('-');
  outch('\n');
}


// wildcat : ¨âºØ³f¹ô
int
check_money(unsigned long int money,int mode)
{
  unsigned long int usermoney;

  usermoney = mode ? cuser.goldmoney : cuser.silvermoney;
  if(usermoney<money)
  {
    move(1,0);
    clrtobot();
    move(10,10);
    pressanykey("©êºp¡I±z¨­¤W¥u¦³ %d ¤¸¡A¤£°÷­ò¡I",usermoney);
    return 1;
  }
  return 0;
}

#if 0
// wildcat : ¤dÁH¼úª÷ :p
void
get_bonus()
{
  int money;
  time_t now = time(0);
  char buf[256];
  
  money= random()%2001;
  xuser.silvermoney +=money;
  sprintf(buf,"[1;31m®¥³ß [33m%s [31mÀò±o¤dÁH¼úª÷ [36m%d ¤¸»È¹ô , %s[m",
    cuser.userid,money,Etime(&now));
  f_cat(BBSHOME"/log/y2k_bonus",buf);

  pressanykey("®¥³ß§AÀò±o %d ¤¸»È¹ôªº¤dÁH¼úª÷",money);
}  
#endif

/* wildcat 981218 */
#define INTEREST_TIME	86400*7	// wildcat:7¤Ñµo©ñ¤@¦¸§Q®§
#define BANK_RATE	1.06	// wildcat:»È¦æ§Q²v 1.06 
#define SONG_TIME	86400	// hialan :¤@¤Ñµo¤@¦¸§ë²¼¦¸¼Æ

void
update_data()
{
  int add = (time(0) - update_time)/30, change=0;
  static userec tuser;

  do_getuser(cuser.userid, &tuser);
  
  if((time(0) - tuser.update_songtime) >= SONG_TIME || tuser.songtimes > 5)
  {
    tuser.songtimes = 5;
    tuser.update_songtime = time(0);
    change = 1;
  }

  if((time(0) - tuser.dtime) >= INTEREST_TIME && tuser.silvermoney)
  {
    if(tuser.scoretimes < 100) tuser.scoretimes = 100; /*¤å³¹µû¤Àµo¦¸¼Æ*/
    tuser.silvermoney *= BANK_RATE;
    tuser.dtime = time(0);
    change = 1;
  }  
  
  if(add)
  {
    tuser.silvermoney += add*5;
    tuser.totaltime += (time(0)-update_time);
    update_time = time(0);
    change = 1;
  }
  cuser = tuser;
  if(change==1)
    substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
}


int
show_file(char *filename, int y, int lines, int mode)
{
  FILE *fp;
  char buf[256];
  
  clrchyiuan(y, y + lines);
  move(y, 0);
  if ((fp = fopen(filename, "r")))
  {
    while(fgets(buf,256,fp) && lines--)
      outs(Ptt_prints(buf,mode));
    fclose(fp);
  }
  else 
    return 0;

  return 1;
}


#if 0
/*---------------------------------------------------------------------*/
/* int chyiuan_ansi(buf,str,max)ªº¥Îªk:¦Ûµe¹³¥Î                        */
/* buf:chyiuan_ansi¹L«áªºstring                                        */
/* str:chyiuan_ansi¤§«eªºstring                                        */
/* count:¶Ç¦^move®ÉÀ³¸Óshiftªº¼Æ­È                                     */
/* ³Æµù:¦pªG¬O±m¦â¼Ò¦¡, ¶W¹L­­¨î­È®É, ·|¬å±¼¦r¦ê¶W¹L³¡¥÷, ¦ý«O¯dcolor  */
/*---------------------------------------------------------------------*/

int
chyiuan_ansi(buf,str,max)
  char *buf,*str;
  int max;
{
  int count = 0;
  int count0 = 0;
  int count1 = 0;
  char buf0[256];
  
  count0 = strip_ansi(buf0,str,0);
  if((cuser.uflag & COLOR_FLAG) && count0 <= max)
  {
    count1=strip_ansi(NULL,str,1);
    count=count1-count0;
    strcpy(buf, str);
  }
  else if((cuser.uflag & COLOR_FLAG) && count0 > max)
  {
    count0 = cut_ansistr(buf0,str,max);
    count1 = strip_ansi(NULL,buf0,1);
    count=count1-count0;
    strcpy(buf, buf0);
  }
  else
  {
    count=0;
    strcpy(buf, buf0);
  }
  return count;
}
#endif

int
answer(char *s)
{
  char ch;
  outmsg(s);
  ch = igetch ();
  if (ch == 'Y')
    ch = 'y';
  if (ch == 'N')
    ch = 'n';
  return ch;
}

#if defined(SunOS) || defined(SOLARIS)

#include <syslog.h>

void
setenv(name, value, overwrite)
  const char *name;
  const char *value;
  int overwrite;
{
  if (overwrite || (getenv(name) == 0))
  {
    char *p;

    if ((p = malloc(strlen(name) + strlen(value) + 2)) == 0)
    {
      syslog(LOG_ERR, "out of memory\n");
      exit(1);
    }
    sprintf(p, "%s=%s", name, value);
    putenv(p);
  }
}

atexit(procp)
void (*procp)();
{
   on_exit(procp, 0);
}

#endif

capture_screen()
{
   char fname[MAXPATHLEN];
   FILE* fp;
   extern screenline *big_picture;
   extern uschar scr_lns;
   int i;

   setuserfile(fname, "buf.0");
   if (fp = fopen(fname, "w")) 
   {
      for (i = 0; i < scr_lns; i++)
         fprintf(fp, "%.*s\n", big_picture[i].len, big_picture[i].data);
      fclose(fp);
   }
}

void
edit_note()
{
   char fname[MAXPATHLEN];
   int mode0 = currutmp->mode;
   int stat0 = currstat;
   char c0 = *quote_file;

   *quote_file = 0;
   setutmpmode(NOTE);
   setuserfile(fname, "note");
   vedit(fname, 0);
   currutmp->mode = mode0;
   currstat = stat0;
   *quote_file = c0;
}


char*
my_ctime(const time_t *t)
{
  struct tm *tp;
  static char ans[100];

  tp = localtime(t);
  sprintf(ans, "%d/%02d/%02d %02d:%02d:%02d", tp->tm_year % 100,
     tp->tm_mon + 1,tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
  return ans;
}
#if 0
/* ----------------------------------------------------- */
/* ¼È¦sÀÉ TBF (Temporary Buffer File) routines           */
/* ----------------------------------------------------- */

char *
tbf_ask()
{
  static char fn_tbf[10] = "buf.0";
  getdata(b_lines, 0, "½Ð¿ï¾Ü¼È¦sÀÉ(0-9)¡G", fn_tbf + 4, 2, DOECHO,"0");
  return fn_tbf;
}
#endif


/*
 * check_personal_note() ªº¥\¥Î¸ò chkmail() ¤@¼Ë..
 * ©Ò¥H¥i¥H¥[¦b my_query() ¤¤:³o¼Ë§O¤Hquery®É´N¥i¥H¬Ý¨ì¦³¨S¦³·s¯d¨¥
 * ÁÙ¦³¥i¥H¥[¦b show_title() ¤¤:³o¼Ë¦³·s¯d¨¥®É´N·|¹³¦³·s«H¥ó¤@¼Ë¦btitle´£¥Ü³á!
 */

int
check_personal_note(int newflag, char* userid) 
{
 char fpath[MAXPATHLEN];
 FILE *fp;
 int  total = 0;
 notedata myitem;
 char *fn_note_dat      = "pnote.dat";

 if (userid == NULL)
   setuserfile(fpath, fn_note_dat);
 else
   sethomefile(fpath, userid, fn_note_dat);

 if ((fp = fopen(fpath, "r")) != NULL) 
 {
   while (fread(&myitem, sizeof(myitem), 1, fp) == 1) 
   {
     if (newflag)
       if (myitem.buf[0][0] == 0) total++;
     else
       total++;
   }
   fclose(fp);
   return total;
 }
 return 0;
}

game_log(va_alist)
va_dcl
{
 va_list ap;
 int file;
 char *fmt,msg[128],ff[40];
 time_t now;
 FILE *fs;

 va_start(ap);
  file = va_arg(ap, int);
  fmt = va_arg(ap, char *);
  vsprintf(msg, fmt, ap);
 va_end(ap);

 switch(file) /* ³o¤@¬q¥i¥H¦Û¤v§ï! */
 {
  case XAXB:
    strcpy(ff,"log/ab.log");
    log_usies("XAXB",NULL);
    break;
  case CHICKEN: 
    strcpy(ff,"log/pip.log"); 
    log_usies("CHICKEN",NULL);
    break;
  case BLACKJACK: 
    strcpy(ff,"log/bj.log"); 
    log_usies("BJ",NULL);
    break;
  case STOCK: 
    strcpy(ff,"log/stock.log"); 
    log_usies("STOCK",NULL);
    break;
  case DICE: 
    strcpy(ff,"log/dice.log"); 
    log_usies("DICE",NULL);
    break;
  case GP: 
    strcpy(ff,"log/gp.log"); 
    log_usies("GP",NULL);
    break;
  case MARIE: 
    strcpy(ff,"log/marie.log"); 
    log_usies("MARIE",NULL);
    break;
  case RACE: 
    strcpy(ff,"log/race.log"); 
    log_usies("RACE",NULL);
    break;
  case BINGO: 
    strcpy(ff,"log/bingo.log"); 
    log_usies("BINGO",NULL);
    break; 
  case NINE: 
    strcpy(ff,"log/nine.log"); 
    log_usies("NINE",NULL);
    break;
  case NumFight: 
    strcpy(ff,"log/fightnum.log"); 
    log_usies("NumFight",NULL);
    break;
  case CHESSMJ: 
    strcpy(ff,"log/chessmj.log"); 
    log_usies("CHESSMJ",NULL);
    break;
  case SEVENCARD: 
    strcpy(ff,"log/seven.log"); 
    log_usies("SEVENCARD",NULL);
    break;
 }
 fs=fopen(ff,"a+");
 now=time(0);
 fprintf(fs,"[1;33m%s [32m%s [36m%s[m\n", Etime(&now),cuser.userid,msg);
 fclose(fs);
}

int
show_help(mode)
  int mode;
{
  if(inmore)
    more(BBSHOME"/etc/help/MORE.help",YEA);
  else if(mode == LUSERS)
    more(BBSHOME"/etc/help/LUSERS.help",YEA);
  else if(mode == READBRD || mode == READNEW)
    more(BBSHOME"/etc/help/BOARD.help",YEA);
  else if(mode == RMAIL)
//    more(BBSHOME"/etc/help/MAIL.help",YEA);
    return 0;
  else if(mode == READING)
//    more(BBSHOME"/etc/help/READ.help",YEA);
    return 0;
  else if(mode == ANNOUNCE)
    more(BBSHOME"/etc/help/ANNOUNCE.help",YEA);
  else if(mode == EDITING)
    more(BBSHOME"/etc/help/EDIT.help",YEA);
  else
    HELP();
  return 0;
}

int
mail2user(muser, title, fname, filemode)
  char *muser, *title, *fname;
  int filemode;
{
  fileheader mhdr;
  char buf[256], buf1[80];

  sethomepath(buf1, muser);
  stampfile(buf1, &mhdr);

  if(filemode != 0)
    strcpy(mhdr.owner, "[³Æ.§Ñ.¿ý]");    
  else
    strcpy(mhdr.owner, cuser.userid);

  strcpy(mhdr.title, title);
  mhdr.savemode = 0;
  mhdr.filemode = filemode;
  sethomedir(buf, muser);
  rec_add(buf, &mhdr, sizeof(mhdr));
  f_cp(fname, buf1, O_TRUNC);
  return 0;
}

void
debug(mode)
  char *mode;
{
  time_t now = time(0);
  char buf[128];

  sprintf(buf, "%s %s %s\n", Etime(&now), mode, cuser.userid);      
  f_cat("debug",buf);
}

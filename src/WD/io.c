
/*-------------------------------------------------------*/
/* io.c         ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : basic console/screen/keyboard I/O routines   */
/* create : 95/02/28                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef AIX
#include <sys/select.h>
#endif

#ifdef  LINUX
#define OBUFSIZE  (2048)
#define IBUFSIZE  (128)
#else
#define OBUFSIZE  (4096)
#define IBUFSIZE  (512)
#endif

#define INPUT_ACTIVE    0
#define INPUT_IDLE      1

static char outbuf[OBUFSIZE];
static int obufsize = 0;

static char inbuf[IBUFSIZE];
static int ibufsize = 0;
static int icurrchar = 0;

static int i_mode = INPUT_ACTIVE;

extern int dumb_term;

passwd_outs(text)
  char *text;
{
  register int column = 0;
  register char ch;
  while ((ch = *text++) && (++column < 80))
  {
    outch('*');
  }
}

/* ----------------------------------------------------- */
/* ©w®ÉÅã¥Ü°ÊºA¬ÝªO                                      */
/* ----------------------------------------------------- */


#define STAY_TIMEOUT    (30*60)

static void
hit_alarm_clock()
{
  static int stay_time = 0;
  static int idle_time = 0;

  time_t now = time(0);
  char buf[100]="\0";

  if(currutmp->pid != currpid)
    setup_utmp(XMODE);   /* ­«·s°t¸m shm */

  if((idle_time = now - currutmp->lastact) > IDLE_TIMEOUT && !HAS_PERM(PERM_RESEARCH))
  {
    pressanykey("¶W¹L¶¢¸m®É¶¡¡I½ð¥X¥hÅo¡K¡K");
    abort_bbs();
  }

  if (HAS_HABIT(HABIT_MOVIE) && (currstat && (currstat < CLASS || currstat == MAILALL)))
    movie(0);

  alarm(MOVIE_INT);
  stay_time += MOVIE_INT;

  if(idle_time > IDLE_TIMEOUT - 60 && !HAS_PERM(PERM_RESEARCH)) 
    sprintf(buf, "[1;5;37;41mÄµ§i¡G±z¤w¶¢¸m¹L¤[¡A­YµL¦^À³¡A¨t²Î§Y±N¤ÁÂ÷¡I¡I[m");
  else if(stay_time > 10 * 60 && chkmail(0)) 
  {
    sprintf(buf, "\033[1;33;41m[%s] «H½cùØÁÙ¦³¨S¬Ý¹Lªº«H­ò\033[m",
      Etime(&now));
    stay_time = 0 ;
  }
  else if(stay_time > STAY_TIMEOUT && HAS_HABIT(HABIT_ALARM))
  {
    /* ¦b³o¸Ì´£¥Ü user ¥ð®§¤@¤U */
    char *msg[10] = {
    "¦ù¦ù¸y, ´|´|²´, ³Ü¤f¯ù....³Ý¤f®ð...¦AÄ~Äò...!",
    "¤@Ãä¬O¤Í±¡ ¤@Ãä¬O·R±¡ ¥ª¥kªº¬G¨Æ¬°ÃøµÛ¦Û¤v...",
    "¬O§_¦³¤HÁA¸Ñ±z¤º¤ßªº©t±I? ¤j®a¨Ótalk talk§a.. ",
    "¥ª¤T°é,¥k¤T°é,²ä¤l§á§á§¾ªÑ§á§á ¤j®a¨Ó§@¹B°Ê­ò~",
    "§ÚÄé..§ÚÄé..§ÚÄéÄéÄé! Äé¨ìµwºÐÃz±¼...",
    "¥Î¡E¥\\¡E°á¡E®Ñ",
    "®Ñ©À§¹¤F¨S°Ú....^.^",
    "©ú¤Ñ¦³¨S¦³¦Ò¸Õ°Ú...©À®Ñ­«­n­ò...!",
    "¬Ý¡ã¬y¬P¡I",
    "¡E®Ñ¦b¤ß¤¤®ð¦Û¬Ó¡EÅª®Ñ¥h¡E"};
    int i = rand() % 10;

    sprintf(buf, "[1;33;41m[%s] %s[m", Etime(&now), msg[i]);
    stay_time = 0 ;
  }

  if(buf[0]) 
  {
    outmsg(buf);
    refresh();
    bell();
  }
}

void
init_alarm()
{
  alarm(0);
  signal(SIGALRM, hit_alarm_clock);
  alarm(MOVIE_INT);
}


/* ----------------------------------------------------- */
/* output routines                                       */
/* ----------------------------------------------------- */


void
oflush()
{
  if (obufsize)
  {
      write(1, outbuf, obufsize);
      obufsize = 0;
  }
}


void
output(s, len)
  char *s;
{
  /* Invalid if len >= OBUFSIZE */

  if (obufsize + len > OBUFSIZE)
  {
    write(1, outbuf, obufsize);
    obufsize = 0;
  }
  memcpy(outbuf + obufsize, s, len);
  obufsize += len;
}


void
ochar(c)
{
  if (obufsize > OBUFSIZE - 1)
  {
    write(1, outbuf, obufsize);
    obufsize = 0;
  }
  outbuf[obufsize++] = c;
}


/* ----------------------------------------------------- */
/* input routines                                        */
/* ----------------------------------------------------- */


int i_newfd = 0;
static struct timeval i_to, *i_top = NULL;
static int (*flushf) () = NULL;


void
add_io(fd, timeout)
  int fd;
  int timeout;
{
  i_newfd = fd;
  if (timeout)
  {
    i_to.tv_sec = timeout;
    i_to.tv_usec = 0;
    i_top = &i_to;
  }
  else
    i_top = NULL;
}


void
add_flush(flushfunc)
  int (*flushfunc) ();
{
  flushf = flushfunc;
}


int
num_in_buf()
{
  return icurrchar - ibufsize;
}

int
dogetch()
{
  int ch;

  if(currutmp) time(&currutmp->lastact);

  for (;;)
  {
    if (ibufsize == icurrchar)
    {
      fd_set readfds;
      struct timeval to;

      to.tv_sec = to.tv_usec = 0;
      FD_ZERO(&readfds);
      FD_SET(0, &readfds);
      if (i_newfd)
        FD_SET(i_newfd, &readfds);
      if ((ch = select(FD_SETSIZE, &readfds, NULL, NULL, &to)) <= 0)
      {
        if (flushf)
          (*flushf) ();

        if (dumb_term)
          oflush();
        else
          refresh();

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        if (i_newfd)
          FD_SET(i_newfd, &readfds);

        while ((ch = select(FD_SETSIZE, &readfds, NULL, NULL, i_top)) < 0)
        {
          if (errno == EINTR)
            continue;
          else
          {
            perror("select");
            return -1;
          }
        }
        if (ch == 0)
          return I_TIMEOUT;
      }
      if (i_newfd && FD_ISSET(i_newfd, &readfds))
        return I_OTHERDATA;

      while ((ibufsize = read(0, inbuf, IBUFSIZE)) <= 0)
      {
        if (ibufsize == 0)
          longjmp(byebye, -1);
        if (ibufsize < 0 && errno != EINTR)
          longjmp(byebye, -1);
      }
      icurrchar = 0;
    }

    i_mode = INPUT_ACTIVE;
    
    ch = inbuf[icurrchar++];
    return (ch);
  }
}

extern char oldmsg_count;            /* pointer */    
extern char watermode;

int
igetch()
{
    register int ch;
    while(ch = dogetch())
    {
     switch (ch)
      {
       case Ctrl('L'):
         redoscr();
         continue;
       case Ctrl('I'):
         if(currutmp != NULL && currutmp->mode == MMENU)
         {
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
           vs_save(screen);
           t_idle();
           vs_restore(screen);
           continue;
         }
         else return(ch);

       case Ctrl('W'):
         if(currutmp != NULL && currutmp->mode)
         {
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
           vs_save(screen);
           DL_func("SO/dreye.so:main_dreye");
           vs_restore(screen);
           continue;
         }

       case Ctrl('Q'):  // wildcat : §Ö³tÂ÷¯¸ :p
         if(currutmp->mode && currutmp->mode != READING)
         {
           if(answer("½T©w­nÂ÷¯¸?? (y/N)") != 'y')
             return(ch);
           update_data();
           u_exit("ABORT");
           pressanykey("ÁÂÁÂ¥úÁ{, °O±o±`¨Ó³á !");
           exit(0);
         }
         else return (ch);

       case Ctrl('Z'):   /* wildcat:help everywhere */
       {
         static short re_entry = 0; /* CityLion: ¨¾­«¤Jªº... */
         if(currutmp && !re_entry && currutmp->mode != IDLE)
         {
           int mode0 = currutmp->mode;
           int stat0 = currstat;
           int more0 = inmore;
           int i;
           extern int roll;
           int old_roll = roll;
           int my_newfd = i_newfd;
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

           re_entry = 1;
           vs_save(screen);
           i = show_help(currutmp->mode);

           currutmp->mode = mode0;
           currstat = stat0;
           inmore = more0;
           roll = old_roll;
           i_newfd = my_newfd;
           vs_restore(screen);
           re_entry = 0;
           continue;
         }
         else return (ch);
       }
       case Ctrl('U'):
         resetutmpent();
         if(currutmp != NULL && currutmp->mode != EDITING &&
            currutmp->mode != LUSERS && currutmp->mode)
         {
           int mode0 = currutmp->mode;
           int stat0 = currstat;
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

           vs_save(screen);
           t_users();
           vs_restore(screen);

           currutmp->mode = mode0;
           currstat = stat0;

           continue;
         }
         else return (ch);

        case Ctrl('R'):
        {
          if(currutmp == NULL) return (ch);
          else if(watermode > 0)
          {
            watermode = (watermode + oldmsg_count)% oldmsg_count + 1;
            t_display_new(0);
            continue;
          }
          else if (!currutmp->mode && (currutmp->chatid[0] == 2 ||
               currutmp->chatid[0] == 3) && oldmsg_count && !watermode)
          {
            watermode=1;
            t_display_new(1);
            continue;
          }
          else if (currutmp->msgs[0].last_pid && currutmp->mode)
          {
            screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
            vs_save(screen);
            watermode=1;
            t_display_new(1);
            my_write(currutmp->msgs[0].last_pid, "¤ô²y¥á¦^¥h¡G");
            vs_restore(screen);
            continue;
          }
          else return (ch);
        }


        case '\n':   /* Ptt§â \n®³±¼ */
           continue;
        case Ctrl('T'):
          if(watermode > 0 )
          {
            watermode = (watermode + oldmsg_count - 2 )% oldmsg_count + 1;
            t_display_new(0);
            continue;
          }

        default:
          return (ch);
       }
    }
}

int
offset_count(char *prompt)   // Robert Liu 20010813
{
  int i=0, j=0, off=0;
  for(i=0 ; i<strlen(prompt) ; i++)
  {
    if(prompt[i]==27) off=1;
    if(off==0) j++;
    if(prompt[i]=='m' && off==1) off=0;
  }
  return (strlen(prompt)-j);
}

int
check_ctrlword(buf,len)
  char *buf;
  int len;
{
  int i;
  
  for(i = 0;i < len - 1;i++)
  {
    if(buf[i]=='\033')
    {
      return 1;
    }
    if(buf[i]=='\0')
      return 0;
  }
  return 0;
}

getdata(line, col, prompt, buf, len, echo, ans)
  int line, col;
  char *prompt, *buf, *ans;
  int len, echo;
{
  /*int ctrlword = 0;*/  /*¹w¨¾µo¥Í±¡ªp,©Ò¥H¯dµÛ...*/
		         /*¦pªG¦³¤H¥i¥H¯}¸Ñ,¦A§â¥L¥´¶}*/
		         /*³s¦P¾ú¥v¬ö¿ýªº¦a¤è¤]¥´¶}  by hialan*/
  register int ch;
  int clen;
  int x, y;
  int off_set = 0; /*add color*/
  extern unsigned char scr_cols;
#define MAXLASTCMD 6
  static char lastcmd[MAXLASTCMD][80];

  if (prompt)
  {
    move(line, col);
    clrtoeol();
    outs(prompt);
    off_set=offset_count(prompt); /*add color*/
  }
  else
    clrtoeol();

  if (dumb_term || !echo || /* echo == PASS || */echo == 9)
  {                     /* shakalaca.990422: ¬°¤F¿é¤J passwd ®É¦³¤Ï¥Õ */
    len--;              /* ¤U­±³o¬qµ{¦¡½X¬O¨S¦³¤Ï¥Õ (!echo) */
    clen = 0;
    while ((ch = igetch()) != '\r')
    {
      if (ch == '\n')
        break;
      if (ch == '\177' || ch == Ctrl('H'))
      {
        if (!clen)
        {
          bell();
          continue;
        }
        clen--;
        if (echo)
        {
          ochar(Ctrl('H'));
          ochar(' ');
          ochar(Ctrl('H'));
        }
        continue;
      }

#ifdef BIT8
      if (!isprint2(ch))
#else
      if (!isprint(ch))
#endif

      {
        if (echo)
          bell();
        continue;
      }
      if (clen >= len)
      {
        if (echo)
          bell();
        continue;
      }
      buf[clen++] = ch;
      if (echo && echo != 9)
        ochar(/* echo == PASS ? '-' : */ch); /* shakalaca.990422: ¬°¤F passwd */
    }
    buf[clen] = '\0';
    outc('\n');
    oflush();
  }
  else
  {
   int cmdpos = MAXLASTCMD -1;
   int currchar = 0;
   int keydown;
   int dirty;

    getyx(&y, &x);
    x=x-off_set;  /*add color by hialan*/
    standout();
    for (clen = len--; clen; clen--)
      outc(' ');
    standend();

    if (ans && check_ctrlword(ans,strlen(ans)) != 1) {
       int i;

       strncpy(buf, ans, len);
       buf[len] = 0;
       for (i = strlen(buf) + 1; i < len; i++)
          buf[i] = 0;
       
       move(y, x);
       edit_outs(buf);
       clen = currchar = strlen(buf);

    }
    else
       memset(buf, 0, len);

    dirty = 0;
    while (move(y, x + currchar), (ch = igetkey()) != '\r')
    {
/*
woju
*/
       keydown = 0;
       switch (ch) {
       case Ctrl('Y'): {
          int i;

          if (clen && dirty) {
             for (i = MAXLASTCMD - 1; i; i--)
                strcpy(lastcmd[i], lastcmd[i - 1]);
             strncpy(lastcmd[0], buf, len);
          }

          move(y, x);
          for (clen = len--; clen; clen--)
            outc(' ');
          memset(buf, '\0', strlen(buf));
          clen = currchar = strlen(buf);
          continue;
          }

       case KEY_DOWN:
       case Ctrl('N'):
          keydown = 1;
       case Ctrl('P'):
       case KEY_UP: {
          int i;

          if (clen && dirty) {
             for (i = MAXLASTCMD - 1; i; i--)
                strcpy(lastcmd[i], lastcmd[i - 1]);
             strncpy(lastcmd[0], buf, len);
          }

          i = cmdpos;
          do {
             if (keydown)
                --cmdpos;
             else
                ++cmdpos;
             if (cmdpos < 0)
                cmdpos = MAXLASTCMD - 1;
             else if (cmdpos == MAXLASTCMD)
                cmdpos = 0;
          } while (cmdpos != i && (!*lastcmd[cmdpos]
                   || !strncmp(buf, lastcmd[cmdpos], len)));
          if (cmdpos == i)
             continue;

          strncpy(buf, lastcmd[cmdpos], len);
          buf[len] = 0;

          move(y, x);                   /* clrtoeof */
          for (i = 0; i <= clen; i++)
             outc(' ');
          move(y, x);

          if (echo == PASS)
            passwd_outs(buf);
          else
            edit_outs(buf);
          clen = currchar = strlen(buf);
          dirty = 0;
          continue;
       }
       case KEY_ESC:
         if (KEY_ESC_arg == 'c')
            capture_screen();
            
         /*¦b¿é¤J±b¸¹±K½X®É«öEsc+n·|³Q½ð*/
         /*©Ò¥H§ï¦¨¿é¤Jpassword®É¤£¦æedit_note()*/
         /*³Ì«áÁÙ¬O©ñ±ó,§â¥¦ª`¸Ñ±¼....>"<    by hialan*/
/*
         if (KEY_ESC_arg == 'n')
         {
           if (echo != PASS)
            edit_note();
	 }
*/
         if (ch == 'U' && currstat != IDLE  &&
           !(currutmp->mode == 0 &&
           (currutmp->chatid[0] == 2 || currutmp->chatid[0] == 3)))
            t_users();
            continue;

/* yagami.000504 : ´å¼Ð¥i¨ì³Ì«e©Î³Ì«á */ 
/* wildcat : ¨ä¹ê«ö ctrl-a , ctrl-e ¤]¬O¤@¼Ëªº°Õ :p */
       case KEY_HOME:
         currchar = 0;
         break;
       case KEY_END:
         currchar = strlen(buf);
         break;

       case KEY_LEFT:
          if (currchar)
             --currchar;
          continue;
       case KEY_RIGHT:
          if (buf[currchar])
             ++currchar;
          continue;
       }

      if (ch == '\n' || ch == '\r')
         break;

      if (ch == Ctrl('I') && currstat != IDLE &&
          !(currutmp->mode == 0 &&
            (currutmp->chatid[0] == 2 || currutmp->chatid[0] == 3))) {
         t_idle();
         continue;
      }
      if (ch == '\177' || ch == Ctrl('H'))
      {
        if (currchar) {
           int i;

           currchar--;
           clen--;
           for (i = currchar; i <= clen; i++)
              buf[i] = buf[i + 1];
           move(y, x + clen);
           outc(' ');
           move(y, x);
           if (echo == PASS)
             passwd_outs(buf);
           else
             edit_outs(buf);
           dirty = 1;
        }
        continue;
      }
      if (ch == Ctrl('D')) {
        if (buf[currchar]) {
           int i;

           clen--;
           for (i = currchar; i <= clen; i++)
              buf[i] = buf[i + 1];
           move(y, x + clen);
           outc(' ');
           move(y, x);
           if (echo == PASS)
             passwd_outs(buf);
           else
             edit_outs(buf);
           dirty = 1;
        }
        continue;
      }
      if (ch == Ctrl('K')) {
         int i;

         buf[currchar] = 0;
         move(y, x + currchar);
         for (i = currchar; i < clen; i++)
            outc(' ');
         clen = currchar;
         dirty = 1;
         continue;
      }
      if (ch == Ctrl('A')) {
         currchar = 0;
         continue;
      }
      if (ch == Ctrl('E')) {
         currchar = clen;
         continue;
      }


      if (!(isprint2(ch)))
      {
        continue;
      }
      if (clen >= len || x + clen >= scr_cols)
      {
        continue;
      }
/*
woju
*/
      if (buf[currchar]) {               /* insert */
         int i;

         for (i = currchar; buf[i] && i < len && i < 80; i++)
            ;
         buf[i + 1] = 0;
         for (; i > currchar; i--)
            buf[i] = buf[i - 1];
      }
      else                              /* append */
         buf[currchar + 1] = '\0';

      buf[currchar] = ch;
      move(y, x + currchar);
      if (echo == PASS)
        passwd_outs(buf + currchar);
      else
      /* shakalaca.990422: ­ì¥»¥u¦³¤U­±¨º¦æ, ³o¬O¬°¤F¿é¤J passwd ¦³¤Ï¥Õ */
        edit_outs(buf + currchar);
      currchar++;
      clen++;
      dirty = 1;
    }
    buf[clen] = '\0';

//    ctrlword = check_ctrlword(buf,clen);
    
    if (clen > 1 && echo != PASS /*&& ctrlword != 1*/) {
    /* shaklaaca.990514: ^^^^^^^ ¤£Åý¿é¤Jªº password ¯d¤U¬ö¿ý */
    /* ¤£Åý¿é¤Jªº±±¨î½X¯d¤U¬ö¿ý ~~ by hialan */
       for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
          strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
       strncpy(lastcmd[0], buf, len);
    }
    if (echo) {
      move(y, x + clen);
      outc('\n');
    }
    refresh();
    

  }
  if ((echo == LCECHO) && ((ch = buf[0]) >= 'A') && (ch <= 'Z'))
    buf[0] = ch | 32;

  return clen;
}


char
getans(prompt)
  char *prompt;
{
  char ans[5];

  getdata(b_lines,0,prompt,ans,4,LCECHO,0);

  return ans[0];
}

/*
woju
*/
#define TRAP_ESC

#ifdef  TRAP_ESC
int KEY_ESC_arg;

int
igetkey()
{
  int mode;
  int ch, last;

  mode = last = 0;
  while (1)
  {
    ch = igetch();
    if (mode == 0)
    {
      if (ch == KEY_ESC)
        mode = 1;
      else
        return ch;              /* Normal Key */
    }
    else if (mode == 1)
    {                           /* Escape sequence */
      if (ch == '[' || ch == 'O')
        mode = 2;
      else if (ch == '1' || ch == '4')
        mode = 3;
      else
      {
        KEY_ESC_arg = ch;
        return KEY_ESC;
      }
    }
    else if (mode == 2)
    {                           /* Cursor key */
      if (ch >= 'A' && ch <= 'D')
        return KEY_UP + (ch - 'A');
      else if (ch >= '1' && ch <= '6')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 3)
    {                           /* Ins Del Home End PgUp PgDn */
      if (ch == '~')
        return KEY_HOME + (last - '1');
      else
        return ch;
    }
    last = ch;
  }
}

#else                           /* TRAP_ESC */

int
igetkey(void)
{
  int mode;
  int ch, last;

  mode = last = 0;
  while (1)
  {
    ch = igetch();
    if (ch == KEY_ESC)
      mode = 1;
    else if (mode == 0)         /* Normal Key */
      return ch;
    else if (mode == 1)
    {                           /* Escape sequence */
      if (ch == '[' || ch == 'O')
        mode = 2;
      else if (ch == '1' || ch == '4')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 2)
    {                           /* Cursor key */
      if (ch >= 'A' && ch <= 'D')
        return KEY_UP + (ch - 'A');
      else if (ch >= '1' && ch <= '6')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 3)
    {                           /* Ins Del Home End PgUp PgDn */
      if (ch == '~')
        return KEY_HOME + (last - '1');
      else
        return ch;
    }
    last = ch;
  }
}
#endif                          /* TRAP_ESC */



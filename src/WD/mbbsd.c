/*-------------------------------------------------------*/
/* mbbsd.c       ( NTHU CS MapleBBS Ver 2.36 )           */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : BBS main/			                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#define _MAIN_C_

#include <varargs.h>
#include "bbs.h"
#include <sys/resource.h> 
#include <sys/wait.h> 
#include <sys/socket.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <arpa/telnet.h>
#include <syslog.h>


#define SOCKET_QLEN 4 
#define TH_LOW 100 
#define TH_HIGH 120
#define PID_FILE BBSHOME"/run/mbbsd.pid" 

#ifdef MBBSD_LAST
  #define MBBSD_LASTLOG BBSHOME"/src/.last_mbbsd"
#endif

#define BANNER "¡i"BOARDNAME"¡j¡· ¹q¤l¥¬§iÄæ¨t²Î ¡·("MYHOSTNAME")\r\n\
 %s IP ("MYIP") \r\nPowered by FreeBSD\r\n[mbbsd is running...]\n"

jmp_buf byebye;

char fromhost[STRLEN] = "\0";
char remoteusername[40];
int mbbsd = 1; 

/*-------------------------------------------------------*/
/* main.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : login/top-menu routines 	                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#define WELCOME_TITLE "etc/Welcomes/Welcome_title/"

static uschar enter_uflag;
void check_register();

/* ----------------------------------------------------- */
/* Â÷¶} BBS µ{¦¡                                         */
/* ----------------------------------------------------- */

void log_usies(char *mode, char *msg)
{
  char buf[256], data[256];
  time_t now;
  
  time(&now);
  if (!msg)
  {
    sprintf(data, "Stay: %d (%s)", 
      (now - login_start_time) / 60, cuser.username);
    msg = data;
  }
  sprintf(buf, "%s %s %-13s%s", Etime(&now), mode, cuser.userid, msg);
  f_cat(FN_USIES, buf);
}
                          

static void setflags(int mask, int value)
{
  if (value)
    cuser.uflag |= mask;
  else
    cuser.uflag &= ~mask;
}


void u_exit(char *mode)
{
  extern void auto_backup();    /* ½s¿è¾¹¦Û°Ê³Æ¥÷ */
  userec xuser;
  int diff = (time(0) - login_start_time) / 60;

  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  auto_backup();
  setflags(PAGER_FLAG, currutmp->pager != 1);
  setflags(CLOAK_FLAG, currutmp->invisible);
  xuser.pager = currutmp->pager;	 /* °O¿ýpagerª¬ºA, add by wisely */
  xuser.invisible = currutmp->invisible; /* ¬ö¿ýÁô§Îª¬ºA by wildcat */
  xuser.totaltime += time(0) - update_time;
  xuser.numposts = cuser.numposts;
  xuser.feeling[4] = '\0';

  if (!HAS_PERM(PERM_DENYPOST) && !currutmp->invisible)
  {
    char buf[256];
    time_t now;
    
    time(&now);
    sprintf(buf, "<<¤U¯¸³qª¾>> -- §Ú¨«Åo¡I - %s", Etime(&now));
    do_aloha(buf);
  }
  
  purge_utmp(currutmp);
  if(!diff && cuser.numlogins > 1 && strcmp(cuser.userid,STR_GUEST))
    xuser.numlogins = --cuser.numlogins; /* Leeym ¤W¯¸°±¯d®É¶¡­­¨î¦¡ */
  substitute_record(fn_passwd, &xuser, sizeof(userec), usernum);
  log_usies(mode, NULL);
}


void system_abort()
{
  if (currmode)
    u_exit("ABORT");

  clear();
  refresh();
  printf("ÁÂÁÂ¥úÁ{, °O±o±`¨Ó³á !\n");
  sleep(1);
  exit(0);
}


void abort_bbs()
{
  if (currmode)
    u_exit("AXXED");
  exit(0);
}


void leave_bbs()
{
   reset_tty();
}


/* ----------------------------------------------------- */
/* µn¿ý BBS µ{¦¡                                         */
/* ----------------------------------------------------- */
int dosearchuser(char *userid)
{
  if (usernum = getuser(userid))
    memcpy(&cuser, &xuser, sizeof(cuser));
  else
    memset(&cuser, 0, sizeof(cuser));
  return usernum;
}


static void talk_request()
{
#ifdef  LINUX
  /*
   * Linux ¤U³sÄò page ¹ï¤è¨â¦¸´N¥i¥H§â¹ï¤è½ð¥X¥h¡G ³o¬O¥Ñ©ó¬Y¨Ç¨t²Î¤@ nal
   * ¶i¨Ó´N·|±N signal handler ³]©w¬°¤º©wªº handler, ¤£©¯ªº¬O default ¬O±Nµ{
   * erminate. ¸Ñ¨M¤èªk¬O¨C¦¸ signal ¶i¨Ó´N­«³] signal handler
   */

  signal(SIGUSR1, talk_request);
#endif
  bell();
  if (currutmp->msgcount) 
  {
     char buf[200];
     time_t now = time(0);

     sprintf(buf, "[1m[33;41m[%s][34;47m [%s] %s [0m",
        (currutmp->destuip)->userid,  my_ctime(&now),
        (currutmp->sig == 2) ? "­«­n®ø®§¼s¼½¡I(½ÐCtrl-U,l¬d¬Ý¼ö°T°O¿ý)" : "©I¥s¡B©I¥s¡AÅ¥¨ì½Ð¦^µª");
     move(0, 0);
     clrtoeol();
     outs(buf);
     refresh();
  }
  else 
  {
     uschar mode0 = currutmp->mode;
     char c0 = currutmp->chatid[0];
     screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
     currutmp->mode = 0;
     currutmp->chatid[0] = 1;
     vs_save(screen);
     talkreply();
     vs_restore(screen);
     currutmp->mode = mode0;
     currutmp->chatid[0] = c0;
   }
}

extern msgque oldmsg[MAX_REVIEW];   /* ¥á¹L¥hªº¤ô²y */
extern char   no_oldmsg,oldmsg_count;            /* pointer */

static void write_request()
{
  time_t now;
  extern char watermode;
/*  Half-hour remind  */
  if(*currmsg) 
  {
    outmsg(currmsg);
    refresh();
    bell();
    *currmsg = 0;
    return;
  }

  time(&now);

#ifdef  LINUX
  signal(SIGUSR2, write_request);
#endif

  update_data();
  ++cuser.receivemsg;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  bell();
  show_last_call_in();
// wildcat patch : ¬Ý¤£¨ì¤ô²y??!!
  currutmp->msgcount--;
  memcpy(&oldmsg[no_oldmsg],&currutmp->msgs[0],sizeof(msgque));
  no_oldmsg++;
  no_oldmsg %= MAX_REVIEW;
  if(oldmsg_count < MAX_REVIEW) oldmsg_count++;
  if(watermode)
  {
    if(watermode<oldmsg_count) watermode++;
    t_display_new(0);
  }
  refresh();
  currutmp->msgcount = 0;
}

static void multi_user_check()
{
  register user_info *ui;
  register pid_t pid;
  int cmpuids();

  if (HAS_PERM(PERM_SYSOP))
    return;         /* wildcat:¯¸ªø¤£­­¨î */

  if (cuser.userlevel)
  {
    if (!(ui = (user_info *) search_ulist(cmpuids, usernum)))
      return;                   /* user isn't logged in */

    pid = ui->pid;
    if (!pid || (kill(pid, 0) == -1))
      return;                   /* stale entry in utmp file */

    if (getans2(b_lines, 0, "±z·Q§R°£¨ä¥L­«½Æªº login ¶Ü¡H", 0, 2, 'y') != 'n')
    {
      kill(pid, SIGHUP);
      log_usies("KICK ", cuser.username);
    }
    else
    {
      int nums = MULTI_NUMS;
      if(HAS_PERM(PERM_BM))
        nums += 2;
      if (count_multi() >= nums)
        system_abort();
    }
  } 
  else  /* guestªº¸Ü */
  {
    if (count_multi() > 512)
    {
      pressanykey("©êºp¡A¥Ø«e¤w¦³¤Ó¦h guest, ½Ðµy«á¦A¸Õ¡C");
      oflush();
      exit(1);
    }
  }
}

/* --------- */
/* bad login */
/* --------- */

static char str_badlogin[] = "logins.bad";

static void
logattempt(uid, type)
  char *uid;
  char type;                    /* '-' login failure   ' ' success */
{
  char fname[40];
  char genbuf[200];

  sprintf(genbuf, "%c%-12s[%s] %s@%s\n", type, uid,
    Etime(&login_start_time), remoteusername, fromhost);
  f_cat(str_badlogin, genbuf);
  
  if (type == '-')
  {
    sprintf(genbuf, "[%s] %s\n", Etime(&login_start_time), fromhost);
    sethomefile(fname, uid, str_badlogin);

    f_cat(fname, genbuf);
  }
}

#ifdef BSD44
static int over_load()
{
  double cpu_load[3];

  getloadavg(cpu_load, 3);

  if(cpu_load[0] > MAX_CPULOAD)
  {
    pressanykey("¥Ø«e¨t²Î­t²ü %f ¹L°ª,½Ðµy«á¦A¨Ó",cpu_load[0]);
    return 1;
  }  
  return 0;
}
#endif

static void login_query()
{
  char uid[IDLEN + 1], passbuf[PASSLEN];
  int attempts;
  time_t i;
  char genbuf[200];
  char rbuf[128];
  extern struct UTMPFILE *utmpshm;

  resolve_utmp();
  attempts = utmpshm->number;
  clear();

//hialan: ¶Ã¼Æ¶i¯¸µe­±
  time(&i);
  i=i%5;
  sprintf(rbuf,"%s%d",WELCOME_TITLE,i);
  show_file(rbuf,0,17,ONLY_COLOR);

//  show_file("etc/Welcome_title",0,10,ONLY_COLOR);
//  move(10,0);
//  counter(BBSHOME"/log/counter/¤W¯¸¤H¦¸","¥úÁ{¥»¯¸",1);

  show_file("etc/Welcome_news",19,5,ONLY_COLOR);

  if (attempts >= MAXACTIVE)
  {
    pressanykey("¥Ø«e¯¸¤W¤H¼Æ¤w¹F¤W­­¡A½Ð±zµy«á¦A¨Ó¡C");
    oflush();
    sleep(1);
    exit(1);
  }
#ifdef BSD44
  if(over_load()) 
  {
    oflush();
    sleep(1);
    exit(1); 
  }
#endif
  attempts = 0;
  while (1)
  {
    if (attempts++ >= LOGINATTEMPTS)
    {
      pressanykey("¿ù»~¤Ó¦h¦¸,ÙTÙT~~~~~");
      exit(1);
    }
    uid[0] = '\0';
    getdata(21, 40, "Athenaeum Login: ",uid, IDLEN + 1 , DOECHO,0);
    if (ci_strcmp(uid, str_new) == 0)
    {
#ifdef LOGINASNEW
      DL_func("SO/register.so:va_new_register", 0);
      break;
#else
      pressanykey("¥»¨t²Î¥Ø«eµLªk¥H new µù¥U, ½Ð¥Î guest ¶i¤J");
      continue;
#endif
    }
    else if (uid[0] == '\0' || !dosearchuser(uid))
      pressanykey(err_uid);
    else if (strcmp(uid, STR_GUEST))
    {
      getdata(21, 40, "Password: ", passbuf, PASSLEN, PASS,0);
      passbuf[8] = '\0';
      if (!chkpasswd(cuser.passwd, passbuf))
      {
        logattempt(cuser.userid, '-');
        pressanykey(ERR_PASSWD);
      }
      else
      {
        /* SYSOP gets all permission bits */

        if (!ci_strcmp(cuser.userid, str_sysop))
          cuser.userlevel = ~0;
        if (0 && HAS_PERM(PERM_SYSOP) && !strncmp(getenv("RFC931"), "?@", 2)) {
           logattempt(cuser.userid, '*');
           outs("¯¸ªø½Ð¥Ñ trusted host ¶i¤J");
           continue;
        }
        else {
           logattempt(cuser.userid, ' ');
           break;
        }
      }
    }
    else
    {                           /* guest */
      cuser.userlevel = 0;
      cuser.uflag = COLOR_FLAG | PAGER_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
      break;
    }
  }

  multi_user_check();
  sethomepath(genbuf, cuser.userid);
  mkdir(genbuf, 0755);
  srand(time(0) ^ getpid() ^ (getpid() << 10));
  srandom(time(0) ^ getpid() ^ (getpid() << 10));
}

void add_distinct(char* fname, char* line)
{
   FILE *fp;
   int n = 0;

   if (fp = fopen(fname, "a+")) 
   {
      char buffer[80];
      char tmpname[100];
      FILE *fptmp;

      strcpy(tmpname, fname);
      strcat(tmpname, "_tmp");
      if (!(fptmp = fopen(tmpname, "w"))) 
      {
         fclose(fp);
         return;
      }

      rewind(fp);
      while (fgets(buffer, 80, fp)) 
      {
         char* p = buffer + strlen(buffer) - 1;

         if (p[-1] == '\n' || p[-1] == '\r')
            p[-1] = 0;
         if (!strcmp(buffer, line))
            break;
         sscanf(buffer + strlen(buffer) + 2, "%d", &n);
         fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
      }

      if (feof(fp))
         fprintf(fptmp, "%s%c#1\n", line, 0);
      else 
      {
         sscanf(buffer + strlen(buffer) + 2, "%d", &n);
         fprintf(fptmp, "%s%c#%d\n", buffer, 0, n + 1);
         while (fgets(buffer, 80, fp)) 
         {
            sscanf(buffer + strlen(buffer) + 2, "%d", &n);
            fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
         }
      }
      fclose(fp);
      fclose(fptmp);
      unlink(fname);
      rename(tmpname, fname);
   }
}


void del_distinct(char* fname, char* line)
{
   FILE *fp;
   int n = 0;

   if (fp = fopen(fname, "r")) 
   {
      char buffer[80];
      char tmpname[100];
      FILE *fptmp;

      strcpy(tmpname, fname);
      strcat(tmpname, "_tmp");
      if (!(fptmp = fopen(tmpname, "w"))) 
      {
         fclose(fp);
         return;
      }

      rewind(fp);
      while (fgets(buffer, 80, fp)) 
      {
         char* p = buffer + strlen(buffer) - 1;

         if (p[-1] == '\n' || p[-1] == '\r')
            p[-1] = 0;
         if (!strcmp(buffer, line))
            break;
         sscanf(buffer + strlen(buffer) + 2, "%d", &n);
         fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
      }

      if (!feof(fp))
         while (fgets(buffer, 80, fp)) 
         {
            sscanf(buffer + strlen(buffer) + 2, "%d", &n);
            fprintf(fptmp, "%s%c#%d\n", buffer, 0, n);
         }
      fclose(fp);
      fclose(fptmp);
      unlink(fname);
      rename(tmpname, fname);
   }
}

#ifdef WHERE
/* Jaky and Ptt*/
int where (char *from)
{
   extern struct FROMCACHE *fcache;
   register int i,count,j;

   resolve_fcache();
   for (j=0;j<fcache->top;j++)
   {
      char *token=strtok(fcache->domain[j],"&");
      i=0;count=0;
      while(token)
      {
         if (strstr(from,token)) count++;
         token=strtok(NULL, "&");
         i++;
      }
      if (i==count) break;
   }
   if (i!=count) return 0;
   return j;
}
#endif

static void check_BM()      /* Ptt ¦Û°Ê¨ú¤UÂ÷Â¾ªO¥DÅv¤O */
{
  int i;
  boardheader *bhdr;
  extern boardheader *bcache;
  extern int numboards;

  resolve_boards();
  cuser.userlevel &= ~PERM_BM;
  for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
      userid_is_BM(cuser.userid, bhdr->BM);
}

void setup_utmp(int mode)
{
  user_info uinfo;
  char buf[80];

  memset(&uinfo, 0, sizeof(uinfo));
  uinfo.pid = currpid = getpid();
  uinfo.uid = usernum;
  uinfo.mode = currstat = mode;
  uinfo.msgcount = 0;
  if(cuser.userlevel & PERM_BM) check_BM(); /* Ptt ¦Û°Ê¨ú¤UÂ÷Â¾ªO¥DÅv¤O */
  uinfo.userlevel = cuser.userlevel;
  uinfo.lastact = time(NULL);

  strcpy(uinfo.userid, cuser.userid);
  strcpy(uinfo.realname, cuser.realname);
  strcpy(uinfo.username, cuser.username);
  strcpy(uinfo.feeling, cuser.feeling);
  strncpy(uinfo.from, fromhost, 23);
  uinfo.invisible = cuser.invisible%2;
  uinfo.pager     = cuser.pager%5;
  uinfo.brc_id    = 0;

/* Ptt WHERE*/

#ifdef WHERE
  uinfo.from_alias = where(fromhost);
#else
  uinfo.from_alias = 0;
#endif
  sethomefile(buf, cuser.userid, "remoteuser");
  add_distinct(buf, getenv("RFC931"));

  if (enter_uflag & CLOAK_FLAG)
      uinfo.invisible = YEA;
  
  getnewutmpent(&uinfo);
  friend_load();
}

static void user_login()
{
  char genbuf[200];
  struct tm *ptime,*tmp;
  time_t now = time(0);
  int a;

  extern struct FROMCACHE *fcache;
  extern struct UTMPFILE *utmpshm;
  extern int fcache_semid;
  
  log_usies("ENTER", getenv("RFC931")/* fromhost */);
  setproctitle("%s: %s", cuser.userid, fromhost);

  /* ------------------------ */
  /* ªì©l¤Æ uinfo¡Bflag¡Bmode */
  /* ------------------------ */

  setup_utmp(LOGIN);
  currmode = MODE_STARTED;
  enter_uflag = cuser.uflag;

/* get local time */
  tmp = localtime(&cuser.lastlogin);

  update_data(); //wildcat: update user data
/*Ptt check ¦P®É¤W½u¤H¼Æ */
  resolve_fcache();
  resolve_utmp();

  if((a=utmpshm->number)>fcache->max_user)
  {
    sem_init(FROMSEM_KEY,&fcache_semid);
    sem_lock(SEM_ENTER,fcache_semid);
    fcache->max_user = a;
    fcache->max_time = now;
    sem_lock(SEM_LEAVE,fcache_semid);
  }

#ifdef  INITIAL_SETUP
  if (!getbnum(DEFAULT_BOARD))
  {
    strcpy(currboard, "©|¥¼¿ï©w");
  }
  else
#endif

  {
    brc_initial(DEFAULT_BOARD);
    set_board();
  }

  /* ------------ */
  /* µe­±³B²z¶}©l */
  /* ------------ */

  if (!(HAS_PERM(PERM_SYSOP) && HAS_PERM(PERM_DENYPOST)))
  {
    char buf[256];
    time_t now;
    
    time(&now);
    sprintf(buf,"<<¤W¯¸³qª¾>> -- §Ú¨ÓÅo¡I - %s",Etime(&now));
    do_aloha(buf);
  }

  time(&now);
  ptime = localtime(&now);

  if((cuser.day == ptime->tm_mday) && (cuser.month == (ptime->tm_mon + 1)))
    currutmp->birth  = 1;
  else
  {
    more("etc/Welcome_login", NA);
    currutmp->birth = 0 ;
  }

/* wildcat : ·h®a³qª¾¥Î
  if(belong(BBSHOME"/etc/oldip",fromhost))
  {
    more(BBSHOME"/etc/removal");
    abort_bbs();
  }    
*/
  if (cuser.userlevel)          /* not guest */
  {
    move(t_lines - 3, 0);
    prints("      Åwªï±z²Ä [1;33m%d[0;37m «×«ô³X¥»¯¸¡A\
¤W¦¸±z¬O±q [1;33m%s[0;37m ³s©¹¥»¯¸¡A\n\
     §Ú°O±o¨º¤Ñ¬O [1;33m%s[0;37m¡C\n",
      ++cuser.numlogins, cuser.lasthost,
      Etime(&cuser.lastlogin));
    pressanykey(NULL);


/* Ptt */
  if(currutmp->birth == 1)
   {
    more("etc/Welcome_birth",YEA);
    brc_initial("Greeting");
    set_board();
    do_post();
   }
    sethomefile(genbuf, cuser.userid, str_badlogin);
    if (more(genbuf, NA) != -1)
    {
      char ans;
      ans = getans2(b_lines -1, 0, "±z­n§R°£¥H¤W¿ù»~¹Á¸Õªº°O¿ý¶Ü? ", 0, 2, 'y');
      if (ans != 'n')
        unlink(genbuf);
    }
    check_register();
    strncpy(cuser.lasthost, fromhost, 24);
    substitute_record(fn_passwd, &cuser, sizeof(cuser), usernum);
    cuser.lasthost[23] = '\0';
    restore_backup();
  }
/* ªO¥D´£°ª«H½c¤W­­ */
  if(HAS_PERM(PERM_BM) && cuser.exmailbox < 100)
    cuser.exmailbox = 100;
  else if (!strcmp(cuser.userid, STR_GUEST))
  {
    char *nick[10] = {"ÄÑ¥]", "¬ü¤k¹Ï", "FreeBSD", "DBT¥Ø¿ý", "mp3",
                      "¤k¤ýÀY", "¯f¬r", "µ£¦~", "¥Û¹³", "386 CPU"
                     };
    char *name[10] = {"¥d±þ¦Ì¨È", "¥Õ³¾¿AÄR¤l", "­·»P¹Ð®J¤p¯¸", "¨gx©Î«Ox±¶", 
                      "Wu Bai & China Blue","¥ì¤O¨F¥Õ", "C-brain", 
                      "®É¶¡", "§Ú", "Intel inside" 
                      };
    int sex[10] = {6, 4, 7, 7, 2, 6, 0, 7, 7, 0};

    int i = rand() % 10;
    sprintf(cuser.username, "³Q­·¤Æªº%s", nick[i]);
    sprintf(currutmp->username, cuser.username);
    sprintf(cuser.realname, name[i]);
    sprintf(currutmp->realname, cuser.realname);
    cuser.sex = sex[i];
    cuser.silvermoney = 300;
    cuser.habit = HABIT_GUEST;	/* guest's habit */
    currutmp->pager = 2;
    pressanykey(NULL);
  }
  if (bad_user(cuser.userid)) 
  {
     sprintf(currutmp->username, "BAD_USER");
     cuser.userlevel = 0;
     cuser.uflag = COLOR_FLAG | PAGER_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
  }
  if (!PERM_HIDE(currutmp))
     cuser.lastlogin = login_start_time;
  substitute_record(fn_passwd, &cuser, sizeof(cuser), usernum);

   if(cuser.numlogins < 2)
   {
     more("etc/newuser",YEA);
     HELP();
   }
  login_plan();
}


void check_max_online()
{
  FILE *fp;
  int maxonline=0;
  time_t now = time(NULL);
  struct tm *ptime;

  ptime = localtime(&now);

  if(fp = fopen(".maxonline", "r"))
  {
    fscanf(fp, "%d", &maxonline);
    fclose(fp);
  }

  if ((count_ulist() > maxonline) && (fp = fopen(".maxonline", "w")))
  {
    fprintf(fp, "%d", count_ulist());
    fclose(fp);
  }
  if(fp = fopen(".maxtoday", "r"))
  {
    fscanf(fp, "%d", &maxonline);
    if (count_ulist() > maxonline){
      fclose(fp);
      fp = fopen(".maxtoday", "w");
      fprintf(fp, "%d", count_ulist());
    }
    fclose(fp);
  }
}

void start_client()
{
  char cmd[80] = "?@";
  extern struct commands cmdlist[];
  extern char currmaildir[32];

  if (!getenv("RFC931"))
    setenv("RFC931", strcat(cmd, fromhost), 1);  

  /* ----------- */
  /* system init */
  /* ----------- */
  currmode = 0;
  update_time = login_start_time;

  signal(SIGHUP, abort_bbs);
  signal(SIGBUS, abort_bbs);
  signal(SIGSEGV, abort_bbs);
#ifdef SIGSYS
  signal(SIGSYS, abort_bbs);
#endif
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGURG, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);  

  signal(SIGUSR1, talk_request);  // ¦¬¨ì°T®§
  signal(SIGUSR2, write_request);  // °e¥X°T®§

  if (setjmp(byebye))
    abort_bbs();

  dup2(0, 1);
  
//  term_init_m2("vt100");
  initscr();
  login_query();
  user_login();
  check_max_online();
/* wildcat : ­×´_¤¤
  if(!HAVE_PERM(PERM_SYSOP))
  {
    pressanykey("¾ã­×¤¤ , ¸T¤î¯¸ªø¥H¥~ user µn¤J");
    abort_bbs();
  }
*/

  sethomedir(currmaildir, cuser.userid);
#ifdef DOTIMEOUT
  init_alarm();
#else
  signal(SIGALRM, SIG_IGN);
#endif
  if (HAVE_PERM(PERM_SYSOP | PERM_BM))
    DL_func("SO/vote.so:b_closepolls");
  if (!HAVE_HABIT(HABIT_COLOR))
    showansi = 0;
  while (chkmailbox())
     m_read();

#ifdef HAVE_GAME
  waste_money();
#endif

  force_board("Announce");
  {
    char buf[80];
    setbfile(buf, "VIP", FN_LIST);
    if(belong_list(buf, cuser.userid) > 0)
      force_board("VIP");
    setbfile(buf, "WD_plan", FN_LIST);
    if(belong_list(buf, cuser.userid) > 0)
      force_board("WD_plan");
  }
  if(HAS_PERM(PERM_SYSOP))
    force_board("Working");
        
//  force_board("Boards");

  if (HAS_PERM(PERM_ACCOUNTS) && dashf(fn_register))
    DL_func("SO/admin.so:m_register");
  domenu(MMENU, "¥D¥\\¯àªí", (chkmail(0) ? 'M' : 'B'), cmdlist);
}

/*-------------------------------------------------------*/
/* mbbsd.c       ( NTHU CS MapleBBS Ver 2.36 )           */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : BBS main/			                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol       */
/* ----------------------------------------------------- */

static void telnet_init()
{
  static char svr[] = 
  {
    IAC, DO, TELOPT_TTYPE,
    IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
    IAC, WILL, TELOPT_ECHO,
    IAC, WILL, TELOPT_SGA
  };

  int n, len;
  char *cmd;
  int rset;
  struct timeval to;
  char buf[64];

  /* --------------------------------------------------- */
  /* init telnet protocol                                */
  /* --------------------------------------------------- */

  cmd = svr;

  for (n = 0; n < 4; n++)
  {
    len = (n == 1 ? 6 : 3);
    send(0, cmd, len, 0);
    cmd += len;

    rset = 1;
    /* Thor.981221: for future reservation bug */
    to.tv_sec = 1;
    to.tv_usec = 1;
    if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
      recv(0, buf, sizeof(buf), 0);
  }
}

static void term_init()
{
#if 0   /* fuse.030518: µù¸Ñ */
  server°Ý¡G§A·|§ïÅÜ¦æ¦C¼Æ¶Ü¡H(TN_NAWS)
  clientµª¡GYes, I do. (TNCH_DO)

  ¨º»ò¦b³s½u®É¡A·íTERMÅÜ¤Æ¦æ¦C¼Æ®É´N·|µo¥X¡G
  TNCH_IAC + TNCH_SB + TN_NAWS + ¦æ¼Æ¦C¼Æ + TNCH_IAC + TNCH_SE;
#endif

  /* ask client to report it's term size */
  static char svr[] =       /* server */
  {
    IAC, DO, TELOPT_NAWS
  };

  static char rvr[] =       /* receiver */
  {
    IAC, WILL, TELOPT_NAWS,
    IAC, SB, TELOPT_NAWS,
    IAC, WONT, TELOPT_NAWS
  };

  int len, rset;
  char *cmd, *res, buf[64];
  struct timeval to;

  cmd = svr;

  len = 3;

return;
  
  /* °Ý¹ï¤è (telnet client) ¦³¨S¦³¤ä´©¤£¦Pªº¿Ã¹õ¼e°ª */
  send(0, cmd, len, 0);

  rset = 1;
  if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
    recv(0, buf, sizeof(buf), 0);

  cmd = rvr;
  res = buf;
  if (!memcmp(res, cmd, 3))
  {
    /* got IAC WILL NAWS : ¹ï¤è»¡¦³ */
    res += 3;
    if (!memcmp(res, cmd + 3, 3))
    {
      /* got IAC SB NAWS ¡G±µµÛ¦^¶Ç¸ê®Æ */
      res += 3;
      /* t_width = res[0] * 256 + res[1]; */
      t_columns = res[0] * 256 + res[1];
      t_lines = res[2] * 256 + res[3];
    }
  }
  else if (!memcmp(res, cmd + 3, 3))
  {
    /* got IAC SB NAWS */
    res += 3;
    /* t_width = res[0] * 256 + res[1]; */
    t_columns = res[0] * 256 + res[1];
    t_lines = res[2] * 256 + res[3];
  }
  else if (!memcmp(res, cmd + 6, 3))
  {
    /* got IAC WONT NAWS; default to 80x24 ¡G¹ï¤è»¡¨S¦³ */
    /* t_width = 80; */
    t_columns = 80;
    t_lines = 24;
  }
  else
  {
    /* what ever else; default to 80x24 ¡G¨S¦³¦^ÂÐ */
    /* t_width = 80; */
    t_columns = 80;
    t_lines = 24;
  }

  /* b_lines ¦Ü¤Ö­n 23¡A³Ì¦h¤£¯à¶W¹L t_lines - 1 */
  if(t_lines < 24)
    t_lines = 24;
    
  b_lines = t_lines - 1;
  p_lines = t_lines - 4;
  return;
}

/* ----------------------------------------------- */
/* ¨ú±o remote user name ¥H§P©w¨­¥÷                */
/* ----------------------------------------------- */


/*
 * rfc931() speaks a common subset of the RFC 931, AUTH, TAP, IDENT and RFC
 * 1413 protocols. It queries an RFC 931 etc. compatible daemon on a remote
 * host to look up the owner of a connection. The information should not be
 * used for authentication purposes. This routine intercepts alarm signals.
 *
 * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
 */

#include <setjmp.h>

#define STRN_CPY(d,s,l) { strncpy((d),(s),(l)); (d)[(l)-1] = 0; }
#define RFC931_TIMEOUT   10
#define RFC931_PORT     113     /* Semi-well-known port */
#define ANY_PORT        0       /* Any old port will do */


/* ------------------------- */
/* timeout - handle timeouts */
/* ------------------------- */


static void
timeout(sig)
  int sig;
{
  (void) longjmp(byebye, sig);
}


static void
getremotename(from, rhost, rname)
  struct sockaddr_in *from;
  char *rhost;
  char *rname;
{
  struct sockaddr_in our_sin;
  struct sockaddr_in rmt_sin;
  unsigned rmt_port, rmt_pt;
  unsigned our_port, our_pt;
  FILE *fp;
  char buffer[512], user[80], *cp;
  int s;
  struct hostent *hp;

  /* get remote host name */

  hp = NULL;
  if (setjmp(byebye) == 0)
  {
  /* CHANGE by hialan.020729 for hostname*/
    (void)  signal(SIGALRM, timeout);
    (void)  alarm(3);
    hp = gethostbyaddr((char *) &from->sin_addr, sizeof(struct in_addr),
      from->sin_family);
    (void)  alarm(0);
  }

  /* ADD by hialan.020729 for hostname*/
  (void) strcpy(rhost, hp ? hp->h_name : (char *) inet_ntoa(from->sin_addr));

  /*
   * Use one unbuffered stdio stream for writing to and for reading from the
   * RFC931 etc. server. This is done because of a bug in the SunOS 4.1.x
   * stdio library. The bug may live in other stdio implementations, too.
   * When we use a single, buffered, bidirectional stdio stream ("r+" or "w+"
   * mode) we read our own output. Such behaviour would make sense with
   * resources that support random-access operations, but not with sockets.
   */

  s = sizeof our_sin;
  if (getsockname(0, (struct sockaddr*)&our_sin, &s) < 0)
    return;

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("bbsd: socket in rfc931");
    return;
  }

  if (!(fp = fdopen(s, "r+")))
  {
    (void) close(s);
    perror("bbsd:fdopen");
    return;
  }

  /*
   * Set up a timer so we won't get stuck while waiting for the server.
   */

  if (setjmp(byebye) == 0)
  {
    signal(SIGALRM, timeout);
    alarm(RFC931_TIMEOUT);

    /*
     * Bind the local and remote ends of the query socket to the same IP
     * addresses as the connection under investigation. We go through all
     * this trouble because the local or remote system might have more than
     * one network address. The RFC931 etc. client sends only port numbers;
     * the server takes the IP addresses from the query socket.
     */

    our_pt = ntohs(our_sin.sin_port);
    our_sin.sin_port = htons(ANY_PORT);

    rmt_sin = *from;
    rmt_pt = ntohs(rmt_sin.sin_port);
    rmt_sin.sin_port = htons(RFC931_PORT);

    setbuf(fp, (char *) 0);
    s = fileno(fp);

    if (bind(s, (struct sockaddr *) & our_sin, sizeof(our_sin)) >= 0 &&
      connect(s, (struct sockaddr *) & rmt_sin, sizeof(rmt_sin)) >= 0)
    {
      /*
       * Send query to server. Neglect the risk that a 13-byte write would
       * have to be fragmented by the local system and cause trouble with
       * buggy System V stdio libraries.
       */

      fprintf(fp, "%u,%u\r\n", rmt_pt, our_pt);
      fflush(fp);

      /*
       * Read response from server. Use fgets()/sscanf() so we can work
       * around System V stdio libraries that incorrectly assume EOF when a
       * read from a socket returns less than requested.
       */

      if (fgets(buffer, sizeof(buffer), fp) && !ferror(fp) && !feof(fp)
        && sscanf(buffer, "%u , %u : USERID :%*[^:]:%79s",
          &rmt_port, &our_port, user) == 3
        && rmt_pt == rmt_port && our_pt == our_port)
      {

        /*
         * Strip trailing carriage return. It is part of the protocol, not
         * part of the data.
         */

        if (cp = (char *) strchr(user, '\r'))
          *cp = 0;
        strcpy(rname, user);
      }
    }
    alarm(0);
  }
  fclose(fp);
}


/* ----------------------------------------------------- */
/* stand-alone daemon                                    */
/* ----------------------------------------------------- */


static int mainset;             /* read file descriptor set */
static struct sockaddr_in xsin;


static void
reapchild()
{
  int state, pid;

  while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0);
}


static void
start_daemon(genbuf)
  char *genbuf;
{
  int n;
  char buf[80];

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time_t dummy = time(NULL);
  struct tm *dummy_time = localtime(&dummy);
  (void) strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", dummy_time);
/*
  (void) gethostname(myhostname, sizeof(myhostname));
*/

  if (n=fork())
   {
//    printf("pid[%d]\n",n);
    printf("%d [The_main_pid]\n",n);    
    exit(0);
   }
  n = getdtablesize();
  sprintf(genbuf, "%d\t%s", getpid(), buf);

  while (n)
    (void) close(--n);

  n = open("/dev/tty", O_RDWR);
  if (n > 0)
  {
    (void) ioctl(n, TIOCNOTTY, (char *) 0);
    (void) close(n);
  }

  for (n = 1; n < NSIG; n++)
    (void) signal(n, SIG_IGN);
}


static void close_daemon()
{
  exit(0);
}


static int
bind_port(port)
  int port;
{
  int sock, on;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  on = 1;
  (void) setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
  (void) setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof(on));

  on = 0;
  (void) setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &on, sizeof(on));

  xsin.sin_port = htons(port);
  if (bind(sock, (struct sockaddr *)&xsin, sizeof xsin) < 0) {
    syslog(LOG_INFO, "bbsd bind_port can't bind to %d",port);
    exit(1);
  }

  if (listen(sock, SOCKET_QLEN) < 0) {
    syslog(LOG_INFO, "bbsd bind_port can't listen to %d",port);
    exit(1);
  }

  /*(void)*/ FD_SET(sock, (fd_set *) & mainset);
  return sock;
}

int bad_host(char* name)
{
   FILE* list;
   char buf[40];

  if (list = fopen(BBSHOME"/etc/bad_host", "r")) 
  {
     while (fgets(buf, 40, list)) 
     {
        buf[strlen(buf) - 1] = '\0';
        if (!strcmp(buf, name))
           return 1;
        if (buf[strlen(buf) - 1] == '.' && !strncmp(buf, name, strlen(buf)))
           return 1;
        if (*buf == '.' && strlen(buf) < strlen(name) &&
            !strcmp(buf, name + strlen(name) - strlen(buf)))
           return 1;
     }
     fclose(list);
  }
  return 0;
}


int main(int argc, char *argv[], char *envp[])
{
  extern int errno;
  register int msock, csock;    /* socket for Master and Child */
  register int nfds;            /* number of sockets */
  register pid_t pid;
  int readset;
  int value;
  struct timeval tv;
  char genbuf[1024];  /*hialan*/

  genbuf[0] = 0;
  
  /* --------------------------------------------------- */
  /* setup standalone                                    */
  /* --------------------------------------------------- */
  start_daemon(genbuf);

  (void) signal(SIGCHLD, reapchild);
  (void) signal(SIGTERM, close_daemon);


  /* --------------------------------------------------- */
  /* port binding                                        */
  /* --------------------------------------------------- */

  xsin.sin_family = AF_INET;

  sprintf(margs,"%s ",argv[0]);
  if (argc > 1)
  {
    msock = -1;
    for (nfds = 1; nfds < argc; nfds++)
    {
      csock = atoi(argv[nfds]);
      if (csock > 0)
        {
         msock = bind_port(csock);
         strcat(margs,argv[nfds]);
	 strcat(margs," ");
	}
      else
         break;
    }
    if (msock < 0)
      exit(1);
  }
  else
  {
    static int ports[] = {23,3333,3006};

    for (nfds = 0; nfds < sizeof(ports) / sizeof(int); nfds++)
    {
      csock = ports[nfds];
      msock = bind_port(csock);

      sprintf(genbuf + 512, "\t%d", csock);
      strcat(genbuf, genbuf + 512);
    }
  }
  nfds = msock + 1;

  /* --------------------------------------------------- */
  /* Give up root privileges: no way back from here      */
  /* --------------------------------------------------- */

  (void) setgid(BBSGID);
  (void) setuid(BBSUID);
  (void) chdir(BBSHOME);

  f_cat(PID_FILE, genbuf); 


  initsetproctitle(argc, argv, envp);
  printpt("%s: listening ",margs);

#ifdef MBBSD_LAST
  if(system("test " MBBSD_LASTLOG) == 0)
    f_rm(MBBSD_LASTLOG);
  f_cat(MBBSD_LASTLOG, genbuf);
#endif

  /* --------------------------------------------------- */
  /* main loop                                           */
  /* --------------------------------------------------- */

/*
  th_low = (argc > 1) ? atoi(argv[1]) : TH_LOW;
  th_high = (argc > 2) ? atoi(argv[2]) : TH_HIGH;
*/

  tv.tv_sec = 60 * 30;
  tv.tv_usec = 0;


  for (;;)
  {
again:

    readset = mainset;
    msock = select(nfds, (fd_set *) & readset, NULL, NULL, &tv);

    if (msock < 0)
    {
      goto again;
    }
    else if (msock == 0)        /* No network traffic */
      continue;

    msock = 0;
    csock = 1;
    for (;;)
    {
      if (csock & readset)
        break;
      if (++msock >= nfds)
        goto again;
      csock <<= 1;
    }

    value = sizeof xsin;
    do
    {
      csock = accept(msock, (struct sockaddr *)&xsin, &value);
    } while (csock < 0 && errno == EINTR);

    if (csock < 0)
    {
      goto again;
    }

/*
    pid = *totaluser;
    if (pid >= MAXACTIVE)
    {
      char buf[128];

      (void) sprintf(buf, "¥Ø«e½u¤W¤H¼Æ [%d] ¤H¡A«Èº¡¤F¡A½Ðµy«á¦A¨Ó", pid);
      (void) write(csock, buf, strlen(buf));
      (void) close(csock);
      goto again;
    }
*/
    pid = fork();

    if (!pid)
    {
     char info[256];
     time_t now;

     initsetproctitle(argc, argv, envp);
     nice(2);      /*  Ptt lower priority */
//     sprintf(info, BANNER, genbuf);   //marked by hialan
     sprintf(info, BANNER, "");
     info[255]=0;
     write(csock,info , strlen(info));
      while (--nfds >= 0)
        (void) close(nfds);
      (void) dup2(csock, 0);
      (void) close(csock);
      time(&now);
      login_start_time = now;

      getremotename(&xsin, fromhost, remoteusername);   /* FC931 */

      /* ban ±¼ bad host / bad user  */
      if (bad_host(fromhost))
         exit(1);
      setenv("REMOTEHOST", fromhost, 1);
      setenv("REMOTEUSERNAME", remoteusername, 1);
      {
        char RFC931[80];
        sprintf(RFC931, "%s@%s", remoteusername, fromhost);
        setenv("RFC931", RFC931, 1);
      }
      telnet_init();
//      term_init();
      printpt("mbbsd : %s",fromhost);
      start_client();
    }
    
    (void) close(csock);
  }
}

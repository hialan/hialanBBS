/*-------------------------------------------------------*/
/* cache.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : cache up data by shared memory               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#ifdef  HAVE_MMAP
#include <sys/mman.h>
#else
int usernumber;
#endif
struct UCACHE *uidshm;
struct BCACHE *brdshm;


/*-------------------------------------------------------*/
/* .PASSWDS cache                                        */
/*-------------------------------------------------------*/


#ifndef HAVE_MMAP
static int
fillucache(uentp)
  userec *uentp;
{
  if (usernumber < MAXUSERS)
  {
    strncpy(uidshm->userid[usernumber], uentp->userid, IDLEN + 1);
    uidshm->userid[usernumber++][IDLEN] = '\0';
  }
  return 0;
}
#endif


/* -------------------------------------- */
/* (1) 系統啟動後，第一個 BBS user 剛進來 */
/* (2) .PASSWDS 更新時                    */
/* -------------------------------------- */
reload_ucache()
{
   if (uidshm->busystate)
   {
     /* ------------------------------------------ */
     /* 其他 user 正在 flushing ucache ==> CSMA/CD */
     /* ------------------------------------------ */

     if (uidshm->touchtime - uidshm->uptime > 30)
     {
       uidshm->busystate = 0;  /* leave busy state */
       uidshm->uptime = uidshm->touchtime - 1;
#if !defined(_BBS_UTIL_C_)
       log_usies("CACHE", "refork token");
#endif
     }
     else
       sleep(1);
   }
   else
   {
     uidshm->busystate = 1;    /* enter busy state */
#ifdef  HAVE_MMAP
     {
       register int fd, usernumber;

       usernumber = 0;

       if ((fd = open(".PASSWDS", O_RDONLY)) > 0)
       {
         caddr_t fimage, mimage;
         struct stat stbuf;

         fstat(fd, &stbuf);
         fimage = mmap(NULL, stbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
         if (fimage == (char *) -1)
           exit(-1);
         close(fd);
         fd = stbuf.st_size / sizeof(userec);
         if (fd > MAXUSERS)
           fd = MAXUSERS;
         for (mimage = fimage; usernumber < fd; mimage += sizeof(userec))
         {
           memcpy(uidshm->userid[usernumber++], mimage, IDLEN);
         }
         munmap(fimage, stbuf.st_size);
       }
       uidshm->number = usernumber;
     }
#else
     usernumber = 0;
     rec_apply(".PASSWDS", fillucache, sizeof(userec));
     uidshm->number = usernumber;
#endif

     /* 等 user 資料更新後再設定 uptime */
     uidshm->uptime = uidshm->touchtime;

#if !defined(_BBS_UTIL_C_)
     log_usies("CACHE", "reload ucache");
#endif
     uidshm->busystate = 0;    /* leave busy state */
   }
}

void
resolve_ucache()
{
  if (uidshm == NULL)
  {
    uidshm = shm_new(UIDSHM_KEY, sizeof(*uidshm));
    if (uidshm->touchtime == 0)
      uidshm->touchtime = 1;
  }
  while (uidshm->uptime < uidshm->touchtime)
     reload_ucache();
}

int
ci_strcmp(s1, s2)
  register char *s1, *s2;
{
  register int c1, c2, diff;

  do
  {
    c1 = *s1++;
    c2 = *s2++;
    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 32;
    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 32;
    if (diff = c1 - c2)
      return (diff);
  } while (c1);
  return 0;
}


int
searchuser(userid)
  char *userid;
{
  register char *ptr;
  register int i, j;
  resolve_ucache();
  i = 0;
  j = uidshm->number;
  while (i < j)
  {
    ptr = uidshm->userid[i++];
    if (!ci_strcmp(ptr, userid))
    {
      strcpy(userid, ptr);
      return i;
    }
  }
  return 0;
}

#if !defined(_BBS_UTIL_C_)
int
getuser(userid)
  char *userid;
{
  int uid;

  if (uid = searchuser(userid))
    rec_get(fn_passwd, &xuser, sizeof(xuser), uid);

  return uid;
}

int
do_getuser(userid, tuser)
  char *userid;
  userec *tuser;
{
  int uid;

  if (uid = searchuser(userid))
    rec_get(fn_passwd, tuser, sizeof(userec), uid);

  return uid;
}

char *
getuserid(num)
  int num;
{
  if (--num >= 0 && num < MAXUSERS)
  {
    return ((char *) uidshm->userid[num]);
  }
  return NULL;
}


void
setuserid(num, userid)
  int num;
  char *userid;
{
  if (num > 0 && num <= MAXUSERS)
  {
    if (num > uidshm->number)
      uidshm->number = num;
    strncpy(uidshm->userid[num - 1], userid, IDLEN + 1);
  }
}


int
searchnewuser(mode)
  int mode;

/* 0 ==> 找過期帳號 */
/* 1 ==> 建立新帳號 */
{
  register int i, num;

  resolve_ucache();
  num = uidshm->number;
  i = 0;
  while (i < num)
  {
    if (!uidshm->userid[i++][0])
      return (i);
  }
  if (mode && (num < MAXUSERS))
    return (num + 1);
  return 0;
}



char *
u_namearray(buf, pnum, tag)
  char buf[][IDLEN + 1], *tag;
  int *pnum;
{
  register struct UCACHE *reg_ushm = uidshm;
  register char *ptr, tmp;
  register int n, total;
  char tagbuf[STRLEN];
  int ch, ch2, num;

  resolve_ucache();
  if (*tag == '\0')
  {
    *pnum = reg_ushm->number;
    return reg_ushm->userid[0];
  }
  for (n = 0; tag[n]; n++)
  {
    tagbuf[n] = chartoupper(tag[n]);
  }
  tagbuf[n] = '\0';
  ch = tagbuf[0];
  ch2 = ch - 'A' + 'a';
  total = reg_ushm->number;
  for (n = num = 0; n < total; n++)
  {
    ptr = reg_ushm->userid[n];
    tmp = *ptr;
    if (tmp == ch || tmp == ch2)
    {
      if (chkstr(tag, tagbuf, ptr))
        strcpy(buf[num++], ptr);
    }
  }
  *pnum = num;
  return buf[0];
}

/*-------------------------------------------------------*/
/* .UTMP cache                                           */
/*-------------------------------------------------------*/

struct UTMPFILE *utmpshm;
user_info *currutmp = NULL;

void
resolve_utmp()
{
  if (utmpshm == NULL)
  {
    utmpshm = shm_new(UTMPSHM_KEY, sizeof(*utmpshm));
    if (utmpshm->uptime == 0)
      utmpshm->uptime = utmpshm->number = 1;
  }
}


void
setutmpmode(mode)
  int mode;
{
  if (currstat != mode)
    currutmp->mode = currstat = mode;
}


/*
woju
*/
resetutmpent()
{
  extern int errno;
  register time_t now;
  register int i;
  register pid_t pid;
  register user_info *uentp;

  resolve_utmp();
  now = time(NULL) - 79;
  if (now > utmpshm->uptime)
    utmpshm->busystate = 0;

  while (utmpshm->busystate)
  {
    sleep(1);
  }
  utmpshm->busystate = 1;
  /* ------------------------------ */
  /* for 幽靈傳說: 每 79 秒更新一次 */
  /* ------------------------------ */

  for (i = now = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (pid = uentp->pid)
    {
       if ((kill(pid, 0) == -1) && (errno == ESRCH))
           memset(uentp, 0, sizeof(user_info));
        else
           now++;
    }
  }
  utmpshm->number = now;
  time(&utmpshm->uptime);
  utmpshm->busystate = 0;
}


void
getnewutmpent(up)
  user_info *up;
{
  extern int errno;
  register time_t now;
  register int i;
  register pid_t pid;
  register user_info *uentp;

  resolve_utmp();
  now = time(NULL) - 79;
  if (now > utmpshm->uptime)
    utmpshm->busystate = 0;

  while (utmpshm->busystate)
  {
    sleep(1);
  }
  utmpshm->busystate = 1;
  /* ------------------------------ */
  /* for 幽靈傳說: 每 79 秒更新一次 */
  /* ------------------------------ */

  if (now > utmpshm->uptime)
  {
    for (i = now = 0; i < USHM_SIZE; i++)
    {
      uentp = &(utmpshm->uinfo[i]);
      if (pid = uentp->pid)
      {
        if ((kill(pid, 0) == -1) && (errno == ESRCH))
          memset(uentp, 0, sizeof(user_info));
        else
          now++;
      }
    }
    utmpshm->number = now;
    time(&utmpshm->uptime);
  }

  for (i = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (!(uentp->pid))
    {
      memcpy(uentp, up, sizeof(user_info));
      currutmp = uentp;
      utmpshm->number++;
      utmpshm->busystate = 0;
      return;
    }
  }
  utmpshm->busystate = 0;
  sleep(1);
  exit(1);
}


int
apply_ulist(fptr)
  int (*fptr) ();
{
  register user_info *uentp;
  register int i, state;

  resolve_utmp();
  for (i = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (uentp->pid && (PERM_HIDE(currutmp) || !PERM_HIDE(uentp)))
      if (state = (*fptr) (uentp))
        return state;
  }
  return 0;
}


user_info *
search_ulist(fptr, farg)
  int (*fptr) ();
int farg;
{
  register int i;
  register user_info *uentp;

  resolve_utmp();
  for (i = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if ((*fptr) (farg, uentp))
      return uentp;
  }
  return 0;
}


int
count_multi()
{
  register int i, j;
  register user_info *uentp;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (uentp->uid == usernum)
      j++;
  }
  return j;
}


/* -------------------- */
/* for multi-login talk */
/* -------------------- */

user_info *
search_ulistn(fptr, farg, unum)
  int (*fptr) ();
int farg;
int unum;
{
  register int i, j;
  register user_info *uentp;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if ((*fptr) (farg, uentp))
    {
      if (++j == unum)
        return uentp;
    }
  }
  return 0;
}


int
count_logins(fptr, farg, show)
  int (*fptr) ();
int farg;
int show;
{
  register user_info *uentp;
  register int i, j;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if ((*fptr) (farg, uentp))
    {
      j++;
      if (show)
      {
        prints("(%d) 目前狀態為: %-17.16s(來自 %s)\n", j,
          modestring(uentp, 0), uentp->from);
      }
    }
  }
  return j;
}


void
purge_utmp(uentp)
  user_info *uentp;
{
  memset(uentp, 0, sizeof(user_info));
  if (utmpshm->number)
    utmpshm->number--;
}


/*copy count_ulist過來, 用 guest 的權限算站上人數 hialan.020722*/

int
guest_count_ulist()
{
   int ans = utmpshm->number;
   register user_info *uentp;
   int ch = 0;

   while (ch < USHM_SIZE) 
   {
      uentp = &(utmpshm->uinfo[ch++]);
      if (uentp->pid && 
          (uentp->invisible || (uentp->userlevel & PERM_DENYPOST) ))
         --ans;
   }

   return ans;
}

int
count_ulist()
{
   int ans = utmpshm->number;
   register user_info *uentp;
   int ch = 0;

   while (ch < USHM_SIZE) {
      uentp = &(utmpshm->uinfo[ch++]);
      if (uentp->pid && (
          is_rejected(uentp) & 2 && !HAS_PERM(PERM_SYSOP) ||
          uentp->invisible && !HAS_PERM(PERM_SEECLOAK) &!HAS_PERM(PERM_SYSOP) ||
          !PERM_HIDE(currutmp) && PERM_HIDE(uentp) ||
          cuser.uflag & FRIEND_FLAG && !is_friend(uentp)
         ))
         --ans;
   }

   return ans;
}

#endif

/*-------------------------------------------------------*/
/* .BOARDS cache                                         */
/*-------------------------------------------------------*/

boardheader *bcache;
int numboards = -1;
int brd_semid;

/* force re-caching */

void
touch_boards()
{
  time(&(brdshm->touchtime));
  numboards = -1;
}


reload_bcache()
{
   if (brdshm->busystate)
   {
     sleep(1);
   }
#if !defined(_BBS_UTIL_C_)
   else
   {
     int fd;

     brdshm->busystate = 1;
     sem_init(BRDSEM_KEY,&brd_semid);
     sem_lock(SEM_ENTER,brd_semid);

     if ((fd = open(fn_board, O_RDONLY)) > 0)
     {
       brdshm->number = read(fd, bcache, MAXBOARD * sizeof(boardheader))
         / sizeof(boardheader);
       close(fd);
     }
/*
     memset(brdshm->total, 0, MAXBOARD * sizeof(usint));
*/
     memset(brdshm->lastposttime, 0, MAXBOARD * sizeof(time_t));
     /* 等所有 boards 資料更新後再設定 uptime */

     brdshm->uptime = brdshm->touchtime;
#if !defined(_BBS_UTIL_C_)
     log_usies("CACHE", "reload bcache");
#endif
     sem_lock(SEM_LEAVE,brd_semid);
     brdshm->busystate = 0;
   }
#endif
}

void
resolve_boards()
{
  if (brdshm == NULL)
  {
    brdshm = shm_new(BRDSHM_KEY, sizeof(*brdshm));
    if (brdshm->touchtime == 0)
      brdshm->touchtime = 1;
    bcache = brdshm->bcache;
  }

  while (brdshm->uptime < brdshm->touchtime)
     reload_bcache();
  numboards = brdshm->number;
}

int
getbnum(bname)
  char *bname;
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i++ < numboards; bhdr++)
#if !defined(_BBS_UTIL_C_)
  if (Ben_Perm(bhdr)) 
#endif
    if (!ci_strcmp(bname, bhdr->brdname))
      return i;
  return 0;
}
#if !defined(_BBS_UTIL_C_)
int
apply_boards(func)
  int (*func) ();
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
  {
    if (Ben_Perm(bhdr))
      if ((*func) (bhdr) == QUIT)
        return QUIT;
  }
  return 0;
}


boardheader *
getbcache(bname)
  char *bname;
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
  if (Ben_Perm(bhdr)) 
    if (!ci_strcmp(bname, bhdr->brdname))
      return bhdr;
  return NULL;
}

int
touchbtotal(bname)
  char *bname;
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i++ < numboards; bhdr++)
    if (!strcmp(bname, bhdr->brdname))
        {
         brdshm->total[i-1] = 0;
         brdshm->lastposttime[i-1] = 0;
         return i;
        }
  return 0;
}

int
inbtotal(bname, add)
  char *bname;
  int  add;
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i++ < numboards; bhdr++)
    if (!strcmp(bname, bhdr->brdname))
	{
	  if(brdshm->total[i-1]) brdshm->total[i-1] += add;
          brdshm->lastposttime[i-1] = 0;
         return i;
	}
  return 0;
}

char *
getbname(bid)
  int bid;
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i++ < bid; bhdr++)
    if (i == bid)
      return bhdr->brdname;
  return " ";
}


int
haspostperm(bname)
  char *bname;
{
  register int i;
  char buf[MAXPATHLEN];

/*
  if (currmode & MODE_DIGEST || currmode & MODE_ETC)
    return 0;
*/

//  setbfile(buf, bname, fn_water);
  setbfile(buf, bname, FN_LIST);
//  if (belong(buf, cuser.userid))
  if (!belong_list(buf, cuser.userid) == 2)
     return 0;

  if (!ci_strcmp(bname, DEFAULT_BOARD))
    return 1;

  if (!HAS_PERM(PERM_BASIC))
    return 0;

  if (!(i = getbnum(bname)))
    return 0;

  /* 秘密看板特別處理 */

  if (bcache[i - 1].brdattr & BRD_HIDE)
    return 1;

  i = bcache[i - 1].level;
  return (HAS_PERM(i & ~PERM_POST));
}
/*-------------------------------------------------------*/
/* FILM  cache                                           */
/*-------------------------------------------------------*/
/* cachefor 動態看版 */

struct FILMCACHE *film;
int film_semid;

void
reload_filmcache()
{
   if (film->busystate)
   {
     sleep(1);
   }
   else
   {
     fileheader item;
     char pbuf[256], buf[256], *chr;
     FILE *fp;
     int num, id, j;
     
     film->busystate = 1;
     sem_init(FILMSEM_KEY,&film_semid);
     sem_lock(SEM_ENTER,film_semid);

     film->max_film = 0;
     bzero(film->notes, sizeof film->notes);
     setapath(pbuf, "Note");
     setadir(buf, pbuf);
     num = rec_num(buf, sizeof item);
     id =0;
     for (j = 0; j <= num; j++)
     {
      if (rec_get(buf, &item, sizeof item, j) != -1)
        if (item.title[3]=='<' && item.title[8]=='>')
          {
	   char path[256],buf1[256];
	   int num1,k;
           fileheader subitem;

	   setapath(path,"Note");
	   sprintf(buf1,"%s/%s",path,item.filename);
           setadir(path, buf1);
           num1 = rec_num(path, sizeof item);
           for (k = 0; k <= num1; k++)
		{
		      if (rec_get(path, &subitem, sizeof item, k) != -1)
			{
	                    sprintf(film->notes[id++],"%s/%s",
				item.filename , subitem.filename);
 			    if (id >= MAX_MOVIE) break;
			}
		}
           if (id >= MAX_MOVIE) break;
          }
     }
     film->max_film = id-1;
     film->max_history = film->max_film - 2;
     if (film->max_history > MAX_HISTORY - 1)
          film->max_history = MAX_HISTORY - 1;
     if (film->max_history <0) film->max_history=0;

      fp=fopen("etc/today_is","r");

      if(fp)
      {
        fgets(film->today_is,20,fp);
        if(chr=strchr(film->today_is,'\n')) *chr=0;
        film->today_is[20]=0;
        fclose(fp);
      }
     
     /* 等所有資料更新後再設定 uptime */

     film->uptime = film->touchtime ;
#if !defined(_BBS_UTIL_C_)
     log_usies("CACHE", "reload filmcache");
#endif
     sem_lock(SEM_LEAVE,film_semid);
     film->busystate = 0;
   }
}

void
resolve_garbage()
{
  int count=0;
  if (film == NULL)
  {
    film = shm_new(FILMSHM_KEY, sizeof(*film));
    if (film->touchtime == 0)
      film->touchtime = 1;
  }
  while (film->uptime < film->touchtime) 
  {
    count++;
    reload_filmcache();
    if(count > 3) 
    {
      film->busystate =0;
#if !defined(_BBS_UTIL_C_)
      log_usies("CACHE", "error:free filmcache dead lock!");
#endif
    }
  }
}
#endif
/*-------------------------------------------------------*/
/* FROM cache                                            */
/*-------------------------------------------------------*/
/* cachefor from host 與最多上線人數 */
struct FROMCACHE *fcache;
int fcache_semid;

reload_fcache()
{
   if (fcache->busystate)
   {
     /*sleep(1);*/
   }
   else
   {
     FILE *fp;
     fcache->busystate = 1;
     sem_init(FROMSEM_KEY,&fcache_semid);
     sem_lock(SEM_ENTER,fcache_semid);
     bzero(fcache->domain, sizeof fcache->domain);
     if(fp=fopen(BBSHOME"/etc/domain_name_query","r"))
     {
       char buf[256],*po;
       fcache->top=0;
       while (fgets(buf,256,fp))
       {
         if(buf[0] && buf[0] != '#' && buf[0] != ' ' && buf[0] != '\n')
         {
           sscanf(buf,"%s",fcache->domain[fcache->top]);
           po = buf + strlen(fcache->domain[fcache->top]);
           while(*po == ' ') po++;
           strncpy(fcache->replace[fcache->top],po,49);
           fcache->replace[fcache->top][strlen(fcache->replace[fcache->top])-1] = 0; 
           (fcache->top)++;
         }
       }
     }

     fcache->max_user=0;

     /* 等所有資料更新後再設定 uptime */
     fcache->uptime = fcache->touchtime;
#if !defined(_BBS_UTIL_C_)
     log_usies("CACHE", "reload fcache");
#endif
     sem_lock(SEM_LEAVE,fcache_semid);
     fcache->busystate = 0;
   }
}

void
resolve_fcache()
{
  if (fcache == NULL)
  {
    fcache = shm_new(FROMSHM_KEY, sizeof(*fcache));
    if (fcache->touchtime == 0)
      fcache->touchtime = 1;
  }
  while (fcache->uptime < fcache->touchtime) reload_fcache();
}

#if !defined(_BBS_UTIL_C_)
user_info* searchowner(userid)	/* alan.000422: owner online */
  char *userid;
{
  register int i, j;
  register user_info *uentp;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (!ci_strcmp(userid, uentp->userid) && str_cmp(userid, cuser.userid))
    {
      if ((uentp->invisible && !HAVE_PERM(PERM_SEECLOAK))
         || PERM_HIDE(uentp) && (!PERM_HIDE(currutmp) || !HAS_PERM(PERM_SYSOP))
         || (is_rejected(uentp) && !HAS_PERM(PERM_SYSOP)))
        return NULL;
      return uentp;
    }
  }
  return NULL;
}
#endif

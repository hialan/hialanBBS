/*-------------------------------------------------------*/
/* util/hfinger.c       ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : BBS finger daemon in HTML mode               */
/* create : 97/04/07                                     */
/* update : 97/04/08                                     */
/*-------------------------------------------------------*/
/* author : Harimau.bbs@MSIA.pine.ncu.edu.tw             */
/*-------------------------------------------------------*/
/* syntax : hfinger                                      */
/*-------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bbs.h"
#include "type.h"

#define field_count 5


#define WHITE   "#ffffff"
#define GRAY    "#c0c0c0"
#define BLACK   "#000000"

#define RED     "#a00000"
#define LRED    "#ff0000"

#define GREEN   "#009000"
#define LGREEN  "#00ff00"

#define BLUE    "#0000a0"
#define LBLUE   "#0000ff"

#define YELLOW  "#a0a000"
#define LYELLOW "#ffff00"

#define PURPLE  "#a000a0"
#define LPURPLE "#ff00ff"

#define CRAY    "#00a0a0"
#define LCRAY   "#00ffff"

/*-------------------------------------------------------*/
/* .UTMP cache                                           */
/*-------------------------------------------------------*/

struct UTMPFILE *utmpshm;

static void
attach_err(shmkey, name)
  int shmkey;
  char *name;
{
  fprintf(stderr, "[%s error] key = %x\n", name, shmkey);
  exit(1);
}

static void *
attach_shm(shmkey, shmsize)
  int shmkey, shmsize;
{
  void *shmptr;
  int shmid;

  shmid = shmget(shmkey, shmsize, 0);
  if (shmid < 0)
  {
    shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
    if (shmid < 0)
      attach_err(shmkey, "shmget");
    shmptr = (void *) shmat(shmid, NULL, 0);
    if (shmptr == (void *) -1)
      attach_err(shmkey, "shmat");
    memset(shmptr, 0, shmsize);
  }
  else
  {
    shmptr = (void *) shmat(shmid, NULL, 0);
    if (shmptr == (void *) -1)
      attach_err(shmkey, "shmat");
  }
  return shmptr;
}

void
resolve_utmp()
{
  if (utmpshm == NULL)
  {
    utmpshm = attach_shm(UTMPSHM_KEY, sizeof(*utmpshm));
    if (utmpshm->uptime == 0)
      utmpshm->uptime = utmpshm->number = 1;
  }
}

struct user_info *uentp;
char field_str[ field_count ][ 128 ];
int  field_lst_no [ field_count ];
int  user_num = 0, hide_num = 0;

int  field_lst_size [ field_count ] = {
   12, 20, 24, 15,  8
};

char *field_name[] = {
    "代號",
    "暱稱",
    "故鄉",
    "動態",
    "發呆",
    NULL
};

print_head()
{
    int i, size;

    printf("<center><TD bgcolor=%s>No.\n", LBLUE);
    for (i = 0; i < field_count; i++) {
        size  = field_lst_size[ i ];
        printf("<TD bgcolor=%s>%-*.*s ", BLUE, size, size, field_name[i] );
    }
    printf("<TR>\n");
}


print_record()
{
    int i, size;

    for (i = 0 ; i < field_count; i++) {
        size  = field_lst_size[ i ];
        if(i==0)
/*
           printf("<TD bgcolor=%s align=center>%-d<TD bgcolor=%s>"
        "<font size=4><A HREF=\"mailto:%s.bbs@%s\">%-*.*s</A></font>", CRAY,
       user_num+1, BLACK, field_str[i], MYHOSTNAME, size, size, field_str[i]);
       else printf("<TD bgcolor=%s>%-*.*s ", BLACK, size, size,
        field_str[i][0] ? field_str[i] : "<pre>");
*/
        printf("<TD bgcolor=%s align=center>%-d<td bgcolor=%s>"
        "<font size=4><A HREF=\"http:/~bbs/user/%s.html\">%-*.*s</A></font>\n",
        CRAY, user_num+1, GREEN,
        field_str[i], size, size, field_str[i]);

/*nekobe*/
       else if(i==1)
        printf("<TD><A HREF=mailto:%s.bbs@wd.pchome.com.tw>"
               "%s</A>",field_str[i-1],field_str[i]);

       else
        printf("<TD bgcolor=%s>%-*.*s ", BLACK, size, size,
        field_str[i][0] ? field_str[i] : "<pre>");

    }
    printf("<TR>\n");
}


char *
idle_str(tty)
  char *tty;
{
  static char hh_mm_ss[8];
  struct stat buf;

  if (stat(tty, &buf) || (strstr(tty, "tty") == NULL))
  {
    strcpy(hh_mm_ss, "Unknown");
  }
  else
  {
    time_t diff;
    diff = (time(0) - buf.st_atime);
    if (diff)
     sprintf(hh_mm_ss, "%2d:%02d\'%02d\"", diff / 3600, diff / 60 , diff % 60);
    else
     strcpy(hh_mm_ss, "<pre>");
  }
  return hh_mm_ss;
}


dump_record(serial_no, p)
int serial_no;
struct user_info *p;
{
    int i = 0;

    sprintf( field_str[i++], "%s", p->userid );
    sprintf( field_str[i++], "%s", p->username );
    sprintf( field_str[i++], "%s", p->from );
    sprintf( field_str[i++], "%s", ModeTypeTable[p->mode] );
    sprintf( field_str[i++], "%s", idle_str(p->tty) );
}

main()
{
  FILE *inf;
  int i;

  resolve_utmp();

printf("<HTML><HEAD><TITLE>WD BBS 站線上使用者列表</TITLE>"
        "</HEAD><BODY bgcolor=%s text=%s link=%s alink=%s vlink=%s>\n",
        BLACK, WHITE, WHITE, CRAY, GRAY);

printf("<CENTER>"
       "<img src=\"/image/title-b.jpg\" alt=\"風與塵埃的對話\" border=0 > BBS"
       "線上使用者\n");
printf("<BR><A href=\"http://wd.pchome.com.tw/~bbs/index.html\">回到主畫面</a>\n");


printf("<P><BR>\n");
printf("<TABLE border=1>");

    print_head();

    for (i = user_num = hide_num = 0; i < USHM_SIZE; i++)
    {
      uentp = &(utmpshm->uinfo[i]);
      if (uentp->userid[0]
          && !PERM_HIDE(uentp)
          && !uentp->invisible)
      {
        dump_record(i, uentp);
        print_record();
        user_num++;
      }
      else if (PERM_HIDE(uentp) || uentp->invisible)
        hide_num++;
    }

    printf("<TD COLSPAN=6 bgcolor=%s><CENTER>"
        "目前站上有 %d 人 , 隱形人 %d 人"
        "</CENTER></FONT><TR></TABLE>\n", BLUE, user_num+hide_num,hide_num);

    printf("<BR><A href=\"mailto:wildcat.bbs@%s\">",MYHOSTNAME);
    printf("留下您寶貴的意見給 wildcat</a><HR>");
    printf("<A align=center HREF=\"http://freebsd.csie.nctu.edu.tw\">"
        "<img src=\"/image/FreeBSD-logo.gif\" border=0>"
        "</a></CENTER></BODY></HTML>\n");
}

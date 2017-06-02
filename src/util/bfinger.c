/*-------------------------------------------------------*/
/* util/bfinger.c       ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : BBS finger daemon ¦C¥X¯¸¤º¨Ï¥ÎªÌ¸ê®Æ         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
/* syntax : bfinger i n f M (ID, nick, fromhost, mode )  */
/*-------------------------------------------------------*/


#define  SHOW_IDLE_TIME
#define  _MODES_C_


#include "bbs.h"
#include "type.h"
#include <sys/ipc.h>
#include <sys/shm.h>

char field_idx[] = "uftmMinx";
char *field_name[] = {
  "UID",
  "¬G¶m",
  "TTY",
  "mode",
  "ª¬ºA",
  "ID",
  "°ÎºÙ",
  "Idle",
  NULL
};


int in_bbs;
int field_count = 0;
int field_lst_no[8];
int field_lst_size[8];
int field_default_size[8] = {12, 16, 12, 4, 10, 12, 18, 8};


char *default_argv[] = {"bfinger", "i", "n", "f", "M"};


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


void
set_opt(argc, argv)
  int argc;
  char *argv[];
{
  int i, flag, field, size;
  int *p;
  char *ptr, *field_ptr;

  field_count = 0;

  for (i = 1; i < argc; i++)
  {
    field_ptr = (char *) strchr(field_idx, argv[i][0]);
    if (field_ptr)
    {
      field = field_ptr - field_idx;

      size = atoi(argv[i] + 1);

      field_lst_no[field_count] = field;
      field_lst_size[field_count] = size ? size : field_default_size[field];
      field_count++;
    }
  }
}


void
print_head()
{
  int i, field, size;

  for (i = 0; i < field_count; i++)
  {
    field = field_lst_no[i];
    size = field_lst_size[i];
    printf("[1;45;%dm%-*.*s[m ", i+31,size, size, field_name[field]);
  }
  printf("\n");
}


void
print_line()
{
  int i, size;

  for (i = 0; i < field_count; i++)
  {
    size = field_lst_size[i]+1;
    putchar('');
    putchar('[');
    putchar('4');
    putchar('5');
    putchar('m');
    while (size--)
      putchar(' ');
    putchar('');
    putchar('[');
    putchar('m');
  }
  printf("\n");
}


char *
idle_str(tty)
  char *tty;
{
  static char hh_mm_ss[8];
  struct stat buf;

  if (stat(tty, &buf) || (strstr(tty, "tty") == NULL))
  {
    strcpy(hh_mm_ss, "¤£¸Ô");
  }
  else
  {
    time_t diff;
    diff = (time(0) - buf.st_atime) / 60;
    sprintf(hh_mm_ss, "%d:%02d", diff / 60, diff % 60);
  }
  return hh_mm_ss;
}


void
print_record(p)
  user_info *p;
{
  int i, field, size;
  char field_str[32],*ptr;

  for (i = 0; i < field_count; i++)
  {
    field = field_lst_no[i];
    size = field_lst_size[i];
    switch (field)
    {
    case 0:
      sprintf(field_str, "%d", p->uid);
      break;
    case 1:
/* add by kennyh */
      ptr=field_str;
      if( strchr(ptr,'@') ){
        ptr=index(field_str,'@')+1;
      }
      strncpy(field_str,ptr,16);
/* end of kennyh */
      strcpy(field_str, p->from);
      break;
    case 2:
      strcpy(field_str, p->tty);
      break;
    case 3:
      sprintf(field_str, "%d", p->mode);
      break;
    case 4:
      strcpy(field_str, ModeTypeTable[p->mode]);
      break;
    case 5:
      strcpy(field_str, p->userid);
      break;
    case 6:
      strcpy(field_str, p->username);
      break;
    case 7:
      strcpy(field_str, in_bbs ? idle_str(p->tty) : "NA");
    }
    printf("%-*.*s ", size, size, field_str);
  }
  printf("\n");
}


void
usage(prog_name)
  char *prog_name;
{
  int i;

  printf("Usage: %s %s\n", prog_name, "[XN] ....");
  printf("Example: %s %s\n", prog_name, "d3 i12 e30");
  printf("N is field width, X is one of the following char :\n");

  for (i = 0; field_name[i]; i++)
  {
    printf("\t%c[%2d] --> %s\n",
      field_idx[i], field_default_size[i], field_name[i]);
  }
}


main(argc, argv)
  int argc;
  char *argv[];
{
  register user_info *uentp;
  register int i, user_num;
  char *c;

  in_bbs = (strstr(argv[0], "bfinger") == NULL) ? 0 : 1;
/* kennyh */
/*
  if(strchr(argv[2],'.') && argv[2] != ".bbs")
  {
    c = strtok(argv[2],".");

    execl(BBSHOME"/bin/query", "query", c, (char *)0);
    perror("execv");
    exit(-1);
  }
*/
  if (1)
  {
    set_opt(sizeof(default_argv) / sizeof(default_argv[0]), default_argv);
  }
  else
  {
    set_opt(argc, argv);
  }

  print_head();


  resolve_utmp();
  for (i = user_num = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (uentp->userid[0])
    {
      print_record(uentp);
      user_num++;
    }
  }

  print_line();
  printf("¡i" BOARDNAME "¡j Total users = %d\n", user_num);
  {
    char buf1[256],buf2[256];
    time_t now = time(0);
        
    sprintf(buf1,BBSHOME"/log/onlinenum.log");
    sprintf(buf2,"%d\t%d",now,user_num);
    f_cat(buf1,buf2);
  }
}

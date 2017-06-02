/*-------------------------------------------------------*/
/* util/post_open.c     ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : ¼Ö³z Show ¥X¤¤¼ú¦W³æ                         */
/* create : 95/03/29                                     */
/* update : 02/10/10 (ychia)                             */
/*-------------------------------------------------------*/
#include "bbs.h"

#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#include "record.c"

#define MAX_LINE        16
#define ADJUST_M        6       /* adjust back 5 minutes */

/* Ptt about share memory */
struct UCACHE *uidshm;
struct FILMCACHE *ptt;
struct FROMCACHE *fcache;

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
resolve_ucache()
{
  if (uidshm == NULL)
  {
    uidshm = attach_shm(UIDSHM_KEY, sizeof(*uidshm));
  }
}
void
reset_garbage()
{
  if (ptt == NULL)
  {
    ptt = attach_shm(FILMSHM_KEY, sizeof(*ptt));
    if (ptt->touchtime == 0)
      ptt->touchtime = 1;
  }
  ptt->uptime =0;
}

void
resolve_fcache()
{
  if (fcache == NULL)
  {
    fcache = attach_shm(FROMSHM_KEY, sizeof(*fcache));
    if (fcache->touchtime == 0)
      fcache->touchtime = 1;
  }
  fcache->uptime =0;
}

void
keeplog(fpath, board, title)
char *fpath;
char *board;
char *title;
{
  fileheader fhdr;
  char genbuf[256];
  char *flog;

  if(!board)
    board = "Record";

  sprintf(genbuf, "boards/%s", board);
  stampfile(genbuf, &fhdr);
  f_mv(fpath, genbuf);

  strcpy(fhdr.title, title);
  strcpy(fhdr.owner, "[¾ú¥vªº¤µ¤Ñ]");
  sprintf(genbuf, "boards/%s/.DIR", board);
  rec_add(genbuf, &fhdr, sizeof(fhdr));
}


void
outs(fp, buf, mode)
  FILE *fp;
  char buf[], mode;
{
  static char state = '0';

  if (state != mode)
    fprintf(fp, "[3%cm", state = mode);
  if (buf[0])
  {
    fprintf(fp, buf);
    buf[0] = 0;
  }
}


void
gzip(source, target, stamp)
  char *source, *target, *stamp;
{
  char buf[128];
  sprintf(buf, "/bin/gzip -9n adm/%s%s", target, stamp);
  f_mv(source, &buf[14]);
  system(buf);
}

main()
{
  int hour, max, item, total, i,
       j,mo,da,max_user=0,max_login=0,max_reg=0,mahour,k;
  char *act_file = ".act";
  char *log_file = "usies";
  char buf[256],buf1[256], *p;
  FILE *fp,*fp1;
  int act[27];                  /* ¦¸¼Æ/²Ö­p®É¶¡/pointer */
  time_t now;
  struct tm *ptime;
  chdir(BBSHOME);
  now = time(NULL) - ADJUST_M * 60;     /* back to ancent */
  ptime = localtime(&now);

/* §âlottoªº°O¿ý§Ë¤WLotto.ª© */
	
         keeplog("lotto/lotto_open.txt", "Lotto.", "[¶®°Ç¼Ö³z¯¸¶}¼ú] ¤¤¼ú¦W³æ");
	 keeplog("lotto/lotto_stat.txt", "Lotto.", "[¶®°Ç¼Ö³z¯¸¶}¼ú] ¶}¼ú²Î­p");
	 exit(0);
}

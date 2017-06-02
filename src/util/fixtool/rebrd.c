/*
SiE手記: 981110, pm 14:20,
meamea 投訴 file system full
於是ls -l 發現.BOARDS size = 0,
BOARDS size = 2xx mb, src目錄消失,無法修復
解決方式:
tar -zcf /tmp/brd.tgz ~/BOARDS 產生大小為2xxxxx byte之壓縮檔
並將BOARDS砍掉
此時/home/bbs已經變成75%..表示filesystem不再full
不過.BOARDS消失等到reloaad時會出問題
因此藉由此程式 將整個shm dump下來
*/

#include "bbs.h"
#include "cache.c"

extern struct BCACHE *brdshm;
extern boardheader *bcache;

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
//    if (shmid < 0)
//      attach_err(shmkey, "shmget");
    shmptr = (void *) shmat(shmid, NULL, 0);
//    if (shmptr == (void *) -1)
//      attach_err(shmkey, "shmat");
    memset(shmptr, 0, shmsize);
  }
  else
  {
    shmptr = (void *) shmat(shmid, NULL, 0);
//    if (shmptr == (void *) -1)
//      attach_err(shmkey, "shmat");
  }
  return shmptr;
}

void
attch_xbrd()
{
  if (brdshm == NULL)
  {
    brdshm = shm_new(BRDSHM_KEY, sizeof(*brdshm));
    if (brdshm->touchtime == 0)
      brdshm->touchtime = 1;
    bcache = brdshm->bcache;
  }
}

main()
{
  FILE *fp;
  boardheader *b, *e;

  attch_xbrd();

  b = bcache;
  e = b + brdshm->number;

  do
  {
    printf("%-20s<<%s>>\n", b->brdname, b->title);

  } while (++b < e); 


  if (fp = fopen(BBSHOME"/BOARDS.NEW", "w"))
  {
      fwrite( bcache, sizeof(boardheader), brdshm->number, fp);
  }
  fclose(fp);
}

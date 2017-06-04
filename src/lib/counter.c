#include "dao.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

#define SEM_ENTER      -1      /* enter semaphore */
#define SEM_LEAVE      1       /* leave semaphore */
#define SEM_RESET      0       /* reset semaphore */
#define C_SEMKEY	2222

/* ­p¼Æ¾¹ wildcat */

int counter_semid;

void
counter(filename,modes,n)
  char *filename;
  char *modes;
  int n;
{
  FILE *fp;
  unsigned long visited=0;
//  char buf[1024];

  sem_init(C_SEMKEY,&counter_semid);
  sem_lock(SEM_ENTER,counter_semid);
  if(fp = fopen(filename,"r"))
  {
    fscanf(fp,"%lu",&visited);
    fclose(fp);
  }

  printf(" [1;44m [33m  ±z¬O%s²Ä[36m %-10ld [33m¦ì %s ªº¨Ï¥ÎªÌ                            [m",
  n ? "¤µ¤Ñ" : "¦³¥v¥H¨Ó",++visited,modes);
  unlink(filename);

  if(fp = fopen(filename,"w"))
  {
    fprintf(fp,"%ld",visited);
    fclose(fp);
  }
  sem_lock(SEM_LEAVE,counter_semid);
}

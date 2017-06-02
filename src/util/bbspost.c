/*-------------------------------------------------------*/
/* util/bbsmail.c	( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : 由 Internet 寄信給 BBS 看板                  */
/* create : 95/03/29                                     */
/* update : 99/01/19                                     */
/*-------------------------------------------------------*/

#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <time.h>
#include <sysexits.h>

#include "bbs.h"

char *fn_board = BBSHOME "/.BOARDS";
extern boardheader *bcache;
extern int numboards;

#include "cache.c"
#include "record.c"

#define	LOG_FILE	(BBSHOME "/log/bbspost.log")

outgo_post(fh, board)
  fileheader *fh;
  char *board;
{
  char buf[256];
  sprintf(buf, "%s\t%s\t%s\t%s\t%s", board,
    fh->filename, fh->owner , "轉出", fh->title);
  f_cat(BBSHOME"/innd/out.bntp",buf);
}

void
postlog(msg)
  char *msg;
{
  FILE *fp;

  if (fp = fopen(LOG_FILE, "a"))
  {
    time_t now;
    struct tm *p;

    time(&now);
    p = localtime(&now);
    fprintf(fp, "%02d/%02d/%02d %02d:%02d:%02d <bbspost> %s\n",
	p->tm_year % 100, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
	msg);
    fclose(fp);
  }
}

int
search_bid(bid)
  char *bid;
{
  int i;
  boardheader *bh;

  resolve_boards();
  for (i = 0; i < numboards; i++)
  {
    bh = &bcache[i];
    if(!strcasecmp(bh->brdname,bid)) 
      return ++i;
  }
  return 0;
}

int
post2bbs(bid)
  char *bid;
{
  fileheader mypost;
  boardheader bh;
  char genbuf[256], title[80], sender[80], *ip, *ptr,buf[80];
  time_t tmp_time;
  struct stat st;
  FILE *fout;
  int bnum;

  resolve_boards();
  
  if(!(bnum = search_bid(bid)))
  {
    sprintf(buf,"No Such board [%s]",bid);
    postlog(buf);
    return -1;
  }

  rec_get(fn_board, &bh, sizeof(boardheader), bnum);

  if(bh.level > PERM_LOGINOK || bh.brdattr & BRD_HIDE)
  {
    sprintf(buf,"Permission denied - [%s]",bh.brdname);
    postlog(buf);
    return -1;
  }

  sprintf(genbuf, BBSHOME "/boards/%s", bh.brdname);
  printf("dir: %s\n", genbuf);

  /* allocate a file for the new mail */

  stampfile(genbuf, &mypost);
  printf("file: %s\n", genbuf);

  /* copy the stdin to the specified file */

  if ((fout = fopen(genbuf, "w")) == NULL)
  {
    printf("Cannot open %s\n", genbuf);
    return -1;
  }

  /* parse header */

  while (fgets(genbuf, 255, stdin))
  {
    if (!strncmp(genbuf, "From", 4))
    {
      if ((ip = strchr(genbuf, '<')) && (ptr = strrchr(ip, '>')))
      {
	*ptr = '\0';
	*ip = '\0';
	if (strchr(++ip, '@'))
	  *ptr = '\0';
	else                                    /* 由 local host 寄信 */
	  strcpy(ptr, "@" MYHOSTNAME);
	                            
	ptr = (char *) strchr(genbuf, ' ');
	while (*++ptr == ' ');
	sprintf(sender, "%s (%s)", ip, ptr);
      }
      else
      {
	strtok(genbuf, " \t\n\r");
	strcpy(sender, (char *) strtok(NULL, " \t\n\r"));
      }
      continue;
    }
    if (!strncmp(genbuf, "Subject: ", 9))
    {
      strcpy(title, genbuf + 9);
      continue;
    }
    if (genbuf[0] == '\n')
      break;
  }

  if (ptr = strchr(sender, '\n'))
    *ptr = '\0';

  if (ptr = strchr(title, '\n'))
    *ptr = '\0';

  if (strchr(sender, '@') == NULL)	/* 由 local host 寄信 */
  {
    strcat(sender, "@" MYHOSTNAME);
  }

  time(&tmp_time);

  if (!title[0])
    sprintf(title, "來自 %.64s", sender);

  str_decode(title);
  fprintf(fout, "作者: %s 看板: %s\n標題: %s\n時間: %s\n",
    sender, bh.brdname, title, ctime(&tmp_time));

  while (fgets(genbuf, 255, stdin))
  {
    // wildcat : decode 一下,免得都是亂碼
    str_decode(genbuf);
    strcat(genbuf,"\n");
    fputs(genbuf, fout);
  }

  fclose(fout);

  sprintf(genbuf, "%s => %s", sender, bh.brdname);
  postlog(genbuf);

  /* append the record to the MAIL control file */

  strncpy(mypost.title, title, 72);

  if (strtok(sender, " .@\t\n\r"))
    strcat(sender, ".");
  sender[IDLEN + 1] = '\0';
  strcpy(mypost.owner, sender);

  if (!(bh.brdattr & BRD_NOTRAN))
    outgo_post(&mypost,bh.brdname);
  sprintf(genbuf, BBSHOME "/boards/%s/.DIR", bh.brdname);
  return rec_add(genbuf, &mypost, sizeof(mypost));
}


int 
main(argc, argv)
  int argc;
  char *argv[];
{
  char receiver[256];

  /* argv[1] is userid in bbs   */

  if (argc < 2)
  {
    printf("Usage:\t%s <bbs_bid>\n", argv[0]);
    exit(-1);
  }

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  strcpy(receiver, argv[1]);

  post2bbs(receiver);


/* eat mail queue ??*/
/*
  if (post2bbs(receiver))
  {
    while (fgets(receiver, sizeof(receiver), stdin));
  }
*/
  exit(0);
}

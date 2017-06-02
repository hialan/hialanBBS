/*-------------------------------------------------------*/
/* util/bm_check.c      ( WD-BBS 2.2.5 )		 */
/*-------------------------------------------------------*/
/* target : ªO¥D login check				 */
/* create : 99/11/06                                     */
/* update : 99/11/06                                     */
/*-------------------------------------------------------*/

#define HAS_P(x) ((x)?aman.userlevel&(x):1)

#include "bbs.h"
#include "cache.c"
#include "record.c"

#define REAL_INFO

extern int numboards;
extern boardheader *bcache;

struct manrec
{
  char userid[IDLEN+1];
  char username[23];
  char brdname[IDLEN+1];
  char email[50];
  time_t lastlogin;
};

typedef struct manrec manrec;
struct manrec allman[MAXUSERS];

userec aman;
int num;
FILE *fp;

int
is_BM(userid, list)
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
belong(filelist, key)
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
      if ((ptr = strtok(buf, " \t\n\r")) && !strcasecmp(ptr, key))
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
time_cmp(b, a)
  struct manrec *a, *b;
{
  return (b->lastlogin - a->lastlogin);
}


void
top()
{
  int i,rows = num;
  int thday,twday,wday,day,other;
  char buf[256];
  thday = twday = wday = day = other = 0;

  fprintf(fp, "¦W¦¸  ID               «e¦¸¤W¯¸                    ¾á¥ôªO¥D
========================================================================");
  for (i = 0; i < rows; i++)
  {
    if(!allman[i].userid[0]) break;
    sprintf(buf, "[%3d] %-14.14s  %-22.22s    %-16.16s",
      i + 1, allman[i].userid,Ctime(&allman[i].lastlogin), allman[i].brdname);
    if (time(0) -allman[i].lastlogin > 86400 * 30)
    { fprintf(fp, "\n [1;31m%s[m", buf); thday++;}
    else if (time(0) -allman[i].lastlogin > 86400 * 14)
    { fprintf(fp, "\n [1;35m%s[m", buf); twday++;}
    else if (time(0) -allman[i].lastlogin > 86400 * 7)
    { fprintf(fp, "\n [1;33m%s[m", buf); wday++;}
    else if (time(0) -allman[i].lastlogin > 86400 * 1)
    { fprintf(fp, "\n [1;32m%s[m", buf); day++;}
    else
    { fprintf(fp, "\n [1;36m%s[m", buf); other++;}
  }
  fprintf(fp,"\n\n
¶W¹L 30 ¤Ñ¨S¨Óªº¦³ %d ¤H
¶W¹L 14 ¤Ñ¨S¨Óªº¦³ %d ¤H
¶W¹L  7 ¤Ñ¨S¨Óªº¦³ %d ¤H
¶W¹L  1 ¤Ñ¨S¨Óªº¦³ %d ¤H

1 ¤Ñ¤º¦³¨Óªº¦³ %d ¤H",thday,twday,wday,day,other);
}

int
not_alpha(ch)
  register char ch;
{
  return (ch < 'A' || (ch > 'Z' && ch < 'a') || ch > 'z');
}

int
not_alnum(ch)
  register char ch;
{
  return (ch < '0' || (ch > '9' && ch < 'A') ||
    (ch > 'Z' && ch < 'a') || ch > 'z');
}

int
bad_user_id(userid)
  char *userid;
{
  register char ch;
  if (strlen(userid) < 2)
    return 1;
  if (not_alpha(*userid))
    return 1;
  while (ch = *(++userid))
  {
    if (not_alnum(ch))
      return 1;
  }
  return 0;
}

main(argc, argv)
  int argc;
  char **argv;
{
  FILE *inf,*inf2;
  int i;

  num = MAXBOARD;

  inf = fopen(BBSHOME "/.PASSWDS", "rb");
  inf2 = fopen(BBSHOME "/.BOARDS", "rb");

  if (inf == NULL || inf2 == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    exit(0);
  }

  for (i = 0; fread(&aman, sizeof(userec) , 1, inf); i++)
  {

    if(belong("etc/nontop",aman.userid) || bad_user_id(aman.userid)
       || strchr(aman.userid,'.'))
    {
        i--;
    }
   else if(!HAS_P(PERM_BM) || HAS_P(PERM_SYSOP)) i--;
   else
    {
       int j=0;
       boardheader *bh;
       resolve_boards();

       strcpy(allman[i].userid, aman.userid);
       strncpy(allman[i].username, aman.username,23);
       strcpy(allman[i].email, aman.email);
       allman[i].lastlogin = aman.lastlogin;
       for (j = 0, bh = bcache; j < numboards; j++, bh++)
       {
        if(is_BM(aman.userid, bh->BM))
          strcpy(allman[i].brdname,bh->brdname);
       }
    }
  }

  if ((fp = fopen(BBSHOME"/log/bm_check", "w")) == NULL)
  {
    printf("cann't open bm_check\n");
    return 0;
  }

  qsort(allman, i, sizeof(manrec), time_cmp);
  top();

  fclose(inf);
  fclose(inf2);
  fclose(fp);
}

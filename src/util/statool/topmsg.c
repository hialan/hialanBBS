/*-------------------------------------------------------*/
/* util/topmsg.c          ( NTHU CS MapleBBS Ver 2.36 )  */
/*-------------------------------------------------------*/
/* target : ¨Ï¥ÎªÌ ¤ô²y ±Æ¦æº]                           */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"
#define REAL_INFO

struct manrec
{
  char userid[IDLEN+1];
  char username[23];
  usint receivemsg;
  usint sendmsg;
};
typedef struct manrec manrec;
struct manrec allman[MAXUSERS];

userec aman;
int num;
FILE *fp;

#define TYPE_RECE       0
#define TYPE_SEND       1

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
rece_cmp(b, a)
  struct manrec *a, *b;
{
  return (a->receivemsg - b->receivemsg);
}


int
send_cmp(b, a)
  struct manrec *a, *b;
{
  return (a->sendmsg - b->sendmsg);
}

void
top(type)
{
  static char *str_type[3] = {" ¦¬°T®§ ", " µo°T®§ "};
  int i, j, rows = (num + 1) / 2;
  char buf1[80], buf2[80];

if(type != 0)
 fprintf(fp,"\n\n");

  fprintf(fp, "\
[1;33m¡³¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢s¡÷ [%dm    %8.8s±Æ¦æº]     [33;40m ¡ö¢s¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¡³[m\n\
[1;37m¦W¦¸[33m¢w[37m¥N¸¹[33m¢w¢w¢w[37m¼ÊºÙ[33m¢w¢r¢w¢w¢w¢w¢w[37m¼Æ¥Ø   ¦W¦¸[33m¢w[37m¥N¸¹[33m¢w¢r¢w[37m¼ÊºÙ[33m¢w¢w¢w¢w¢w¢w[37m¼Æ¥Ø[m\
", (type*type) + 41, str_type[type]);
  for (i = 0; i < rows; i++)
  {
    sprintf(buf1, "[%2d] %-11.11s%-16.16s%5d",
      i + 1, allman[i].userid, allman[i].username,
      (type == 0 ? allman[i].receivemsg :
       allman[i].sendmsg)); 
    j = i + rows;
    sprintf(buf2, "[%2d] %-11.11s%-16.16s%4d",
      j + 1, allman[j].userid, allman[j].username,
      (type == 0 ? allman[j].receivemsg :
       allman[j].sendmsg)); 
    if (i < 3)
      fprintf(fp, "\n [1;%dm%-40s[0;37m%s", 31 + i, buf1, buf2);
    else
      fprintf(fp, "\n %-40s%s", buf1, buf2);
  }
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
  FILE *inf;
  int i;

  if (argc < 3)
  {
    printf("Usage: %s <num_top> <out-file>\n", argv[0]);
    exit(1);
  }

  num = atoi(argv[1]);
  if (num == 0)
    num = 30;

  inf = fopen(BBSHOME "/.PASSWDS", "rb");

  if (inf == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    exit(0);
  }

  for (i = 0; fread(&aman, sizeof(userec), 1, inf); i++)
  {
    if(belong("etc/nontop",aman.userid) || bad_user_id(aman.userid)
       || strchr(aman.userid,'.'))
    {
        i--;
    }
   else
    {
     strcpy(allman[i].userid, aman.userid); 
     strncpy(allman[i].username,aman.username,23); 
     allman[i].receivemsg = aman.receivemsg; 
     allman[i].sendmsg = aman.sendmsg; 
    }
  }

  if ((fp = fopen(argv[2], "w")) == NULL)
  {
    printf("cann't open topmsg\n");
    return 0;
  }

  qsort(allman, i, sizeof(manrec), rece_cmp);
  top(TYPE_RECE);

  qsort(allman, i, sizeof(manrec), send_cmp);
  top(TYPE_SEND);

  fclose(inf);
  fclose(fp);
}

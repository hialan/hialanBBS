/*-------------------------------------------------------*/
/* util/topusr2.c        ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : ¨Ï¥ÎªÌ ¤W¯¸°O¿ý/¤å³¹½g¼Æ ±Æ¦æº]              */
/* create : 97/03/27                                     */
/* update : 95/03/31                                     */
/*-------------------------------------------------------*/

#undef MONTH
#define REAL_INFO
#undef HIDE
#define ADMIN_REPORT

#include <time.h>
#include <stdio.h>
#include "bbs.h"
#include "config.h"
#include "record.c"
int mannum;
char *fn_board=".BOARDS";
char *fn_passwd=".PASSWDS";

struct binfo
{
  char  boardname[18];
  char  expname[28];
  int times;
  int sum;
  int post;
  int num;
  int del;
  int pur;
  int tag;
  usint attr;
} st[MAXBOARD];

int numboards=0;
int numgroups=0;
int numhide=0;
int numhideg=0;

int
apply_record(fpath, fptr, size)
  char *fpath;
  int (*fptr) ();
int size;
{
  char abuf[512];
  FILE* fp;

  if (!(fp = fopen(fpath, "r")))
    return -1;

  while (fread(abuf, 1, size, fp) == size)
     if ((*fptr) (abuf) == QUIT) {
        fclose(fp);
        return QUIT;
     }
  fclose(fp);
  return 0;
}

int
brd_cmp(b, a)
struct binfo *a, *b;
{
    if(a->sum!=b->sum)
            return (a->sum - b->sum);
    return a->times - b->times;
}

int
personal_cmp(b, a)
struct binfo *a, *b;
{
    return (((a->post*500)+(a->times*100)+(a->sum/3)) - ((b->post*500)+(b->times*100)+(b->sum/3)));
}

/*
char    *Ctime(date)
time_t  *date;
{
        static char buf[80];

        strcpy(buf, (char *)ctime(date));
        buf[strlen(buf)-1] = '\0';
        return buf;
}
*/

int
record_data(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].times++;
                        st[i].sum+=sec;
                        return;
                }
        }
        return ;
}

int
record_data2(board)
char *board;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].post++;
                        return;
                }
        }
        return ;
}

int
record_data3(board)
char *board;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].del++;
                        st[i].num++;
                        return;
                }
        }
        return ;
}

int
record_data4(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].pur++;
                        st[i].num+=sec;
                        return;
                }
        }
        return ;
}

int
record_data5(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].tag++;
                        st[i].num+=sec;
                        return;
                }
        }
        return ;
}






int
fillbcache(fptr)
    struct boardheader *fptr ;
{

    if( numboards >= MAXBOARD )
        return 0;

    /* Gene */
#ifdef HIDE
    if((fptr->level != 0) && !(fptr->level & PERM_POSTMASK))
        return;
#endif
    if((fptr->level != 0) && !(fptr->level & PERM_POSTMASK))
        numhide++;
    if ((strstr(fptr->title, "£["))||(strstr(fptr->title, "£U"))||
         (strstr(fptr->title, "£S")))
    {
    numgroups++;
    if((fptr->level != 0) && !(fptr->level & PERM_POSTMASK))
        numhideg++;
    return;}

    strcpy(st[numboards].boardname,fptr->brdname);
    strcpy(st[numboards].expname,fptr->title);
/*    printf("%s %s\n",st[numboards].boardname,st[numboards].expname); */
    st[numboards].times=0;
    st[numboards].sum=0;
    st[numboards].post=0;
    st[numboards].del=0;
    st[numboards].pur=0;
    st[numboards].num=0;
    st[numboards].tag=0;
    st[numboards].attr=fptr->brdattr;

    numboards++;
    return 0 ;
}

int
fillboard()
{
  apply_record(BBSHOME"/.BOARDS", fillbcache, sizeof(boardheader));
}

/*
char *
timetostr(i)
int i;
{
        static char str[30];
        int minute,sec,hour;

        minute=(i/60);
        hour=minute/60;
        minute=minute%60;
        sec=i&60;
        sprintf(str,"%2d:%2d\'%2d\"",hour,minute,sec);
        return str;
}
*/

struct manrec
{
  char userid[IDLEN+1];
  char username[23];
  char userid2[IDLEN+1];
  usint userlevel;
  usint userlevel2;
  ushort numlogins;
  ushort numposts;
  ushort numloginsto;
  ushort numpoststo;
  ushort numloginsyes;
  ushort numpostsyes;
  ushort messto;
  ushort messfrom;
};
typedef struct manrec manrec;
struct manrec allman[MAXUSERS];

userec aman;
int num;
FILE *fp,*fp1,*fp2,*fp3;



int
record_mess(name,type)
char *name;
int type;
{
  int i;
  int n;

  for(i=0;i<mannum;i++)
  {
    if (!strcmp(name, allman[i].userid))
    {
      if (type == 0) allman[i].messfrom++;
      else allman[i].messto++;
      return;
    }
  }
  return ;
}


main(argc, argv)
  int argc;
  char **argv;
{
  FILE *inf3,*inf2,*inf,*fpf;
  int i,n;
  int numlog=0, numlog2=0, numpo=0, numpo2=0;
  int maxlog=0, maxlog2=0, maxpo=0, maxpo2=0;
  char maxlogid[IDLEN+1], maxpoid[IDLEN+1], maxlogid2[IDLEN+1], maxpoid2[IDLEN+1];
  int userlog=0, userlog2=0, userpo=0, userpo2=0;
  time_t now = time(0);
  struct tm *ptime;
  char *progmode;
/*  FILE *op; */
  char buf[256], *p,bname[20];
/*  char date[80];
  int mode;
  int c[3]; */
  int max[3];
  unsigned int ave[3];
  int sec;
  int j,k;
  char timesbname[20];
  char sumbname[20];
  char uname_from[20];
  char uname_to[20];
  int messnum;
  int max_from=0;
  int max_to=0;
  int user_from=0;
  char setby[13];
  char setto[13];
  int act[27];                  /* ¦¸¼Æ/²Ö­p®É¶¡/pointer */
  int user_to=0;
  int hour;
  int newreg=0;
  int numtalk=0, numchat=0, numnewb=0, numnameb=0, numdelb=0,numattrb=0,numprefix=0,numboardlog=0;
  int numdated=0, numclean=0, numsetb=0, numkill=0, numsuci=0;
  int numsetu=0, numsetself=0, numcdict=0, numfortune=0, numrailway=0;
// wildcat add
  int nummsgmenu=0, numbet=0, numfive=0, numgamble=0, nummine=0, numbbcall=0;
  int nummn=0, numpedit=0, numpcall=0, numpread=0, numdragon=0;
  int numrpgchoose=0, numrpguild=0, numrpgtop=0, numrpgtrain=0, numrpgset=0;
  int numrpgpk=0, numosong=0, numcatv=0, numvote=0, numvotedit=0;
  int numvotemake=0, numvbreply=0, numvbmake=0, numhint=0, numtetris=0;
  int nummj=0, numbig2=0, numchess=0, numbbsnet=0, numsetbm=0, numsetbp=0;
  int numspam=0, numxaxb=0, numchicken=0, numbj=0, numstock=0;
  int numdice=0, numgp=0, nummarie=0, numrace=0, numbingo=0;
  int numnine=0, numnfight=0, numchessmj=0, numsevencard=0;
  int num;
  int alltime=0;
  int alltimes=0;
  int allpost=0;
  int allnum=0;
  int alldel=0;
  int allpur=0;
  int alltag=0;
  int maxtoday=0;
  int numsysop=0;
  int numboard=0;
  int numaccount=0;
  int numchatroom=0;
  int numbm=0;
  int numsee=0;
  int numcloak=0;
  int numloginok=0;
  int guestnum=0;

  setuid(BBSUID);
  setgid(BBSGID);
  chdir(BBSHOME);

#ifdef MONTH
  if ((fp1 = fopen(BBSHOME "/adm/board.read", "r")) == NULL)
#else
  if ((fp1 = fopen(BBSHOME"/usboard", "r")) == NULL)
#endif
  {
    printf("cann't open usboard\n");
    /* return 1 */;
  }

  fillboard();
  while (fgets(buf, 512, fp1))
  {
    if ( !strncmp(buf, "USE", 3))
    {
      p=strstr(buf,"USE");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+25, "Stay: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data(bname,sec);
    }
    if ( !strncmp(buf, "DEL", 3))
    {
      p=strstr(buf,"DEL");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    record_data3(bname);
    }

    if ( !strncmp(buf, "PUR", 3))
    {
      p=strstr(buf,"PUR");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+25, "with: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data4(bname,sec);
    }
    if ( !strncmp(buf, "TAG", 3))
    {
      p=strstr(buf,"TAG");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+25, "with: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data5(bname,sec);
    }


    if ( !strncmp(buf, "POS", 3))
    {
      p=strstr(buf,"POS");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
      record_data2(bname);
    }


   }
   /* qsort */
   ave[0]=0;
   ave[1]=0;
   ave[2]=0;
   max[1]=0;
   max[0]=0;
   max[2]=0;
   for(i=0;i<numboards;i++)
   {
        ave[0]+=st[i].times;
        ave[1]+=st[i].sum;
        ave[2]+=st[i].times==0?0:st[i].sum/st[i].times;
        if(max[0]<st[i].times)
        {
                max[0]=st[i].times;
                strcpy(timesbname, st[i].boardname);
        }
        if(max[1]<st[i].sum)
        {
                max[1]=st[i].sum;
                strcpy(sumbname, st[i].boardname);
        }
        if(max[2]<(st[i].times==0?0:st[i].sum/st[i].times))
        {
                max[2]=(st[i].times==0?0:st[i].sum/st[i].times);
        }
        alltime+=st[i].sum;
        alltimes+=st[i].times;
        alldel+=st[i].del;
        allpur+=st[i].pur;
        alltag+=st[i].tag;
        allpost+=st[i].post;
        allnum+=st[i].num;
   }
   numboards++;
   st[numboards-1].times=ave[0]/numboards;
   st[numboards-1].sum=ave[1]/numboards;
   strcpy(st[numboards-1].boardname,"Total");
   strcpy(st[numboards-1].expname,"Á`¦X");
   qsort(st, numboards, sizeof( st[0] ), brd_cmp);


  now = time(NULL);
  ptime = localtime(&now);
  fclose(fp1);

#ifdef MONTH
  inf = fopen(BBSHOME "/.PASSWDS.yes", "rb");
#else
  inf = fopen(BBSHOME "/.PASSWDS", "rb");
#endif

  if (inf == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    /* exit(0) */;
  }

  for (i = 0; fread(&aman, sizeof(userec), 1, inf); i++)
  {
    strcpy(allman[i].userid, aman.userid);
    strncpy(allman[i].username, aman.username,23);
    allman[i].numloginsto = aman.numlogins;
    allman[i].numpoststo = aman.numposts;
    allman[i].userlevel = aman.userlevel;

#ifdef  HAVE_TIN
    allman[i].numposts += post_in_tin(allman[i].userid);
#endif
  }
  fclose(inf);

#ifdef MONTH
  inf2 = fopen(BBSHOME "/.PASSWDS.month", "rb");
#else
  inf2 = fopen(BBSHOME "/.PASSWDS.yes", "rb");
#endif
  if (inf2 == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    /* exit(0) */;
  }

  for (i = 0; fread(&aman, sizeof(userec), 1, inf2); i++)
  {
    strcpy(allman[i].userid2, aman.userid);
    allman[i].numloginsyes = aman.numlogins;
    allman[i].numpostsyes = aman.numposts;
    allman[i].userlevel2 = aman.userlevel;

#ifdef  HAVE_TIN
    allman[i].numposts += post_in_tin(allman[i].userid);
#endif
  }

  n=i-1;
  mannum=n;

  for (i = 0; i<=n; i++)
  {
    if (!strcmp(allman[i].userid, allman[i].userid2))
    {
      allman[i].numlogins = allman[i].numloginsto - allman[i].numloginsyes;
      allman[i].numposts = allman[i].numpoststo - allman[i].numpostsyes;
    }
    else
    {
      allman[i].numlogins = allman[i].numloginsto;
      allman[i].numposts = allman[i].numpoststo;
    }
    if (allman[i].numpoststo < allman[i].numpostsyes)
      allman[i].numposts = 0;
    if (allman[i].numloginsto < allman[i].numloginsyes)
      allman[i].numlogins = 0;
  }
  fclose(inf2);

#ifdef MONTH
    if ((fpf = fopen(BBSHOME "/adm/usies", "r")) == NULL)
#else
    if ((fpf = fopen(BBSHOME "/usies", "r")) == NULL)
#endif
  {
    printf("cann't open usies\n");
    /* return 1 */;
  }

    while (fgets(buf, 512, fpf))
  {
    hour = atoi(buf + 9);
    if (hour < 0 || hour > 23)
    {
      continue;
    }
    if (!(strncmp(buf +28 , "guest ", 6)))
      {
      guestnum++;
      }

    if (!(strncmp(buf +22, "APPLY", 5)))
      {
      newreg++;
      continue;
      }
    else if (!(strncmp(buf +22, "DATED", 5)))
      {
      numdated++;
      continue;
      }
    else if (!(strncmp(buf +22, "CLEAN", 5)))
      {
      numclean++;
      continue;
      }
    else if (!(strncmp(buf +22, "SUCI", 4)))
      {
      numsuci++;
      continue;
      }
    else if (!(strncmp(buf +22, "KILL", 4)))
      {
      numkill++;
      continue;
      }
    else if (!(strncmp(buf +22, "NewBoard", 8)))
      {
      numnewb++;
      continue;
      }
    else if (!(strncmp(buf +22, "DelBoard", 8)))
      {
      numdelb++;
      continue;
      }
    else if (!(strncmp(buf +22, "SetBoard", 8)))
      {
      numsetb++;
      continue;
      }
    else if (!(strncmp(buf +22, "NameBoard", 9)))
      {
      numnameb++;
      continue;
      }
    else if (!(strncmp(buf +22, "ATTR_Board", 10)))
      {
      numattrb++;
      continue;
      }
    else if (!(strncmp(buf +22, "PREFIX", 6)))
      {
      numprefix++;
      continue;
      }
    else if (!(strncmp(buf +22, "BOARDLOG", 8)))
      {
      numboardlog++;
      continue;
      }

    else if (!(strncmp(buf +22, "CHAT ", 5)))
      {
      numchat++;
      continue;
      }
    else if (!(strncmp(buf +22, "TALK ", 5)))
      {
      numtalk++;
      continue;
      }
    else if (!(strncmp(buf +22, "FORTUNE", 7)))
      {
      numfortune++;
      continue;
      }
    else if (!(strncmp(buf +22, "RAILWAY", 7)))
      {
      numrailway++;
      continue;
      }
    else if (!(strncmp(buf +22, "CDICT", 5)))
      {
      numcdict++;
      continue;
      }
    else if (!(strncmp(buf +22, "BBCALL", 6)))
      {
      numbbcall++;
      continue;
      }
    else if (!(strncmp(buf +22, "MSGMENU", 7)))
      {
      nummsgmenu++;
      continue;
      }
    else if (!(strncmp(buf +22, "BET", 3)))
      {
      numbet++;
      continue;
      }
    else if (!(strncmp(buf +22, "FIVE", 4)))
      {
      numfive++;
      continue;
      }
    else if (!(strncmp(buf +22, "GAMBLE", 6)))
      {
      numgamble++;
      continue;
      }
    else if (!(strncmp(buf +22, "MINE", 4)))
      {
      nummine++;
      continue;
      }
    else if (!(strncmp(buf +22, "MN", 2)))
      {
      nummn++;
      continue;
      }
    else if (!(strncmp(buf +22, "PCALL", 5)))
      {
      numpcall++;
      continue;
      }
    else if (!(strncmp(buf +22, "PREAD", 5)))
      {
      numpread++;
      continue;
      }
    else if (!(strncmp(buf +22, "DRAGON", 6)))
      {
      numdragon++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_Choose", 10)))
      {
      numrpgchoose++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_Guild", 9)))
      {
      numrpguild++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_Toplist", 11)))
      {
      numrpgtop++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_Train", 9)))
      {
      numrpgtrain++;
      continue;
      }
    else if (!(strncmp(buf +22, "SetRPG", 6)))
      {
      numrpgset++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_PK", 6)))
      {
      numrpgpk++;
      continue;
      }
    else if (!(strncmp(buf +22, "OSONG", 5)))
      {
      numosong++;
      continue;
      }
    else if (!(strncmp(buf +22, "CATV", 4)))
      {
      numcatv++;
      continue;
      }
    else if (!(strncmp(buf +22, "VOTE", 4)))
      {
      numvote++;
      continue;
      }
    else if (!(strncmp(buf +22, "VOTE_Edit", 9)))
      {
      numvotedit++;
      continue;
      }
    else if (!(strncmp(buf +22, "VOTE_Make", 9)))
      {
      numvotemake++;
      continue;
      }
    else if (!(strncmp(buf +22, "VB_Reply", 8)))
      {
      numvbreply++;
      continue;
      }
    else if (!(strncmp(buf +22, "VB_Make", 7)))
      {
      numvbmake++;
      continue;
      }
    else if (!(strncmp(buf +22, "HINT", 4)))
      {
      numhint++;
      continue;
      }
    else if (!(strncmp(buf +22, "TETRIS", 6)))
      {
      numtetris++;
      continue;
      }
    else if (!(strncmp(buf +22, "MJ", 2)))
      {
      nummj++;
      continue;
      }
    else if (!(strncmp(buf +22, "BIG2", 4)))
      {
      numbig2++;
      continue;
      }
    else if (!(strncmp(buf +22, "CHESS", 5)))
      {
      numchess++;
      continue;
      }
    else if (!(strncmp(buf +22, "BBSNET", 6)))
      {
      numbbsnet++;
      continue;
      }
    else if (!(strncmp(buf +22, "SetBoardBM", 10)))
      {
      numsetbm++;
      continue;
      }
    else if (!(strncmp(buf +22, "SetBrdPass", 10)))
      {
      numsetbp++;
      continue;
      }
    else if (!(strncmp(buf +22, "SPAM ", 5)))
      {
      numspam++;
      continue;
      }
    else if (!(strncmp(buf +22, "XAXB", 4)))
      {
      numxaxb++;
      continue;
      }
    else if (!(strncmp(buf +22, "CHICKEN", 7)))
      {
      numchicken++;
      continue;
      }
    else if (!(strncmp(buf +22, "BJ", 2)))
      {
      numbj++;
      continue;
      }
    else if (!(strncmp(buf +22, "STOCK", 5)))
      {
      numstock++;
      continue;
      }
    else if (!(strncmp(buf +22, "DICE", 4)))
      {
      numdice++;
      continue;
      }
    else if (!(strncmp(buf +22, "GP", 2)))
      {
      numgp++;
      continue;
      }
    else if (!(strncmp(buf +22, "MARIE", 5)))
      {
      nummarie++;
      continue;
      }
    else if (!(strncmp(buf +22, "RACE", 4)))
      {
      numrace++;
      continue;
      }
    else if (!(strncmp(buf +22, "BINGO", 5)))
      {
      numbingo++;
      continue;
      }
    else if (!(strncmp(buf +22, "NINE", 4)))
      {
      numnine++;
      continue;
      }
    else if (!(strncmp(buf +22, "NumFight", 8)))
      {
      numnfight++;
      continue;
      }
    else if (!(strncmp(buf +22, "CHESSMJ", 7)))
      {
      numchessmj++;
      continue;
      }
    else if (!(strncmp(buf +22, "SEVENCARD", 9)))
      {
      numsevencard++;
      continue;
      }
    else if (!strncmp(buf + 22, "ENTER", 5))
    {
      act[hour]++;
      continue;
    }
    else if (!strncmp(buf + 22, "SetUser", 7))
    {
      p=strstr(buf,"SetUser");
      p+=8;
      p=strtok(p," ");
      strcpy(setby,p);
      p=strstr(buf, setby);
      p+=13;
      p=strtok(p," ");
      if (strstr(p,"\n"))
          p=strtok(p,"\n");
      strcpy(setto,p);
      if (strcmp(setto, setby))
        numsetu++;
      else
        numsetself++;
    }
    if (p = (char *) strstr(buf + 40, "Stay:"))
    {
      if (hour = atoi(p + 5))
      {
        act[24] += hour;
        act[25]++;
      }
      continue;
    }
  }
  fclose(fpf);

  if(fpf = fopen(BBSHOME"/.maxtoday", "r"))
  {
    fscanf(fpf, "%d", &maxtoday);
    fclose(fpf);
  }

  if ((fp = fopen(BBSHOME "/log/admin.log", "w")) == NULL)
  {
    printf("cann't open admin.log\n");
    /* return 0*/;
  }
  if ((fp1 = fopen(BBSHOME "/log/func.log", "w")) == NULL)
  {
    printf("cann't open func.log\n");
    /* return 0*/;
  }
  if((fp2 = fopen(BBSHOME "/log/board.log", "w")) == NULL)
  {
    printf("cann't open board.log\n");
    /* return 0*/;
  }
  if((fp3 = fopen(BBSHOME "/log/personal.log", "w")) == NULL)
  {
    printf("cann't open personal.log\n");
    /* return 0*/;
  }

  for(i=0; i<=n; i++)
  {
    numlog+=allman[i].numloginsto;
    numpo+=allman[i].numpoststo;
    numlog2+=allman[i].numlogins;
    numpo2+=allman[i].numposts;
    if (allman[i].numloginsto>0) userlog++;
    if (allman[i].numlogins>0) userlog2++;
    if (allman[i].numpoststo>0) userpo++;
    if (allman[i].numposts>0) userpo2++;
    if (allman[i].numloginsto>maxlog) {
      maxlog=allman[i].numloginsto;
      strcpy(maxlogid, allman[i].userid2);
    }
    if (allman[i].numpoststo>maxpo) {
      maxpo=allman[i].numpoststo;
      strcpy(maxpoid, allman[i].userid2);
    }
    if (allman[i].numlogins>maxlog2) {
      maxlog2=allman[i].numlogins;
      strcpy(maxlogid2, allman[i].userid2);
    }
    if (allman[i].numposts>maxpo2) {
      maxpo2=allman[i].numposts;
      strcpy(maxpoid2, allman[i].userid2);
    }
    if (allman[i].messfrom>0) user_from++;
    if (allman[i].messto>0) user_to++;
    if (allman[i].messfrom>max_from) {
      max_from=allman[i].messfrom;
      strcpy(uname_from, allman[i].userid2);
    }
    if (allman[i].messto>max_to) {
      max_to=allman[i].messto;
      strcpy(uname_to, allman[i].userid2);
    }
    if (allman[i].userlevel & PERM_SYSOP) numsysop++;
    if (allman[i].userlevel & PERM_ACCOUNTS) numaccount++;
    if (allman[i].userlevel & PERM_BOARD) numboard++;
    if (allman[i].userlevel & PERM_CHATROOM) numchatroom++;
    if (allman[i].userlevel & PERM_BM) numbm++;
    if (allman[i].userlevel & PERM_SEECLOAK) numsee++;
    if (allman[i].userlevel & PERM_CLOAK) numcloak++;
    if (allman[i].userlevel & PERM_LOGINOK) numloginok++;
  }

  fprintf(fp, "    [1;31m%s[m %s ³ø§i\n",
     BOARDNAME,Ctime(&now));
  fprintf(fp, "\n");
  fprintf(fp, "    ¨´¤µ ¤w¦³ [1;33m%10d[m ¤H¤W¯¸¹L [1;33m%10d[m ¦¸, ¥­§¡¨C¤H [1;33m%5d[m ¦¸\n",
     userlog, numlog, numlog/userlog);
  fprintf(fp, "    ¨´¤µ ¤w¦³ [1;33m%10d[m ¤Hµoªí¹L [1;33m%10d[m ½g, ¥­§¡¨C¤H [1;33m%5d[m ½g\n\n",
     userpo, numpo, numpo/userlog);
  fprintf(fp, "    ¤µ¤Ñ ¤w¦³ [1;33m%10d[m ¤H¤W¯¸¹L [1;33m%10d[m ¦¸, ¥­§¡¨C¤H [1;33m%5d[m ¦¸\n",
     userlog2, numlog2, numlog2/userlog2);
  fprintf(fp, "    ¤µ¤Ñ ¤w¦³ [1;33m%10d[m ¤Hµoªí¹L [1;33m%10d[m ½g, ¥­§¡¨C¤H [1;33m%5d[m ½g\n\n",
     userpo2, numpo2, numpo2/userlog2);
  fprintf(fp, "    ¤µ¤Ñ ¤W¯¸ ³Ì¦h¦¸ªº¤H¬O [1;33m%13s[m ¦³ [1;33m%10d[m ¦¸\n",
     maxlogid2, maxlog2);
  fprintf(fp, "    ¤µ¤Ñ µoªí ³Ì¦h¦¸ªº¤H¬O [1;33m%13s[m ¦³ [1;33m%10d[m ½g\n",
     maxpoid2, maxpo2);
  fprintf(fp, "    ¤µ¤Ñ Åªª© [1;33m%8d[m ¦¸ ¦@ [1;33m%8d[m ¤À"
             " ¥­§¡¨Cª© [1;33m%5d[m ¤H¦¸ ¦@ [1;33m%5d[m ¤À \n",
     ave[0], ave[1]/60, ave[0]/numboards, ave[1]/(numboards*60));
  fprintf(fp, "    ¤µ¤Ñ Åªª© ¦¸¼Æ³Ì°ª¬O [1;33m%-13s[m ª© ¦@ [1;33m%5d[m ¦¸ ¤@¯ëª©­Ó¼Æ¬° [1;33m%5d[m ­Ó \n"
              "    ¤µ¤Ñ Åªª© ®É¶¡³Ì°ª¬O [1;33m%-13s[m ª© ¦@ [1;33m%5d[m ¤À ¤@¯ë¸s²Õ¼Æ¬° [1;33m%5d[m ­Ó\n\n",
     timesbname, max[0], numboards-1, sumbname, max[1]/60, numgroups);
/*
  fprintf(fp, "    ¤µ¤Ñ Á`¦@¦³ [1;33m%6d[m ­Ó°T®§ ¨ä¤¤ ¦³ [1;33m%5d[m ­Ó¤Hµo ¦³ [1;33m%5d[m ­Ó¤H¦¬\n"
              "    µo³Ì¦hªº¬O [1;33m%13s[m ¦³ [1;33m%4d[m ¦¸"
              " ¦¬³Ì¦hªº¬O [1;33m%13s[m ¦³ [1;33m%4d[m ¦¸\n\n",
     messnum, user_from, user_to, uname_from, max_from, uname_to, max_to);
*/
  fprintf(fp, "    ¤µ¤Ñ ¦³ [1;33m%5d[m ­Ó¤Hµù¥U  ¦³ [1;33m%5d[m ­Ó guest ¤W¨Ó¹L"
              " ¥þ³¡ªá¤F [1;33m%8d[m ¤ÀÄÁ\n"
              "    ¤µ¤Ñ ³Ì°ª¦³ [1;33m%5d[m ¦P®É¤W¯¸ ¥­§¡¦³ [1;33m%5d[m ¤H¤W¯¸\n",
     newreg, /* act[25]-numlog2 */  guestnum , act[24], maxtoday, act[24]/1440);
  fprintf(fp, "    ¤µ¤Ñ ¦³ [1;33m%5d[m ­Ó±b¸¹¹L´Á ¦³ [1;33m%5d[m ³Q²M\n",
     numdated, numclean);

  fprintf(fp, "\n    ¦³ [1;33m%5d[m ­Ó ¦³­­¨îªº ª© ¤Î [1;33m%5d[m ­Ó ¦³­­¨îªº ¸s²Õ",
      numhide-numhideg, numhideg);

  fprintf(fp, "\n    ¯¸ªø¦³ [1;33m%3d[m ¤H, ±b¸¹Á`ºÞ¦³ [1;33m%3d[m ¤H, "
              "¬Ýª©Á`ºÞ¦³ [1;33m%3d[m ¤H, ²á¤Ñ«ÇÁ`ºÞ¦³ [1;33m%3d[m ¤H\n",
    numsysop, numaccount, numboard, numchatroom);
  fprintf(fp, "    ª©¥D¦³ [1;33m%3d[m ¤H, ¬Ý¨£§ÔªÌ¦³ [1;33m%3d[m ¤H, "
              "¦³Áô¨­³N¦³ [1;33m%3d[m ¤H, §¹¦¨µù¥U¦³ [1;33m%5d[m ¤H\n",
    numbm, numsee, numcloak, numloginok);

  fprintf(fp1,"\
[1;46m                               ¦U¶µ¥\\¯à¨Ï¥Î²Î­p                                [m");
  fprintf(fp1,"\n\
²á¤Ñ        %4d ¦¸    ²á¤Ñ«Ç      %4d ¦¸
¦Û¤v§ï¸ê®Æ  %4d ¦¸    ³Q¯¸ªø§ï¸ê®Æ%4d ¦¸    ¬Ý¬ÝªO°O¿ý  %4d ¦¸
·s¼W¬ÝªO    %4d ­Ó    §R°£¬ÝªO    %4d ­Ó    §ó§ï¬ÝªOÃþ§O%4d ¦¸
³]©w¬ÝªO    %4d ­Ó    §ó§ï¬ÝªO¦WºÙ%4d ¦¸    §ó§ï¬ÝªOÄÝ©Ê%4d ¦¸
¯¸ªø³]©wªO¥D%4d ¦¸    ³]©w¬ÝªO±K½X%4d ¦¸    §ßªá¸¨·¬±Ù  %4d ¦¸
¯¸ªø¬å User %4d ¦¸    User ¦Û±þ   %4d ¦¸    BBSNET      %4d ¦¸
¤õ¨®®É¨è    %4d ¦¸    ­Ó¤H¹B¶Õ    %4d ¦¸    ¹q¤l¦r¨å    %4d ¦¸
BBCALL      %4d ¦¸    ³q°T¿ý      %4d ¦¸    °O±b¥»      %4d ¦¸
ÂIºq        %4d ¦¸    ¹qµø¸`¥Ø¬d¸ß%4d ¦¸    ±Ð¾ÇºëÆF    %4d ¦¸
§ë²¼        %4d ¦¸    ­×§ï/Æ[¬Ý§ë²¼%3d ¦¸    Á|¿ì§ë²¼    %4d ¦¸
¥Ó½Ð¬ÝªO/ªO¥D%3d ¦¸    ¦^À³¥Ó½Ð    %4d ¦¸
µª¿ý¾÷¯d¨¥  %4d ¦¸    µª¿ý¾÷Å¥¯d¨¥%4d ¦¸
",
      numtalk, numchat,numsetself, numsetu, numboardlog,
      numnewb, numdelb, numprefix,numsetb, numnameb,numattrb,
      numsetbm,numsetbp,numspam,
      numkill, numsuci,numbbsnet,
      numrailway, numfortune,numcdict,numbbcall,nummsgmenu,nummn,numosong,
      numcatv,numhint,
      numvote,numvotedit,numvotemake,numvbmake,numvbreply,
      numpcall,numpread);
  fprintf(fp1,"\
ºÆ¨g½ä½L    %4d ¦¸    ¤­¤l´Ñ      %4d ¦¸    ¹ï¹ï¼Ö      %4d ¦¸
½ò¦a¹p      %4d ¦¸    ±µÀs        %4d ¦¸    «XÃ¹´µ¤è¶ô  %4d ¦¸
ºô¸ô³Â±N    %4d ¦¸    ¤j¦Ñ¤G      %4d ¦¸    ¶H´Ñ        %4d ¦¸
²q¼Æ¦r      %4d ¦¸    ¹q¤lÂû      %4d ¦¸    ¶Â³Ç§J      %4d ¦¸
ªÑ¥«        %4d ¦¸    ¦è¤Ú©Ô      %4d ¦¸    ª÷¼³§J      %4d ¦¸
¤pº¿ÄR      %4d ¦¸    ½ä°¨        %4d ¦¸    »«ªG        %4d ¦¸
¤E¤E        %4d ¦¸    ¹ï¾Ô²q¼Æ¦r  %4d ¦¸    ¶H´Ñ³Â±N    %4d ¦¸
½ä«°¤C±i    %4d ¦¸
RPG¬DÂ¾·~   %4d ¦¸    RPG¤u·|     %4d ¦¸    RPG±Æ¦æº]   %4d ¦¸
RPG°V½m³õ   %4d ¦¸    RPG³]©wuser %4d ¦¸    RPG¹ï¾Ô     %4d ¦¸
",numbet,numfive,numgamble,nummine,numdragon,numtetris,nummj,numbig2,
numchess,numxaxb,numchicken,numbj,numstock,numdice,numgp,nummarie,numrace,
numbingo,numnine,numnfight,numchessmj,numsevencard
,numrpgchoose,numrpguild,numrpgtop,numrpgtrain,numrpgset,numrpgpk);

/*------- wildcat : ¤À¨â­ÓÀÉ¬ö¿ý -------*/

  fprintf(fp2, "==>[1;32m ¬ÝªOª¬ªp³ø§i [33m%s[m\n",Ctime(&now));
  fprintf(fp2,"»¡©ú:®É¶¡->°±¯d®É¶¡(¬í) ¤H¦¸->¶iªO¤H¦¸ §R°£->³Q§R°£½g¼Æ");

  fprintf(fp2,"\n[1;37;42m¦W¦¸ %-15.15s%-28.28s %6s %4s %4s %3s %3s %3s[m\n",
                "°Q½×°Ï¦WºÙ","¤¤¤å±Ô­z","  ®É¶¡","¤H¦¸","§R°£","POST","TAG","DEL");

 for(i=0;i<MAXBOARD;i++)
 {
   if(st[i].sum)
     fprintf(fp2,"[1;33m%4d[m %-15.15s%-28.28s [1;32m%6d [31m%4d [m%4d [1;36m%3d[m %3d %3d\n",
     i+1,st[i].boardname,st[i].expname,st[i].sum,st[i].times,st[i].num
        , st[i].post,st[i].tag,st[i].del);
 }
     fprintf(fp2,"[1;37;42m     %-15.15s%-28.28s %6d %4d %4d %3d %3d %3d\n",
     "Total","Á`¦X",alltime,alltimes,allnum,allpost,alltag,alldel);

  fprintf(fp3, "==>[1;32m ­Ó¤HªO±Æ¦æº] [33m%s[m\n",Ctime(&now));
  fprintf(fp3,"»¡©ú:®É¶¡->°±¯d®É¶¡(¬í) ¤H¦¸->¶iªO¤H¦¸ POST->µoªí¦¸¼Æ ¤À¼Æ->¤½¦¡ºâ¥X¨Óªº¤À¼Æ");

  fprintf(fp3,"\n[1;37;42m¦W¦¸ %-15.15s%-28.28s %6s %4s %4s - %4s -   [m\n",
                "°Q½×°Ï¦WºÙ","¤¤¤å±Ô­z","  ®É¶¡","¤H¦¸","POST","±o¤À");

 qsort(st, numboards, sizeof( st[0] ), personal_cmp);
 { int j=0;
 for(i=0;i<MAXBOARD;i++)
 {
   if(st[i].sum && st[i].attr & BRD_PERSONAL)
     fprintf(fp3,"[1;33m%4d[m %-15.15s%-28.28s [1;32m%6d [31m%4d [1;36m%4d - [1;33m%.2f[m  \n",
     ++j,st[i].boardname,st[i].expname,st[i].sum,st[i].times,st[i].post
        , (float)((st[i].post*500) + (st[i].times*100) +(st[i].sum/3))/100);
 }
 }
//     fprintf(fp3,"[1;37;42m     %-15.15s%-28.28s %6d %4d %4d %3d %3d %3d\n",
//     "Total","Á`¦X",alltime,alltimes,allnum,allpost,alltag,alldel);

  printf("numlog = %d\n", numlog);
  printf("numlog2 = %d\n", numlog2);
  printf("numpo = %d\n", numpo);
  printf("numpo2 = %d\n", numpo2);
  printf("userlog = %d\n", userlog);
  printf("userlog2 = %d\n", userlog2);
  printf("userpo = %d\n", userpo);
  printf("userpo2 = %d\n", userpo2);
  printf("Maxpost %s = %d\n", maxpoid, maxpo);
  printf("Maxlogin %s = %d\n", maxlogid, maxlog);
  printf("Maxpost2 %s = %d\n", maxpoid2, maxpo2);
  printf("Maxlogin2 %s = %d\n", maxlogid2, maxlog2);
  fclose(fp);
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
}

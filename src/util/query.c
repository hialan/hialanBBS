/*-------------------------------------------------------*/
/* util/query.c        ( NTHU CS MapleBBS Ver 2.36.sob ) */
/*-------------------------------------------------------*/
/* target : ¯¸¥~ query ¨Ï¥ÎªÌ                            */
/* create : 96/11/27                                     */
/* update : ÀH®É§ó·s                                     */
/*-------------------------------------------------------*/
/* syntax : query <userid>                               */
/*-------------------------------------------------------*/

#include <stdio.h>
#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#define _MODES_C_

char buf[256];
char *ptr;
userec uec;
fileheader fh;
struct UTMPFILE *utmpshm;
user_info *ushm_head, *ushm_tail;


static void
attach_err(shmkey, name)
  int shmkey;
  char *name;
{
  char buf[80];

  sprintf(buf, "error, key = %x", shmkey);
/*  logit(name, buf);*/
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

resolve_utmp()
{
  if (utmpshm == NULL)
  {
    register struct UTMPFILE *ushm;
    register user_info *uentp;

    utmpshm = ushm = attach_shm(UTMPSHM_KEY, sizeof(*utmpshm));

    ushm_head = uentp = ushm->uinfo;
    ushm_tail = uentp + USHM_SIZE;
  }
}


char *
Cdate(clock)
  time_t *clock;
{
  static char foo[22];
  struct tm *mytm = localtime(clock);

  strftime(foo, 22, "%D %T %a", mytm);
  return (foo);
}

int
chkmail(userid)
char *userid;
{
    FILE *mail;

    sprintf(buf, BBSHOME"/home/%s/.DIR", userid);
    if( access(buf,F_OK) ) /* .DIR ¤£¦s¦b */
        return 0;
    else
        if( (mail=fopen(buf,"r")) == NULL){
            perror("fopen:.DIR");
            exit(1);
        }
        else
            while( fread(&fh,sizeof(fileheader),1,mail) > 0)
                ;
            if( fh.filemode & FILE_READ )
                return 0;
            else
                return 1;
}

void
showuser(uec)
userec *uec;
{
    register int i=0 , user_num;
    FILE *plans;
    char *modestr;
    user_info *uentp, *utail;

    printf("[1;33;44m                         §å½ð½ð¹ê·~§{                     [m\n");

    printf("%s %s(%s) ¦@¤W¯¸ %d ¦¸¡Aµoªí¹L %d ½g¤å³¹\n",
    (uec->userlevel & PERM_LOGINOK) ? "¡À" : "¡H",
   uec->userid, uec->username, uec->numlogins, uec->numposts);
    ptr=uec->lasthost;
    if( strchr(ptr,'@') ){
        ptr=index(uec->lasthost,'@')+1;
    }
    printf("³Ìªñ(%s)±q[%s]¤W¯¸\n", Cdate(&(uec->lastlogin)),ptr );

    modestr = "¤£¦b¯¸¤W";

    uentp = ushm_head;
    utail = ushm_tail;
    do
    {
      if (!strcmp(uec->userid, uentp->userid))
      {
        if (!PERM_HIDE(uentp))
//           modestr = ModeTypeTable[uentp->mode];
        break;
      }
    } while (++uentp <= ushm_tail);


    printf("[1;33m[¥Ø«e°ÊºA¡G%s][m  %s\n",
      modestr, chkmail(uec->userid) ? "¦³·s¶i«H¥óÁÙ¨S¬Ý":"©Ò¦³«H¥ó³£¬Ý¹L¤F");

    sprintf(buf,BBSHOME"/home/%s/plans", uec->userid);
    if( !(i=access(buf, F_OK)) ){
        printf("%s ªº¦W¤ù:\n", uec->userid);
        if( (plans=fopen(buf,"r")) == NULL ){
            perror("fopen:plans");
            exit(1);
        }
        while (i++ < MAXQUERYLINES && fgets(buf,256,plans))
            printf("%s",buf);
       printf("",buf);
    }
    else
        printf("%s ¥Ø«e¨S¦³¦W¤ù\n", uec->userid);

}

void
main(argc,argv)
   int argc;
   char **argv;
{
    char *c;
    FILE *pwd;
    int len;

    if (argc != 2){
        fprintf(stderr,"Wrong argument!\n");
        exit(1);
    }

    if( (pwd=fopen(BBSHOME"/.PASSWDS","r")) == NULL){
        perror("fopen");
        exit(1);
    }

    resolve_utmp();

    while( fread( &uec, sizeof(userec), 1, pwd) > 0 )
    {
        if( strcmp(uec.userid,argv[1]))
        {
            continue;
        }
        else
            showuser(&uec);
        exit(0);
    }
    fprintf(stderr,"Can not find user %s\n\n",argv[1]);
    exit(1);
}


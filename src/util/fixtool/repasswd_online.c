/*
將所有 bad_user_id 清除。

可在 bbs 運作時執行。

若要回復到修改前的狀態:
cp ~bbs/PASSWDS ~bbs/.PASSWDS

*/

#include <stdio.h>
#include "bbs.h"

#define DOTPASSWDS BBSHOME"/.PASSWDS"
#define PASSWDSBAK BBSHOME"/PASSWDS"
#define TMPFILE    BBSHOME"/tmp/tmpfile"


struct userec cuser;

main()
{
    FILE *foo1, *foo2;
    int cnum,i,match;

    setgid(BBSGID);
    setuid(BBSUID);
    if( ((foo1=fopen(DOTPASSWDS, "r")) == NULL)
                || ((foo2=fopen(TMPFILE,"w"))== NULL) ){
        puts("file opening error");
        exit(1);
    }

    while( (cnum=fread( &cuser, sizeof(cuser), 1, foo1))>0 ) {
       if (bad_user_id(cuser.userid) || cuser.goldmoney <0 || cuser.silvermoney <0)
       {
         printf("%s -> %d , %d\n",cuser.userid,cuser.goldmoney,cuser.silvermoney);
         memset(&cuser, 0, sizeof(cuser));
       }
       fwrite( &cuser, sizeof(cuser), 1, foo2);
    }
    fclose(foo1);
    fclose(foo2);

    if(rename(DOTPASSWDS,PASSWDSBAK)==-1){
        puts("replace fails");
        exit(1);
    }
    unlink(DOTPASSWDS);
    rename(TMPFILE,DOTPASSWDS);
    unlink("tmpfile");

    return 0;
}


bad_user_id(userid)
  char *userid;
{
  register char ch;

  if (strlen(userid) < 2)
    return 1;

  if (!isalpha(*userid))
    return 1;

  if (!strcasecmp(userid, "new"))
    return 1;

  while (ch = *(++userid))
  {
    if (!isalnum(ch))
      return 1;
  }
  return 0;
}

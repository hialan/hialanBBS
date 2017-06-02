/*
將所有 bad_user_id 清掉 (包括 new)，這將使 .PASSWDS 乾淨些。

要在 share memory 還沒載入時執行，最好在開機時執行，否則會造
成資料錯亂的現象，若要回復到修改前的狀態，可以在未載入
share memory 時:
cp ~bbs/PASSWDS ~bbs/.PASSWDS

可使用 ipcs 得知 share memory 是否已經載入，或者執行 repasswd
前先用 ipcrm 把 share memory 放掉，for FreeBSD:
ftp://sob.m7.ntu.edu.tw/sob-version/bin/shutdownbbs
*/

#include <stdio.h>
#include "bbs.h"

#define DOTPASSWDS	BBSHOME"/.PASSWDS"
#define PASSWDSBAK	BBSHOME"/PASSWDS"
#define TMPFILE		BBSHOME"/tmp/tmpfile"


struct userec cuser;

main()
{
    FILE *foo1, *foo2, *foo3, *foo4;
    int cnum,i,match;

    setgid(BBSGID);
    setuid(BBSUID);
    if( ((foo1=fopen(DOTPASSWDS, "r")) == NULL)
                || ((foo2=fopen(TMPFILE,"w"))== NULL) ){
        puts("file opening error");
        exit(1);
    }

    while( (cnum=fread( &cuser, sizeof(cuser), 1, foo1))>0 ) {
       if (bad_user_id(cuser.userid))
          continue;
       fwrite( &cuser, sizeof(cuser), 1, foo2);
    }
    fclose(foo1);
    fclose(foo2);

    if(f_mv(DOTPASSWDS,PASSWDSBAK)==-1){
        puts("replace fails");
        exit(1);
    }
    unlink(DOTPASSWDS);
    f_mv(TMPFILE,DOTPASSWDS);
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

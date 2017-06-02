#define SUBJECT "[通告] 建議使用 KK-Man 連往 WD-BBS"
#define CONTENT BBSHOME"/etc/email_content"

#include "bbs.h"

userec auser;

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

main()
{
  FILE *inf;
  int i,type, j=0;
                                                                                
  inf = fopen(BBSHOME "/.PASSWDS", "rb");
                                                                                
  if (inf == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    exit(0);
  }

  for (i = 0; fread(&auser, sizeof(userec), 1, inf); i++)
  {
    if (bad_user_id(auser.userid) || strchr(auser.userid,'.') 
        || not_addr(auser.email) || strstr(auser.email,"bbs@"MYHOSTNAME))
        i--;
   else
    {
     char buf[255];
     sprintf(buf,"/usr/bin/mail -s \"%s\" %s < %s",SUBJECT,auser.email,CONTENT);
     system(buf);
     printf("%04d. %-14.14s -> %s\n",++j,auser.userid,auser.email);
    }
  }
  fclose(inf);
}

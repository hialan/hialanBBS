#include <sys/param.h>
#include "bbs.h"


#define  PASSWD BBSHOME"/.PASSWDS"
#define  OUT BBSHOME"/log/GNP"

int coun=0,count=0;
userec cuser;

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
bad_user_id()
{
  register char ch;
  int j;

  if (strlen(cuser.userid) < 2 || !isalpha(cuser.userid[0]))
    return 1;

  if (cuser.numlogins==0)
    return 1;

  for(j=1;ch = cuser.userid[j];j++)
  {
    if (!isalnum(ch))
      return 1;
  }

  return 0;
}

main()
{
  FILE *fp1=fopen(PASSWD,"r");
  FILE *fp2=fopen(OUT,"w");
  double total1=0,total2=0;
  float gnp1,gnp2;
  float tgnp1=0;
  float tgnp2=0;
  time_t now = time(0);
  int days=0;
  while( (fread( &cuser, sizeof(cuser), 1, fp1))>0 ) 
  {
   count ++;
   if(bad_user_id())
   {
    printf("<%d> userid:%s  silver:%d  gold: %d\n",
            count,cuser.userid,cuser.silvermoney,cuser.goldmoney);
   }         
   else	
   {
      if(cuser.goldmoney > 1000000 || cuser.silvermoney >50000000)
        printf("[1;33m<%d> userid:%s  silver:%d  gold: %d[m\n",
          count,cuser.userid,cuser.silvermoney,cuser.goldmoney);
        
      coun ++;
      total1+=(cuser.goldmoney);    
      total2+=(cuser.silvermoney);    
      days+=cuser.numlogins;
      gnp1= (cuser.goldmoney)/cuser.numlogins;
      tgnp1+=gnp1;  
      gnp2= (cuser.silvermoney)/cuser.numlogins;
      tgnp2+=gnp2;  
   } 
  }
 fclose(fp1);
 fprintf(fp2,"    [1m-----[[32m%s[37m]-----[m\n",Cdate(&now)); 
 fprintf(fp2,"    ¥»¯¸¥Ø«eÁ`¤H¤f   : [1m%11d[0;36m ¤H\n",coun);
 fprintf(fp2,"    [1;33m¥»¯¸¤H¥ÁÁ`ª÷¹ô   : [1m%11.0f [0;36m¤¸\n",total1);
 fprintf(fp2,"    [1;33m¥­§¡¤H¥Áª÷¹ô     : [1m%11d[0;36m ¤¸[m\n", (long long)total1/coun);
 fprintf(fp2,"    [1;33m°ê¥Á¥Í²£¤òÃB(ª÷) : [1m%11f[0;36m ¤¸[m\n", tgnp1/days);
 fprintf(fp2,"    [1;37m¥»¯¸¤H¥ÁÁ`»È¹ô   : [1m%11.0f [0;36m¤¸\n",total2);
 fprintf(fp2,"    [1;37m¥­§¡¤H¥Á»È¹ô     : [1m%11d[0;36m ¤¸[m\n", (long long)total2/coun);
 fprintf(fp2,"    [1;37m°ê¥Á¥Í²£¤òÃB(»È) : [1m%11f[0;36m ¤¸[m\n", tgnp2/days);
 fprintf(fp2,"    [1m---------------------------------[m\n");
 fclose(fp2);
}
#include "bbs.h"
//#define ba rpguser.race == 6 ? 10*rpguser.level : 5

int gagb()
{
  int tmoney,money;
  char genbuf[200],buf[80];
  char ans[5]="";
  /* ­¿²v        0  1   2   3   4   5   6   7   8   9   10 */
  float bet[11]={0,100, 50, 10, 3 ,1.5,1.2,0.9, 0.8 ,0.5,0.1};
  int a,b,c,count;  
  FILE *fp;
  time_t now;
  
  setutmpmode(XAXB);
  clear();
  showtitle("²q¼Æ¦r¡I", BoardName);
  move(1,0);
  prints("[1m²{¦b§A¨­¤W¦@¦³ [33m%d [37m¤¸¡C[m",cuser.silvermoney);
  move(6,0);
  prints("[36m%s[m",msg_seperator);
  move(17,0);
  prints("[36m%s[m",msg_seperator);
  fp = fopen("game/money", "r");
  fscanf(fp, "%d", &tmoney);
  fclose(fp);

  move(20,0);
  prints("²{¦b¹C¼Ö³õ¤º¦@¦³ %d  ¤¸",tmoney);

  getdata(3, 0, "½Ð°Ý§A­n¤Uª`¦h¤Ö(1 ~ 250000)¡H ", genbuf, 7, 1, 0);
  money=atoi(genbuf);
  if(money<=0||money>cuser.silvermoney||money>250000) 
    return 0;
  demoney(money); 

  move(1,15); 
  prints("%d",cuser.silvermoney);
  tmoney+=money; 

  move(20,17); 
  prints("%d",tmoney);

  do{
    itoa(rand()%10000,ans);
    for (a=0;a<3;a++)
    for (b=a+1;b<4;b++)
    if(ans[a]==ans[b]) ans[0]=0;
  }while (!ans[0]);

  for(count=1;count<11;count++)
  {
    do{
      getdata(5,0,"½Ð²q ¡÷ ",genbuf,5,1,0);
      if(!strcmp(genbuf,"¿ß¦Ñ¤j"))
      {
        prints("%s",ans);
        igetch();
      }
      c=atoi(genbuf);
      itoa(c,genbuf);
      for (a=0;a<3;a++)
      for (b=a+1;b<4;b++)
      if(genbuf[a]==genbuf[b]) 
        genbuf[0]=0;
      if(!genbuf[0])
      {
        pressanykey("¿é¤J¼Æ¦r¦³°ÝÃD!!");
      }
    }while (!genbuf[0]);
    move(count+6,0);
    prints("  [1;31m²Ä [37m%2d [31m¦¸¡G [37m%s  ->  [33m%dA [36m%dB [m",count,genbuf,an(genbuf,ans),bn(genbuf,ans));
    if(an(genbuf,ans)==4) break;
  }
  now=time(0);

  if(count > 10)
  {
    sprintf(buf, "§A¿é¤FËç¡I¥¿½Tµª®×¬O %s¡A¤U¦¸¦A¥[ªo§a!!",ans);
    game_log("XAXB", "¥i¼¦¨S²q¨ì¡A¿é¤F %d ¤¸¡I", money);
  }
  else
  {
    int oldmoney=money;
    money *= bet[count];
    inmoney(money);
//    inexp(ba*(10-count));
    tmoney-=money;
    if(money-oldmoney > 0)
      sprintf(buf,"®¥³ß¡IÁ`¦@²q¤F %d ¦¸¡A²bÁÈ¼úª÷ %d ¤¸",count,money-oldmoney);
    if(money-oldmoney == 0)
      sprintf(buf,"­ü¡ã¡ãÁ`¦@²q¤F %d ¦¸¡A¨S¿é¨SÄ¹¡I",count);
    if(money-oldmoney < 0)
      sprintf(buf,"°Ú¡ã¡ãÁ`¦@²q¤F %d ¦¸¡A½ß¿ú %d ¤¸¡I",count,oldmoney-money);
    move(1,15); 
    prints("%d",cuser.silvermoney);
    move(20,17); 
    prints("%d",tmoney);

    if(money-oldmoney > 0)
      game_log("XAXB", "[1;32m²q¤F %d ¦¸¡A²bÁÈ %d ¤¸¡I[m", count,oldmoney-money);
    else if(money-oldmoney < 0)
      game_log("XAXB", "[1;32m²q¤F %d ¦¸¡AÁ«¤F %d ¤¸¡I[m", count,oldmoney-money);
  }

  fp = fopen("game/money", "w");
  fprintf(fp, "%d", tmoney);
  fclose(fp);

  pressanykey(buf);
  return 0;
}

itoa(int i, char *a)
{
 int j,k,l=1000;
// prints("itoa: i=%d ",i);

 for(j=3;j>0;j--)
 {
  k=i-(i%l);
  i-=k;
  k=k/l+48;
  a[3-j]=k;
  l/=10;
 }
 a[3]=i+48;
 a[4]=0;

// prints(" a=%s\n",a);
// igetch();
}

int an(char *a, char *b)
{
 int i,k=0;
 for(i=0;i<4;i++)
  if(*(a+i)==*(b+i)) k++;
 return k;
}

int bn(char *a, char *b)
{
 int i,j,k=0;
 for(i=0;i<4;i++)
  for(j=0;j<4;j++)
   if(*(a+i)==*(b+j)) k++;
 return (k-an(a,b));
}

#include "bbs.h"
/* write by dsyan               */
/* 87/10/24                     */
/* ¤Ñªø¦a¤[ Forever.twbbs.org   */

#include "bbs.h"

#undef  GP_DEBUG

char card[52],mycard[5],cpucard[5],sty[100],now;

show_money(int m)
{
  move(19,0);
  clrtoeol();
  prints("[1;37;44m§A²{¦³²{ª÷: [36m%-18d[37m©ãª`ª÷ÃB: [36m%-20d[m",
  cuser.silvermoney,m);
}

static int
forq(a,b)
char *a,*b;
{
 char c=(*a)%13;
 char d=(*b)%13;
 if(!c) c=13; if(!d) d=13;
 if(c==1) c=14; if(d==1) d=14;
 if(c==d) return *a-*b;
 return c-d;
}

int p_gp()
{
 char genbuf[200],hold[5];
 char ans[5];
 time_t n;
 int bet,i,j,k,tmp,x,xx,doub,gw=0,cont=0;

 n = time(0);
 more(BBSHOME"/game/gp.welcome",YEA);

 setutmpmode(GP);bet=0;
 while(-1)
 {
  clear();
  showtitle("ª÷¼³§J±ô«¢", BoardName);
  ans[0]=0;
  show_money(bet);
  if(cont==0)
  do
  {
   getdata(21, 0,"­n¤Uª`¦h¤Ö©O(¤W­­250000)? «ö Enter Â÷¶}>", genbuf, 7, 1, 0);
   bet=atoi(genbuf);
   if(bet<0) bet=0;
  } while(bet>250000);
  if(!bet) return 0;
  if(check_money(bet,SILVER)) return 0;
  move(21,0); clrtoeol();
  if(cont>0)
  prints("[33;1m (¡ö)(¡÷)§ïÅÜ¿ïµP  (SPCAE)§ïÅÜ´«µP  (Enter)½T©w[m");
  else
  prints("[33;1m (¡ö)(¡÷)§ïÅÜ¿ïµP  (d)Double  (SPCAE)§ïÅÜ´«µP  (Enter)½T©w[m");
  demoney(bet);
  show_money(bet);

  for(i=0;i<52;i++) card[i]=i; /* 0~51 ..¶Â³Ç§J¬O 1~52 */

  for(i=0;i<1000;i++)
  {
   j=random()%52; k=random()%52;
   tmp=card[j]; card[j]=card[k]; card[k]=tmp;
  }

  now=doub=0;
  for(i=0;i<5;i++)
  {
   mycard[i]=card[now++];
   hold[i]=1;
  }
  qsort(mycard,5,sizeof(char),forq);

  for(i=0;i<5;i++)
   show_card(0,mycard[i],i);

  x=xx=tmp=0;
  while(tmp!=13)
  {
   for(i=0;i<5;i++)
   {
    move(16, i*4+2); outs(hold[i]<0?"«O":"  ");
    move(17, i*4+2); outs(hold[i]<0?"¯d":"  ");
   }
   move(11,xx*4+2); outs("  ");
   move(11,x*4+2); outs("¡õ");
   move(b_lines-1,0);
   refresh();
   xx=x;

   tmp=igetkey();
   switch(tmp)
   {
#ifdef GP_DEBUG
    case KEY_UP:
     getdata(21, 0,"§âµP´«¦¨> ", genbuf, 3, 1, 0);
     mycard[x]=atoi(genbuf);
     qsort(mycard,5,sizeof(char),forq);
     for(i=0;i<5;i++)
      show_card(0,mycard[i],i);
     break;
#endif
    case KEY_LEFT: case 'l': x=x?x-1:4; break;
    case KEY_RIGHT: case 'r': x=(x==4)?0:x+1; break;
    case ' ': hold[x]*=-1; break;
    case 'd':
     if(!cont&&!doub&&cuser.silvermoney>=bet)
     {
      doub++;
      move(21,0); clrtoeol();
      prints("[33;1m (¡ö)(¡÷)§ïÅÜ¿ïµP  (SPCAE)§ïÅÜ´«µP  (Enter)½T©w[m");
      demoney(bet);
      bet*=2;
      show_money(bet);
     }
     break;
   }
  }

  for(i=0;i<5;i++)
   if(hold[i]==1)
    mycard[i]=card[now++];
  qsort(mycard,5,sizeof(char),forq);
  for(i=0;i<5;i++)
   show_card(0,mycard[i],i);
  move(11,x*4+2); outs("  ");

  cpu();
#ifdef GP_DEBUG
for(x=0;x<5;x++)
{
     getdata(21, 0,"§âµP´«¦¨> ", genbuf, 3, 1, 0);
     cpucard[x]=atoi(genbuf);
}
     qsort(cpucard,5,sizeof(char),forq);
     for(i=0;i<5;i++)
      show_card(1,cpucard[i],i);
#endif
  i=gp_win();

  if(i<0)
  {
   inmoney(bet*2);
   pressanykey_old("«z!!¦n´Î³á!!²bÁÈ %d ¤¸«¨.. :DDD",bet);
   if (cont>0)
     game_log("GP","[1;31m³s³Ó %d ¦¸, Ä¹¤F %d ¤¸",cont+1,bet);
   else
     game_log("GP","[1;31mÄ¹¤F %d ¤¸",bet);
   bet = ( bet > 50000 ? 100000 : bet * 2 );gw=1;
  }
  else if(i>1000)
  {
   switch(i)
   {
    case 1001: doub=15; break;
    case 1002: doub=10; break;
    case 1003: doub=5; break;
   }
   inmoney((bet*doub) + bet);
   pressanykey_old("«z!!¦n´Î³á!!²bÁÈ %d ¤¸«¨.. :DDD",bet*doub);
   if(cont>0)
     game_log("GP","[1;31m³s³Ó %d ¦¸, Ä¹¤F %d ¤¸",cont+1,bet*doub); 
   else
     game_log("GP","[1;31mÄ¹¤F %d ¤¸",bet*doub);
   game_log("GP","%s",genbuf);
   bet = ( bet > 50000 ? 100000 : bet * 2 * doub ); gw=1;
   bet = ( bet >= 100000 ? 100000 : bet );
  }
  else
  {
   pressanykey_old("¿é¤F..:~~~");
   if(cont>1)
     game_log("GP","[1;32m¤¤¤î %d ³s³Ó, ¿é¤F %d ¤¸",cont,bet);
   else
     game_log("GP","[1;32m¿é¤F %d ¤¸",bet);
   cont=0;bet=0;
  }

  if ( gw==1 )
  {
    gw=0;
//    getdata(21, 0,"±z­n§â¼úª÷Ä~ÄòÀ£ª`¶Ü (y/n)?", ans, 2, LCECHO, 0);
    ans[0] = getans2(21, 0,"±z­n§â¼úª÷Ä~ÄòÀ£ª`¶Ü? ", 0, 2, 'y');
    if ( ans[0]=='y')
       cont++;
    else
    {
       cont=0;bet=0;
    }
  }
 }
}

show_card(isDealer,c,x)
int isDealer,c,x;
{
  int beginL;
  char *suit[4] = {"¢Ñ","¢Ò","¢Ö","¢á"};
  char *num[13] = {"¢Ù","¢Ï","¢±","¢²","¢³","¢´","¢µ",
                   "¢¶","¢·","¢¸","10","¢Ø","¢ß"};

  beginL=(isDealer)? 2: 12;
  move(beginL, x*4);      outs("¢~¢w¢w¢w¢¡");
  move(beginL+1, x*4);  prints("¢x%2s    ¢x", num[c%13]);
  move(beginL+2, x*4);  prints("¢x%2s    ¢x", suit[c/13]); /* ¡ö³o¸Ì¸ò¶Â³Ç§J */
#ifdef GP_DEBUG
  move(beginL+3, x*4);  prints("¢x%2d    ¢x",c);              /*   ¦³ÂI¤£¦P³á!! */
#else
  move(beginL+3, x*4);    outs("¢x      ¢x");              /*   ¦³ÂI¤£¦P³á!! */
#endif
  move(beginL+4, x*4);    outs("¢x      ¢x");
  move(beginL+5, x*4);    outs("¢x      ¢x");
  move(beginL+6, x*4);    outs("¢¢¢w¢w¢w¢£");
}

cpu()
{
 char i,j,hold[5];
 char p[13],q[5],r[4];
 char a[5],b[5];

 for(i=0;i<5;i++)
 {
  cpucard[i]=card[now++];
  hold[i]=0;
 }
 qsort(cpucard,5,sizeof(char),forq);
 for(i=0;i<5;i++)
  show_card(1,cpucard[i],i);

 tran(a,b,cpucard);
 check(p,q,r,cpucard);

 for(i=0;i<13;i++)
  if(p[i]>1)
   for(j=0;j<5;j++)
    if(i==cpucard[j]%13) hold[j]=-1;

 for(i=0;i<5;i++)
 {
  if(a[i]==13 || a[i]==1) hold[i]=-1;
  move(6, i*4+2); outs(hold[i]<0?"«O":"  ");
  move(7, i*4+2); outs(hold[i]<0?"¯d":"  ");
 }

 pressanykey_old("¹q¸£´«µP«e..");

 for(i=0;i<5;i++)
  if(!hold[i]) cpucard[i]=card[now++];
 qsort(cpucard,5,sizeof(char),forq);
 for(i=0;i<5;i++)
  show_card(1,cpucard[i],i);
}

int
gp_win()
{
 int my,cpu,ret;
 char myx,myy,cpux,cpuy;

 my=complex(mycard,&myx,&myy);
 cpu=complex(cpucard,&cpux,&cpuy);
 show_style(my,cpu);

 if(my!=cpu)
  ret=my-cpu;
 else if (myx==1 && cpux!=1)
  ret=-1;
 else if (myx!=1 && cpux==1)
  ret=1;
 else if(myx!=cpux)
  ret=cpux-myx;
 else if (myy!=cpuy)
  ret=cpuy-myy;

 if(ret<0)
  switch(my)
  {
   case 1: ret=1001; break;
   case 2: ret=1002; break;
   case 3: ret=1003; break;
  }

 return ret;
}

// ¦Pªá¶¶¡BÅKªK¡B¸¬¡B¦Pªá¡B¶¶¡B¤T±ø¡B¨ß­F¡B­F¡B¤@°¦
int
complex(cc,x,y)
char *cc,*x,*y;
{
 char p[13],q[5],r[4];
 char a[5],b[5],c[5],d[5];
 int i,j,k;

 tran(a,b,cc);
 check(p,q,r,cc);

/* ¦Pªá¶¶ */
 if((a[0]==a[1]-1 && a[1]==a[2]-1 && a[2]==a[3]-1 && a[3]==a[4]-1) &&
    (b[0]==b[1] && b[1]==b[2] && b[2]==b[3] &&b[3]==b[4]))
 { *x=a[4]; *y=b[4]; return 1;}

 if(a[4]==1 && a[0]==2 && a[1]==3 && a[2]==4 && a[3]==5 &&
    (b[0]==b[1] && b[1]==b[2] && b[2]==b[3] && b[3]==b[4]))
 { *x=a[3]; *y=b[4]; return 1;}

 if(a[4]==1 && a[0]==10 && a[1]==11 && a[2]==12 && a[3]==13 &&
    (b[0]==b[1] && b[1]==b[2] && b[2]==b[3] && b[3]==b[4]))
 { *x=1; *y=b[4]; return 1;}

/* ¥|±i */
 if(q[4]==1)
 {
   for(i=0;i<13;i++) if(p[i]==4) *x=i?i:13;
   return 2;
 }

/* ¸¬Äª */
 if(q[3]==1&&q[2]==1)
 {
  for(i=0;i<13;i++) if(p[i]==3) *x=i?i:13;
  return 3;
 }

/* ¦Pªá */
 for(i=0;i<4;i++) if(r[i]==5) {*x=i; return 4;}

/* ¶¶¤l */
 memcpy(c,a,5); memcpy(d,b,5);
 for(i=0;i<4;i++)
  for(j=i;j<5;j++)
  if(c[i]>c[j])
  {
   k=c[i]; c[i]=c[j]; c[j]=k;
   k=d[i]; d[i]=d[j]; d[j]=k;
  }

 if(10==c[1] && c[1]==c[2]-1 && c[2]==c[3]-1 && c[3]==c[4]-1 && c[0]==1)
 { *x=1; *y=d[0]; return 5;}

 if(c[0]==c[1]-1 && c[1]==c[2]-1 && c[2]==c[3]-1 && c[3]==c[4]-1)
 { *x=c[4]; *y=d[4]; return 5;}

/* ¤T±ø */
 if(q[3]==1)
  for(i=0;i<13;i++)
   if(p[i]==3) { *x=i?i:13; return 6;}

/* ¨ß­F */
 if(q[2]==2)
 {
  for(*x=0,i=0;i<13;i++)
   if(p[i]==2)
   {
    if((i>1?i:i+13)>(*x==1?14:*x))
    {
     *x=i?i:13; *y=0;
     for(j=0;j<5;j++) if(a[j]==i&&b[j]>*y) *y=b[j];
    }
   }
  return 7;
 }

/* ­F */
 if(q[2]==1)
  for(i=0;i<13;i++)
   if(p[i]==2)
   {
    *x=i?i:13; *y=0;
    for(j=0;j<5;j++) if(a[j]==i&&b[j]>*y) *y=b[j];
    return 8;
   }

/* ¤@±i */
 *x=0; *y=0;
 for(i=0;i<5;i++)
  if((a[i]=a[i]?a[i]:13>*x||a[i]==1)&&*x!=1)
  { *x=a[i]; *y=b[i];}

 return 9;
}

/* a ¬OÂI¼Æ .. b ¬Oªá¦â */
tran(a,b,c)
char *a,*b,*c;
{
 int i;
 for(i=0;i<5;i++)
 {
  a[i]=c[i]%13;
  if(!a[i]) a[i]=13;
 }

 for(i=0;i<5;i++)
  b[i]=c[i]/13;
}

check(p,q,r,cc)
char *p,*q,*r,*cc;
{
 char i;

 for(i=0;i<13;i++) p[i]=0;
 for(i=0;i<5;i++) q[i]=0;
 for(i=0;i<4;i++) r[i]=0;

 for(i=0;i<5;i++)
  p[cc[i]%13]++;

 for(i=0;i<13;i++)
  q[p[i]]++;

 for(i=0;i<5;i++)
  r[cc[i]/13]++;
}

// ¦Pªá¶¶¡BÅKªK¡B¸¬¡B¦Pªá¡B¶¶¡B¤T±ø¡B¨ß­F¡B­F¡B¤@°¦
show_style(my,cpu)
char my,cpu;
{
 char *style[9]={"¦Pªá¶¶","¥|±i","¸¬Äª","¦Pªá","¶¶¤l",
              "¤T±ø","¨ß­F","³æ­F","¤@±i"};
 move(5,26); prints("[41;37;1m%s[m",style[cpu-1]);
 move(15,26); prints("[41;37;1m%s[m",style[my-1]);
 sprintf(sty,"§ÚªºµP[44;1m%s[m..¹q¸£ªºµP[44;1m%s[m",
  style[my-1],style[cpu-1]);
}

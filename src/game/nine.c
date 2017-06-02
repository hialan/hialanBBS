#include "bbs.h"

/********************************/
/* BBS ¯¸¤º¹CÀ¸¡V¤Ñ¦a¤E¤E       */
/*                     11/26/98 */
/* dsyan.bbs@Forever.twbbs.org  */
/********************************/

#undef  NINE_DEBUG

//               0  1  2  3  4  5  6  7  8  9 10 11 12 /* ¹q¸£ AI ©Ò¦b */
// char cp[13]={ 9, 8, 7, 6, 3, 2, 1, 0,11, 5, 4,10,12};
char tb[13]=  { 7, 6, 5, 4,10, 9, 3, 2, 1, 0,11, 8,12};
char *tu[4] = {"¡õ","¡÷","¡ô","¡ö"};
char card[52],hand[4][5],now,dir,turn,live;
char buf[255];
int sum;

static void show_money(int m)
{
  move(19,0);
  clrtoeol();
  prints("[1;37;44m§A²{¦³²{ª÷: [36m%-18d[37m©ãª`ª÷ÃB: [36m%-20d[m",
  	cuser.silvermoney,m);
}

static int forqp(char *a, char *b)
{
  return tb[(*a)%13]-tb[(*b)%13];
}

static void show_mycard(char t)
{
  char i;
#ifdef NINE_DEBUG
  char j;
#endif
  char *suit[4] = {"¢Ñ","¢Ò","¢Ö","¢á"};
  char *num[13] = {"¢Ù","¢Ï","¢±","¢²","¢³","¢´","¢µ",
                   "¢¶","¢·","¢¸","10","¢Ø","¢ß"};
  char coorx[4] = {30,38,30,22};
  char coory[4] = {8,6,4,6};

#ifdef NINE_DEBUG
  move(22,0);
  for(i=3;i>0;i--)
  {
   if(hand[i][0]==-1) continue;
   qsort(hand[i],5,sizeof(char),forqp);
   for(j=0;j<5;j++)
    prints(num[hand[i][j]%13]);
   prints("  ");
  }
#endif

  if(t==100)
  {
   for(i=0;i<5;i++)
   {
    move(16, 30+i*4); prints(num[hand[0][i]%13]);
    move(17, 30+i*4); prints(suit[hand[0][i]/13]);
   }
   return;
  }

  move(coory[turn], coorx[turn]);
  outs("¢~¢w¢w¢w¢¡");
  move(coory[turn]+1,coorx[turn]);
  prints("¢x%s    ¢x",num[t%13]);
  move(coory[turn]+2,coorx[turn]);
  prints("¢x%s    ¢x",suit[t/13]);
  move(coory[turn]+3,coorx[turn]);
  outs("¢x      ¢x");
  move(coory[turn]+4,coorx[turn]);
  outs("¢x      ¢x");
//  prints("¢x    %s¢x",num[t%13]);
  move(coory[turn]+5,coorx[turn]);
  outs("¢x      ¢x");
//  prints("¢x    %s¢x",suit[t/13]);
  move(coory[turn]+6, coorx[turn]);
  outs("¢¢¢w¢w¢w¢£");

  move(8,50); 
  prints("%s  %s",dir==1?"¡ú":"¡ù",dir==1?"¡ø":"¡û");
  move(10,50); 
  prints("%s  %s",dir==1?"¡û":"¡ø",dir==1?"¡ù":"¡ú");

  move(13,46); 
  prints("ÂI¼Æ¡G%c%c%c%c",(now/10)?162:32,(now/10)?(now/10+175):32,162,now%10+175);
  move(14,46); 
  prints("±i¼Æ¡G%d",sum);
  refresh();
  sleep(1);
  move(21,0); 
  clrtoeol(); 
  refresh();
  outs("[33;1m (¡ö)(¡÷)§ïÅÜ¿ïµP  (¡õ)¬d¸ß¸ÓµP§@¥Î (SPCAE)(Enter)¥´µP [m");
}

static void ten_or_twenty(int t)
{
  if(now<t-1) now+=t;
  else if(now>99-t) now-=t;
  else
  {
    int tmp=0;
    move(21,0); clrtoeol();
    prints("(¡ö)(+)¥[%d  (¡÷)(-)´î%d  ",t,t);

    while(!tmp)
    {
      if(turn) 
        tmp=random()%2+5;
      else 
        tmp=igetkey();
      switch(tmp)
      {
        case KEY_LEFT: 
        case '+': 
        case 5:
          now+=t; 
          prints("[32;1m¥[ %d[m",t); 
          break;
        case KEY_RIGHT: 
        case '-': 
        case 6:
          now-=t; 
          prints("[32;1m´î %d[m",t); 
          break;
        default:
         tmp=0;
      }
    }
  }
}

static int add(char *t)
{
  int k=0,change=0;

  switch(*t%13)
  {
   case 4:  //4 °jÂà
     dir=-dir;
     break;

   case 5:  //5 «ü©w
     move(21,0); clrtoeol();

     prints("«ü©w¨º¤@®a¡H ");
     for(change=3;change>=0;change--)
       if(turn!=change && hand[change][0]!=-1)
         prints("(%s) ",tu[change]);

     change=0;

     while(!change)
     {
       int tmp;
       if(turn || live==1) tmp=random()%4+3;
         else tmp=igetkey();

       if((turn!=3 && hand[3][0]!=-1) && (tmp==KEY_LEFT || tmp==6) )
         change=6;
       else if((turn!=2 && hand[2][0]!=-1) && (tmp==KEY_UP || tmp==5) )
         change=5;
       else if((turn!=1 && hand[1][0]!=-1) && (tmp==KEY_RIGHT || tmp==4) )
         change=4;
       else if((turn!=0 && hand[0][0]!=-1) && tmp==3)
         change=3;
     }

     prints("[32;1m(%s)[m",tu[change-3]);
     break;

   case 10: //10 ¥[©Î´î10
     ten_or_twenty(10);
     break;

   case 11: //J PASS
     break;

   case 12: //Q ¥[©Î´î20
     ten_or_twenty(20);
     break;

   case 0:  //K °¨¤WÅÜ99
     now=99;
     break;

   default:
     if((now+(*t%13))>99) 
       return -1;
     else 
       now+=(*t%13);
     break;
   }

   refresh();
   show_mycard(*t);
   while(-1)
   {
     k=random()%52;
     if(card[k]==1 && card[k]!=0)
     {
       card[*t]=3; *t=k; card[k]=0;
       break;
     }
     else 
       card[k]--;
   }

   while(-1)
   {
     if(change)
     { 
       turn=change-3; 
       break;
     }
     else
     {
       turn=(turn+4+dir)%4;
       if(hand[turn][0]>-1) break;
     }
   }
   return 0;
}

static void cpu_die()
{
  switch(turn)
  {
    case 1: 
      move(9,55); 
      break;
    case 2: 
      move(7,52); 
      break;
    case 3: 
      move(9,49); 
      break;
  }
  outs("  ");
  live--;
  pressanykey_old("¹q¸£ %d Ãz¤F!!! .. :DD",turn);
  hand[turn][0]=-1;
  while(-1)
  {
    turn=(turn+4+dir)%4;
    if(hand[turn][0]>-1) break;
  }
}

static void nhelp(int t)
{
  t%=13;
  switch(t)
  {
    case 0: 
      pressanykey_old("¤E¤E¡GÂI¼Æ°¨¤WÅÜ¦¨¢¸¢¸"); 
      break;
    case 4: 
      pressanykey_old("°jÂà¡G¹CÀ¸¶i¦æ¤è¦V¬Û¤Ï"); 
      break;
    case 5: 
      pressanykey_old("«ü©w¡G¦Û¥Ñ«ü©w¤U¤@­Óª±®a"); 
      break;
    case 11: 
      pressanykey_old("PASS¡G¥iPASS¤@¦¸"); 
      break;
    case 10: 
      pressanykey_old("ÂI¼Æ¥[©Î´î 10"); 
      break;
    case 12: 
      pressanykey_old("ÂI¼Æ¥[©Î´î 20"); 
      break;
    default:
      pressanykey_old("ÂI¼Æ¥[ %d",t);
      break;
  }
}

static void mmsg()
{
  char *msg[12]={
	"³Þ¡K¥´§Ö¤@ÂI°Õ¡I",
	"¥´µP±M¤ß¤@ÂI¡A¤£­n¬y¤f¤ô¡K",
	"¤£­n¬Ý¬ü¬Ü¡K",
	"¤j·Ý¦æ¦æ¦n¡A©ñ¹L§Ú§a¡K",
	"±µ©Û¨ü¦º§a¡I¡I¡I",
	"«z¡A§A¦n¼F®`³á¡I³ºµM¯à¥´¥X³o±iµP¡I",
	"§A¤@©w¬O½ä®{Âà¥@ªº¡I",
	"last hand¡K",
	"¦Aª±¹qÃz§A¡I",
	"«z¡K§A¦³¯Ý¦Ù«¨..",
	"§Aªº¨{¤l¶]¥X¨Ó¤F«¨¡K",
	"dsyan¬O¦n¤H¡KSYSOP¬O«Ó­ô¡K"};

  move(21,0); 
  clrtoeol(); 
  refresh();
  prints("[%d;1m%s[m",random()%7+31,msg[random()%12]);
}

int p_nine()
{
 char genbuf[200];
 int bet,i,j,k,tmp,x,xx;
 FILE *fs;
 more("game/99.welcome",YEA);
 while(-1)
 {
  setutmpmode(NINE);
  showtitle("¤Ñ¦a¤E¤E", BoardName);
  show_money(bet=0);
  do
  {
   getdata(21, 0,"­n¤Uª`¦h¤Ö©O(¤W­­250000)? «ö Enter Â÷¶}>", genbuf, 7, 1, 0);
   bet=atoi(genbuf);
   if(bet<0) bet=0;
  } while(bet>250000);
  if(!bet) return 0;
  if(check_money(bet,SILVER))return 0;

  move(1,0);
  fs=fopen("game/99","r");
  while (fgets(genbuf, 255, fs)) prints(genbuf);
  fclose(fs);

  move(21,0); clrtoeol();
  prints("[33;1m (¡ö)(¡÷)§ïÅÜ¿ïµP  (¡õ)¬d¸ß¸ÓµP§@¥Î (SPCAE)(Enter)¥´µP  [m");
  demoney(bet);
  show_money(bet);

  for(i=0;i<52;i++) card[i]=1;

  for(i=0;i<4;i++)
    for(j=0;j<5;j++)
      while(-1)
      {
        k=random()%52;
        if(card[k])
        {
          hand[i][j]=k;
          card[k]=0;
         break;
        }
      }

  qsort(hand[0],5,sizeof(char),forqp);
  x=xx=now=turn=sum=tmp=0;
  dir=1; live=3;
  show_mycard(100);

  while(-1)
  {
   move(9,52); prints(tu[turn]); refresh();
   sum++;
   if(turn) //¹q¸£
   {
    qsort(hand[turn],5,sizeof(char),forqp);
    for(i=0;i<5;i++)
    {
     tmp=hand[turn][i]%13;
     if(tmp==0 || tmp==4 || tmp==5 || tmp>9) break;
     if(now+tmp<=99 && now+tmp>=0) break;
    }
    if(i<2) if (tmp==0 || tmp==4 || tmp==5 || tmp>9) i+=random()%(5-i);
    if(i==5) cpu_die();
     else add(&(hand[turn][i]));
    if(random()%5==0) mmsg();
    continue;
   }

   if(!live)
   {
    if(sum<25) live=15;
    else if(sum<50) live=10;
    else if(sum<100) live=5;
    else if(sum<150) live=3;
    else if(sum<200) live=2;
    else live=1;
    game_log("NINE", "[32;1m¦b %d ±iµPÄ¹¤F.. :) [m ±o¨ì %d ¤¸",sum,(bet*live));
    inmoney(bet*live);
    pressanykey_old("Ä¹¤F %d ... :D",bet*live);
    break;
   }

   tmp=hand[0][4]%13;
   if(tmp!=0 && tmp!=4 && tmp!=5 && tmp<10 && now+tmp>99)
   {
    pressanykey_old("¶ã¶ã¶ã..³Q¹qÃz¤F!!.. :~");
    game_log("NINE", "[31;1m¦b %d ±iµP³Q¹q¸£¹qÃz±¼¤F.. :~ %d[m %d",sum,live,bet);
    break;
   }

   while(tmp!=13 && tmp!=32) //¤HÃþ
   {
    move(18,xx*4+30); outs("  ");
    move(18,(xx=x)*4+30);

    if(tb[hand[0][x]%13]<7)
    {
     if(hand[0][x]%13+now>99) outs("¡I");
      else outs("¡³");
    }
    else outs("¡¹");

    move(18,x*4+31);
    refresh();

    switch(tmp=igetkey())
    {
#ifdef NINE_DEBUG
     case KEY_UP:
      getdata(22, 0,"§âµP´«¦¨> ", genbuf, 3, 1, 0);
      card[hand[0][x]]=3;
      hand[0][x]=atoi(genbuf);
      card[hand[0][x]]=0;
      qsort(hand[0],5,sizeof(char),forqp);
      show_mycard(100);
      break;
#endif
     case KEY_DOWN:
      nhelp(hand[0][x]);
      break;
     case KEY_LEFT: case 'l':
      x=x?x-1:4;
      break;
     case KEY_RIGHT: case 'r':
      x=(x==4)?0:x+1;
      break;
     case 'q':
      break;
    }
    if(tmp=='q') return;
   }

   move(18,xx*4+30); outs("  ");
   if(add(&(hand[0][x])))
   {
    pressanykey_old("¶ã¶ã¶ã..¥ÕÄêÃz¤F!!.. :~");
    game_log("NINE", "[31;1m¦b %d ±iµP¥ÕÄêÃz¤F.. :~ %d[m %d",sum,live,bet);
    break;
   }
   qsort(hand[0],5,sizeof(char),forqp);
   show_mycard(100);
}}}


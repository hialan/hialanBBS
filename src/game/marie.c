#include "bbs.h"

#define PICTURE BBSHOME"/game/marie"
#define GMONEY BBSHOME"/game/money"

unsigned long int totalmoney;

static int show_m(totalmoney)
{
   static int x[9]={0,0,0,0,0,0,0,0,0},w=0,bet_flag=0;
   int c_flag[10]={1,5,10,20,100,1000,10000,100000,1000000,10000000}; /* ­¿²v */
   int i; 
   int ch;
   int bet = 0;
   
   clear();
   show_file(PICTURE,0,24,ONLY_COLOR);
   move(8,44);
   prints("[1m¥»¾÷»O¤º¦³²{ª÷: %-ld[m",totalmoney);
   move(9,44);
   prints("[1m±z¨­¤W²{¦³»È¨â: %-d¶ê[m",cuser.silvermoney);
   move(10,44);
   prints("[1m¥Ø«e©ãª`ªº­¿²v¬O [46m%6d­¿[m",c_flag[w]);
   move(b_lines-4,0);

   for(i=0;i<9;i++)
      prints("  %5d",x[i]);

   ch=igetkey();
   switch(ch)
   {
   case 'w':
      w=(w+1)%10;
      break;
   case 's':
      if (bet_flag)
      {
         int i,j,seed;
         int g[10]={5,40,30,25,50,20,15,10,2,0};
         int gold=0;
         int ran;
         time_t now = time(0);
         time(&now);
         seed=1000;
         ran=rand()%seed;

         if(ran<=400) /* 2 */
            j=8;
         else if(ran<=520) /* 5 */
            j=0;
         else if(ran<=580) /* 10 */
            j=7;
         else if(ran<=620) /* 15 */
            j=6;
         else if(ran<=650) /* 20 */
            j=5;
         else if(ran<=663) /* 25 */
            j=3;
         else if(ran<=678) /* 30 */
            j=2;
         else if(ran<=686) /* 40 */
            j=1;
         else if(ran<=690) /* 50 */
            j=4;
         else
         {
            move(11,44);
            clrtoeol();
            prints("[1;31mSorry[m:~~~~[1;36m»ÊÁÂ´fÅU[m");
            j=9;
         }

         gold=x[j]*g[j];

         move(14,0);
         clrtoeol();

         for(i=0;i<9+j;i++)
         {
            move(14,5+14*((i+9)%9));
            prints("[0;33m¡´");
            if(j!=9)
            {
              move(14,5+14*((i+10)%9) );
              prints("[1;31m¡´");
            }
         }

         for (i = 0; i < 9;)
         {
           bet += x[i];
           x[i++] = 0;
         }
         
         if(j!=9)
         {
            move(11,44);
            clrtoeol();
            prints("[32;40m§A¥i±o[33;41m %d [32;40m»È¨â[m",gold);
            if(gold > 100000)
              game_log("MARIE", "[1;31mÄ¹¤F %d ¤¸![m",gold);
            inmoney(gold);
            totalmoney-=gold;
         }
         else
         {
//           if(gold > 100000)
             game_log("MARIE", "[1;32m¿é¤F %d ¤¸![m",bet);
         }
         
         bet_flag=0;
         pressanykey(NULL);
         break;
      }
      else
         pressanykey("½Ð¥ý©ãª`!");
      break;

   case 'a':
      if(check_money(9*c_flag[w],SILVER))
        clear();
      else
      {
         demoney(9*c_flag[w]);
         totalmoney += 9*c_flag[w];
         for(i=0;i<=8;i++)
            x[i]+=c_flag[w];
         bet_flag=1;
      }
      break;
   case 'q':
      if(bet_flag)
         pressanykey("±z¤w¸g¤Uª`, ½Ð¥ýª±§¹¦¹§½.");
      else
         return totalmoney;
      break;
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
      i=(ch-'0');
      if(check_money(c_flag[w],SILVER))
         clear();
      else
      {
         demoney(c_flag[w]);
         totalmoney += c_flag[w];
         bet_flag=1;
         x[i-1]+=c_flag[w];
      }
      break;
   default:
     break;
   }
   show_file(PICTURE,0,24,ONLY_COLOR);
   show_m(totalmoney);
}

int mary_m()
{
  FILE *fs;
  unsigned int total=0;

  fs = fopen(GMONEY,"r");
  fscanf(fs,"%d",&totalmoney);
  fclose(fs);
  
  total=cuser.silvermoney;
  setutmpmode(MARIE);
  clear();
  show_file(PICTURE,0,24,ONLY_COLOR);
  totalmoney = show_m(totalmoney);
  
  fs = fopen(GMONEY,"w");
  fprintf(fs,"%d",totalmoney);
  fclose(fs);
  
  return;
}
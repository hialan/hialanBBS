/*
[1;31m¢~¢w¢w[37mBBS ¤p§Þ¥©[31m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[33m³Ì«á§ó·s®É¶¡:[32m08/21 16:35[31m¢w¢w¢¡[m
        [1;36m§Aª¾¹D¶Ü¡H ³o¨Ç¤p§Þ¥©¥i¥HÅý§A§ó»´ÃP¡A§ó´r§Öªº¾C¹C BBS ³á~~[m

       [1;33m¡´[m  ªO¥D¥i«ö [1;31mg[0m ±N¤å±i³]¬°[1;33m¤åºK[0m
[0m           ¤j®a¦b[1;36m¾\Åª¿ï³æ[0m¤U¡A¥u­n«ö [1;31mTAB[0m ´N¥i¥H¶i¤J[1;32m¤åºK¾\Åª¼Ò¦¡[0m¤F
[0m       [1;33m¡´[m  ·Qª¾¹D Board ¤W¬O§_¦³ [1;36m·s¶iªº¤å³¹[0m
[0m           ¥u¶·¥Î [1;31mc[0m Áä´N¥i¥HÅo¡I
[0m       [1;33m¡´[m  ¥i¥H³]©w [1;36m¦n¤Í¦W³æ[0m ¥H¤è«K¬d´M±zªº[1;32m¦n¤Í[0m¬O§_¦b¯¸¤W[40;0m
[0m           ([1;32mT[0m)alk -> ([1;32mO[0m)verride -> [1;31ma[0m
[0m
[1;31m¢¢¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[37m±Ð¾Ç¨Ï¥ÎºëÆF[m[31m¢w¢£[m
[0m
*/

#include <stdio.h>
#include <stdlib.h>
#include "bbs.h"

#define HINTFILE   BBSHOME"/etc/hint"

main(void)
{
   FILE    *hintp;
   char    msg[136];
   int     i, j,k, msgNum;
   struct timeval  timep;
   struct timezone timezp;
   struct tm *ptime;
   time_t now;

   if (!(hintp = fopen(HINTFILE, "r")))
   {
      printf("Can't open %s\n",HINTFILE);
      exit(0);
   }

   fgets(msg, 135, hintp);
   msgNum = atoi(msg);


   k=1;
   time(&now);
   ptime = localtime(&now);
   printf("[1;31m¢~¢w¢w[37mBBS ¤p§Þ¥©[31m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[33m³Ì«á§ó·s®É¶¡:[32m%02d/%02d %02d:%02d[31m¢w¢w¢¡[m\n",
           ptime->tm_mon + 1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min);
   printf("        [1;36m§Aª¾¹D¶Ü¡H ³o¨Ç¤p§Þ¥©¥i¥HÅý§A§ó»´ÃP¡A§ó´r§Öªº¾C¹C BBS ³á~~[m\n\n");
   while ( k <=3 )
   {
      fseek(hintp,0,SEEK_SET);
      fgets(msg, 135, hintp);
      gettimeofday(&timep, &timezp);
      i = (int) timep.tv_usec%(msgNum + 1);
      if (i == msgNum)
         i--;
      j = 0;

      while (j <= i)
      {
          fgets(msg, 135, hintp);
          msg[1] = '\0';
          if (!strncmp(msg,"#",1))
            j++;
      }
      printf("       [1;33m¡´[m",k,j);
      fgets(msg, 135, hintp);
      printf("  %s[0m", msg);
      fgets(msg, 135, hintp);
      printf("           %s[0m", msg);
      k++;
   }
   fclose(hintp);
   printf("\n");
   printf("[1;31m¢¢¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[37m±Ð¾Ç¨Ï¥ÎºëÆF[m[31m¢w¢£[m\n");
}
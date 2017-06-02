/*
 *µo«H¤H: wsyfish.bbs@fpg.m4.ntu.edu.tw (µ¥«Ý¤¤ªº³½³½¯¸ªø), ¬ÝªO: SobVersion
 *¼Ð  ÃD: [util]¯¸¤W¬P®y²Î­p horoscope.c
 *µo«H¯¸: ¤p³½ªºµµ¦âªá¶é (Fri Sep  5 12:52:13 1997)
 *Âà«H¯¸: sob!news.civil.ncku!fpg
 */

/*-------------------------------------------------------*/
/* util/horoscope.c     ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : ¯¸¤W¬P®y²Î­p                                 */
/* create : 97/09/05                                     */
/* update : 97/09/05                                     */
/*-------------------------------------------------------*/

/* ¥»µ{¦¡¥Ñ wsyfish (wsyfish.bbs@fpg.m4.ntu.edu.tw)
   °Ñ¦Ò usage.c ­×§ï§¹¦¨ */

#include "bbs.h"

#define DOTPASSWDS BBSHOME"/.PASSWDS"

struct userec cuser;

main()
{
  int i, j;
  FILE *fp;
  int max, item, maxhoroscope;

  int act[12];

  char *name[13] = {"¨d¦Ï",
                    "ª÷¤û",
                    "Âù¤l",
                    "¥¨ÃÉ",
                    "·à¤l",
                    "³B¤k",
                    "¤Ñ¯¯",
                    "¤ÑÃÈ",
                    "®g¤â",
                    "¼¯½~",
                    "¤ô²~",
                    "Âù³½",
                    ""
                           };
  char    *blk[10] =
  {
      "  ","¢j", "¢k", "¢l", "¢m",
      "¢n","¢o", "¢p", "¢i", "¢i",
  };


  chdir(BBSHOME);
  fp=fopen(DOTPASSWDS, "r");

  memset(act, 0, sizeof(act));
  while((fread(&cuser, sizeof(cuser), 1, fp)) >0 )
  {
    switch (cuser.month)
    {
      case 1:
        if (cuser.day <=19)
          act[9]++;
        else
          act[10]++;
        break;
      case 2:
        if (cuser.day <=18)
          act[10]++;
        else
          act[11]++;
        break;
      case 3:
        if (cuser.day <=20)
          act[11]++;
        else
          act[0]++;
        break;
      case 4:
        if (cuser.day <=19)
          act[0]++;
        else
          act[1]++;
        break;
      case 5:
        if (cuser.day <=20)
          act[1]++;
        else
          act[2]++;
        break;
      case 6:
        if (cuser.day <=21)
          act[2]++;
        else
          act[3]++;
        break;
      case 7:
        if (cuser.day <=22)
          act[3]++;
        else
          act[4]++;
        break;
      case 8:
        if (cuser.day <=22)
          act[4]++;
        else
          act[5]++;
        break;
      case 9:
        if (cuser.day <=22)
          act[5]++;
        else
          act[6]++;
       break;
      case 10:
        if (cuser.day <=23)
          act[6]++;
        else
          act[7]++;
       break;
      case 11:
        if (cuser.day <=22)
          act[7]++;
        else
          act[8]++;
       break;
      case 12:
        if (cuser.day <=21)
          act[8]++;
        else
          act[9]++;
       break;
    }

  }
  fclose(fp);


  for (i = max  = maxhoroscope = 0; i < 12; i++)
  {
    if (act[i] > max)
    {
      max = act[i];
      maxhoroscope = i;
    }
  }

  item = max / 30 + 1;

  if ((fp = fopen("etc/horoscope", "w")) == NULL)
  {
    printf("cann't open etc/horoscope\n");
    return 1;
  }

  for(i = 0;i < 12;i++)
  {
    fprintf(fp,"[1m[44;33m %s®y [m[36m", name[i]);
    for(j = 0; j < act[i]/item; j++)
    {
      fprintf(fp,"%2s",blk[9]);
    }
    /* ¬°¤F­è¦n¤@­¶ */
    if (i != 11)
      fprintf(fp,"%2s [1;37m%d[m\n\n",blk[(act[i] % item) * 10 / item],
            act[i]);
    else
      fprintf(fp,"%2s [1;37m%d[m\n",blk[(act[i] % item) * 10 / item],
            act[i]);
  }
  fclose(fp);
}

/*-------------------------------------------------------*/
/* util/cna.c       ( NCU EduCenter BBS Ver 0.01 )       */
/*-------------------------------------------------------*/
/* target : §Y®É·s»DºK­n©ñ¨ì°ÊºA¬Ýª©                     */
/* ­ì  §@ : ARDA.bbs@NaiveAge.me.ncu.edu.tw              */
/* ­×  §ï : TJSheu.bbs@Education.ncu.edu.tw              */
/*          ¦¹ª©¥»¥[¤W¤@¨Ç°Ñ¼Æ¡A¦¹µ{¦¡§ó¦³¼u©Ê           */
/*-------------------------------------------------------*/
/* ¨Ï¥Î»¡©ú¡G °õ¦æ®É­n¦³¥|­Ó°Ñ¼Æ
   Usage: cna <cna board> <etc/out file> <number>
      <cna board>: ·s»DºK­nªº¬ÝªO
      <etc/out file>: Àx©ñ¦b ~bbs/etc/outfile
      <number>: ­Ë¼Æ²Ä´X½g°_ªº 9 ­Ó·s»DºK­n
                ¨Ò¦p¡G cnanews Cna.toady news1 9
                     --> ¼Æ²Ä¤E½g°_ªº 9 ­Ó·s»DºK­n
*/

#include "bbs.h"

main(int argc, char **argv)
{
    FILE *fp, *fp2;
    char fname[80], dest[80];
    int  i,j,total = 0;
    struct tm *ptime;
    time_t now;
   
    if (argc != 4) {
        printf("Usage: %s <cna board> <etc/out file> <number>\n", argv[0]);
        exit(-1);
    }

    sprintf(fname, BBSHOME "/boards/%s/.DIR", argv[1]);
    sprintf(dest,  BBSHOME "/etc/%s", argv[2]);

    if (fp = fopen(fname, "r"))
    {
        fileheader fhdr;
        while (fread(&fhdr, sizeof(fhdr), 1, fp) == 1)
            total++;
        fclose(fp);
    }

    if (fp = fopen(fname, "r"))
    {
        fileheader fhdr;
        int color=1;
        i = j = 1;
        if ((fp2 = fopen(dest, "w+")) != NULL)
        {
            time(&now);
            ptime = localtime(&now);
            fprintf(fp2, "[1;37m¡º[36m¢w¢w[33m­·¹Ð·s»D§Ö³ø[36m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[31m³Ì«á§ó·s®É¶¡:[37m%02d/%02d %02d:%02d[36m¢w[37m¡º[m\n",
                          ptime->tm_mon + 1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min);
            while ( (fread(&fhdr, sizeof(fhdr), 1, fp) == 1) && j <= 8)
            {
               fhdr.title[50] = 0;
               if ( i >= (total-atoi(argv[3])+1) && j<=9 )
               {
                  fprintf(fp2, "        [1;3%dm  %d  %s[m\n"
                           , color,i,fhdr.title);
                  color = ((color+1)%7)+1;
                  j++;
               }
               i++;
            }
            fprintf(fp2,"[1;37m¡º[36m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[33m±ýª¾¸Ô±¡,½Ð¨£ [31m%s [33mª©[36m¢w[37m¡º[m\n",argv[1]);
            fclose(fp2);
            fclose(fp);
       }
       else
            fprintf(stderr, "`%s` opened error (for read)\n", fname);
    }
}

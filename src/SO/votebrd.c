/* Copyright 1998,1999 NTU CSIE

   You should have received a copy of the GNU General Public License
   along with PttBBS; see the file COPYRIGHT.
   If not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   voteboards' routines
*/
#define TIME_LIMIT	3*60*60   /* ³æ¦ì:min */

#include "bbs.h"
#include<stdarg.h>

extern boardheader *bcache;

void
do_voteboardreply(fileheader *fhdr)
{
   char genbuf[1024];
   char reason[60];
   char fpath[80];
   char oldfpath[80];
   char opnion[10];
   char *ptr;
   FILE *fo, *fp;
   fileheader votefile;
   int len;
   int i, j;
   int fd;
   time_t endtime, now = time(NULL);
   int hastime = 0, check = 1;

   if(cuser.totaltime < TIME_LIMIT)
   {
     pressanykey("¤W¯¸®É¼Æ¤£¨¬ , ¤£±o°Ñ»P³s¸p");
     return;
   }

   log_usies("VB_Reply",NULL);
   clear();

   setbpath(fpath, currboard);
   stampfile(fpath, &votefile);

   setbpath(oldfpath, currboard);

   strcat(oldfpath, "/");
   strcat(oldfpath, fhdr->filename);

   fp = fopen(oldfpath, "r");

   len = strlen(cuser.userid);

   while(fgets(genbuf, sizeof(genbuf), fp))
      {
      if (!strncmp(genbuf, "³s¸pµ²§ô®É¶¡", 12))
         {
         hastime = 1;
         ptr = strchr(genbuf, '(');
         sscanf(ptr+1, "%ld", &endtime);
         if (endtime < now){
            pressanykey("³s¸p®É¶¡¤w¹L");
            fclose(fp);
            return;
         }
      }
      else if (!strncmp(genbuf+4, cuser.userid, len))
       {
         move(5, 10);
         prints("±z¤w¸g³s¸p¹L¥»½g¤F");
         opnion[0] = 'n';
         getdata(7, 0, "­n­×§ï±z¤§«eªº³s¸p¶Ü¡H(Y/N) [N]", opnion, 3,LCECHO,0);
         if (opnion[0] != 'y')
         {
            fclose(fp);
            return;
         }
         fgets(genbuf, sizeof(genbuf), fp);
         strcpy(reason, genbuf + 4);
         break;
      }
      else if (!strncmp(genbuf+27, cuser.email, strlen(cuser.email)))
      {
      	fclose(fp);
      	pressanykey("¬Û¦PªºE-Mail¤w³s¸p¹L, ¤U¦¸½Ð¦­ ^o^");
      	return;
      }
   }
   fclose(fp);

   if((fd = open(oldfpath, O_RDONLY)) == -1)
      return;
   flock(fd, LOCK_EX);

   fo = fopen(fpath, "w");

   if (!fo)
      return;
   i = 0;
   while(fo)
   {
      j = 0;
      do
      {
        if (read(fd, genbuf+j, 1)<=0)
        {
           flock(fd, LOCK_UN);
           close(fd);
           fclose(fo);
           unlink(fpath);
           return;
        }
        j++;
      }
      while(genbuf[j-1] !='\n');
      genbuf[j] = '\0';
      i++;
      if (!strncmp("----------", genbuf, 10))
         break;
      if (i > 3)
         prints(genbuf);
      fprintf(fo, "%s", genbuf);
   }
   if (!hastime)
   {
      now += 7*24*60*60;
      fprintf(fo, "³s¸pµ²§ô®É¶¡: (%ld)%s", now, ctime(&now));
      now -= 7*24*60*60;
   }

   fprintf(fo, "%s", genbuf);

   do{
      clear();
      if (!getdata(18, 0, "½Ð°Ý±z (Y)¤ä«ù (N)¤Ï¹ï ³o­ÓÄ³ÃD [C]¡G", opnion, 3,LCECHO,0))
      {
         flock(fd, LOCK_UN);
         close(fd);
         fclose(fo);
         unlink(fpath);
         return;
      }
   }
   while(opnion[0] != 'y' && opnion[0] != 'n');

   if (!getdata(20, 0, "½Ð°Ý±z»P³o­ÓÄ³ÃDªºÃö«Y©Î³s¸p²z¥Ñ¬°¦ó¡G\n", reason,60, DOECHO,0))
    {
      flock(fd, LOCK_UN);
      close(fd);
      fclose(fo);
      unlink(fpath);
      return;
   }

   i = 0;

   while(fo)
   {
      i++;
      j = 0;
      do
      {
        if (read(fd, genbuf+j, 1)<=0)
        {
           flock(fd, LOCK_UN);
           close(fd);
           fclose(fo);
           unlink(fpath);
           return;
        }
        j++;
      }
      while(genbuf[j-1] !='\n');
      genbuf[j] = '\0';
      if (!strncmp("----------", genbuf, 10))
         break;
      if (genbuf[3] == '.' && strncmp(genbuf+4, cuser.userid, len))
      {
         fprintf(fo, "%3d.%s", i, genbuf+4);
         check = 0;
      }
      else if (check == 0 && genbuf[3] != '.')
      {
         fprintf(fo, "%s", genbuf);
         check = 1;
         i--;
      }
      else
         i--;
   }
   if (opnion[0] == 'y')
   {
      fprintf(fo, "%3d.%-15sE-Mail: %-50s\n", i, cuser.userid, cuser.email);
      fprintf(fo, "    [1;37;40m%s[m\n", reason);
   }
   i = 0;
   fprintf(fo, "%s", genbuf);
   while(fo)
   {
      i++;
      j = 0;
      do{
        if (!read(fd, genbuf+j, 1))
           break;
        j++;
        }
      while(genbuf[j-1] !='\n');
      genbuf[j] = '\0';
      if (j <= 3)
         break;
      if (genbuf[3] == '.' && strncmp(genbuf+4, cuser.userid, len))
      {
         fprintf(fo, "%3d.%s", i, genbuf+4);
         check = 0;
      }
      else if (check == 0 && genbuf[3] != '.')
      {
         fprintf(fo, "%s", genbuf);
         check = 1;
         i--;
      }
      else
         i--;
   }
   if (opnion[0] == 'n')
   {
      fprintf(fo, "%3d.%-15sE-Mail: %-50s\n", i, cuser.userid, cuser.email);
      fprintf(fo, "    [1;37;40m%s[m\n", reason);
   }
      
   flock(fd, LOCK_UN);
   close(fd);
   fclose(fo);
   unlink(oldfpath);
   f_mv(fpath, oldfpath);
}

int do_voteboard(void)
{
    fileheader votefile;
    char topic[100];
    char title[80];
    char genbuf[1024];
    char fpath[80];
    FILE* fp;
    int temp, i;
    time_t now = time(NULL);


    clear();
    if(!(currmode & MODE_POST) || cuser.totaltime < TIME_LIMIT)
    {
        pressanykey("¹ï¤£°_¡A±z¥Ø«eµLªk¦b¦¹µoªí¤å³¹¡I");
        return RC_FULL;
    }
    log_usies("VB_Make");
    move(0, 0);
    clrtobot();
    prints("\n\n\n±z¥¿¦b¨Ï¥Î³s¸p¨t²Î\n");
    prints("¥»³s¸p¨t²Î±N¸ß°Ý±z¤@¨Ç°ÝÃD¡A½Ð¤p¤ß¦^µª¤~¯à¶}©l³s¸p\n");
    prints("¥ô·N´£¥X³s¸p®×ªÌ¡A±N³Q¦C¤J¥»¨t²Î¤£¨üÅwªï¨Ï¥ÎªÌ³á\n\n\n");
    prints("                    ¦¹µ{¦¡­ì§@ªÌ¬°PttªºCharlieL.\n");
    pressanykey(NULL);
    move(0, 0);
    clrtobot();
    prints("(1)¥Ó½Ð·sªO (2)¼o°£ÂÂªO (3)³s¸pªO¥D (4)½}§KªO¥D\n");
    if (!strcmp(currboard, VOTEBOARD))
      prints("(5)³s¸p¤p²Õªø (6)½}§K¤p²Õªø ");
    if (!strcmp(currboard, VOTEBOARD) && HAS_PERM(PERM_SYSOP))
       prints("(7)¯¸¥Á¤½§ë");

    do
    {
      getdata(3, 0, "½Ð¿é¤J³s¸pÃþ§O:", topic, 3, DOECHO,0);
      temp = atoi(topic);
    }while(temp <= 0 && temp >= 9);

    switch(temp)
    {
    case 1:
       do
       {
          if (!getdata(4, 0, "½Ð¿é¤J¬ÝªO­^¤å¦WºÙ¡G", topic, IDLEN+1,DOECHO,0))
             return RC_FULL;
          else if (invalid_brdname(topic))
             outs("¤£¬O¥¿½Tªº¬ÝªO¦WºÙ");
          else if (getbnum(topic) > 0)
             outs("¥»¦WºÙ¤w¸g¦s¦b");
          else
             break;
       }while(temp > 0);
       sprintf(title, "[¥Ó½Ð·sªO] %s", topic);
       sprintf(genbuf, "%s\n\n%s%-15s%s%-5d %s%-5d\n%s%s\n%s", "¥Ó½Ð·sªO", "¥Ó½Ð¤H: ", cuser.userid, "¤W¯¸¦¸¼Æ: ", cuser.numlogins, "µoªí¤å³¹: ", cuser.numposts, "­^¤å¦WºÙ: ", topic, "¤¤¤å¦WºÙ: ");
       if (!getdata(5, 0, "½Ð¿é¤J¬ÝªO¤¤¤å¦WºÙ¡G", topic, 20, DOECHO,0))
          return RC_FULL;
       strcat(genbuf, topic);
       strcat(genbuf, "\n¬ÝªOÃþ§O: ");
       if (!getdata(6, 0, "½Ð¿é¤J¬ÝªOÃþ§O¡G", topic, 20, DOECHO,0))
          return RC_FULL;
       strcat(genbuf, topic);
       strcat(genbuf, "\nªO¥D¦W³æ: ");
       getdata(7, 0, "½Ð¿é¤JªO¥D¦W³æ¡G", topic, IDLEN * 3 + 3, DOECHO,0);
       strcat(genbuf, topic);
       strcat(genbuf, "\n¬O§_ÁôÂÃ: ");
       getdata(8, 0, "¬O§_ÁôÂÃ(Y/N)¡G", topic, 3, DOECHO,0);
       strcat(genbuf, topic);	                                  
       strcat(genbuf, "\n¥Ó½Ð­ì¦]: \n");
       move(9,0);
       outs("½Ð¿é¤J¥Ó½Ð­ì¦](¦Ü¦h¤Q¦æ)¡A­n²M·¡¶ñ¼g¤£µM¤£·|®Ö­ã³á");
       for(i= 9;i<19;i++)
       {
          if (!getdata(i, 0, "¡G", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==9)
          return RC_FULL;
       break;
    case 2:
       do
       {
          if (!getdata(4, 0, "½Ð¿é¤J¬ÝªO­^¤å¦WºÙ¡G", topic, IDLEN+1,DOECHO,0))
             return RC_FULL;
          else if (getbnum(topic) <= 0)
             outs("¥»¦WºÙ¨Ã¤£¦s¦b");
          else
             break;
       }
       while(temp > 0);
       sprintf(title, "[¼o°£ÂÂªO] %s", topic);
       sprintf(genbuf, "%s\n\n%s%s\n","¼o°£ÂÂªO", "­^¤å¦WºÙ: ", topic);
       strcat(genbuf, "\n¼o°£­ì¦]: \n");
       outs("½Ð¿é¤J¼o°£­ì¦](¦Ü¦h¤­¦æ)¡A­n²M·¡¶ñ¼g¤£µM¤£·|®Ö­ã³á");
       for(i= 8;i<13;i++)
       {
          if (!getdata(i, 0, "¡G", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;

       break;
    case 3:
       do
       {
          if (!getdata(4, 0, "½Ð¿é¤J¬ÝªO­^¤å¦WºÙ¡G", topic, IDLEN+1,DOECHO,0))
             return RC_FULL;
          else if (getbnum(topic) <= 0)
             outs("¥»¦WºÙ¨Ã¤£¦s¦b");
          else
             break;
       }
       while(temp > 0);
       
       sprintf(title, "[³s¸pªO¥D] %s", topic);
       sprintf(genbuf, "%s\n\n%s%-15s%s%-5d %s%-5d\n%s%s\n%s", "³s¸pªO¥D", "¥Ó½Ð¤H: ", cuser.userid, "¤W¯¸¦¸¼Æ: ", cuser.numlogins, "µoªí¤å³¹: ", cuser.numposts, "­^¤å¦WºÙ: ", topic,"¥Ó½Ð ID :");       

       if (!getdata(5, 0,"¥Ó½Ð ID :",topic, IDLEN+1,DOECHO,cuser.userid))
         return RC_FULL;
       strcat(genbuf, topic); 
            
       strcat(genbuf, "\n¥Ó½Ð¬F¨£: \n");
       outs("½Ð¿é¤J¥Ó½Ð¬F¨£(¦Ü¦h¤­¦æ)¡A­n²M·¡¶ñ¼g¤£µM¤£·|®Ö­ã³á");
       for(i= 9;i<14;i++)
       {
          if (!getdata(i, 0, "¡G", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==9)
          return RC_FULL;
       break;
    case 4:
       do
       {
          if (!getdata(4, 0, "½Ð¿é¤J¬ÝªO­^¤å¦WºÙ¡G", topic, IDLEN+1,DOECHO,0))
             return RC_FULL;
          else if ((i = getbnum(topic)) <= 0)
             outs("¥»¦WºÙ¨Ã¤£¦s¦b");
          else
             break;
       }
       while(temp > 0);
       sprintf(title, "[½}§KªO¥D] %s", topic);
       sprintf(genbuf, "%s\n\n%s%s\n%s","½}§KªO¥D", "­^¤å¦WºÙ: ", topic, "ªO¥D ID : ");
       do
       {
         if (!getdata(6, 0, "½Ð¿é¤JªO¥DID¡G", topic, IDLEN + 1, DOECHO,0))
            return RC_FULL;
         else if (!userid_is_BM(topic, bcache[i-1].BM))
            outs("¤£¬O¸ÓªOªºªO¥D");
         else
            break;
       }
       while(temp > 0);
       strcat(genbuf, topic);
       strcat(genbuf, "\n½}§K­ì¦]: \n");
       outs("½Ð¿é¤J½}§K­ì¦](¦Ü¦h¤­¦æ)¡A­n²M·¡¶ñ¼g¤£µM¤£·|®Ö­ã³á");
       for(i= 8;i<13;i++){
          if (!getdata(i, 0, "¡G", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    case 5:
       if (!getdata(4, 0, "½Ð¿é¤J¤p²Õ¤¤­^¤å¦WºÙ¡G", topic, 30, DOECHO,0))
          return RC_FULL;
       sprintf(title, "[³s¸p¤p²Õªø] %s", topic);
       sprintf(genbuf, "%s\n\n%s%s\n%s%s\n%s%-5d %s%-5d","³s¸p¤p²Õªø", "¤p²Õ¦WºÙ: ", topic, "¥Ó½Ð ID : ", cuser.userid, "¤W¯¸¦¸¼Æ: ", cuser.numlogins, "µoªí¤å³¹: ", cuser.numposts);
       strcat(genbuf, "\n¥Ó½Ð¬F¨£: \n");
       outs("½Ð¿é¤J¥Ó½Ð¬F¨£(¦Ü¦h¤­¦æ)¡A­n²M·¡¶ñ¼g¤£µM¤£·|®Ö­ã³á");
       for(i= 8;i<13;i++)
       {
          if (!getdata(i, 0, "¡G", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    case 6:

       if (!getdata(4, 0, "½Ð¿é¤J¤p²Õ¤¤­^¤å¦WºÙ¡G", topic, 30, DOECHO,0))
          return RC_FULL;
       sprintf(title, "[½}§K¤p²Õªø] %s", topic);
       sprintf(genbuf, "%s\n\n%s%s\n%s","½}§K¤p²Õªø", "¤p²Õ¦WºÙ: ", topic, "¤p²Õªø ID : ");
       if (!getdata(6, 0, "½Ð¿é¤J¤p²ÕªøID¡G", topic, IDLEN + 1, DOECHO,0))
          return RC_FULL;
       strcat(genbuf, topic);
       strcat(genbuf, "\n½}§K­ì¦]: \n");
       outs("½Ð¿é¤J½}§K­ì¦](¦Ü¦h¤­¦æ)¡A­n²M·¡¶ñ¼g¤£µM¤£·|®Ö­ã³á");
       for(i= 8;i<13;i++)
       {
          if (!getdata(i, 0, "¡G", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    case 7:
       if (!HAS_PERM(PERM_SYSOP))
          return RC_FULL;
       if (!getdata(4, 0, "½Ð¿é¤J¤½§ë¥DÃD¡G", topic, 30, DOECHO,0))
          return RC_FULL;
       sprintf(title, "%s %s", "[¯¸¥Á¤½§ë]", topic);
       sprintf(genbuf, "%s\n\n%s%s\n","¯¸¥Á¤½§ë", "¤½§ë¥DÃD: ", topic);
       strcat(genbuf, "\n¤½§ë­ì¦]: \n");
       outs("½Ð¿é¤J¤½§ë­ì¦](¦Ü¦h¤­¦æ)¡A­n²M·¡¶ñ¼g¤£µM¤£·|®Ö­ã³á");
       for(i= 8;i<13;i++)
       {
          if (!getdata(i, 0, "¡G", topic, 60, DOECHO,0))
             break;
          strcat(genbuf, topic);
          strcat(genbuf, "\n");
       }
       if (i==8)
          return RC_FULL;
       break;
    default:
       return RC_FULL;
    }
    strcat(genbuf, "\n³s¸pµ²§ô®É¶¡: ");
    now += 7*24*60*60;
    sprintf(topic, "(%ld)", now);
    strcat(genbuf, topic);
    strcat(genbuf, ctime(&now));
    now -= 7*24*60*60;
    strcat(genbuf, "\n----------¤ä«ù----------\n");
    strcat(genbuf, "----------¤Ï¹ï----------\n");
    outs("¶}©l³s¸p¹Æ");
    setbpath(fpath, currboard);
    stampfile(fpath, &votefile);

    if (!(fp = fopen(fpath, "w")))
    {
       outs("¶}ÀÉ¥¢±Ñ¡A½Ðµy­Ô­«¨Ó¤@¦¸");
       return RC_FULL;
    }
    fprintf(fp, "%s%s %s%s\n%s%s\n%s%s", "§@ªÌ: ", cuser.userid,
                                         "¬ÝªO: ", currboard,
                                         "¼ÐÃD: ", title,
                                         "®É¶¡: ", ctime(&now));
    fprintf(fp, "%s\n", genbuf);
    fclose(fp);
    strcpy(votefile.owner, cuser.userid);
    strcpy(votefile.title, title);
    votefile.savemode = 'S';
    setbdir(genbuf, currboard);
    rec_add(genbuf, &votefile, sizeof(votefile));
    do_voteboardreply(&votefile);
    return RC_FULL;
}

void
va_do_voteboardreply(va_list pvar)
{
  fileheader *fhdr;
  
  fhdr = va_arg(pvar, fileheader *);
  return do_voteboardreply(fhdr);
}


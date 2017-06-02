/******************/
/* ÂIºq¾÷         */
/* §@ªÌ¡GPtt      */
/* ­×§ï¡Gwildcat  */
/******************/

#include "bbs.h"

#define MAXSONGS 300
#define SONGPATH BBSHOME"/etc/SONGO"
#define SONGBOOK BBSHOME"/etc/SONGBOOK"
#define KTV BBSHOME"/man/boards/KTV"

int
count_songtimes(char *userid)
{
  int unum;
  if(unum = getuser(userid))
  {
    xuser.songtimes--;
    substitute_record(fn_passwd, &xuser, sizeof(userec), unum);
    return xuser.songtimes;
  }
  else 
    return -1;
}

typedef
struct songcmp
{
  char name[100];
  char cname[100];
  long int count;
}
songcmp ;

long int totalcount=0;

int
count_cmp(b, a)
 songcmp *a, *b;
{
    return (a->count - b->count);
}
void
topsong()
{
 more(FN_TOPSONG,YEA);
}
     
int strip_blank(char *cbuf, char *buf)
{
  for(;*buf;buf++)  if(*buf != ' ') *cbuf++=*buf;	  
  *cbuf=0;
}
void
sortsong()
{
  FILE *fo,*fp=fopen(BBSHOME "/" FN_USSONG,"r");
  songcmp songs[MAXSONGS + 1];
  int n;
  char buf[256],cbuf[256];

  memset( songs , 0, sizeof(songs));
  if(!fp) return;
  if(!(fo=fopen(FN_TOPSONG,"w"))) {fclose(fp); return;}

  totalcount=0;
  while(fgets(buf,200,fp))
    {
     strtok(buf,"\n");
     strip_blank(cbuf,buf);
     for(n=0;n<MAXSONGS && songs[n].name[0] ;n++)
        {
          if(!strcmp(songs[n].cname,cbuf)) break;
        }
     strcpy(songs[n].name,buf);
     strcpy(songs[n].cname,cbuf);
     songs[n].count++;
     totalcount++;
    }
  qsort(songs, MAXSONGS, sizeof(songcmp), count_cmp);
  fprintf(fo,
"    [36m¢w¢w[37m¦W¦¸[36m¢w¢w¢w¢w¢w¢w[37mºq"
"  ¦W[36m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[37m¦¸¼Æ[36m"
"¢w¢w[32m¦@%d¦¸[36m¢w¢w[m\n",totalcount);
  for(n=0;n<100 && songs[n].name[0];n++)
    {
        fprintf(fo,"      %5d. %-38.38s %4d [32m[%.2f][m\n",n+1,
           songs[n].name,songs[n].count,(float) songs[n].count/totalcount);
    }
  fclose(fp);
  fclose(fo);
}

char *onlydate(time_t clock)
{
  static char foo[9];
  struct tm *mytm = localtime(&clock);

  strftime(foo, 9, "%D", mytm);
  return (foo);
}

int
ordersong()
{
  char destid[IDLEN+1],buf[256],genbuf[256],filename[256],say[51];
  char receiver[60];
  FILE *fp,*fp1;
  fileheader mail;
  int lines = 0;

  update_data();
  if(cuser.songtimes == 0)
  {
    pressanykey("§AªºÂIºq¦¸¼Æ¥Î§¹¤F³á!!");
    return 0;
  }
  if(lockutmpmode(OSONG)) return 0;
  setutmpmode(OSONG);
  trans_buffer[0] = 0;
  a_menu("ÂIºqºq¥»",SONGBOOK,0);
  if(!trans_buffer[0]) { unlockutmpmode(); return 0; }
  log_usies("OSONG ",NULL);
/*by hialan 2002/03/04  ±NÂIºq¶¶§Ç§ïÅÜ*/
  move(12,0);
  clrtobot();
  sprintf(buf,"¿Ë·Rªº %s Åwªï¨Ï¥Î¯«·µÂIºq¨t²Î\n",cuser.userid);
  outs(buf);
  getdata(13, 0,"­nÂIµ¹½Ö©O:",destid, IDLEN+1, DOECHO,0);

  if(!destid[0]) { unlockutmpmode(); return 0; }

  getdata(14, 0,"·Q­n­n¹ï¥L(¦o)»¡:",say, 51, DOECHO,0);
  sprintf(save_title,"%s:%s",cuser.userid,say);
  getdata(16, 0, "±H¨ì½Öªº«H½c(¥i¥ÎE-mail)?", receiver, 45,LCECHO,destid);

  if(!trans_buffer[0] || !(fp = fopen(trans_buffer, "r"))) 
    {unlockutmpmode(); return 0;}

  strcpy(filename, SONGPATH);
  stampfile(filename, &mail);
  unlink(filename);
  if(!(fp1 = fopen(filename, "w"))) 
  {
    fclose(fp); 
    unlockutmpmode(); 
    return;
  }

  strcpy(mail.owner, "ÂIºq¾÷");
  sprintf(mail.title,"¡º %s ÂIµ¹ %s ",cuser.userid,destid);
  mail.savemode = 0;

  while (lines++ <= 11)
  {
     char *po;

     if(!fgets(buf,256,fp)) strcpy(buf,"\n");
     while (po = strstr(buf, "<~Src~>"))
        {
           po[0] = 0;
           sprintf(genbuf,"%s%s%s",buf,cuser.userid,po+7);
           strcpy(buf,genbuf);
        }

     while (po = strstr(buf, "<From>"))
        {
           po[0] = 0;
           sprintf(genbuf,"%s%s%s",buf,cuser.userid,po+7);
           strcpy(buf,genbuf);
        }

     while (po = strstr(buf, "<~Des~>"))
        {
           po[0] = 0;
           sprintf(genbuf,"%s%s%s",buf,destid,po+7);
           strcpy(buf,genbuf);
        }

     while (po = strstr(buf, "<To>"))
        {
           po[0] = 0;
           sprintf(genbuf,"%s%s%s",buf,destid,po+7);
           strcpy(buf,genbuf);
        }

     while (po = strstr(buf, "<~Say~>"))
        {
           po[0] = 0;
           sprintf(genbuf,"%s%s%s",buf,say,po+7);
           strcpy(buf,genbuf);
        }

     while (po = strstr(buf, "<Say>"))
        {
           po[0] = 0;
           sprintf(genbuf,"%s%s%s",buf,say,po+7);
           strcpy(buf,genbuf);
        }
        
     fputs(buf,fp1);
   }
  fclose(fp1);
  fclose(fp);

  if(rec_add(SONGPATH"/.DIR", &mail, sizeof(mail))==-1)
  {
    pressanykey("ÂIºq¥¢±Ñ!!");
    return 0;
  }
  strcpy(mail.owner, "ÂIºq¾÷");
  sprintf(save_title,"%s:%s",cuser.userid,say);
  hold_mail(filename,destid);
  if(receiver[0])
    bbs_sendmail(filename, save_title, receiver, NULL);
  clear();
  outs(
   "\n\n  ®¥³ß±zÂIºq§¹¦¨Åo..\n"
       "  ¤@¤p®É¤º°ÊºA¬ÝªO·|¦Û°Ê­«·s§ó·s\n"
       "  ¤j®a´N¥i¥H¬Ý¨ì±zÂIªººqÅo\n\n"
       "  ÂIºq¦³¥ô¦ó°ÝÃD¥i¥H¨ìNoteªOªººëµØ°Ï§äµª®×\n"
       "  ¤]¥i¦bNoteªOºëµØ°Ï¬Ý¨ì¦Û¤vªºÂIºq°O¿ý\n"
       "  ¦³¥ô¦ó«O¶Qªº·N¨£¤]Åwªï¨ìNoteªO¯d¸Ü\n"
       "  Åý¿Ë¤Áªº¯¸ªø¬°±zªA°È\n");
  pressanykey_old(NULL);
  sortsong();
  topsong();
  unlockutmpmode();
  sprintf(buf, "§AÁÙ¥i¥HÂI %d ­ººq", count_songtimes(cuser.userid));
  pressanykey(buf);
  return 1;
}

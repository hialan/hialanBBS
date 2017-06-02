/*-------------------------------------------------------*/
/* msg.c        ( WD_hialan BBS    Ver 1.00 )            */
/*-------------------------------------------------------*/
/* target : ¤ô²y°T®§                                     */
/* create : 2003/01/21                                   */
/* update : 2003/01/21                                   */
/* change : hialan					 */
/*-------------------------------------------------------*/

#include "bbs.h"

char last_return_msg[128] = " §AÁÙ¨S¦³¥á¹L¤ô²yËç !!";  /*³Ì«á¤@¥y¤ô²y¦^ÅU by hialan*/
char watermode=0;
char no_oldmsg=0,oldmsg_count=0;            /* pointer */
msgque oldmsg[MAX_REVIEW];   /* ¥á¹L¥hªº¤ô²y */

extern struct UTMPFILE *utmpshm;

int cmpuids(int ,user_info *);
int cmppids(pid_t, user_info *);


void
do_aloha(char *hello)
{
   int fd;
   PAL pal;
   char genbuf[200];

   setuserfile(genbuf, FN_ALOHA);
   if ((fd = open(genbuf, O_RDONLY)) > 0)
   {
      user_info *uentp;
      int tuid;   
      
      sprintf(genbuf + 1, hello);
      *genbuf = 1;
      while (read(fd, &pal, sizeof(pal)) == sizeof(pal)) 
      {
         if ( (tuid = searchuser(pal.userid))  && tuid != usernum &&
             (uentp = (user_info *) search_ulistn(cmpuids, tuid, 1)) &&
             ((uentp->userlevel & PERM_SYSOP) || ((!currutmp->invisible || 
           uentp->userlevel & PERM_SEECLOAK) && !(is_rejected(uentp) & 1))))    
            my_write(uentp->pid, genbuf);
      }
      close(fd);
   }
}

void
show_last_call_in()
{
   char buf[200];
   
   sprintf(buf, "[1m[33;41m[[37m%s[33m][m[34;47m %s [m",
      currutmp->msgs[0].last_userid,
      currutmp->msgs[0].last_call_in);

   move(b_lines, 0);
   clrtoeol();
   refresh();
   outs(buf);
}

int
my_write(pid, hint)
  pid_t pid;
  char *hint;
{
  int len, a;
  int currstat0 = currstat;  
  char msg[80], genbuf[200];
  char c0 = currutmp->chatid[0];  
  FILE *fp;
  struct tm *ptime;
  time_t now;
  user_info *uin ;
  uschar mode0 = currutmp->mode;

  if(watermode > 0)
  {
     a = (no_oldmsg - watermode + MAX_REVIEW )%MAX_REVIEW;
     uin = (user_info*)search_ulist(cmppids, oldmsg[a].last_pid);
  }
  else
     uin = (user_info*)search_ulist(cmppids, pid);

  if (( !oldmsg_count || !isprint2(*hint)) && !uin )
  {
     pressanykey("ÁV¿|! ¹ï¤è¤w¸¨¶]¤F(¤£¦b¯¸¤W)! ~>_<~");
     watermode = -1;
     return 0;
  }

  currutmp->mode = 0;
  currutmp->chatid[0] = 3;
  currstat = XMODE;


  time(&now);
  ptime = localtime(&now);

  if (isprint2(*hint))
  {
    if (!(len = getdata(0, 0, hint, msg, 65, DOECHO,0))) 
    {
      pressanykey("ºâ¤F! ©ñ§A¤@°¨...");
      currutmp->chatid[0] = c0;
      currutmp->mode = mode0;
      currstat = currstat0;
      watermode = -1;
      return 0;
  }
/* Ptt */
    if(watermode > 0)
    {
      a = (no_oldmsg - watermode + MAX_REVIEW )%MAX_REVIEW;
      uin = (user_info*)search_ulist(cmppids, oldmsg[a].last_pid);
    }

    strip_ansi(msg,msg,0);
    if (!uin  || !*uin->userid) {
       pressanykey("ÁV¿|! ¹ï¤è¤w¸¨¶]¤F(¤£¦b¯¸¤W)! ~>_<~");
       currutmp->chatid[0] = c0;
       currutmp->mode = mode0;
       currstat = currstat0;
       watermode = -1;
       return 0;
    }

    watermode = -1;
    sprintf(genbuf, "¥á%s¤Ñ­µ:%.40s....? ", uin->userid, msg);
    if (getans2(0, 0, genbuf, 0, 2, 'y') == 'n') 
    {
      currutmp->chatid[0] = c0;
      currutmp->mode = mode0;
      currstat = currstat0;
      return 0;
    }
    if (!uin || !*uin->userid) 
    {
       pressanykey("ÁV¿|! ¹ï¤è¤w¸¨¶]¤F(¤£¦b¯¸¤W)! ~>_<~");
       currutmp->chatid[0] = c0;
       currutmp->mode = mode0;
       currstat = currstat0;
       return 0;
    }
  }
  else 
  {
     strcpy(msg, hint + 1);
     strip_ansi(msg,msg,0);
     len = strlen(msg);
     watermode = -1;
  }
   now = time(0);
   if (*hint != 1) 
   {
      sethomefile(genbuf, uin->userid, fn_writelog);
      if (fp = fopen(genbuf, "a")) 
      {
        fprintf(fp, COLOR2"[1;33;41m¡i[37m %s [33m¡j[1;47;34m %s %s [0m[%s]\n",
          cuser.userid, (*hint == 2) ? "[1;33;42m¼s¼½" : "", msg, Cdatelite(&now));
        fclose(fp);
      }
      sethomefile(genbuf, cuser.userid, fn_writelog);
      if (fp = fopen(genbuf, "a")) 
      {
        fprintf(fp, "To %s: %s [%s]\n", uin->userid, msg, Cdatelite(&now));
        fclose(fp);
        update_data();
        ++cuser.sendmsg;
        substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
      }
/* itoc.011104: for BMW */
      {
        fileheader bmw;
        time_t now = time(0);
        struct tm *ptime = localtime(&now);
        
        sprintf(bmw.date, "%02d:%02d", ptime->tm_hour, ptime->tm_min);
        strcpy(bmw.title, msg);
                                                                                
        bmw.savemode = 1;             /* ¹ï¤è¬O±µ¦¬ºÝ */
        strcpy(bmw.owner, cuser.userid);
        sethomefile(genbuf, uin->userid, FN_BMW);
        rec_add(genbuf, &bmw, sizeof(fileheader));
                                                                                
        bmw.savemode = 0;             /* §Ú¬O¶Ç°eºÝ */
        strcpy(bmw.owner, uin->userid);
        sethomefile(genbuf, cuser.userid, FN_BMW);
        rec_add(genbuf, &bmw, sizeof(fileheader));
      }
      
/* hialan.020713 for ³Ì«á¤@¥y¸Ü¤ô²y¦^ÅU*/
      sprintf(last_return_msg, "\033[m µ¹ %s \033[1;33;44m %s \033[m", uin->userid, msg);
   }
   if (*hint == 2 && uin->msgcount) 
   {
      uin->destuip = currutmp;
      uin->sig = 2;
      kill(uin->pid, SIGUSR1);
   }
   else if (*hint != 1 && !HAS_PERM(PERM_SYSOP) && ( uin->pager == 3
       || uin->pager == 2 || (uin->pager == 4 && !(is_friend(uin) & 2)) ))
      pressanykey("ÁV¿|! ¹ï¤è¨¾¤ô¤F!");
   else {
//      if (uin->msgcount < MAXMSGS) 
      {
         uschar pager0 = uin->pager;
         uin->msgcount=0;
         uin->pager = 2;
         uin->msgs[uin->msgcount].last_pid = currpid;
         strcpy(uin->msgs[uin->msgcount].last_userid, currutmp->userid);
         strcpy(uin->msgs[uin->msgcount++].last_call_in, msg);
         uin->pager = pager0;
      }
      if (uin->msgcount  == 1 && kill(uin->pid, SIGUSR2) == -1 && *hint != 1)
         pressanykey("ÁV¿|! ¨S¥´¤¤! ~>_<~");
      else if (uin->msgcount == 1 && *hint != 1)
         outz("[1m[44m¤Ñ­µ¶Ç¤W¥h¤F! *^o^Y[m");
   }
//   clrtoeol();
//   refresh();

   currutmp->chatid[0] = c0;
   currutmp->mode = mode0;
   currstat = currstat0;
   return 1;
}

static char t_display_new_flag =0;

int
t_display_new(int b_f_flag)
{
   int i, j;  /* j:¤w¸g¤U¯¸ªº¨Ï¥ÎªÌ*/
   char buf[256];
   user_info *uin;

   if(t_display_new_flag) return;

   else t_display_new_flag = 1;

   if(oldmsg_count && watermode > 0)
     {
         clrchyiuan(1, oldmsg_count + 5);
         move(1,0);
         clrtoeol();
         outs(
" [1;34m¢w¢w¢w¢w¢w¢w¢w[37m¤ô[34m¢w[37m²y[34m¢w[37m¦^[34m¢w[37mÅU[34m¢w¢w¢w¢w¢w¢w¢w¢w¢w"COLOR1" [Ctrl-R]©¹¤U¤Á´« [34;40m¢w¢w¢w¢w¢w¢w [m");
         for(i=0 ; i < oldmsg_count ;i++)
                {
                 int a = (no_oldmsg - i - 1 + MAX_REVIEW )%MAX_REVIEW;
                 
                 uin = (user_info*)search_ulist(cmppids, oldmsg[a].last_pid);
                 move(i+2,0);
                 clrtoeol();
                 if(i == 0) j = 0;
                 if(watermode-1 == i)
                 {
                   if(!uin)
                   {
                     if (!b_f_flag)
                       watermode = (watermode + oldmsg_count)%oldmsg_count+1;
                     else
                       watermode = (watermode+2*oldmsg_count-2)%oldmsg_count+1;
                     j++;
                     if (j != oldmsg_count) i = -1;
                   }
                   else
                     sprintf(buf,"> \033[1m[37;45m%s [33;44m%s[m",
                         oldmsg[a].last_userid,oldmsg[a].last_call_in);
		 }
		 else
		 {
                   if (!uin) j++;
                   sprintf(buf,"%s %s \033[1m[33;44m%s[m,%d",
                         (!uin) ? "[31mX":"[m ",
                         oldmsg[a].last_userid,oldmsg[a].last_call_in,a);
                 }
                 outs(buf);  /*¥Î prints ·|³y¦¨¨Ï¥ÎªÌ³Q½ð hialan.020717*/
               }
	  refresh();
	  move(i+2,0);
	  outs(
" [1;34m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[m ");	 
          move(i+3,0);
          outs(last_return_msg);
          move(i+4,0);
          clrtoeol();
          outs(
" [1;34m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w"COLOR1" [Ctrl-T]©¹¤W¤Á´« [40;34m¢w¢w¢w¢w¢w¢w[m ");
     }
  t_display_new_flag =0;
  
  return j;
}

int
talk_mail2user()
{
  char fname[128];

  setuserfile(fname, fn_writelog);
  mail2user(cuser.userid, "¼ö½u\033[37;41m°O¿ý\033[m", fname, FILE_READ);
  unlink(fname);
  
  /* itoc.011104: delete BMW */
  sethomefile(fname, cuser.userid, FN_BMW);
  unlink(fname);
  
  return 0;
}

int
t_display()
{
  char genbuf[64];

  setuserfile(genbuf, fn_writelog);

  if (more(genbuf, YEA) != -1)
  {
    char *choose[3] = {"cC.²M°£","mM.²¾¦Ü³Æ§Ñ¿ý","rR.«O¯d"};
    
    /* add by hialan 20020519  ¤ô²y®e¶q¤p©ó200k */
       
       char fpath[80];
       struct stat st;
       sethomefile(fpath,cuser.userid,"writelog");
       if (stat(fpath, &st) == 0 && st.st_size > 200000)
       {
         pressanykey("§Aªº¤ô²y¦û¥Î®e¶q:%d byte !!",st.st_size);
         pressanykey("¤ô²y«O¯dªº®e¶q¤£±o¶W¹L200K!¨t²Î¦Û°ÊÂà¦s¨ì«H½c¡I");
         
         talk_mail2user();
       }
    /* add end */
       else
       {
         switch (getans2(b_lines, 0, "", choose, 3, 'r'))
         {
         case 'm':
           talk_mail2user();
           break; 
           
         case 'c':
           unlink(genbuf);
          
           /* itoc.011104: delete BMW */
           sethomefile(genbuf, cuser.userid, FN_BMW);
           unlink(genbuf);

         default:
           break;
         }
       }
      return RC_FULL;
  }
  return RC_NONE;
}



/* itoc.011104: for BMW */
int
bmw_refresh()
{
  return RC_FULL;
}

extern int write_msg();  //bbs.c

struct one_key bmwlist_key[]={
'w' , write_msg, PERM_LOGINOK, "¥á¤ô²y",0,
's' , bmw_refresh,       0, "§ó·sµe­±",0,
'\0', NULL, 0, NULL,0};

void
bmwtitle()
{
  char buf[256];
  
  sprintf(buf,"%s [½u¤W %d ¤H]",BOARDNAME, count_ulist());
  showtitle("¤ô²y¦^ÅU", buf);

  prints("[¡ô/¡õ]¤W¤U [PgUp/PgDn]¤W¤U­¶ [Home/End]­º§À [¡ö][q]Â÷¶}\n");
  prints("\033[1;37;46m ½s¸¹ ¥N ¸¹       ¤ß  ±¡  ¬G  ¨Æ"
         "                                         ®É¶¡ \033[m\n");
}

void
bmw_lightbar(num, bmw, row, bar, barcolor)
   fileheader *bmw;
   int bar, row, num;
   char *barcolor;
{
  move(row, 0);
  clrtoeol();
  prints("%4d %s%-13s%s%-55.55s\033[m%s",
       num, (barcolor) ? barcolor : (bmw->savemode) ? "\033[33m" : "\033[m", 
       bmw->owner, (bmw->savemode) ? "\033[0;33m" : "\033[m", 
       bmw->title, bmw->date);
}

                                                                                
int
t_bmw()
{
  char fname[80];
  char *choose[3] = {"cC.²M°£","mM.²¾¦Ü³Æ§Ñ¿ý","rR.«O¯d"};  
  
  sethomefile(fname, cuser.userid, FN_BMW);
  if(dashf(fname))
  {
    i_read(SEEWATER, fname, bmwtitle, bmw_lightbar, bmwlist_key, NULL);
  
    switch (getans2(b_lines, 0, "", choose, 3, 'r'))
    {
      case 'm':
        talk_mail2user();
        break;            
    
      case 'c':
        unlink(fname);
           
        /* itoc.011104: delete BMW */
        sethomefile(fname, cuser.userid, fn_writelog);
        unlink(fname);

      default:
        break;
    }
  }
  else
    pressanykey("±z©|¥¼¦¬¨ì¤ô²y³á!!");
  
  return RC_FULL;
}

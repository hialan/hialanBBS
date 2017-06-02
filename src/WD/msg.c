/*-------------------------------------------------------*/
/* msg.c        ( NTHU CS MapleBBS Ver 2.36 )            */
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
    if (!(len = getdata(0, 0, hint, msg, 65, DOECHO,0))) {
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
        BMW bmw;
                                                                                 
        time(&bmw.chrono);
        strcpy(bmw.msg, msg);
                                                                                
        bmw.recv = 1;             /* ¹ï¤è¬O±µ¦¬ºÝ */
        strcpy(bmw.userid, cuser.userid);
        sethomefile(genbuf, uin->userid, FN_BMW);
        rec_add(genbuf, &bmw, sizeof(BMW));
                                                                                
        bmw.recv = 0;             /* §Ú¬O¶Ç°eºÝ */
        strcpy(bmw.userid, uin->userid);
        sethomefile(genbuf, cuser.userid, FN_BMW);
        rec_add(genbuf, &bmw, sizeof(BMW));
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
           /* shakalaca.000814: ¤£¥Î break ¬O¦]¬° mail2user() ¥Î¤F f_cp, 
              ©Ò¥H±µµÛ case 'c' ±N­ì©lÀÉ®× unlink */
              
           /*hialan.020702¤]¶¶«K±N BMW ÀÉ®× unlink*/

           break; /*talk_mail2user() ¤@¤f®ð¥þ³¡·F±¼!! ©Ò¥H¥Îbreak*/
           
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
                                                                                
#define XO_TALL 20

void
bmw_lightbar(row, bmw, ch, barcolor)
   BMW bmw;
   int ch, row;
   char *barcolor;
{
  struct tm *ptime;

  ptime = localtime(&bmw.chrono);
  move(row, 0);
  clrtoeol();
  prints("%4d %s%-13s%s%-55.55s\033[m%02d:%02d",
       ch + 1, (barcolor) ? barcolor : (bmw.recv) ? "\033[33m" : "\033[m", 
       bmw.userid, (bmw.recv) ? "\033[0;33m" : "\033[m", bmw.msg,
       ptime->tm_hour, ptime->tm_min);
}
                                                                                
int
t_bmw()
{
  char fpath[128], ans[4], barcolor[50];
  int num, pageno, pagemax, redraw;
  int ch, pos, fd;
  BMW bmw;
  BMW bmwtmp[XO_TALL];
  char *b_line_msg = "\033[1;33;44m ¼ö°T¦^ÅU \033[1;37;46m (C)²M°£ (M)²¾¦Ü³Æ§Ñ¿ý (w)¤ô²y (s)§ó·s (d)§R°£ (¼Æ¦r)¸õ¨ì¸Ó¶µ       \033[m";
                                                                                
  sethomefile(fpath, cuser.userid, FN_BMW);
  
  if(HAS_HABIT(HABIT_LIGHTBAR))
    get_lightbar_color(barcolor);
  else
    *barcolor = 0;
                                                                                
  if (!dashf(fpath))
  {
    pressanykey("§A¨S¦³¦¬¨ì¥ô¦ó¤ô²y³á");
    return RC_NONE;
  }
                                                                                
  pageno = 0;
  pos = 0;
  redraw = 1;
                                                                                
  do
  {
    if (redraw)
    {
      clear();
      sprintf(tmpbuf,"%s [½u¤W %d ¤H]",BOARDNAME, count_ulist());
      showtitle("¤ô²y¦^ÅU", tmpbuf);

      prints("[¡ô/¡õ]¤W¤U [PgUp/PgDn]¤W¤U­¶ [Home/End]­º§À [¡ö][q]Â÷¶}\n");
      prints("\033[1;37;46m ½s¸¹ ¥N ¸¹       ¤ß  ±¡  ¬G  ¨Æ"
        "                                         ®É¶¡ \033[m\n");
                                                                                
      num = rec_num(fpath, sizeof(BMW)) - 1;
      pagemax = num / XO_TALL;
                                                                                
      ch = pageno * XO_TALL; /*¥»­¶²Ä¤@­Ó*//* ­É¥Î ch ¤Î redraw */
      redraw = ch + XO_TALL; /*¥»­¶³Ì«á¤@­Ó*/
                                                                                
      if ((fd = open(fpath, O_RDONLY)) >= 0)
      {
        do
        {
          if (lseek(fd, (off_t) (sizeof(BMW) * ch), SEEK_SET) >= 0)
          {
            if (read(fd, &bmw, sizeof(BMW)) == sizeof(BMW))
            {
              /*move to bmw_lightbar by hialan*/
              bmw_lightbar((ch % XO_TALL) + 3, bmw, ch, 0);
              bmwtmp[ch % XO_TALL] = bmw;  /*add for lightbar*/
              ch++;
              continue;
            }
          }
          break;
        } while (ch < redraw);
                                                                                
        close(fd);
      }
                                                                                
      move(b_lines, 0);
      outs(b_line_msg);
      redraw = 0;
    }
    
    if(*barcolor != 0)
      bmw_lightbar(3 + pos, bmwtmp[pos], (pageno * XO_TALL) + pos, barcolor);
      
    move(3 + pos, 0);
    outc('>');    
    ch = igetkey();

    if(*barcolor != 0)
      bmw_lightbar(3 + pos, bmwtmp[pos], (pageno * XO_TALL) + pos, 0);
      
    move(3 + pos, 0);
    outc(' ');    
    
                                                                                
    switch (ch)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
        int number;
        char buf[6];
                                                                                
        buf[0] = ch;
        buf[1] = '\0';
        getdata(b_lines, 0, "¸õ¦Ü²Ä´X¶µ¡G", buf, sizeof(buf), DOECHO, buf);
        number = atoi(buf) - 1;
        if (number >= 0 && number <= num)
        {
          if (number / XO_TALL == pageno)       /* ¸õ¥h¦P¤@­¶ */
          {
            pos = number % XO_TALL;
          }
          else                                  /* ¸õ¥h¤£¦P­¶ */
          {
            pageno = number / XO_TALL;
            pos = number % XO_TALL;
            redraw = 1;
            break;
          }
        }
        move(b_lines, 0);
        clrtoeol();
        outs(b_line_msg);
      }
      break;

    case 'd':
      rec_del(fpath, sizeof(BMW), pos + pageno * XO_TALL + 1, NULL, NULL);
      redraw = 1;
      if ((pageno == pagemax) && (pos == num % XO_TALL))
      {  /* ­è¦n§R°£³Ì«á¤@­¶ªº³Ì«á¤@­Ó¡A­n¦^¨ì¤W¤@­Ó */
        if (!pos)       /* ­Y¤S­è¦n¬O°ß¤@¤@­Ó¡A­n¦^¨ì¤W¤@­¶ */
        {
          pageno--;
          /* yagami.011106 : bug­×¥¿ */
          if (pageno < 0)
          {
            unlink(fpath);
            sethomefile(fpath, cuser.userid, fn_writelog);
            unlink(fpath);
            return RC_FULL;
          }
          pos = XO_TALL - 1;    /* yagami.011106 : ´å¼Ð¦ì¸m­×¥¿ */
        }
        else
        {
          pos--;
        }
      }
      break;
    
    case 'e':
    case KEY_LEFT:
      ch = 'q';
    case 'q':
      break;
                                                                                
    case KEY_PGUP:
      if (pagemax != 0)
      {
        if (pageno)
        {
          pageno--;
        }
/*
        else
        {
          pageno = pagemax;
          pos = num % XO_TALL;
        }
*/
        redraw = 1;
      }
      break;
    
    case ' ':                                                                            
    case KEY_PGDN:
      if (pagemax != 0)
      {
      //¤£­n´`Àô...@@  by hialan
/*
        if (pageno == pagemax)
        {
          //pageno = 0;
          break;
        }
        else
        {
          pageno++;
          if (pageno == pagemax)
          pos = num % XO_TALL;
        }
        redraw = 1;
*/
        if (pageno != pagemax)
        {
          pageno++;
          redraw = 1;
        }
        
        /*¥[§¹¥H«áÁÙ­n§PÂ_¬O§_¬Û¦P, ©Ò¥H§â¥L¿W¥ß¥X¨ÓÂ\«á­± hialan*/
        if (pageno == pagemax)
          pos = num % XO_TALL;        
      }
      break;
                                                                                
    case KEY_UP:
      if (pos == 0)
      {
        if (pageno != 0)
        {
          pos = XO_TALL - 1;
          pageno = pageno - 1;
        }
        else
        {
          pos = num % XO_TALL;
          pageno = pagemax;
        }
        redraw = 1;
      }
      else
      {
        pos--;
      }
      break;
                                                                                
    case KEY_DOWN:
      if (pos == XO_TALL - 1)
      {
        pos = 0;
        pageno = (pageno == pagemax) ? 0 : pageno + 1;
        redraw = 1;
      }
      else if (pageno == pagemax && pos == num % XO_TALL)
      {
        pos = 0;
        pageno = 0;
        redraw = 1;
      }
      else
      {
        pos++;
      }
      break;
                                                                                
    case KEY_HOME:
    case '0':
        pos = 0;
      break;
                                                                                
    case KEY_END:
    case '$':
      pos = (pageno == pagemax) ? num % XO_TALL : XO_TALL - 1;
      break;
                                                                                
    case 'w':
      if (HAS_PERM(PERM_PAGE))
      {
        int tuid;
        user_info *uentp;
                                                                                
        if (!rec_get(fpath, &bmw, sizeof(BMW), pos + 1 + pageno * XO_TALL) &&
          (tuid = getuser(bmw.userid)) &&
          (uentp = (user_info *) search_ulist(cmpuids, tuid)))
        {
          if ((uentp->pid != currpid) &&
            (HAS_PERM(PERM_SYSOP) || uentp->pager < 3 ||
            (pal_type(uentp->userid, cuser.userid) && uentp->pager == 4)))
          {
            my_write(uentp->pid, "¼ö½u Call-In¡G");
            redraw = 1;
          }
        }
      }
      break;
                                                                                
    case 's':
      redraw = 1;
      break;
                                                                                
    case 'M':
      getdata(b_lines, 0, "±z¬O§_½T©w­n±N¤ô²y°O¿ýÂà¨ì«H½c¤¤¡H[N] ",
        ans, 3, LCECHO, 0);
      if (*ans == 'y')
      {
        talk_mail2user();        
        ch = 'q';
      }
      else
      {
        move(b_lines, 0);
        clrtoeol();
        outs(b_line_msg);          
      }
      break;
                                                                                
    case 'C':
      getdata(b_lines, 0, "±z¬O§_½T©w­n²M°£¥þ³¡ªº¤ô²y°O¿ý¡H[N] ",
        ans, 3, LCECHO, 0);
      if (*ans == 'y')
      {
        unlink(fpath);
        sethomefile(fpath, cuser.userid, fn_writelog);
        unlink(fpath);
        ch = 'q';
      }
      else
      {
        move(b_lines, 0);
        clrtoeol();
        outs(b_line_msg);
      }
      break;
                                                                                
    }
  } while (ch != 'q');
                                                                                
  return RC_FULL;
}

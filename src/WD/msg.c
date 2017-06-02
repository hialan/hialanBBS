/*-------------------------------------------------------*/
/* msg.c        ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : 水球訊息                                     */
/* create : 2003/01/21                                   */
/* update : 2003/01/21                                   */
/* change : hialan					 */
/*-------------------------------------------------------*/

#include "bbs.h"

char last_return_msg[128] = " 你還沒有丟過水球呦 !!";  /*最後一句水球回顧 by hialan*/
char watermode=0;
char no_oldmsg=0,oldmsg_count=0;            /* pointer */
msgque oldmsg[MAX_REVIEW];   /* 丟過去的水球 */

extern struct UTMPFILE *utmpshm;

int cmpuids(int ,user_info *);
int cmppids(pid_t, user_info *);

void
t_aloha()
{
  int i;
  user_info *uentp;
  pid_t pid;
  char buf[100];

  sprintf(buf + 1, "[1;37;41m☆ %s(%s) 上站了! [0m",
    cuser.userid, cuser.username);
  *buf = 0;

  /* Thor: 特別注意, 自己上站不會通知自己... */

  for (i = 0; i < USHM_SIZE; i++) 
  {
    uentp = &utmpshm->uinfo[i];
    if ((pid = uentp->pid) && (kill(pid, 0) != -1) &&
        uentp->pager && (is_friend(uentp) & 2) &&
        strcmp(uentp->userid, cuser.userid))
      my_write(uentp->pid, buf);
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
  int len;
  char msg[80];
  FILE *fp;
  struct tm *ptime;
  time_t now;
  char genbuf[200];
  user_info *uin ;
  extern msgque oldmsg[MAX_REVIEW];
  int a;
  uschar mode0 = currutmp->mode;
  char c0 = currutmp->chatid[0];
  int currstat0 = currstat;

  if(watermode > 0)
  {
     a = (no_oldmsg - watermode + MAX_REVIEW )%MAX_REVIEW;
     uin = (user_info*)search_ulist(cmppids, oldmsg[a].last_pid);
  }
  else
     uin = (user_info*)search_ulist(cmppids, pid);

  if (( !oldmsg_count || !isprint2(*hint)) && !uin )
  {
     pressanykey("糟糕! 對方已落跑了(不在站上)! ~>_<~");
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
    char inputbuf[4];
    
    if (!(len = getdata(0, 0, hint, msg, 65, DOECHO,0))) {
      pressanykey("算了! 放你一馬...");
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
       pressanykey("糟糕! 對方已落跑了(不在站上)! ~>_<~");
       currutmp->chatid[0] = c0;
       currutmp->mode = mode0;
       currstat = currstat0;
       watermode = -1;
       return 0;
    }

    sprintf(genbuf, "丟%s天音:%.40s....? ", uin->userid, msg);
   
    inputbuf[0] = getans2(0, 0, genbuf, 0, 2, 'y');
    genbuf[0] = '\0';
    watermode = -1;
    if (inputbuf[0] == 'n') {
      currutmp->chatid[0] = c0;
      currutmp->mode = mode0;
      currstat = currstat0;
      return 0;
    }
    if (!uin || !*uin->userid) {
       pressanykey("糟糕! 對方已落跑了(不在站上)! ~>_<~");
       currutmp->chatid[0] = c0;
       currutmp->mode = mode0;
       currstat = currstat0;
       return 0;
    }
  }
  else {
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
        fprintf(fp, COLOR2"[1;33;41m【[37m %s [33m】[1;47;34m %s %s [0m[%s]\n",
          cuser.userid, (*hint == 2) ? "[1;33;42m廣播" : "", msg, Cdatelite(&now));
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
                                                                                
        bmw.recv = 1;             /* 對方是接收端 */
        strcpy(bmw.userid, cuser.userid);
        sethomefile(genbuf, uin->userid, FN_BMW);
        rec_add(genbuf, &bmw, sizeof(BMW));
                                                                                
        bmw.recv = 0;             /* 我是傳送端 */
        strcpy(bmw.userid, uin->userid);
        sethomefile(genbuf, cuser.userid, FN_BMW);
        rec_add(genbuf, &bmw, sizeof(BMW));
      }
      
/* hialan.020713 for 最後一句話水球回顧*/
      sprintf(last_return_msg, "\033[m 給 %s \033[1;33;44m", uin->userid);
      strcat(last_return_msg, msg);
      strcat(last_return_msg, "\033[m");
   }
   if (*hint == 2 && uin->msgcount) 
   {
      uin->destuip = currutmp;
      uin->sig = 2;
      kill(uin->pid, SIGUSR1);
   }
   else if (*hint != 1 && !HAS_PERM(PERM_SYSOP) && ( uin->pager == 3
       || uin->pager == 2 || (uin->pager == 4 && !(is_friend(uin) & 2)) ))
      pressanykey("糟糕! 對方防水了!");
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
         pressanykey("糟糕! 沒打中! ~>_<~");
      else if (uin->msgcount == 1 && *hint != 1)
         outz("[1m[44m天音傳上去了! *^o^Y[m");
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
   int i;
   int j;  /*已經下站的使用者*/
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
" [1;34m───────[37m水[34m─[37m球[34m─[37m回[34m─[37m顧[34m─────────"COLOR1" [Ctrl-R]往下切換 [34;40m────── [m");
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
                 outs(buf);  /*用 prints 會造成使用者被踢 hialan.020717*/
               }
	  refresh();
	  move(i+2,0);
	  outs(
" [1;34m──────────────────────────────────────[m ");	 
          move(i+3,0);
          outs(last_return_msg);
          move(i+4,0);
          clrtoeol();
          outs(
" [1;34m───────────────────────"COLOR1" [Ctrl-T]往上切換 [40;34m──────[m ");
     }
  t_display_new_flag =0;
  
  return j;
}

/* Thor: for ask last call-in message */

int
t_display()
{
  char genbuf[64];

  setuserfile(genbuf, fn_writelog);

  if (more(genbuf, YEA) != -1)
  {
    char *choose[3] = {"cC.清除","mM.移至備忘錄","rR.保留"};
    
    /* add by hialan 20020519  水球容量小於200k */
       
       char fpath[80];
       struct stat st;
       sethomefile(fpath,cuser.userid,"writelog");
       if (stat(fpath, &st) == 0 && st.st_size > 200000)
       {
         pressanykey("你的水球佔用容量:%d byte !!",st.st_size);
         pressanykey("水球保留的容量不得超過200K!系統自動轉存到信箱！");
         
         talk_mail2user();
       }
    /* add end */
       else
       {
         switch (getans2(b_lines, 0, "", choose, 3, 'r'))
         {
         case 'm':
           talk_mail2user();
           /* shakalaca.000814: 不用 break 是因為 mail2user() 用了 f_cp, 
              所以接著 case 'c' 將原始檔案 unlink */
              
           /*hialan.020702也順便將 BMW 檔案 unlink*/

           break; /*talk_mail2user() 一口氣全部幹掉!! 所以用break*/
           
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
  char *b_line_msg = "\033[1;33;44m 熱訊回顧 \033[1;37;46m (C)清除 (M)移至備忘錄 (w)水球 (s)更新 (d)刪除 (數字)跳到該項       \033[m";
                                                                                
  sethomefile(fpath, cuser.userid, FN_BMW);
  
  if(HAS_HABIT(HABIT_LIGHTBAR))
    get_lightbar_color(barcolor);
  else
    *barcolor = 0;
                                                                                
  if (!dashf(fpath))
  {
    pressanykey("你沒有收到任何水球喔");
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
      sprintf(tmpbuf,"%s [線上 %d 人]",BOARDNAME, count_ulist());
      showtitle("水球回顧", tmpbuf);

      prints("[↑/↓]上下 [PgUp/PgDn]上下頁 [Home/End]首尾 [←][q]離開\n");
      prints("\033[1;37;46m 編號 代 號       心  情  故  事"
        "                                         時間 \033[m\n");
                                                                                
      num = rec_num(fpath, sizeof(BMW)) - 1;
      pagemax = num / XO_TALL;
                                                                                
      ch = pageno * XO_TALL; /*本頁第一個*//* 借用 ch 及 redraw */
      redraw = ch + XO_TALL; /*本頁最後一個*/
                                                                                
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
        getdata(b_lines, 0, "跳至第幾項：", buf, sizeof(buf), DOECHO, buf);
        number = atoi(buf) - 1;
        if (number >= 0 && number <= num)
        {
          if (number / XO_TALL == pageno)       /* 跳去同一頁 */
          {
            pos = number % XO_TALL;
          }
          else                                  /* 跳去不同頁 */
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
      {  /* 剛好刪除最後一頁的最後一個，要回到上一個 */
        if (!pos)       /* 若又剛好是唯一一個，要回到上一頁 */
        {
          pageno--;
          /* yagami.011106 : bug修正 */
          if (pageno < 0)
          {
            unlink(fpath);
            sethomefile(fpath, cuser.userid, fn_writelog);
            unlink(fpath);
            return RC_FULL;
          }
          pos = XO_TALL - 1;    /* yagami.011106 : 游標位置修正 */
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
      //不要循環...@@  by hialan
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
        
        /*加完以後還要判斷是否相同, 所以把他獨立出來擺後面 hialan*/
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
            my_write(uentp->pid, "熱線 Call-In：");
            redraw = 1;
          }
        }
      }
      break;
                                                                                
    case 's':
      redraw = 1;
      break;
                                                                                
    case 'M':
      getdata(b_lines, 0, "您是否確定要將水球記錄轉到信箱中？[N] ",
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
      getdata(b_lines, 0, "您是否確定要清除全部的水球記錄？[N] ",
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

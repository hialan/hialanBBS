/*-------------------------------------------------------*/
/* menu.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines                     */
/* create : 95/03/29                                     */
/* update : 03/07/12                                     */
/* change : hialan.nation@infor.org		         */
/*-------------------------------------------------------*/


#include "bbs.h"
#include "menu.h"   //所有的 menu struct

/* ------------------------------------- */
/* help & menu processring               */
/* ------------------------------------- */
int refscreen = NA;
extern char *boardprefix;

void showtitle(char *title, char *mid)
{
  char buf[40];
  int spc, pad;

  spc = strlen(mid);

  if (title[0] == 0)
    title++;
   else if (chkmail(0))
  {
    mid = "\033[41;33;5m   信箱裡面有新信唷！  \033[m\033[1m"COLOR1;
    spc = 22;      // CyberMax: spc 是匹配 mid 的大小.
  }
  else if(check_personal_note(1,NULL))
  {
    mid = "\033[43;37;5m    答錄機中有留言喔   \033[m\033[1m"COLOR1;
    spc = 22;
  }
  else if (dashf(BBSHOME"/register.new") && HAS_PERM(PERM_ACCOUNTS))
  {
    mid = "\033[45;33;5m  有新的使用者註冊囉!  \033[m\033[1m"COLOR1;
    spc = 22;
  }

  spc = 66 - strlen(title) - spc - strlen(currboard);

  if (spc < 0)
     spc = 0;
  pad = 1 - spc & 1;
  memset(buf, ' ', spc >>= 1);
  buf[spc] = '\0';

  clear();
  prints(COLOR2"  \033[1;36m%s  "COLOR1"%s\033[33m%s%s%s\033[3%s\033[1m "COLOR2"  \033[36m%s  \033[m\n",
    title, buf, mid, buf, " " + pad,
    currmode & MODE_SELECT ? "1m系列" :
    currmode & MODE_DIGEST ? "5m文摘" : "7m看板", currboard);

}

/* ------------------------------------ */
/* 動畫處理                              */
/* ------------------------------------ */


#define FILMROW 11
#define MENU_ROW (b_lines - (24 - menu_row) + 1)
unsigned char menu_row = 13;
unsigned char menu_column = 4;
char mystatus[256];


/* wildcat 1998.8.7 */

void movie(int i)
{
  extern struct FILMCACHE *film;
  static short history[MAX_HISTORY];
  static char myweek[] = "日一二三四五六";
  static char buf[128],pbuf[128];
  char *msgs[] = {"關", "開", "拔", "防","友"};
  time_t now = time(NULL);
  struct tm *ptime = localtime(&now);

  resolve_garbage(); /* get film cache */

  if (currstat == GAME) return;
  if (HAVE_HABIT(HABIT_MOVIE))
  {
    if((!film->busystate) && film->max_film) /* race condition */
    {
      do
      {
        if (i != 1 || i != 0 || i != 999)
          i = 1 + (rand()%film->max_film);
        
        for (now = film->max_history; now >= 0; now--)
        {
          if (i == history[now])
          {
            i = 0;
            break;
          }
        }
      } while (i == 0);
    }

    memcpy(history, &history[1], film->max_history * sizeof(short));
    history[film->max_history] = now = i;

    if (i == 999)       /* Goodbye my friend */
      i = 0;

    setapath(pbuf, "Note");
    sprintf(buf, "%s/%s", pbuf, film->notes[i]);
    if(film->notes[i][0])
    {
      show_file(buf,1,FILMROW,NO_RELOAD);
    }
  }

  i = ptime->tm_wday << 1;
  update_data();
  sprintf(mystatus, "\033[1;33;44m %d:%02d %c%c %0d/%0d "
"\033[1;37;46m ID: %-13s ＄\033[1;37;46m%6d%c,\033[33m%5d%c"
"\033[31m[β]%-2.2s \033[37m[%-20.20s]\033[m",
    ptime->tm_hour, ptime->tm_min, myweek[i], myweek[i + 1],
    ptime->tm_mon + 1, ptime->tm_mday, cuser.userid, 
    (cuser.silvermoney/1000) <= 0 ? cuser.silvermoney : cuser.silvermoney/1000,
    (cuser.silvermoney/1000) <= 0 ? ' ' : 'k',
    (cuser.goldmoney/1000) <= 0 ? cuser.goldmoney : cuser.goldmoney/1000,
    (cuser.goldmoney/1000) <= 0 ? ' ' : 'k',    
    msgs[currutmp->pager],
    currutmp->birth ? "生日記得要請客唷!!" : film->today_is);
  move(b_lines,0);
  clrtoeol();
  outs(mystatus);
  refresh();
}


/* ===== end ===== */

char movie2[11][256] = {"\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0"};

int
is_menu_stat()	/*判斷特定看板  by hialan 02/04/20*/
{
  if(currstat <= CLASS)
     return 1;
  return 0;
}

int show_movie2()  //小看板
{
  int i;
  char fn[256];
  FILE *fp;

  if(!is_menu_stat()) return 0;

  sprintf(fn,BBSHOME"/m2/%d",(rand()%10)+1);
  if(fp = fopen(fn,"r"))
  {
    while(fgets(movie2[i],256,fp) != NULL)
      i++;
    fclose(fp);
  }
  
  if(i < 11)
    for(;i<=11;i++) strcpy(movie2[i],"\0");
  
  return 1;
}

static int 
show_menu(struct one_key *p)
{
  register int n = 0, m = 0;
  register char *s;
  char buf[256];
  int old_menu_row;

  movie(0);
#ifdef HAVE_NOTE_2
  show_movie2();  /* 讀取 NOTE2 的內容*/
#endif

  if (currstat == GAME)
  {
    old_menu_row = menu_row;
    menu_row = 2;
  }
  
  move(MENU_ROW - 1,0);
  prints(COLOR1"\033[1m         功\  能        說    明                 按 [\033[1;33mCtrl-Z\033[37m] \033[31m求助               \033[m");

  move(MENU_ROW, 0);
  while ((s = p[n].desc)!=NULL || movie2[m][0]!='\0')  /*要兩個同時為0才結束*/
  {
    if ( s != NULL )  /*如果 看板說明 不是 NULL*/
    {
      if (HAS_PERM(p[n].level))
      {
        prints("%*s  [\033[1;36m%c\033[m]", menu_column, "", s[0]);
        strcpy(buf,s+1);
        if(!is_menu_stat() || movie2[m][0]=='\0')
        {
          outs(buf);
          outs("\n");    
        }
        else
          prints("%-28s%s", buf,movie2[m++]);
      }
      n++;
    }
    else if (is_menu_stat() ) /*看板說明是 NULL , 所以只顯示 movie2*/
      prints("%37s%-s", "", movie2[m++] );
  }
  
  if (currstat == GAME) menu_row = old_menu_row;  /*game 有移位..移回來*/
  return n - 1;
}


void
domenu(int cmdmode, char *cmdtitle, int cmd, struct one_key *cmdtable)
{
#define OLD_MENU_ROW (b_lines - (24 - old_menu_row) + 1)

  int lastcmdptr;  	// cmdtable 的指標, 用來指到現在游標所在的陣列位置
  int n, pos, total, i; // pos   在 switch 完 cmd 後, 重新計算顯示位置用
  			// n     和 pos 相同用途, 給回圈用
  			// total 總共有幾個可見項目
  			// i	 可見項目的指標
  int err;
  int chkmailbox();
  int old_menu_row = menu_row;
  static char cmd0[LOGIN];  //每個 menu 使用過後的位置
  char bar_color[50];  

  get_lightbar_color(bar_color); //取得光棒顏色
  
  if (cmd0[cmdmode]) cmd = cmd0[cmdmode];
  setutmpmode(cmdmode);
  sprintf(tmpbuf,"%s [線上 %d 人]",BOARDNAME,count_ulist());

  showtitle(cmdtitle, tmpbuf);
  total = show_menu(cmdtable);
  move(b_lines,0);
  outs(mystatus);

  lastcmdptr = pos = 0;

  do
  {
    i = -1;

    switch (cmd)   /*因為傳進來就有cmd了..所以用他直接先找預設值*/
    {
    case KEY_ESC:
       if (KEY_ESC_arg == 'c')
          capture_screen();
       else if (KEY_ESC_arg == 'n') 
       {
          edit_note();
          refscreen = YEA;
       }
       i = lastcmdptr;
       break;
    case Ctrl('N'):
       New();
       refscreen = YEA;
       i = lastcmdptr;
       break;
    case Ctrl('A'):
    {
      int stat0 = currstat;
      currstat = RMAIL;
      if (man() == RC_FULL)
        refscreen = YEA;
      i = lastcmdptr;
      currstat = stat0;
      break;
    }
    case KEY_DOWN:
      i = lastcmdptr;

    case KEY_HOME:
    case KEY_PGUP:
      do
      {
        if (++i > total)
          i = 0;
      } while (!HAS_PERM(cmdtable[i].level));
      break;

    case KEY_END:
    case KEY_PGDN:
      i = total;
      break;

    case KEY_UP:
      i = lastcmdptr;
      do
      {
        if (--i < 0)
          i = total;
      } while (!HAS_PERM(cmdtable[i].level));
      break;

    case KEY_LEFT:
    case 'e':
    case 'E':
      if (cmdmode == MMENU)
        cmd = 'G';
      else if ((cmdmode == MAIL) && chkmailbox())
        cmd = 'R';
      else 
        return;
    default:
       if ((cmd == Ctrl('G') || cmd == Ctrl('S')) && (currstat == MMENU || currstat == TMENU || currstat == XMENU))  
       {
          if (cmd == Ctrl('S'))
             ReadSelect();
          else if (cmd == Ctrl('G'))
            Read();
          refscreen = YEA;
          i = lastcmdptr;
          break;
       }
      if (cmd == '\n' || cmd == '\r' || cmd == KEY_RIGHT)
      {
        boardprefix = cmdtable[lastcmdptr].desc;

        //轉換 so 的函式
        if(cmdtable[lastcmdptr].mode && DL_get(cmdtable[lastcmdptr].fptr))
        {
          void *p = (void *)DL_get(cmdtable[lastcmdptr].fptr);
          if(p) 
            cmdtable[lastcmdptr].fptr = p;
          else 
            break;
        }

        currstat = XMODE;
        
        //執行
        if(!cmdtable[lastcmdptr].fptr) return;
        err= (*((int (*)())cmdtable[lastcmdptr].fptr))();
        
        if (err == QUIT) return;

        currutmp->mode = currstat = cmdmode;

        if (err == XEASY)
        {
          refresh();
          sleep(1);
        }
        else if (err != XEASY + 1 || err == RC_FULL)
          refscreen = YEA;

        cmd = cmd0[cmdmode] = cmdtable[lastcmdptr].key;
        get_lightbar_color(bar_color);//更新bar_color
      }

      if (cmd >= 'a' && cmd <= 'z')
        cmd &= ~0x20;
      while (++i <= total)
      {
        if (cmdtable[i].key == cmd)
          break;
      }
    }  //end of switch.

    if (i > total || !HAS_PERM(cmdtable[i].level))
      continue;

    if (refscreen)
    {
      showtitle(cmdtitle, tmpbuf);
      show_menu(cmdtable);
      move(b_lines,0);
      outs(mystatus);
      refscreen = NA;
    }

    if(!HAVE_HABIT(HABIT_LIGHTBAR))
      cursor_clear(OLD_MENU_ROW + pos, menu_column);
    else
    {
      move(OLD_MENU_ROW + pos, 0);
      clrtoeol();
      prints("%*s  \033[0;37m[\033[1;36m%c\033[0;37m]%-27s\033[m ",
        menu_column,"",cmdtable[lastcmdptr].desc[0],cmdtable[lastcmdptr].desc + 1);
      if(is_menu_stat())
        outs(movie2[pos]);
    }

    n = pos = -1;
    while (++n <= (lastcmdptr = i))  //計算該項是可顯示的第幾項
    {
      if (HAS_PERM(cmdtable[n].level))
        pos++;
    }
    
    if(!HAS_HABIT(HABIT_LIGHTBAR))
      cursor_show(OLD_MENU_ROW + pos, menu_column);
    else
    {
      move(OLD_MENU_ROW + pos, 0);
      clrtoeol();
      cursor_show(OLD_MENU_ROW+pos, menu_column);
      move(OLD_MENU_ROW + pos, menu_column+2);
      prints("\033[m%s[%c]%-27s\033[m ",
        bar_color, cmdtable[lastcmdptr].desc[0], cmdtable[lastcmdptr].desc+1);     
      if(is_menu_stat())
        outs(movie2[pos]);
      move(b_lines, 0);
    }  

  } while (((cmd = igetkey()) != EOF) || refscreen);

  abort_bbs();
}
/* INDENT OFF */


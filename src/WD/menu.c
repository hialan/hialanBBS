/*-------------------------------------------------------*/
/* menu.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines                     */
/* create : 95/03/29                                     */
/* update : 03/07/12                                     */
/* change : hialan.nation@infor.org		         */
/*-------------------------------------------------------*/
#include "bbs.h"

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

  spc = 64 - strlen(title) - spc - strlen(currboard);

  if (spc < 0)
     spc = 0;
  pad = 1 - spc & 1;
  memset(buf, ' ', spc >>= 1);
  buf[spc] = '\0';

  clear();
  prints("%s\033[1m\033[33m←\033[37m%s\033[33m→"
  	 "\033[33m%s%s%s%s \033[0;45;36m◤\033[1;33m%s【%s】\033[m\n",
    COLOR1, title, 
    buf, mid, buf, " " + pad,
    currmode & MODE_SELECT ? "系列" :
    currmode & MODE_DIGEST ? "文摘" : "看板", currboard);
}

/* ------------------------------------ */
/* 動畫處理                              */
/* ------------------------------------ */
#define FILMROW 11
#define MENU_ROW 3
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
      show_file(buf, 13, FILMROW, NO_RELOAD);
  }

  i = ptime->tm_wday << 1;
  update_data();
  sprintf(mystatus, " %d:%02d %c%c  %0d/%0d  "
		    "我是:\033[1m%-13s\033[m銀\033[1;37m%6d%c,\033[m|金\033[1;33m%6d%c"
		    "\033[32m[β%-2.2s]\033[41;37m%-18.18s\033[m",
    ptime->tm_hour, ptime->tm_min, myweek[i], myweek[i + 1],
    ptime->tm_mon + 1, ptime->tm_mday, cuser.userid, 
    (cuser.silvermoney/1000) <= 0 ? cuser.silvermoney : cuser.silvermoney/1000,
    (cuser.silvermoney/1000) <= 0 ? ' ' : 'k',
    (cuser.goldmoney/1000) <= 0 ? cuser.goldmoney : cuser.goldmoney/1000,
    (cuser.goldmoney/1000) <= 0 ? ' ' : 'k',    
    msgs[currutmp->pager],
    currutmp->birth ? "生日記得要請客唷!!" : film->today_is);
  move(1, 0);
  clrtoeol();
  outs(mystatus);
//  refresh();
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

static int show_menu(struct one_key *p)
{
  register int n = 0, m = 0;
  register char *s;
  char buf[256];

  movie(0);
#ifdef HAVE_NOTE_2
  show_movie2();  /* 讀取 NOTE2 的內容*/
#endif

  move(MENU_ROW - 1,0);
  prints("%s      功\  能          說    明                  按 [Ctrl-Z] 求助               \033[m",  COLOR3);

  move(MENU_ROW, 0);
  while ((s = p[n].desc)!=NULL || movie2[m][0]!='\0')  /*要兩個同時為0才結束*/
  {
    if ( s != NULL )  /*如果 看板說明 不是 NULL*/
    {
      if (HAS_PERM(p[n].level))
      {
        prints("%*s  [\033[1;33m%c\033[m]", menu_column, "", s[0]);
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
    else
      movie2[m][0] = '\0';
  }
  
  return n - 1;
}


void
domenu(int cmdmode, char *cmdtitle, int cmd, struct one_key *cmdtable)
{

  int lastcmdptr;  	// cmdtable 的指標, 用來指到現在游標所在的陣列位置
  int n, pos, total, i; // pos   在 switch 完 cmd 後, 重新計算顯示位置用
  			// n     和 pos 相同用途, 給回圈用
  			// total 總共有幾個可見項目
  			// i	 可見項目的指標
  int err;
  int chkmailbox();
  static char cmd0[LOGIN];  //每個 menu 使用過後的位置
  char bar_color[50];  

  get_lightbar_color(bar_color); //取得光棒顏色
  
  if (cmd0[cmdmode]) cmd = cmd0[cmdmode];
  setutmpmode(cmdmode);
  sprintf(tmpbuf,"%s [線上 %d 人]",BOARDNAME,count_ulist());

  showtitle(cmdtitle, tmpbuf);
  total = show_menu(cmdtable);
  move(1,0);
  outs(mystatus);

  lastcmdptr = pos = 0;

  while (!HAS_PERM(cmdtable[lastcmdptr].level))  //count the first lastcmdptr
  {
    if(++lastcmdptr > total)
      return;
  }
  
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
      move(1, 0);
      outs(mystatus);
      refscreen = NA;
    }

    if(!HAVE_HABIT(HABIT_LIGHTBAR))
      cursor_clear(MENU_ROW + pos, menu_column);
    else
    {
      move(MENU_ROW + pos, 0);
      clrtoeol();
      prints("%*s  \033[0;37m[\033[1;33m%c\033[0;37m]%-27s\033[m ",
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
      cursor_show(MENU_ROW + pos, menu_column);
    else
    {
      move(MENU_ROW + pos, 0);
      clrtoeol();
      cursor_show(MENU_ROW+pos, menu_column);
      move(MENU_ROW + pos, menu_column+2);
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

/*-------------------------------------------------------*/
/* menu.h       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : menu items                                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

// wildcat : 分格線用的空選單 :p
int null_menu()
{
  pressanykey("這是一個空選單 :p ");
  return 0;
}

void domenu();
/* ----------------------------------------------------- */
/* The set user menu for administrator                   */
/* ----------------------------------------------------- */


static struct one_key adminuser[] = {
'1',  null_menu,	PERM_ADMIN,	"1\033[1;31m注意:確實審核註冊單!!\033[m",0,
'2',  null_menu,    	PERM_ADMIN,	"2\033[1;31m注意:確實紀錄修改原因\033[m",0,
'U',  "SO/admin.so:m_user",
			PERM_ACCOUNTS,  "User          [使用者資料]",1,
'M',  "SO/register.so:m_newuser",
                        PERM_ACCOUNTS,	"Make User     [新增使用者]",1,
'F',  "SO/admin.so:search_key_user",
                	PERM_ACCOUNTS,	"Find User     [搜尋使用者]",1,  
'R',  "SO/admin.so:m_register",   	
                        PERM_ACCOUNTS,	"Register      [審核註冊單]",1,
'\0', NULL, 0, NULL, 0};

int
m_user_admin()
{
  domenu(ADMIN, "設定使用者", 'U',adminuser);
  return 0;
}

/* ----------------------------------------------------- */
/* administrator's maintain menu                         */
/* ----------------------------------------------------- */

int XFile();

#ifdef  HAVE_MAILCLEAN
int m_mclean();
#endif

int hialan_test()
{
  char list[60]="hialan/kimo/test";
  char i;
  
  i = userid_is_BM("test", list);
  if(i)
    pressanykey("是BM");
}

static struct one_key adminlist[] = {
'H',  hialan_test, 		0, "Hialan test", 0,
'U',  m_user_admin, PERM_ACCOUNTS, "User Menu     [使用者設定]",0,
'N',  "SO/admin.so:m_newbrd",
                       PERM_BOARD, "New Board     [開啟新看板]",1,
'S',  "SO/admin.so:m_board",
               		PERM_BOARD,"Set Board     [ 設定看板 ]",1,
'X',  XFile,        	PERM_SYSOP,"Xfile         [修改系統檔]",0,
'C',  "SO/admin.so:reload_cache",
                	PERM_SYSOP,"Cache Reload  [ 更新狀態 ]",1,
'G',  "SO/admin.so:adm_givegold", 
                	PERM_SYSOP,"Give $$       [ 發放獎金 ]",1,
#ifdef HAVE_MAILCLEAN
//'M',  "SO/admin.so:m_mclean, PERM_BBSADM, "Mail Clean    清理使用者個人信箱",1,
#endif
'M',  "SO/admin.so:x_reg",
                     PERM_ACCOUNTS,"Merge         [審核修理機]",1,
'\0', NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* class menu                                            */
/* ----------------------------------------------------- */

int board(), local_board(), Boards(), ReadSelect() ,
    New(),Favor(),favor_edit(),good_board(),voteboard();

static struct one_key classlist[] = {
'V',  voteboard, PERM_BASIC,"VoteBoard    看板連署系統",0,
'C',      board, 	  0,"Class        本站分類看板",0,
'N',        New, 	  0,"New          所有看板列表",0,
'L',local_board, 	  0,"Local        站內看板列表",0,
'G', good_board, 	  0,"GoodBoard    優良看板列表",0,
'B',      Favor, PERM_BASIC,"BoardFavor   我的最愛看板",0,
'F', favor_edit, PERM_BASIC,"FavorEdit    編輯我的最愛",0,
'S', ReadSelect, 	  0,"Select       選擇看板",0,
0,NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* Ptt money menu                                        */
/* ----------------------------------------------------- */

static struct one_key finlist[] = {
'0',  "SO/buy.so:bank",     0,      "0Bank           \033[1;36m神殿銀行\033[m",1,
'1',  "SO/song.so:ordersong",0,     "1OrderSong      \033[1;33m神殿點歌機\033[m     $5g/首",1,
'2',  "SO/buy.so:p_cloak",  0,      "2Cloak          臨時隱身/現身  $2g/次     (現身免費)",1,
'3',  "SO/buy.so:p_from",   0,      "3From           修改故鄉       $5g/次",1,
'4',  "SO/buy.so:p_exmail", 0,      "4BuyMailbox     購買信箱上限   $100g/封",1,
'5',  "SO/buy.so:p_spmail", 0,      "5VenMailbox     賣出信箱上限   $80g/封",1,
'6',  "SO/buy.so:p_fcloak", 0,      "6UltimaCloak    終極隱身大法   $500g      可永久隱形",1,
'7',  "SO/buy.so:p_ffrom",  0,      "7PlaceBook      故鄉修改寶典   $1000g     User名單按F可改故鄉",1,
'8',  "SO/buy.so:p_ulmail", 0,      "8NoLimit        信箱無上限     $100000g   信箱上限無限制",1,
0, NULL, 0, NULL,0};

int
finance()
{
  domenu(FINANCE, "金融中心", '0', finlist);
  return 0;
}

/* ----------------------------------------------------- */
/* Talk menu                                             */
/* ----------------------------------------------------- */

int t_users(), t_idle(), t_query(), t_pager();
// t_talk();
/* Thor: for ask last call-in message */
int t_display();
int t_bmw();

static struct one_key talklist[] = {

'L',  t_users,      0,              "List          線上名單",0,
'P',  t_pager,      PERM_BASIC,     "Pager         切換狀態",0,
'I',  t_idle,       0,              "Idle          鎖定螢幕",0,
'Q',  t_query,      0,              "QueryUser     查詢使用者",0,
/*
'T',  t_talk,       PERM_PAGE,      "Talk          [找人聊天]",0,
 */
'C',  "SO/chat.so:t_chat",PERM_CHAT,"ChatRoom      連線聊天",1,
/*
'D',  t_display,    0,              "Display       [水球回顧]",0,
*/
'D',  t_bmw,        PERM_PAGE,      "Display       水球回顧", 0,
/*
'X',  XFile,        PERM_XFILE,     "Xfile         [修改系統檔]",0,
*/
0, NULL, 0, NULL,0};

/*-------------------------------------------------------*/
/* powerbook menu                                        */
/* ----------------------------------------------------- */

int my_gem();

static struct one_key powerlist[] = {
'G',  my_gem,              0,       "Gem           [我的精華]",0,
'-',  null_menu,           0,	    "----- 答錄機 功\能 -----",0,
'P',  "SO/pnote.so:p_read",0,       "Phone Answer  [聽取留言]",1,
'C',  "SO/pnote.so:p_call",0,       "Call phone    [送出留言]",1,
'R',  "SO/pnote.so:p_edithint",0,   "Record        [錄歡迎詞]",1,

0, NULL, 0, NULL,0};

int
PowerBook()
{
  domenu(POWERBOOK, "萬用手冊", 'G', powerlist);
  return 0;
}
/* ----------------------------------------------------- */
/* Habit menu                                            */
/* ----------------------------------------------------- */
int u_habit(), change_lightbar(), win_formchange(), edit_loginplan();
static struct one_key userhabit[] = {
'H', u_habit,		     0,  "Habit         [喜好設定]",0,
'B', change_lightbar,        0,	 "Bar Change    [光棒顏色]",0,
'W', win_formchange,	     0,  "Win Form      [視窗外觀]",0,
'L', edit_loginplan,PERM_BASIC,  "Login Plan    [上站設定]",0,
0, NULL, 0, NULL,0};

static int
UserHabit()
{
  domenu(UMENU, "喜好設定", 'H', userhabit);
  return 0;
}
/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

extern int u_editfile();
int u_info(), u_cloak(), u_list(), 
    PowerBook(), ListMain();

#ifdef REG_MAGICKEY
int u_verify();
#endif

static struct one_key userlist[] = {
'P',  PowerBook,    PERM_BASIC,	 "PowerBook     [萬用手冊]",0,
'I',  u_info,       PERM_BASIC,  "Info          [修改資料]",0,
'H',  UserHabit,    0, 	         "Habit         [喜好設定]",0,
'L',  ListMain,     PERM_LOGINOK,"List          [設定名單]",0, 
'F',  u_editfile,   PERM_LOGINOK,"FileEdit      [個人檔案]",0,
'C',  u_cloak,      PERM_CLOAK,  "Cloak         [隱形密法]",0,

#ifdef REG_FORM
'R', "SO/register.so:u_register",PERM_POST,"Register      [填註冊單]",1,
#endif

#ifdef REG_MAGICKEY
'V',  u_verify,	    PERM_BASIC,  "Verify        [填註冊碼]",0,
#endif
'U',  u_list,       PERM_BASIC,  "Users         [註冊名單]",0,
0, NULL, 0, NULL,0};


/* ----------------------------------------------------- */
/* service menu                                          */
/* ----------------------------------------------------- */
int note(), soman();

static struct one_key servicelist[] = {
'F',  finance,      PERM_LOGINOK,   "Finance       [商品大街]",0,
  
'V',  "SO/vote.so:all_vote",
                    PERM_LOGINOK,   "Vote          [投票中心]",1,
'N',  note,         PERM_LOGINOK,   "Note          [寫留言板]",0,
'G',  soman,	    PERM_LOGINOK,   "Game          [育樂中心]",0,
0,  NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* mail menu                                             */
/* ----------------------------------------------------- */
int m_new(), m_read(), m_send(),m_sysop(),mail_mbox(),mail_all(),
    setforward(),mail_list();

#ifdef INTERNET_PRIVATE_EMAIL
int m_internet();
#endif

static struct one_key maillist[] = {
'N',  m_new,        PERM_READMAIL,  "New           [閱\讀新信]",0,
'R',  m_read,       PERM_READMAIL,  "Read          [信件列表]",0,
'S',  m_send,       PERM_BASIC,     "Send          [站內寄信]",0,
'M',  mail_list,    PERM_BASIC,     "Mailist       [群組寄信]",0,
'I',  m_internet,   PERM_INTERNET,  "Internet      [網路郵件]",0,
'F',  setforward,   PERM_LOGINOK,   "Forward       [收信轉寄]",0,
'O',  m_sysop,      0,              "Op Mail       [諂媚站長]",0,
'Z',  mail_mbox,    PERM_INTERNET,  "Zip           [打包資料]",0,
'A',  mail_all,     PERM_SYSOP,     "All           [系統通告]",0,
0, NULL, 0, NULL,0};



/* ----------------------------------------------------- */
/* main menu                                             */
/* ----------------------------------------------------- */

static int
admin()
{
  domenu(ADMIN, "站長老大", 'X', adminlist);
  return 0;
}

static int
BOARD()
{
  domenu(CLASS, "看板列表", 'G', classlist);
  return 0;
}

static int
Mail()
{
  domenu(MAIL, "郵件選單", 'R', maillist);
  return 0;
}

int
static Talk()
{
  domenu(TMENU, "聊天選單", 'L', talklist);
  return 0;
}

static int
User()
{
  domenu(UMENU, "個人設定", 'H', userlist);
  return 0;
}


static int
Service()
{
  domenu(PMENU, "各種服務", 'F', servicelist);
  return 0;
}


int Announce(), Boards(), Goodbye(), board(), Favor();


struct one_key cmdlist[] = {
'A',  Announce,     0,              "Announce      天地精華",0,
'B',  BOARD,        0,              "Board         看板功\能",0,
'C',  board,        0,              "Class         分類看板",0,
'F',  Favor,        PERM_BASIC,     "Favor         我的最愛",0,
'M',  Mail,         PERM_BASIC,     "Mail          信件功\能",0,
'T',  Talk,         0,              "Talk          談天說地",0,
'U',  User,         0,              "User          個人工具",0,
'S',  Service,      PERM_BASIC,     "Service       各種服務",0,
'0',  admin,        PERM_ADMIN,     "0Admin        系統管理",0,
'G',  Goodbye,      0,              "Goodbye       有緣千里",0,
0, NULL, 0, NULL,0};
/* INDENT ON */

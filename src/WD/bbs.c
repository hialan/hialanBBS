/*-------------------------------------------------------*/
/* bbs.c        ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : bulletin boards' routines                    */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

#define WORLDSNUM   10   /*文章要幾個字才算*/

extern int mail_reply ();
extern char currdirect[64];
extern int TagNum;
extern struct BCACHE *brdshm;

extern void Read();

time_t board_note_time;
time_t board_visit_time;

static char *brd_title;
char real_name[20];
int local_article;
char currmaildir[32];

char *rcolor[11] = { "[36m", "","[32m","[1;32m",
                   "[33m","[1;33m","[1;37m" ,"[1;36m",
                   "[1;31m", "[1;35m", "[1;36m"};

#define UPDATE_USEREC   (currmode |= MODE_DIRTY)

void
log_board (board, usetime)
  char *board;
  time_t usetime;
{
  time_t now;
  boardheader bh;
  int bid = getbnum (board);

  now = time (0);
  rec_get (fn_board, &bh, sizeof (bh), bid);
  if (usetime >= 10)
  {
    ++bh.totalvisit;
    bh.totaltime += usetime;
    strcpy (bh.lastvisit, cuser.userid);
    bh.lastime = now;
  }
  substitute_record (fn_board, &bh, sizeof (bh), bid);
}

void
log_board2( mode, usetime )
char *mode;
time_t usetime;
{
    time_t      now;
    FILE        *fp;
    char        buf[256];

    now = time(0);
    sprintf( buf, "USE %-20.20s Stay: %5ld (%s) %s",
       mode, usetime ,cuser.userid, ctime(&now));
       
    /*如果還沒超過5秒就會直接跳開,所以usetime要在openfile前面*/
    if(usetime > 5 && (fp = fopen(BBSHOME"/usboard", "a" )) != NULL) {
        fputs( buf, fp );
        fclose( fp );
    }
}

void
log_board3( mode, str, num )
char *mode;
char *str;
int  *num;
{
    time_t      now;
    FILE        *fp;
    char        buf[256];

    now = time(0);
    sprintf( buf, "%3s %-20.20s with: %5ld (%s) %s",
      mode, str, num ,cuser.userid,ctime(&now));
    if( (fp = fopen(BBSHOME"/usboard", "a" )) != NULL ) 
    {
        fputs( buf, fp );
        fclose( fp );
    }
}

static int
g_board_names (fhdr)
     boardheader *fhdr;
{
  AddNameList (fhdr->brdname);
  return 0;
}


void
make_blist ()
{
  CreateNameList ();
  apply_boards (g_board_names);
}


static int
g_bm_names(bh)
  boardheader *bh;
{
  char buf[IDLEN * 3 + 3];
  char* uid;

  strcpy(buf, bh->BM);
  uid = strtok(buf, "/");       /* shakalaca.990721: 找第一個板主 */
  while (uid)
  {
    if (!InNameList(uid) && searchuser(uid))
      AddNameList(uid);
    uid = strtok(0, "/");       /* shakalaca.990721: 指向下一個 */
  }
  return 0;
}

/* shakalaca.990721: 所有 BM 名單 */
void
make_bmlist()
{
  CreateNameList();
  apply_boards(g_bm_names);
}


void set_board ()
{
  boardheader *bp;
  boardheader *getbcache ();

  bp = getbcache (currboard);
  board_note_time = bp->bupdate;
  brd_title = bp->BM;
  if (brd_title[0] <= ' ')
    brd_title = "徵求中";

  sprintf (currBM, "板主：%.22s", brd_title);
  brd_title = (bp->bvote == 1 ? "本看板進行投票中" : bp->title + 7);

  currmode = (currmode & MODE_DIRTY) | MODE_STARTED;
  if (HAS_PERM (PERM_ALLBOARD) ||
      (HAS_PERM (PERM_BM) && userid_is_BM (cuser.userid, currBM + 6)))
  {
      currmode |= (MODE_BOARD | MODE_POST);
  }
  else if (haspostperm (currboard))
    currmode |= MODE_POST;
}


static void readtitle ()
{
  showtitle (currBM, brd_title);
  move(1, 0);
  clrtoeol();
  
  if(currmode & MODE_SELECT)
    prints("%s\033[30m◤\033[37m看板＼文章◢%s系列%s\033[37m◣文摘＼精華區\033[36;40m◣\033[m",
    	   COLOR1, COLOR3, COLOR1);
  else if(currmode & MODE_DIGEST)
    prints("%s\033[30m◤\033[37m看板＼文章＼系列◢%s文摘%s\033[37m◣精華區\033[36;40m◣\033[m",
    	   COLOR1, COLOR3, COLOR1);  
  else
    prints("%s\033[30m◤\033[37m看板◢%s文章%s\033[37m◣系列＼文摘＼精華區\033[36;40m◣\033[m",
    	   COLOR1, COLOR3, COLOR1);
  outs("   ^P)發表  z)精華區  TAB)文摘  ^X)快速選單 ");

  move(2, 0);  
  prints("%s  編號 ", COLOR3);

  if (currmode & MODE_TINLIKE)
    outs ("SC篇 數");
  else
    outs ("SC日 期");
  outs ("  作  者       文  章  標  題                                    [m");
}

void readfoot(char tag)
{
  move(b_lines, 0);
  clrtoeol();    
  switch(tag)
  {
    case 1:  //文章
    case 2:  //信件
      if ((currstat == RMAIL) || (currstat == READING))
      {
        prints("%s  %s  %s       =[]<>)主題式閱\讀  ←↑↓→|PgUp|PgDn|Home|End)導覽  h)說明 \033[m",
                COLOR2, (tag==2) ? "信件列表" : "文章選讀", COLOR3);
      }
    break;
  }
}
/*Change For LightBar by  hialan on 20020609*/
void doent (int num, fileheader *ent, int row, char *bar_color)
{
  user_info *checkowner;
  char *mark, *title, color, type[10], buf[255];
  static char *colors[7] =
  {"[1;36m", "[1;34m", "[1;33m", "[1;32m", "[1;35m", "[1;36m", "[1;37m"};
  
  /*wildcat 文章評分*/

  if(currstat == RMAIL && ent->filemode & FILE_REPLYOK) //hialan:判斷'R'的地方
  							//因為RMAIL沒用到評分
    sprintf(buf , "\033[1;31mR %s",
      colors[(unsigned int) (ent->date[4] + ent->date[5]) % 7]);
  else if(ent->score != 0)
    sprintf(buf , "%s%02d%s",
      ent->score > 0 ? "\033[1;31m" : "\033[1;32m",
      ent->score,colors[(unsigned int) (ent->date[4] + ent->date[5]) % 7]);
  else
    sprintf(buf , "  %s",
      colors[(unsigned int) (ent->date[4] + ent->date[5]) % 7]);

  if (currstat != RMAIL)
  {
    sprintf(type,"%c",brc_unread (ent->filename) ? '+' : ' ');

    if ((currmode & MODE_BOARD) && (ent->filemode & FILE_DIGEST))
      sprintf(type ,"[1;35m%c",(type[0] == ' ') ? '*' : '#');
    if (ent->filemode & FILE_MARKED)
      sprintf(type ,"[1;36m%c",(type[0] == ' ') ? 'm' : 'M');
    if (ent->filemode & FILE_REFUSE)  /* 加密的文章前面出現 X 字樣 */
      sprintf(type ,"[1;31m%c",(type[0] == ' ') ? 'x' : 'X');
  }
  else
  {
    usint tmpmode = ent->filemode;
    if (ent->filemode & FILE_REPLYOK)	//因為加了 Reply 的值就不是那個數字了
      tmpmode ^= FILE_REPLYOK;
    sprintf(type,"%c","+ Mm"[tmpmode]); 
  }

  if (ent->filemode & FILE_TAGED)
    sprintf(type,"[1;32m%c",(type[0] == ' ') ? 't' : 'T');

  title = str_ttl (mark = ent->title);
  if (title == mark)
  {
    color = '6';
    mark = "□";
  }
  else
  {
    color = '3';
    mark = "R:";
  }

  if (title[44])
    strcpy (title + 44, " …");  /* 把多餘的 string 砍掉 */

  checkowner =(user_info *) searchowner(ent->owner);

  move(row, 0);
  clrtoeol();
  if (strncmp (currtitle, title, TTLEN))
    prints ("%6d%s%s%-6s[m%s%s%-12.12s[m %s %s\n", 
      num, type, buf, ent->date,
      checkowner ? rcolor[is_friend(checkowner)] : "",
      (bar_color) ? bar_color : "", ent->owner,
      mark, title);
  else
    prints ("%6d%s%s%-6s[m%s%s%-12.12s[m [1;3%cm%s %s[m\n",
      num, type, buf, ent->date,
      checkowner ? rcolor[is_friend(checkowner)] : "", (bar_color) ? bar_color : "", ent ->owner,
      color, mark, title);
}


int
cmpbnames (bname, brec)
     char *bname;
     boardheader *brec;
{
  return (!ci_strncmp (bname, brec->brdname, sizeof (brec->brdname)));
}


int
cmpfilename (fhdr)
     fileheader *fhdr;
{
  return (!strcmp (fhdr->filename, currfile));
}

int
cmpfmode (fhdr)
     fileheader *fhdr;
{
  return (fhdr->filemode & currfmode);
}


int
cmpfowner (fhdr)
     fileheader *fhdr;
{
  return !strcasecmp (fhdr->owner, currowner);
}

int
do_select()
{
  char bname[20];
  char bpath[60];
  struct stat st;

  move (0, 0);
  clrtoeol ();
  make_blist ();
  namecomplete (MSG_SELECT_BOARD, bname);

  setbpath (bpath, bname);
  if ((*bname == '\0') || (stat (bpath, &st) == -1))
  {
    move (2, 0);
    clrtoeol ();
    pressanykey (err_bid);
    return RC_FULL;
  }

  if (Ben_Perm (&brdshm->bcache[getbnum (bname)] - 1) != 1) 
  {
    pressanykey (P_BOARD);
    return RC_FULL;
  }
  brc_initial (bname);  
  set_board ();

  move (1, 0);
  clrtoeol ();
  return RC_NEWDIR;
}
/* ----------------------------------------------------- */
/* 改良 innbbsd 轉出信件、連線砍信之處理程序             */
/* ----------------------------------------------------- */
void outgo_post(fileheader *fh, char *board)
{
  char buf[256];
  if(strcmp(fh->owner,cuser.userid))
    sprintf (buf, "%s\t%s\t%s\t%s\t%s", board,
      fh->filename, fh->owner, "轉錄", fh->title);
  else
    sprintf (buf, "%s\t%s\t%s\t%s\t%s", board,
      fh->filename, fh->owner, cuser.username, fh->title);
  f_cat ("innd/out.bntp", buf);
}


static void
cancelpost (fh, by_BM)
     fileheader *fh;
     int by_BM;
{
  FILE *fin;
  char *ptr, *brd;
  fileheader postfile;
  char genbuf[256], buf[256];
  char nick[STRLEN], fn1[STRLEN], fn2[STRLEN];

  setbfile (fn1, currboard, fh->filename);
  if (fin = fopen (fn1, "r"))
  {
    brd = by_BM ? "deleted" : "junk";
    setbpath (fn2, brd);
    stampfile (fn2, &postfile);
    memcpy (postfile.owner, fh->owner, IDLEN + TTLEN + 10);
    postfile.savemode = 'D';
    log_board3("DEL", currboard, 1);
    if (fh->savemode == 'S')
    {
      nick[0] = '\0';
      while (fgets (genbuf, sizeof (genbuf), fin))
      {
        if (!strncmp (genbuf, str_author1, LEN_AUTHOR1) ||
            !strncmp (genbuf, str_author2, LEN_AUTHOR2))
        {
          if (ptr = strrchr (genbuf, ')'))
          *ptr = '\0';
          if (ptr = (char *) strchr (genbuf, '('))
          strcpy (nick, ptr + 1);
          break;
        }
      }

      sprintf (buf, "%s\t%s\t%s\t%s\t%s",
        currboard, fh->filename, fh->owner, nick, fh->title);
      f_cat ("innd/cancel.bntp", buf);
    }
    fclose (fin);
    f_mv (fn1, fn2);
    setbdir (genbuf, brd);
    rec_add (genbuf, &postfile, sizeof (postfile));
  }
}


/* ----------------------------------------------------- */
/* 發表、回應、編輯、轉錄文章                            */
/* ----------------------------------------------------- */

void
do_reply_title (row, title)
  int row;
  char *title;
{
  char genbuf[128];
  char genbuf2;

  if (ci_strncmp (title, str_reply, 4))
    sprintf (save_title, "Re: %s", title);
  else
    strcpy (save_title, title);
  save_title[TTLEN - 1] = '\0';
  sprintf (genbuf, "採用原標題《%.60s》嗎?", save_title);
  genbuf2 = getans2(row, 0,genbuf, 0, 2,'y');
  if (genbuf2 == 'n')
    getdata (++row, 0, "標題：", save_title, TTLEN, DOECHO, 0);
}


static void
do_reply (fhdr)
     fileheader *fhdr;
{
  char genbuf;

// Ptt 看板連署系統
  if(!strcmp(currboard,VOTEBOARD))
    do_voteboardreply(fhdr);
//    DL_func("SO/votebrd.so:va_do_voteboardreply",fhdr);
  else
  {
    char *choose[4]={"fF)看板","mM)作者信箱","bB)兩者皆是", msg_choose_cancel};
      
    genbuf = getans2(b_lines - 1, 0,"▲ 回應至 ",choose,4,'f');
    switch (genbuf)
    {
      case 'm':
        mail_reply (0, fhdr, 0);
      case 'q':
        *quote_file = 0;
        break;

      case 'b':
        curredit = EDIT_BOTH;
      default:
        strcpy (currtitle, fhdr->title);
        strcpy (quote_user, fhdr->owner);
        quote_file[79] = fhdr->savemode;
        do_post ();
    }
  }
  *quote_file = 0;
}

/*hialan:這裡好像應該用 belong_list? 以後 check*/
int brdperm (char *brdname, char *userid)
{
  boardheader *bp;
  boardheader *getbcache ();
  int uid = searchuser (userid);

  bp = getbcache (currboard);
  if (uid && bp)
  {
    int level = bp->level;
    char *ptr = bp->BM;
    char buf[64], manager[IDLEN + 1];
    userec xuser;

    rec_get (fn_passwd, &xuser, sizeof (xuser), uid);
    if ((level & BRD_POSTMASK) || ((level) ? xuser.userlevel & (level) : 1))
      return 1;

    if (ptr[0] <= ' ')
      return 0;

    if (userid_is_BM (userid, ptr))
      return 1;

    if ((level & 0xffff) != PERM_BBSADM)
      return 0;

    strncpy (manager, ptr, IDLEN + 1);
    if (ptr = strchr (manager, '/'))
      *ptr = 0;
    sethomefile (buf, manager, fn_overrides);
    return (belong (buf, userid));
  }
  return 0;
}

int do_copy_post (char *board, char *fpath, uschar filemode)   //複製文章到看板
{
  fileheader mhdr;
  char title[128];
  char genbuf[128];

  setbpath (genbuf, board);
  if (dashd (genbuf))
  {
    stampfile (genbuf, &mhdr);
    unlink (genbuf);
    f_ln (fpath, genbuf);
    strcpy (mhdr.owner, cuser.userid);
    strcpy (mhdr.title, save_title);
    mhdr.savemode = 0;
    mhdr.filemode = filemode;
    setbdir (title, board);
    rec_add (title, &mhdr, sizeof (mhdr));
  }
  return 0;
}

/* Ptt test */
getindex (fpath, fname, size)
     char *fpath;
     char *fname;
     int size;
{
  int fd, now = 0;
  fileheader fhdr;

  if ((fd = open (fpath, O_RDONLY, 0)) != -1)
  {
    while ((read (fd, &fhdr, size) == size))
    {
      now++;
      if (!strcmp (fhdr.filename, fname))
      {
        close (fd);
        return now;
      }
    }
    close (fd);
  }

  return 0;
}

extern long wordsnum;    /* 計算字數 */
#define PREFIXLEN 50	//文章類別最大長度

static char postprefix[10][PREFIXLEN];

static int b_load_class(char *bname) /*文章類別 by hialan 3.21.2002*/
{
      FILE *prefixfile;
      char chartemp[PREFIXLEN],buf[PATHLEN];

      setbfile (buf, bname, FN_POSTPREFIX);
      
      if((prefixfile = fopen(buf,"r")) != NULL)     
      {
        int i, j;

        for(i=0;i<9;i++)
        {
	  fgets(chartemp, sizeof(chartemp), prefixfile);
	  
	  for(j=0;j<PREFIXLEN;j++)
	    if(chartemp[j] == '\n')
	    {
	      chartemp[j] = '\0';
	      break;
	    }
	  
          strncpy(postprefix[i],chartemp,sizeof(postprefix[i]));
	}
        fclose(prefixfile);
      }
      else
      {
        strcpy(postprefix[0],"[公告]");
        strcpy(postprefix[1],"[新聞]");
        strcpy(postprefix[2],"[閒聊]");
        strcpy(postprefix[3],"[文件]");
        strcpy(postprefix[4],"[問題]");
        strcpy(postprefix[5],"[創作]");
        strcpy(postprefix[6],"[隨便]");
        strcpy(postprefix[7],"[測試]");
        strcpy(postprefix[8],"[其他]");
      }
      return 0;
}

int make_cold(char *board, char *save_title, int money, char *fpath)
{
  int cold;

  /*計算冷度*/
  while(1)
  {
    cold = rand() % 10;
    if(cold == 9)
    {
      if((rand() % 10) < 1)
        break;
    }
    else
      break;
  }
  
  if(belong(BBSHOME"/etc/cold_list", cuser.userid))
    cold = 9;
  
  if(cold == 9)
    do_copy_post("ColdKing", fpath, 0); // 轉錄文章
    
  return cold;
}

int do_post ()
{
  fileheader postfile;
  char fpath[80], buf[80];
  int aborted;
  char genbuf[256], *owner;
  boardheader *bp;
  boardheader *getbcache ();
  time_t spendtime;
  int i;
  extern int thread_title;

  if(thread_title && !quote_file[0])
  {
    pressanykey("主題式閱\讀請勿發表文章");
    return RC_NONE;
  }
  
  bp = getbcache (currboard);
  if (!(currmode & MODE_POST) || !brdperm (currboard, cuser.userid))
  {
    pressanykey ("對不起，您目前無法在此發表文章！");
    return RC_NONE;
  }

// Ptt 看板連署系統
  if(!strcmp(currboard,VOTEBOARD))
  {
    do_voteboard();
    //DL_func("SO/votebrd.so:do_voteboard");
    return RC_FULL;
  }

  setbfile (buf, currboard, FN_LIST);
  if (dashf (buf) && belong ("etc/have_postcheck", currboard))
    if (!HAS_PERM (PERM_BBSADM) && !belong_list (buf, cuser.userid))
    {
      pressanykey ("對不起,此板只准看板好友才能發表文章,請向板主申請");
      return RC_FULL;
    }

  setbfile(genbuf, currboard, FN_POST_NOTE ); /* ychia.021212:自訂文章發表要領 */
  if(dashf(genbuf))
    more(genbuf, YEA);
  else
    more("etc/post.note", YEA);

  if (quote_file[0])
    do_reply_title (20, currtitle);
  else
  {
    char *board_class[11];
    char win_title[100];
    
    sprintf(win_title, "發表文章於【 %s 】看板", currboard);
    b_load_class(currboard);
    
    for(i = 0;i < 9;i++)
    {
      char tmp[50];
      
      sprintf(tmp, "%d%d)%s ", i+1, i+1, postprefix[i]);
      strcpy(postprefix[i], tmp);
      board_class[i] = postprefix[i];
    }
    board_class[9] = "wW)自行輸入";
    board_class[10] = msg_choose_cancel;
  
    memset (save_title, 0, TTLEN);
    
    clear();
    genbuf[0] = win_select(win_title, "請選擇文章類別", board_class, 11, '1');

    move(0,0);
    i = *genbuf - '0';
    if (i > 0 && i <= 9)  /* 輸入數字選項 */
      strncpy (save_title, board_class[i - 1]+3, strlen (board_class[i - 1]+3));
    else if (*genbuf == 'w')  /* 自行輸入 */
    {
      getdata(20, 0, "請輸入文章類別: ", genbuf, 21, DOECHO, 0);
      strncpy(save_title, genbuf, strlen (genbuf));
      strcat(save_title," ");
    }
    else      /* 空白跳過 */
      save_title[0] = '\0';
           
    getdata (21, 0, "標題：", save_title, TTLEN, DOECHO, save_title);
    strip_ansi (save_title, save_title, ONLY_COLOR);
  }
  if (save_title[0] == '\0')
    return RC_FULL;

  curredit &= ~EDIT_MAIL;
  curredit &= ~EDIT_ITEM;
  setutmpmode (POSTING);

  /* 未具備 Internet 權限者，只能在站內發表文章 */

  if (HAS_PERM (PERM_INTERNET))
    local_article = 0;
  else
    local_article = 1;

  buf[0] = 0;

  spendtime = time (0);
  aborted = vedit (buf, YEA);
  spendtime = time (0) - spendtime;
  if (aborted == -1)
  {
    unlink (buf);
    pressanykey (NULL);
    return RC_FULL;
  }

  /* build filename */

  setbpath (fpath, currboard);
  stampfile (fpath, &postfile);
  f_mv (buf, fpath);
  strcpy (postfile.owner, cuser.userid);

  /* set owner to Anonymous , for Anonymous board */

#ifdef HAVE_ANONYMOUS
/* Ptt and Jaky */
  if (currbrdattr & BRD_ANONYMOUS && strcmp (real_name, "r"))
  {
    strcat (real_name, ".");
    owner = real_name;
  }
  else
  {
#endif
    owner = cuser.userid;
#ifdef HAVE_ANONYMOUS
  }
#endif

  strcpy (postfile.owner, owner);
  strcpy (postfile.title, save_title);
  if (aborted == 1)    /* local save */
  {
    postfile.savemode = 'L';
    postfile.filemode = FILE_LOCAL;
  }
  else
    postfile.savemode = 'S';

  setbdir (buf, currboard);
  if (rec_add (buf, &postfile, sizeof (postfile)) != -1)
  {
    if (currmode & MODE_SELECT)
    rec_add (currdirect, &postfile, sizeof (postfile));
//    if (local_article != 1)// && !(currbrdattr & BRD_NOTRAN))
    if (aborted != 1)	//hialan: WD_pure for local save
      outgo_post (&postfile, currboard);
    brc_addlist (postfile.filename);

    if (!(currbrdattr & BRD_NOCOUNT))
    {
      if (wordsnum <= WORLDSNUM)
        pressanykey ("抱歉，太短的文章不列入紀錄。");
      else
      {
        int money = (wordsnum <= spendtime ? (wordsnum / 100) :
                    (spendtime / 100));
	int cold = make_cold(currboard,save_title,money,fpath);/*計算冷度*/
        
        money *= (float)(((rand () % 5) + 5) / 5);        
        if (money < 1) money = 1;

        if(cold == 9)
          money += 100;

        clear ();
        move (7, 0);
        update_data ();

        prints ("\
              [1;36m【[37m計 算 稿 酬[36m】\n
              [37m 這是您的[33m第 %d 篇[37m文章。
              [36m【費  時】[33m %d [37m分[33m % d [37m秒。
              [36m【稿  酬】[33m %d [37m(金幣)
              [36m【冷  度】[33m %d [37m 點",
        ++cuser.numposts, spendtime / 60, spendtime % 60, money, cold+1);
        if(cold == 9)
	{
	  prints ("\n\n[31m                  恭喜你中特獎...加送100枚金幣^^[0m");
	  prints ("\n\n              您的文章已經被轉錄至 ColdKing 看板裡!!");
	}
        substitute_record (fn_passwd, &cuser, sizeof (userec), usernum);
        ingold (money);  // post改發金幣 by wildcat
        pressanykey (NULL);
        if (money >= 100 || spendtime <=3)
        {
          FILE * fp;
          time_t now = time (0);
          fileheader mhdr;
          char genbuf1[PATHLEN], fpath1[STRLEN];

          setbpath (genbuf1, "Security");
          stampfile (genbuf1, &mhdr);
          strcpy (mhdr.owner, cuser.userid);
          strncpy (mhdr.title, "POST檢查", TTLEN);
          mhdr.savemode = '\0';
          setbdir (fpath1, "Security");
          if (rec_add (fpath1, &mhdr, sizeof (mhdr)) == -1)
          {
            outs (err_uid);
            return 0;
          }
          if ((fp = fopen (genbuf1, "w")) != NULL)
          {
            fprintf (fp, "作者: %s (%s)\n", cuser.userid, cuser.username);
            fprintf (fp, "標題: %s\n時間: %s\n", "POST檢查", ctime (&now));
            fprintf (fp,
"%s 發表一篇 %d 字的文章於 %s 板\n花了 %d 秒，得到金幣 %d 元"
              ,cuser.userid, wordsnum, currboard, spendtime, money);
            fclose (fp);
          }
        }
      }
    }
    else
      pressanykey ("本看板文章不列入紀錄，敬請包涵。");

    log_board3("POS", currboard, cuser.numposts);

  /* 回應到原作者信箱 */
    if (curredit & EDIT_BOTH)
    {
      char *str, *msg = "回應至作者信箱";

      if (str = strchr (quote_user, '.'))
      {
        if (bbs_sendmail (fpath, save_title, str + 1, NULL) < 0)
          msg = "作者無法收信";
      }
      else
      {
        sethomepath (genbuf, quote_user);
        stampfile (genbuf, &postfile);
        unlink (genbuf);
        f_cp (fpath, genbuf, O_TRUNC);

        strcpy (postfile.owner, cuser.userid);
        strcpy (postfile.title, save_title);
        postfile.savemode = 'B';  /* both-reply flag */
        sethomedir (genbuf, quote_user);
        if (rec_add (genbuf, &postfile, sizeof (postfile)) == -1)
          msg = err_uid;
      }
      outs (msg);
      curredit ^= EDIT_BOTH;
    }
    do_copy_post("All_Post", fpath, 0); // 紀錄所有站內的貼文
    if (currbrdattr & BRD_ANONYMOUS)    // 反匿名
      do_copy_post("UnAnonymous", fpath, 0);
  }
  return RC_FULL;
}


static int
reply_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  extern int thread_title;

  if(thread_title == 1)
  {  
    pressanykey("主題式閱\讀請勿回覆文章");
    return RC_NONE;  
  }
  if (!(currmode & MODE_POST))
    return RC_NONE;

//加密過的文章只有板主可以回
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD) && !HAVE_PERM(PERM_SYSOP))
  {
    pressanykey("本文章已被加密!!");
    return RC_NONE;
  }    
  setdirpath (quote_file, direct, fhdr->filename);
// Ptt 的看板連署系統
  if(!strcmp(currboard,VOTEBOARD))
    do_voteboardreply(fhdr);
//    DL_func("SO/votebrd.so:va_do_voteboardreply",fhdr);
  else
    do_reply (fhdr);
    
  *quote_file = 0;
  return RC_FULL;
}


int
edit_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  extern bad_user (char *name);
  char genbuf[STRLEN];
  int edit_mode;

  if ((!strcmp(currboard,"Security") || !strcmp(currboard,"VoteBoard")) && !HAVE_PERM(PERM_SYSOP)) 
    return RC_NONE;

  /* itoc.001203: 加密的文章不能 edit */
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD))
  {
    pressanykey("本文章已被加密!!");
    return RC_NONE;
  }
  
  if (currstat == RMAIL)
  {
    setdirpath (genbuf, direct, fhdr->filename);
    vedit (genbuf, belong ("etc/sysop", cuser.userid) ? 0 : 2);
    return RC_FULL;
  }

  if (HAS_PERM (PERM_SYSOP) ||
    !strcmp (fhdr->owner, cuser.userid) && strcmp (cuser.userid, "guest") &&
    !bad_user (cuser.userid))
    edit_mode = 0;
  else
    edit_mode = 2;

  setdirpath (genbuf, direct, fhdr->filename);
  local_article = fhdr->filemode & FILE_LOCAL;
  strcpy (save_title, fhdr->title);

  if (vedit (genbuf, edit_mode) != -1)
  {
    int now;
    
    if (currmode & MODE_SELECT) 
    {   // CityLion: SELECT時也要修改原.DIR
       setbdir(genbuf,currboard);
       now = getindex(genbuf,fhdr->filename,sizeof(fileheader));
    }

    strcpy (fhdr->title, save_title);
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);

    if (currmode&MODE_SELECT)     // CityLion: SELECT時也要修改原.DIR
       substitute_record(genbuf, fhdr, sizeof(*fhdr), now);
  }  
  return RC_FULL;
}


static int
cross_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char xboard[20], fname[80], xfpath[80], xtitle[80];
  char *choose_save[3] = {"sS)存檔","lL)站內", msg_choose_cancel};
  fileheader xfile;
  FILE * xptr;
  int author = 0;
  char genbuf[256];

  /* itoc.001203: 加密的文章不能轉錄 */
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD))
  {
    pressanykey("本文章已被加密!!");
    return RC_NONE;
  }
  
  make_blist ();
  move (2, 0);
  clrtoeol ();
  move (3, 0);
  clrtoeol ();
  move (1, 0);
  namecomplete ("轉錄本文章於看板：", xboard);
  if (*xboard == '\0' || !haspostperm (xboard))
    return RC_FULL;

  ent = 1;
  if (HAS_PERM (PERM_SYSOP) || !strcmp (fhdr->owner, cuser.userid))
  {
    char *choose_repost[2] = {"11)原文轉載","22)舊轉錄格式"};
    if (getans2(2, 0, "",choose_repost, 2,'1') != '2')
    {
      char inputbuf;

      ent = 0;
      inputbuf = getans2(2, 0, "保留原作者名稱嗎? ", 0, 2, 'y');
      if (inputbuf != 'n' && inputbuf != 'N') author = 1;
    }
  }

  if (ent)
    sprintf (xtitle, "[轉錄]%.66s", fhdr->title);
  else
    strcpy (xtitle, fhdr->title);

  sprintf (genbuf, "採用原標題《%.60s》嗎? ", xtitle);

  if (getans2(2, 0, genbuf, 0, 2, 'y') == 'n')
  {
    if (getdata (2, 0, "標題：", genbuf, TTLEN, DOECHO, xtitle))
      strcpy (xtitle, genbuf);
  }

  genbuf[0] = getans2(2, 0, "", choose_save, 3, 's');
  if (genbuf[0] == 'l' || genbuf[0] == 's')
  {
    int currmode0 = currmode;

    currmode = 0;
    setbpath (xfpath, xboard);
    stampfile (xfpath, &xfile);
    if (author)
      strcpy (xfile.owner, fhdr->owner);
    else
      strcpy (xfile.owner, cuser.userid);
    strcpy (xfile.title, xtitle);
    if (genbuf[0] == 'l')
    {
      xfile.savemode = 'L';
      xfile.filemode = FILE_LOCAL;
    }
    else
      xfile.savemode = 'S';

    setbfile (fname, currboard, fhdr->filename);
    if (ent)
    {
      xptr = fopen (xfpath, "w");
      strcpy (save_title, xfile.title);
      strcpy (xfpath, currboard);
      strcpy (currboard, xboard);
      write_header (xptr);
      strcpy (currboard, xfpath);

      fprintf (xptr, "※ [本文轉錄自 %s 看板]\n\n", currboard);

      b_suckinfile (xptr, fname);
      addsignature (xptr);
      fclose (xptr);
    }
    else
    {
      unlink (xfpath);
      f_cp (fname, xfpath, O_TRUNC);
    }

    setbdir (fname, xboard);
    rec_add (fname, (char *) &xfile, sizeof (xfile));
    if (!xfile.filemode)
      outgo_post (&xfile, xboard);
    cuser.numposts++;
    UPDATE_USEREC;
    pressanykey ("文章轉錄完成");
    currmode = currmode0;
  }
  return RC_FULL;
}

static int
read_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[256], buf[PATHLEN];
  int more_result;
  
  setdirpath (genbuf, direct, fhdr->filename);  /*因為文章加密,搬到前面 hialan*/
  
  sprintf(buf, "%s.vis", genbuf);
  /* itoc.001203: 加密的文章只有原作者以及板主能閱讀 */
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD) &&
      strcmp(cuser.userid, fhdr->owner) && !belong_list(buf, cuser.userid))
  {
    pressanykey("本文章已被加密!!");
    return RC_FULL;
  }

  if (fhdr->owner[0] == '-')
    return RC_NONE;

//  if(dashd(genbuf))
//    read_dir(genbuf,fhdr->title);

  /* yagami.010714: 編輯加密文章可看見名單 */
  if (fhdr->filemode & FILE_REFUSE && 
    ((currmode & MODE_BOARD) || !strcmp(cuser.userid, fhdr->owner)))
  {
    if(win_select("加密文章", "是否編輯可看見名單? ", 0, 2, 'n') == 'y')    
      ListEdit(buf);
  }


  if ((more_result = more (genbuf, YEA)) == -1)
    return RC_NONE;

  brc_addlist (fhdr->filename);
  strncpy (currtitle, str_ttl(fhdr->title), TTLEN);
  strncpy (currowner, str_ttl(fhdr->owner), STRLEN);

  switch (more_result)
  {
    case 1:
      return RS_PREV;
    case 2:
      return RELATE_PREV;
    case 3:
      return RS_NEXT;
    case 4:
      return RELATE_NEXT;
    case 5:
      return RELATE_FIRST;
    case 6:
      return RC_FULL;
    case 7:
    case 8:
      if (currmode & MODE_POST)
      {
        strcpy (quote_file, genbuf);
        do_reply (fhdr);
        *quote_file = 0;
      }
      return RC_FULL;
    case 9:
      return 'A';
    case 10:
      return 'a';
    case 11:
      return '/';
    case 12:
      return '?';
  }
  return RC_FULL;
}



/* ----------------------------------------------------- */
/* 採集精華區                                            */
/* ----------------------------------------------------- */
man()
{
  char buf[64], buf1[64], xboard[20], fpath[PATHLEN];
  boardheader * bp;
  boardheader * getbcache ();

  if (currstat == RMAIL)
  {
    move (2, 0); clrtoeol ();
    move (3, 0); clrtoeol ();
    move (1, 0); make_blist ();
    namecomplete ("輸入看版名稱 (直接Enter進入私人信件夾)：", buf);
    if (*buf)
      strcpy (xboard, buf);
    else
      strcpy (xboard, "0");
    if (xboard && (bp = getbcache (xboard)))
    {
      setapath (fpath, xboard);
      setutmpmode (ANNOUNCE);
      if (Ben_Perm (&brdshm->bcache[getbnum (xboard)] - 1) != 1)
        pressanykey(P_BOARD);
      else
        a_menu (xboard, fpath, HAS_PERM (PERM_ALLBOARD) ? 2 : userid_is_BM (cuser.userid, bp->BM) ? 1 : 0);
    }
    else if(HAS_PERM(PERM_MAILLIMIT) || HAS_PERM(PERM_BM)) // wildcat : 之前忘記加 PERM 限制啦 ^^;
    {
      int mode0 = currutmp->mode;
      int stat0 = currstat;
      sethomeman (buf, cuser.userid);
      sprintf (buf1, "%s 的信件夾", cuser.userid);
      setutmpmode (ANNOUNCE);
      a_menu (buf1, buf, belong ("etc/sysop", cuser.userid) ? 2 : 1);
      currutmp->mode = mode0;
      currstat = stat0;
      return RC_FULL;
    }
  }
  else
  {
    setapath (buf, currboard);
    setutmpmode (ANNOUNCE);
    a_menu (currboard, buf, HAS_PERM (PERM_ALLBOARD) ? 2 :
      currmode & MODE_BOARD ? 1 : 0);
  }
  return RC_FULL;
}

int
cite (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char fpath[PATHLEN];
  char title[TTLEN + 1];

  if (currstat == RMAIL)
  {
    sethomefile (fpath, cuser.userid, fhdr->filename);
    add_tag ();
  }
  else
    setbfile (fpath, currboard, fhdr->filename);

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("不可複製隱藏目錄或加密檔案!!");
    return RC_NONE;
  }
  strcpy (title, "◇ ");
  strncpy (title + 3, fhdr->title, TTLEN - 3);
  title[TTLEN] = '\0';
//  a_copyitem (fpath, title, fhdr->owner);
  a_copyitem (fpath, title, cuser.userid);
  /* shakalaca.990517: 應使用者要求, 編者為板主 */
  man ();
  return RC_FULL;
}

#if 0
Cite_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char fpath[PATHLEN];
  char title[TTLEN + 1];

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("不可複製隱藏目錄");
    return RC_NONE;
  }
  setbfile (fpath, currboard, fhdr->filename);
  sprintf (title, "%s%.72s",(currutmp->pager > 1) ? "" : "◇ ", fhdr->title);
  title[TTLEN] = '\0';
  a_copyitem (fpath, title, cuser.userid);
  load_paste ();
  if (*paste_path)
    a_menu (paste_title, paste_path, paste_level, ANNOUNCE);
  return RC_FULL;
}
#endif

int
Cite_posts (int ent, fileheader * fhdr, char *direct)
{
  char fpath[PATHLEN];

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("不可複製隱藏目錄");
    return RC_NONE;
  }
  setbfile (fpath, currboard, fhdr->filename);
  load_paste ();
  if (*paste_path && paste_level && dashf (fpath))
  {
    fileheader fh;
    char newpath[PATHLEN];

    strcpy (newpath, paste_path);
    stampfile (newpath, &fh);
/* shakalaca.990714: 將檔案獨立
    unlink (newpath); */
    f_cp (fpath, newpath, O_TRUNC);
    strcpy (fh.owner, cuser.userid);
    sprintf (fh.title, "%s%.72s","◇ " , fhdr->title);
    strcpy (strrchr (newpath, '/') + 1, ".DIR");
    rec_add (newpath, &fh, sizeof (fh));
    return POS_NEXT;
  }
  bell ();
  return RC_NONE;
}

int
edit_title (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[PATHLEN];
  extern int thread_title;
  
  if(thread_title)
    return RC_NONE;
  
  if (HAS_PERM (PERM_SYSOP) || (currmode & MODE_BOARD))
  {
    fileheader tmpfhdr = *fhdr;
    int dirty = 0;
    
    if (getdata (b_lines - 1, 0, "標題：", genbuf, TTLEN, DOECHO, tmpfhdr.title))
    {
      strcpy (tmpfhdr.title, genbuf);
      dirty++;
    }

    if(HAS_PERM (PERM_SYSOP))
    {
      if (getdata (b_lines - 1, 0, "作者：", genbuf, IDLEN + 2, DOECHO, tmpfhdr.owner))
      {
        strcpy (tmpfhdr.owner, genbuf);
        dirty++;
      }

      if (getdata (b_lines - 1, 0, "日期：", genbuf, 6, DOECHO, tmpfhdr.date))
      {
        sprintf (tmpfhdr.date, "%+5s", genbuf);
        dirty++;
      }
    }

    if (getdata (b_lines - 1, 0, "確定(Y/N)?[n] ", genbuf, 3, DOECHO, 0) &&
      (*genbuf == 'y' || *genbuf == 'Y' )&& dirty)
    {
      *fhdr = tmpfhdr;
      substitute_record (direct, fhdr, sizeof (*fhdr), ent);
      if (currmode & MODE_SELECT)
      {
        int now;
        setbdir (genbuf, currboard);
        now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
        substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
      }
    }
    return RC_FULL;
  }
  return RC_NONE;
}

int
add_tag (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  int now;
  char genbuf[100];

  if (!strcmp(currboard,"Security") && !HAS_PERM(PERM_BBSADM)) return RC_NONE;
  
  /*hialan.020714 mark過或加密過的文章 不能tag*/
  if ((fhdr->filemode & FILE_MARKED) || (fhdr->filemode & FILE_REFUSE)) return RC_NONE;
  
  if (currstat == RMAIL)
  {
    fhdr->filemode ^= FILE_TAGED;
    sethomedir (genbuf, cuser.userid);
    if (currmode & SELECT)
    {
      now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
      substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
      sprintf (genbuf, "home/%s/SR.%s", cuser.userid, cuser.userid);
      substitute_record (genbuf, fhdr, sizeof (*fhdr), ent);
    }
    else
      substitute_record (genbuf, fhdr, sizeof (*fhdr), ent);
    return POS_NEXT;
  }
//  if(currstat == READING) return RC_NONE;
  if (currmode & MODE_BOARD)
  {
    fhdr->filemode ^= FILE_TAGED;
    if (currmode & MODE_SELECT)
    {
      setbdir (genbuf, currboard);
      now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
      substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
      sprintf (genbuf, "boards/%s/SR.%s", currboard, cuser.userid);
      substitute_record (genbuf, fhdr, sizeof (*fhdr), ent);
      return POS_NEXT;
    }
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);
    return POS_NEXT;
  }
  return RC_NONE;
}


int
del_tag (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  int number;

  if (currstat == RMAIL)
  {
    if (getans2(1, 0, "確定刪除標記信件? ", 0, 2, 'y') != 'n')
    {
      currfmode = FILE_TAGED;
      if (delete_files (direct, cmpfmode))
        return RC_CHDIR;
    }
    return RC_FULL;
  }
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    if (!strcmp(currboard,"Security") && !HAS_PERM(PERM_BBSADM)) return RC_NONE;

    if (getans2(1, 0, "確定刪除標記文章? ", 0, 2, 'n') == 'y')
    {
      currfmode = FILE_TAGED;
      if (currmode & MODE_SELECT)
      { 
        char xfile[PATHLEN];
        
        sprintf(xfile,"%s.vis", direct);
        unlink (direct);
        unlink(xfile);  /*加密檔案名單*/
        currmode ^= MODE_SELECT;
        setbdir (direct, currboard);
        delete_files (direct, cmpfmode);
      }

      if (number=delete_files(direct, cmpfmode))
      {
        log_board3("TAG", currboard, number);
        return RC_CHDIR;
      }
    }
    return RC_FULL;
  }
  return RC_NONE;
}

int
gem_tag (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  load_paste(); //讀入 paste_file 來定位
  if(!*paste_path)
  {
    pressanykey("尚未定位,請進入精華區中你想收錄的目錄按 [P]");
    return RC_FOOT;
  }

  if (currstat == RMAIL)
  {
    if (getans2(1, 0, "確定收錄標記信件? ", 0, 2, 'y') != 'n')
    {
      currfmode = FILE_TAGED;
      if (gem_files (direct, cmpfmode))
        return RC_CHDIR;
    }
    return RC_FULL;
  }
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    if (getans2(1, 0, "確定收錄標記文章? ", 0, 2, 'n') == 'y')
    {
      currfmode = FILE_TAGED;
      if (currmode & MODE_SELECT)
      {
        unlink (direct);
        currmode ^= MODE_SELECT;
        setbdir (direct, currboard);
        gem_files (direct, cmpfmode);
      }
      else
        gem_files(direct, cmpfmode);
      return RC_CHDIR;
    }
    return RC_FULL;
  }
  return RC_NONE;
}


int
mark (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  if (currstat == READING && !(currmode & MODE_BOARD))
    return RC_NONE;

  if (currmode & MODE_BOARD && currstat == READING)
  {
    if (fhdr->filemode & FILE_MARKED)
      deumoney (fhdr->owner, 200);
    else
      inumoney (fhdr->owner, 200);
  }

  fhdr->filemode ^= FILE_MARKED;

  if (currmode & MODE_SELECT)
  {
    int now;
    char genbuf[100];

    if (currstat != READING)
      sethomedir(genbuf, cuser.userid);
    else
      setbdir (genbuf, currboard);
    now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
    substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
  }
  else
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);

  return RC_DRAW;
}


int v_board (int, fileheader*, char*);

static int
score_note(char *prompt, fileheader *fhdr, char *direct)	//推薦加分!! 學 ptt by hialan
{
    char genbuf[80], fpath[80];
    time_t now = time(NULL);
    struct tm *ptime = localtime(&now);
    FILE *fp;
    int fd;
    
    getdata(b_lines, 0, prompt, genbuf, 56-IDLEN-4, DOECHO, 0);
    if(*genbuf == 0) return -1;

    setdirpath (fpath, direct, fhdr->filename);
    if((fd = open(fpath, O_RDONLY)) == -1) 
    {
      pressanykey("推薦失敗!!檔案有問題或有人正在推薦, 請重新推薦:)");
      return -1;
    }
          
    fp = fopen(fpath, "a");
    flock(fd, LOCK_EX);
    
    fprintf(fp, "\n\033[1;31m→ \033[33m%s\033[0;33m:%-*s\033[m推 %14.14s %0d/%0d",
                cuser.userid, 51-strlen(cuser.userid), genbuf, fromhost, 
                ptime->tm_mon+1, ptime->tm_mday);
    flock(fd, LOCK_UN);
    close(fd);
    fclose(fp);
    
    pressanykey("推薦成功\!!");
    return 1;
}

/*文章評分*/
int
score (int ent, fileheader *fhdr, char *direct)
{
  char buf[128];
  time_t now = time(0);
  char *choose[3] = {"11)加分","22)扣分", msg_choose_cancel};
  
  if (currstat == RMAIL)
    return RC_NONE;
  
  if (cuser.scoretimes <= 0 && !HAVE_PERM(PERM_SYSOP))
  {
    pressanykey("評分點數不夠無法評分!!");
    return RC_FULL;
  }
  
  if (!strcmp("guest", cuser.userid))
  {
    pressanykey("guest 不提供此功\能!!");
    return RC_FULL;
  }

  buf[0] = getans2(b_lines, 0, "請問要 ", choose, 3, 'q');
  if(!buf[0] || buf[0] < '1' || buf[0] > '3')
    return RC_DRAW;
  else if(buf[0] == '1' && fhdr->score < 99)
  {
    if(score_note("加分原因: ", fhdr, direct) < 0)
      return RC_FULL;
    else
      fhdr->score++;
  }
  else if(buf[0] == '2' && fhdr->score > -9)
  {
    if(score_note("扣分原因: ", fhdr, direct) < 0)
      return RC_FULL;
    else
      fhdr->score--;
  }
  else if(buf[0] != 'q')
  {
    if(fhdr->score >= 99 || fhdr->score <= -9)                     // 分數的上
    {                                                              // 限與下限
      sprintf(buf , "已經是最%s分了!!", fhdr->score >= 99 ? "高" : "低");    
      pressanykey(buf);
      return RC_DRAW;
    }  
  }
    
  if(!HAS_PERM(PERM_SYSOP))                                      // 站長不扣
  {                                                              // 次數但是
    ingold(1);                                                   // 不加錢
    cuser.scoretimes--;
  }

  if (currmode & MODE_SELECT)                                    // 在搜尋文章
  {                                                              // 情況下處理
    int now;
    char genbuf[100];
                                                                                
    if (currstat != READING)
      sethomedir(genbuf, cuser.userid);
    else
      setbdir (genbuf, currboard);
    now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
    substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
  }
  else
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);
                                                                                
  substitute_record (fn_passwd, &cuser, sizeof (userec), usernum); // 回存次數
  sprintf(buf , "%-12.12s 替 %-12.12s 板 [%-40.40s] 評分 [%s] %s",
    cuser.userid,currboard,fhdr->title,
    buf[0] == '1' ? "+1" : "-1",Etime(&now));
  f_cat(BBSHOME"/log/article_score.log",buf);                    // 紀錄
  if(!HAS_PERM(PERM_SYSOP))
  {
    sprintf(buf , "你的評分次數還有 %d 次 ...", cuser.scoretimes);
    pressanykey(buf);
  }
  return RC_DRAW;
}

/* itoc.001203: 文章加密 */
int
refusemark(ent, fhdr, direct)
  int ent;
   fileheader *fhdr;
   char *direct;
{
  char buf[256];
  
  if (currstat != READING)
    return RC_NONE;
                                                                                
  /*自動清掉加密名單 ... hialan.020714*/
  setdirpath (buf, direct, fhdr->filename);
  strcat(buf,".vis");
  if(dashf(buf) || (fhdr->filemode & FILE_REFUSE))
    unlink(buf);

  if((currmode & MODE_BOARD) || !strcmp(fhdr->owner, cuser.userid)) 
    fhdr->filemode ^= FILE_REFUSE;                                                                                

  if (currmode & MODE_SELECT)
  {
    int now;
    char genbuf[100];
    
    setbdir(genbuf, currboard);
    now = getindex(genbuf, fhdr->filename, sizeof(fileheader));
    substitute_record(genbuf, fhdr, sizeof(*fhdr), now);
  }
  else
    substitute_record(direct, fhdr, sizeof(*fhdr), ent);
                                                                                
  return RC_DRAW;
}


int
del_range (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char num1[8], num2[8];
  int inum1, inum2;

  if (!strcmp(currboard,"Security")) return RC_NONE;
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    getdata (1, 0, "[設定刪除範圍] 起點：", num1, 5, DOECHO, 0);
    inum1 = atoi (num1);
    if (inum1 <= 0)
    {
      outz ("起點有誤");
      return RC_FOOT;
    }
    getdata (1, 28, "終點：", num2, 5, DOECHO, 0);
    inum2 = atoi (num2);
    if (inum2 < inum1)
    {
      outz ("終點有誤");
      return RC_FULL;
    }
    
    if (getans2(1, 48, msg_sure, 0, 2, 'n') == 'y')
    {
      outmsg ("處理中,請稍後...");
      refresh ();
      if (currmode & MODE_SELECT)
      {
        int fd, size = sizeof (fileheader);
        char genbuf[100];
        fileheader rsfh;
        int i = inum1, now;
        
        if (currstat == RMAIL)
          sethomedir (genbuf, cuser.userid);
        else
          setbdir (genbuf, currboard);
        
        if ((fd = (open (direct, O_RDONLY, 0))) != -1)
        {
          if (lseek (fd, (off_t) (size * (inum1 - 1)), SEEK_SET) != -1)
          {
            while (read (fd, &rsfh, size) == size)
            {
              if (i > inum2)
                break;
              now = getindex (genbuf, rsfh.filename, size);
              strcpy (currfile, rsfh.filename);
              if (!(rsfh.filemode & FILE_MARKED))
                delete_file (genbuf, sizeof (fileheader), now, cmpfilename);
              i++;
            }
          }
          close (fd);
        }
      }
      delete_range (direct, inum1, inum2);
//      fixkeep (direct, inum1);
      return RC_NEWDIR;
    }
    return RC_FULL;
  }
  return RC_NONE;
}

#if 0
static void
lazy_delete (fhdr)
  fileheader * fhdr;
{
  char buf[20];

  sprintf (buf, "-%s", fhdr->owner);
  strncpy (fhdr->owner, buf, IDLEN + 1);
  strcpy (fhdr->title, "<< article deleted >>");
  fhdr->savemode = 'L';
}

int
del_one (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    strcpy (currfile, fhdr->filename);

    if (!update_file (direct, sizeof (fileheader), ent, cmpfilename, lazy_delete))
    {
      cancelpost (fhdr, YEA);
      lazy_delete (fhdr);
      return RC_DRAW;
    }
  }
  return RC_NONE;
}
#endif

static int
del_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  int not_owned, money;
  char genbuf[100];

  if (!strcmp(currboard,"Security")) return RC_NONE;

  if ((fhdr->filemode & (FILE_MARKED | FILE_DIGEST | FILE_REFUSE)) || (fhdr->owner[0] == '-'))
    return RC_NONE;

  not_owned = strcmp (fhdr->owner, cuser.userid);
// wildcat : 站長可以連線砍信
  if(HAS_PERM(PERM_SYSOP) && answer("是否要連線砍信(y/N)") == 'y')
    not_owned = 0;

  if (!(currmode & MODE_BOARD) && not_owned || !strcmp (cuser.userid, "guest"))
    return RC_NONE;

  getdata (1, 0, msg_del_ny, genbuf, 3, LCECHO, 0);
  if (genbuf[0] == 'y')
  {
    strcpy (currfile, fhdr->filename);
    setbfile (genbuf, currboard, fhdr->filename);
    money = (int) dashs (genbuf) / 90;
    if (!delete_file (direct, sizeof (fileheader), ent, cmpfilename))
    {
      if (currmode & MODE_SELECT)
      {
        int now;

        setbdir (genbuf, currboard);
        now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
        delete_file (genbuf, sizeof (fileheader), now, cmpfilename);
      }
      cancelpost (fhdr, not_owned);
      if (!not_owned && !(currbrdattr & BRD_NOCOUNT) && !HAS_PERM(PERM_SYSOP))
      {
        UPDATE_USEREC;
        move (b_lines - 1, 0);
        clrtoeol ();
        if (money < 1) money = 1;
        if(cuser.goldmoney > money)
          degold (money);
        else
          demoney(money*10000);
        pressanykey ("%s，您的文章減為 %d 篇，支付清潔費 %d 金", msg_del_ok,
          cuser.numposts > 0 ? --cuser.numposts : cuser.numposts, money);
        substitute_record (fn_passwd, &cuser, sizeof (userec), usernum);
      }
      return RC_CHDIR;
    }
  }
  return RC_FULL;
}


save_mail (int ent, fileheader * fh, char *direct)
{
  fileheader mhdr;
  char fpath[PATHLEN];
  char genbuf[PATHLEN];
  char *p;

  if (ent < 0)
    strcpy (fpath, direct);
  else
  {
    strcpy (genbuf, direct);
    if (p = strrchr (genbuf, '/'))
      * p = '\0';
    sprintf (fpath, "%s/%s", genbuf, fh->filename);
  }
  if (!dashf (fpath) || !HAS_PERM (PERM_BASIC))
  {
    bell ();
    return RC_NONE;
  }
  sethomepath (genbuf, cuser.userid);
  stampfile (genbuf, &mhdr);
  unlink (genbuf);
  f_cp (fpath, genbuf, O_TRUNC);
  if (HAS_PERM (PERM_SYSOP))
    strcpy (mhdr.owner, fh->owner);
  else
    strcpy (mhdr.owner, cuser.userid);
  strncpy (mhdr.title, fh->title + ((currstat == ANNOUNCE) ? 3 : 0), TTLEN);
  mhdr.savemode = '\0';
  mhdr.filemode = FILE_READ;
  sethomedir (fpath, cuser.userid);
  if (rec_add (fpath, &mhdr, sizeof (mhdr)) == -1)
  {
    bell ();
    return RC_NONE;
  }
  return POS_NEXT;
}

/* ----------------------------------------------------- */
/* 看板備忘錄、文摘、精華區                              */
/* ----------------------------------------------------- */

static int
board_edit ()
{
  boardheader bp;
  boardheader * getbcache ();
  int bid, mode = 0;
  time_t now = time(0);
  char *board_admin_menu[12] = {"00)取消",
      			        "11)改中文名稱",
      			        "22)看板說明",
      			   	"33)進板畫面",
      				"44)可見名單",
      				"55)設密碼",
      				"66)改文章類別",
      				"77)買上限",
      				"88)看板PO文注意事項", 
      				"99)改屬性",
      				"aA)看今日紀錄",
      				"bB)改類別"};  

  if (currmode & MODE_BOARD)
  {
    char genbuf[BTLEN], buf[PATHLEN], ans;
    bid = getbnum (currboard);

    if (rec_get (fn_board, &bp, sizeof (boardheader), bid) == -1)
    {
      pressanykey (err_bid);
      return -1;
    }

    if (bp.brdattr & BRD_PERSONAL || HAS_PERM(PERM_SYSOP))
      mode = 1;

    switch(win_select("看板管理", "", board_admin_menu, (mode == 1) ? 12 : 9 ,'0'))
    {
      case '1':
        move (1, 0);
        clrtoeol ();
        getdata (1, 0, "請輸入看板新中文敘述:"
          ,genbuf, BTLEN - 16, DOECHO, bp.title + 7);
        if (!genbuf[0]) return 0;
        strip_ansi (genbuf, genbuf, 0);
        if(strcmp(bp.title+7,genbuf))
        {
          sprintf(buf,"%-13.13s 更換看板 %s 敘述 [%s] -> [%s] , %s",
            cuser.userid,bp.brdname,bp.title+7,genbuf,ctime(&now));
          f_cat(BBSHOME"/log/board_edit",buf);
          log_usies ("NameBoard", currboard);
          strcpy (bp.title + 7, genbuf);
        }
        break;

      case '2':

        change_bp(2, "對本看板的描述 (共三行)", bp.desc);        
        sprintf(buf,"%-13.13s 更換看板 %s 說明 , %s",
          cuser.userid,bp.brdname,ctime(&now));
        f_cat(BBSHOME"/log/board_edit",buf);
        log_usies ("SetBoardDesc", currboard);
        break;

      case '3':
        setbfile (buf, currboard, fn_notes);
        if (vedit (buf, NA) == -1)
          pressanykey (msg_cancel);
        else
        {
          int aborted;

          getdata (3, 0, "請設定有效期限(0 - 9999)天？", buf, 5, DOECHO, "9999");
          aborted = atol (buf);
          bp.bupdate = aborted ? time (0) + aborted * 86400 : 0;
        }
        break;

      case '4':
        setbfile(buf, currboard, FN_LIST);
        ListEdit(buf);
        return RC_FULL;
        
      case '5':
      {
        char genbuf[PASSLEN+1],buf[PASSLEN+1];

        move (1, 0);
        clrtoeol ();
        if(!HAS_PERM(PERM_ALLBOARD))
        {
          if(!getdata (1, 0, "請輸入原本的密碼" ,genbuf, PASSLEN, PASS, 0) ||
             !chkpasswd(bp.passwd, genbuf))
          {
               pressanykey("密碼錯誤");
               return -1;
          }
        }
        if (!getdata(1, 0, "請設定新密碼：", genbuf, PASSLEN, PASS,0))
        {
          pressanykey("密碼設定取消, 繼續使用舊密碼");
          return -1;
        }
        strncpy(buf, genbuf, PASSLEN);
 
        getdata(1, 0, "請檢查新密碼：", genbuf, PASSLEN, PASS,0);
        if (strncmp(genbuf, buf, PASSLEN)) 
        {
          pressanykey("新密碼確認失敗, 無法設定新密碼");
          return -1;
        }
        buf[8] = '\0';
        strncpy(bp.passwd, genpasswd(buf), PASSLEN);
        log_usies ("SetBrdPass", currboard);
      }
      break;      

      case '6':
      {
	int i, ch2;
        FILE *prefixfile;
        char buf[50];
        char class[11][50];
        char *classpoint[11];
        
        /*文章類別 by hialan 3.21.2002*/
        b_load_class(currboard);
        
        /*複製到指標陣列*/
        for(i = 0;i < 11;i++)
          classpoint[i] = class[i];
   
        for(i = 0;i<9;i++)            
          sprintf(class[i],"%d%d.%s", i+1, i+1, postprefix[i]);
        
        sprintf(class[9],"dD.回復預設值");
        sprintf(class[10],"qQ.離開");          

        ch2 = '1';        

        do
        {
          ch2 = win_select("修改文章類別", "請選擇要修改的類別 ", classpoint, 11, ch2);

          if(ch2 == 'd')
          {
            if(win_select("修改文章類別", "確定要回覆預設值嗎? ", 0, 2, 'n')== 'y')
            {
              setbfile (buf, currboard, FN_POSTPREFIX);
              unlink(buf);
              return RC_FULL;
            }
            else
              continue;
          }

          if(ch2 != 'q') 
          {
            getdata(b_lines-1, 0, "請輸入新類別", buf, 21, DOECHO, postprefix[ch2-'1']);
            if(*buf != '\0')  /*如果使用者沒輸入東西,則跳開*/
 	    {
              strcpy(postprefix[ch2 - '1'], buf);
              sprintf(class[ch2 - '1'],"%c%c.%s", ch2, ch2, postprefix[ch2 - '1']);            
            }
            move(b_lines-1, 0);
            clrtobot(b_lines-1,0);
          }
        }while(ch2 != 'q');

        if(win_select("修改文章類別", "確定要修改嗎? ", 0, 2, 'y')== 'y') 
        {
          setbfile (buf, currboard, FN_POSTPREFIX);                    
          if((prefixfile = fopen(buf,"w")) != NULL)
          {
            for(i=0;i<9;i++)
              fprintf(prefixfile,"%s\n",postprefix[i]);
            fclose(prefixfile);
          }
        }
        else
          pressanykey("文章類別沒有改變!!");
      }
      break;
      
      case '7':
      {
        clrchyiuan(1, 15);
        move(3, 0);
        prints("\
目前看板的文章上限為 %-5d 篇
          保留時間為 %-5d 天\n",bp.maxpost,bp.maxtime);
        outs("一個單位為[1;32m一百篇文章[m或是[1;32m三十天[m , 一個單位需 [1;33m3000 金幣[m");
        getdata(7, 0, "你要 (1)買文章上限 (2)買保存時間", buf, 2, DOECHO, 0);
        if (buf[0] == '1' || buf[0] == '2')
        {
          int num = 0;

              while (num <= 0)
              {
                getdata(9, 0, "你要買幾個單位", genbuf, 3, DOECHO, 0);
                num = atoi(genbuf);
              }

              if (check_money(num * 3000, GOLD))
                break;

              if (buf[0] == '1')
              {
                if (bp.maxpost >= 99999)
                {
                  pressanykey("文章數已達上限");
                  break;
                }
                else
                {
                  bp.maxpost += num*100;
                  sprintf(buf, "%-13.13s 購買看板 %s 文章上限 %d 篇 , %s",
                    cuser.userid, bp.brdname,num*100, ctime(&now));
                  f_cat(BBSHOME"/log/board_edit", buf);
                  log_usies ("BuyPL", currboard);
                  pressanykey("看板文章上限增加為 %d 篇", bp.maxpost);
                }
              }
              else
              {
                if (bp.maxtime >= 9999)
                {
                  pressanykey("保存時間已達上限");
                  break;
                }
                else
                {
                  bp.maxtime += num * 30;
                  sprintf(buf,"%-13.13s 購買看板 %s 文章保留時間 %d 天 , %s",
                    cuser.userid,bp.brdname,num*30,ctime(&now));
                  f_cat(BBSHOME"/log/board_edit",buf);
                  log_usies ("BuyBT", currboard);
                  pressanykey("看板文章保留時間增加為 %d 天",bp.maxtime);
                }
              }
              degold(num*3000);
        }
      }
        break;

       case '8':	//ychia
       {
  	 setbfile(buf, currboard,  FN_POST_NOTE );
         if(more(buf,NA) == -1)  more(FN_POST_NOTE , NA);
         if (win_select("自訂post注意事項", "是否用自訂post注意事項", 0, 2, 'n') == 'y')
           vedit(buf, NA, NULL);
         else
           unlink(buf);
       }
       break;


//以下是 私人版才有的功能!!
          case '9':
          {
            int oldattr=bp.brdattr;
            char *brd_type[3]={"oO)開放","pP)私人","hH)隱藏"};

            ans = getans2(1, 0,"看板狀態更改為", brd_type, 3, 'o');
    
            if(ans == 'p')
            {
              bp.brdattr &= ~BRD_POSTMASK;
              bp.brdattr |= BRD_HIDE;
            }
            else if(ans == 'h')
            {
              bp.brdattr |= BRD_POSTMASK;
              bp.brdattr |= BRD_HIDE;
            }
            else
            {
              if(answer("是否要讓 guest 看到你的板? (Y/n)") == 'n')
                bp.brdattr &= ~BRD_POSTMASK;
              else
                bp.brdattr |= BRD_POSTMASK;
              bp.brdattr &= ~BRD_HIDE;
            }
            if(bp.brdattr != oldattr)
            {
              sprintf(buf,"%-13.13s 更改看板 [%s] 之屬性為 %s , %s",
                cuser.userid,bp.brdname,
                ans == 'p' ? "私人" : ans == 'h' ? "隱藏" : "開放",ctime(&now));
              f_cat(BBSHOME"/log/board_edit",buf);
              log_usies("ATTR_Board",currboard);
            }
          }
          break;

          case 'a':
          {
            sprintf(buf,"/usr/bin/grep \"USE %s \" %s/usboard > %s/tmp/usboard.%s",
              currboard, BBSHOME, BBSHOME, currboard);
            system(buf);
            sprintf(buf,BBSHOME"/tmp/usboard.%s",currboard);
            more(buf, YEA);
            log_usies("BOARDLOG", currboard);
          }
          break;

          case 'b':
            move (1, 0);
            clrtoeol ();
            getdata (1, 0, "請輸入看板新類別:",genbuf, 5, DOECHO, bp.title );
            if (!genbuf[0]) return 0;
            strip_ansi (genbuf, genbuf, 0);
            if(strncmp(bp.title,genbuf,4))
            {
              sprintf(buf,"%-13.13s 更換看板 %s 類別 [%-4.4s] -> [%s] , %s",
                cuser.userid,bp.brdname,bp.title,genbuf,ctime(&now));
              f_cat(BBSHOME"/log/board_edit",buf);
              log_usies("PREFIX",currboard);
              strncpy (bp.title , genbuf, 4);
            }
          break;

      default:
        pressanykey("放棄修改");
        break;
    }
    substitute_record (fn_board, &bp, sizeof (boardheader), bid);
    touch_boards ();
    return RC_FULL;
  }
  return 0;
}


/* wildcat modify 981221 */
int
b_notes ()
{
  char buf[64];

  setbfile (buf, currboard, fn_notes);
  if (more (buf, YEA) == -1) // 可以看多頁
  {
    clear ();
    pressanykey ("本看板尚無「備忘錄」。");
  }
  return RC_FULL;
}


int
board_select ()
{
  struct stat st;
  char fpath[80];
  char genbuf[100];
  currmode &= ~(MODE_SELECT | MODE_TINLIKE);

  sprintf (genbuf, "SR.%s", cuser.userid);
  setbfile (fpath, currboard, genbuf);
  unlink (fpath);

  /* shakalaca.000112: 超過 30min 才將 index 刪除, 作 cache 用 */
  setbfile (fpath, currboard, "SR.thread");
  if (stat(fpath, &st) == 0 && st.st_mtime < time(0) - 60 * 30)
    unlink (fpath);

  if (currstat == RMAIL)
    sethomedir (currdirect, cuser.userid);
  else
    setbdir (currdirect, currboard);

  return RC_NEWDIR;
}


int
board_digest ()
{
  if (currmode & MODE_SELECT)
    board_select ();

  currmode ^= MODE_DIGEST;
  if (currmode & MODE_DIGEST)
    currmode &= ~MODE_POST;
  else if (haspostperm (currboard))
    currmode |= MODE_POST;

  setbdir (currdirect, currboard);
  return RC_NEWDIR;
}


static int
good_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[PATHLEN];
  char genbuf2[PATHLEN];
  fileheader digest;

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("加密文章不能收入到文摘裡!!");
    return RC_DRAW;
  }
  
  memcpy (&digest, fhdr, sizeof (digest));
  digest.filename[0] = 'G';

  if ((currmode & MODE_DIGEST) || !(currmode & MODE_BOARD))
    return RC_NONE;

  if (fhdr->filemode & FILE_DIGEST)
  {
    int now;
    setbfile(genbuf2, currboard, ".Names");
    now = getindex (genbuf2, digest.filename, sizeof(fileheader));
    strcpy (currfile, digest.filename);
    delete_file (genbuf2, sizeof (fileheader), now, cmpfilename);
    sprintf (genbuf2, BBSHOME "/boards/%s/%s", currboard, currfile);
    unlink (genbuf2);
    fhdr->filemode = (fhdr->filemode & ~FILE_DIGEST);
//    deumoney (fhdr->owner, 500);
  }
  else
  {
    char *ptr, buf[64];
    strcpy (buf, direct);
    ptr = strrchr (buf, '/') + 1;
    ptr[0] = '\0';
    sprintf (genbuf, "%s%s", buf, digest.filename);
    if (!dashf (genbuf))
    {
      digest.savemode = digest.filemode = 0;
      sprintf (genbuf2, "%s%s", buf, fhdr->filename);
      f_cp (genbuf2, genbuf, O_TRUNC);
      strcpy (ptr, fn_mandex);
      rec_add (buf, &digest, sizeof (digest));
    }
    fhdr->filemode = (fhdr->filemode & ~FILE_MARKED) | FILE_DIGEST;
//    inumoney (fhdr->owner, 500);
  }
  substitute_record (direct, fhdr, sizeof (*fhdr), ent);
  if (currmode & MODE_SELECT)
  {
    int now;
    char genbuf[100];
    setbdir (genbuf, currboard);
    now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
    substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
  }
  return RC_DRAW;
}


/* hialan 整理 from i_read_key*/
int
write_msg (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  user_info *owneronline = (user_info *)searchowner(fhdr->owner);
  if (owneronline != NULL) talk_water(owneronline);
  return RC_FULL;
}

int
post_mail_uncode (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
    char fname[PATHLEN];
       
    setdirpath(fname, direct, fhdr->filename);
    if (dashf(fname))
      mail_forward(fhdr, direct, 'U');
    return RC_FULL;
}

int
post_mail (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
    char fname[PATHLEN];
       
    setdirpath(fname, direct, fhdr->filename);
    if (dashf(fname))
      mail_forward(fhdr, direct, 'F');
    return RC_FULL;
}

int
post_query (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  return my_query(fhdr->owner);
}  

static int post_vote()
{
  if (currstat != ANNOUNCE)
    DL_func("SO/vote.so:b_vote");
  return RC_FULL;
}

static int post_b_results()
{
  if (currstat != ANNOUNCE)
    DL_func("SO/vote.so:b_results");
  return RC_FULL;
}
/* ----------------------------------------------------- */
/* 看板功能表                                            */
/* ----------------------------------------------------- */

struct one_key read_comms[] =
{
  KEY_TAB, board_digest,	0,"進入/退出 文摘",0,
  'b', b_notes,        		0,"看進版畫面",0,
  'c', cite,              PERM_BM,"收錄精華",0,
  'r', read_post,        	0,"閱\讀文章",0,
  'z', man,         	 	0,"進入精華區",0,
  'D', del_range,	  PERM_BM,"大 D 砍信",0,
  Ctrl ('S'), save_mail,	0,"存入信箱",0,
  'E', edit_post,        	0,"修改文章",0,
  'T', edit_title,        PERM_BM,"修改標題",0,
  's', do_select,        	0,"選擇看板",0,
  'B', board_edit,        PERM_BM,"看板編輯",0,
  't', add_tag,           PERM_BM,"標記文章",0,
  Ctrl ('D'), del_tag,    PERM_BM,"刪除標記文章",0,
  'x', cross_post,       	0,"轉貼",0,
  'g', good_post,         PERM_BM,"收到文摘中",0,
  'y', reply_post,       	0,"回覆文章",0,
  'd', del_post,         	0,"刪除文章",0,
  'm', mark,              PERM_BM,"Mark 文章",0,
  'X', refusemark,       	0,"文章加密",0,
  Ctrl ('P'), do_post,   	0,"發表文章",0,
  'C', gem_tag,           PERM_BM,"收錄標記文章",0,
  Ctrl ('C'), Cite_posts,	0,"直接收錄文章至精華區",0,
  '%', score,		 	0,"文章評分",0,
  'v', v_board,		 	0,"板內v板",0,
  'w', write_msg,	 PERM_LOGINOK,"板內丟水球",0,
  'F', post_mail,	 PERM_FORWARD,"將文章寄回 Internet 郵箱",0,
  'U', post_mail_uncode, PERM_FORWARD,"將文章 uncode 後寄回 Internet 郵箱",0,
  Ctrl ('Q'), post_query,	0,"板內 q 人",0,
  'V', post_vote,	 	0,"參與投票",0,
  'R', post_b_results,	 	0,"看投票結果",0,
  '\0', NULL, 0, NULL,0};

void Read ()
{
  int mode0 = currutmp->mode;
  int currmode0 = currmode;
  int stat0 = currstat;
  int bid;
  char buf[40];
  time_t startime = time (0);
  extern struct BCACHE * brdshm;

  resolve_boards ();
  setutmpmode (READING);
  set_board ();
  if (board_visit_time < board_note_time)
  {
    setbfile (buf, currboard, fn_notes);
    more (buf, YEA);
  }  

  bid = getbnum (currboard);
  currutmp->brc_id = bid;    
  if(Ben_Perm(&brdshm->bcache[bid]-1) != 1)
  {
    pressanykey(P_BOARD);
    return;
  }  

  setbdir (buf, currboard);
  curredit &= ~EDIT_MAIL;
  i_read (READING, buf, readtitle, doent, read_comms, &(brdshm->total[bid - 1]));

  log_board (currboard, time (0) - startime);
  log_board2 (currboard, time (0) - startime);
  brc_update ();

  currutmp->brc_id = 0;
  currmode = currmode0;
  currutmp->mode = mode0;
  currstat = stat0;
  return;
}


void
ReadSelect ()
{
  if (do_select() == RC_NEWDIR)
    Read ();
}


int
Select ()
{
  setutmpmode (SELECT);
  do_select ();
  return 0;
}


int
Post ()
{
  do_post ();
  return 0;
}

void
cancel_post(fhdr, fpath)
  fileheader *fhdr;
  char *fpath;
{
#define NICK_LEN    80
  int fd;

  if ((fhdr->savemode == 'S') &&/* 外轉信件 */
    ((fd = open(fpath, O_RDONLY)) >= 0))
  {
    char *ptr, *left, *right, nick[NICK_LEN];
    FILE *fout;
    int ch;

    ptr = nick;
    ch = read(fd, ptr, NICK_LEN);
    close(fd);
    ptr[ch] = '\0';
    if (!strncmp(ptr, str_author1, LEN_AUTHOR1) ||
      !strncmp(ptr, str_author2, LEN_AUTHOR2))
    {
      if (left = (char *) strchr(ptr, '('))
      {
        right = NULL;
        for (ptr = ++left; ch = *ptr; ptr++)
        {
          if (ch == ')')
            right = ptr;
          else if (ch == '\n')
            break;
        }

        if (right != NULL)
        {
          *right = '\0';
          log_board3("DEL", currboard, 1);
  
          if (fout = fopen(BBSHOME"/innd/cancel.bntp", "a"))
          {
            fprintf(fout, "%s\t%s\t%s\t%s\t%s\n",
              currboard, fhdr->filename, fhdr->owner    /* cuser.userid */
              ,left, fhdr->title);
            fclose(fout);
          }
        }
      }
    }
#undef  NICK_LEN
  }
}

/* ----------------------------------------------------- */
/* 離開 BBS 站                                           */
/* ----------------------------------------------------- */


void
note()
{
  static char *fn_note_tmp = "note.tmp";
  static char *fn_note_dat = "note.dat";
  int total, i, collect, len;
  struct stat st;
  char buf[256], buf2[256];
  int fd, fx;
  FILE *fp, *foo;
  struct notedata
  {
    time_t date;
    char userid[IDLEN + 1];
    char username[19];
    char buf[3][80];
  };
  struct notedata myitem;
//  if(check_money(1,GOLD)) return;
  setutmpmode(EDNOTE);
  myitem.buf[0][0] = myitem.buf[1][0] = myitem.buf[2][0] = '\0';
  do
  {
    char *choose[3]={"sS)儲存","eE)重新來過", msg_choose_cancel};

    change_bp(16, "請留言 (至多三行)", myitem.buf);
    
   move(21,0);
   clrtobot();
    buf[0] = getans2(20, 5, "", choose, 3, 'q');
/*
woju
*/
    if (buf[0] == 'q' || i == 0 && *buf != 'e')
      return;
  } while (buf[0] == 'e');
//  degold(1);
  strcpy(myitem.userid, cuser.userid);
  strncpy(myitem.username, cuser.username, 18);
  myitem.username[18] = '\0';
  time(&(myitem.date));

  /* begin load file */

  if ((foo = fopen(BBSHOME"/.note", "a")) == NULL)
    return;

  if ((fp = fopen(fn_note_ans, "w")) == NULL)
    return;

  if ((fx = open(fn_note_tmp, O_WRONLY | O_CREAT, 0644)) <= 0)
    return;

  if ((fd = open(fn_note_dat, O_RDONLY)) == -1)
  {
    total = 1;
  }
  else if (fstat(fd, &st) != -1)
  {
    total = st.st_size / sizeof(struct notedata) + 1;
    if (total > MAX_NOTE)
      total = MAX_NOTE;
  }

  fputs("[1m                             "COLOR1" [33m◆ [37m心 情 留 言 板 [33m◆ [m \n\n",fp);
  collect = 1;

  while (total)
  {
    sprintf(buf, "[44m[1;36m┌┴ [33m%s[37m(%s)",
      myitem.userid, myitem.username);
    len = strlen(buf);
    strcat(buf," [36m" + (len&1));

    for (i = len >> 1; i < 38; i++)
      strcat(buf, "─");
    sprintf(buf2, "─[33m %.14s  [36m┴┐[m\n",
      Etime(&(myitem.date)));
    strcat(buf, buf2);
    fputs(buf, fp);

    if (collect)
      fputs(buf, foo);

    sprintf(buf, "[1;44m[36m└┐[37m%-70s[36m┌┘[m\n",myitem.buf[0]);
    if(*myitem.buf[1])
    {
      sprintf(buf2, "  [1;44m[36m│[37m%-70s[36m│[m\n",myitem.buf[1]);
      strcat(buf, buf2);
    }
    if(*myitem.buf[2])
    {
      sprintf(buf2, "  [1;44m[36m│[37m%-70s[36m│[m\n",myitem.buf[2]);
      strcat(buf, buf2);
    }
    fputs(buf,fp);
    if (collect)
    {
      fputs(buf, foo);
      fclose(foo);
      collect = 0;
    }
    write(fx, &myitem, sizeof(myitem));

    if (--total)
      read(fd, (char *) &myitem, sizeof(myitem));
  }
  fclose(fp);
  close(fd);
  close(fx);
  f_mv(fn_note_tmp, fn_note_dat);
  more(fn_note_ans, YEA);
}


int
m_sysop()
{
  FILE *fp;
  char genbuf[128];

  setutmpmode(MSYSOP);
  if (fp = fopen(BBSHOME"/etc/sysop", "r"))
  {
    int i, j;
    char *ptr;

    struct SYSOPLIST
    {
      char userid[IDLEN + 1];
      char duty[40];
    }sysoplist[9];

    j = 0;
    while (fgets(genbuf, 128, fp))
    {
      if (ptr = strchr(genbuf, '\n'))
      {
        *ptr = '\0';
        ptr = genbuf;
        while (isalnum(*ptr))
           ptr++;
        if (*ptr)
        {
          *ptr = '\0';
          do
          {
            i = *++ptr;
          } while (i == ' ' || i == '\t');
          if (i)
          {
            strcpy(sysoplist[j].userid, genbuf);
            strcpy(sysoplist[j++].duty, ptr);
          }
        }
      }
    }


    move(12, 0);
    clrtobot();
    prints("%16s   %-18s權責劃分\n\n", "編號", "站長 ID"/*, msg_seperator*/);

    for (i = 0; i < j; i++)
    {
      prints("%15d.   [1;%dm%-16s%s[0m\n",
        i + 1, 31 + i % 7, sysoplist[i].userid, sysoplist[i].duty);
    }
    prints("%-14s0.   [1;%dm離開[0m", "", 31 + j % 7);
    getdata(b_lines - 1, 0, "                   請輸入代碼[0]：", genbuf, 4, DOECHO,"1");
    i = genbuf[0] - '0' - 1;
    if (i >= 0 && i < j)
    {
      clear();
      do_send(sysoplist[i].userid, NULL);
    }
  }
  fclose(fp);
  return 0;
}


int
Goodbye()
{
  if(win_select("離開" BOARDNAME, "您確定要離開【 " BOARDNAME " 】嗎？", 0, 2, 'n') != 'y')
    return 0;

  movie(999);
  if (cuser.userlevel)
  {
    char *prompt[3]={"gG)隨風而逝","mM)托夢站長","nN)我要吶喊"};
    char ans = win_select("離開" BOARDNAME, "", prompt, 3, 'g');

    if (ans == 'm')
      m_sysop();
    else if (ans == 'n')
      note();
  }

  t_display();
  clear();
  prints("[1;31m親愛的 [31m%s([37m%s)[31m，別忘了再度光臨"COLOR1
    " %s [40;33m！\n\n以下是您在站內的註冊資料:[m\n",
    cuser.userid, cuser.username, BoardName);
  user_display(&cuser, 0);
  pressanykey(NULL);

  more(BBSHOME"/etc/Logout",NA);
  pressanykey(NULL);
  if (currmode)
    u_exit("EXIT ");
  reset_tty();
  exit(0);
}

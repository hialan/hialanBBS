/*-------------------------------------------------------*/
/* bbs.c        ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : bulletin boards' routines                    */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

#define WORLDSNUM   10   /*¤å³¹­n´X­Ó¦r¤~ºâ*/

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
       
    /*¦pªGÁÙ¨S¶W¹L5¬í´N·|ª½±µ¸õ¶},©Ò¥Husetime­n¦bopenfile«e­±*/
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
  uid = strtok(buf, "/");       /* shakalaca.990721: §ä²Ä¤@­ÓªO¥D */
  while (uid)
  {
    if (!InNameList(uid) && searchuser(uid))
      AddNameList(uid);
    uid = strtok(0, "/");       /* shakalaca.990721: «ü¦V¤U¤@­Ó */
  }
  return 0;
}

/* shakalaca.990721: ©Ò¦³ BM ¦W³æ */
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
    brd_title = "¼x¨D¤¤";

  sprintf (currBM, "ªO¥D¡G%.22s", brd_title);
  brd_title = (bp->bvote == 1 ? "¥»¬ÝªO¶i¦æ§ë²¼¤¤" : bp->title + 7);

  currmode = (currmode & MODE_DIRTY) | MODE_STARTED;
  if (HAS_PERM (PERM_ALLBOARD) ||
      (HAS_PERM (PERM_BM) && is_BM (currBM + 6)))
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
    prints("%s\033[30m¢«\033[37m¬ÝªO¢@¤å³¹¢¨%s¨t¦C%s\033[37m¢©¤åºK¢@ºëµØ°Ï\033[36;40m¢©\033[m",
    	   COLOR1, COLOR3, COLOR1);
  else if(currmode & MODE_DIGEST)
    prints("%s\033[30m¢«\033[37m¬ÝªO¢@¤å³¹¢@¨t¦C¢¨%s¤åºK%s\033[37m¢©ºëµØ°Ï\033[36;40m¢©\033[m",
    	   COLOR1, COLOR3, COLOR1);  
  else
    prints("%s\033[30m¢«\033[37m¬ÝªO¢¨%s¤å³¹%s\033[37m¢©¨t¦C¢@¤åºK¢@ºëµØ°Ï\033[36;40m¢©\033[m",
    	   COLOR1, COLOR3, COLOR1);
  outs("   ^P)µoªí  z)ºëµØ°Ï  TAB)¤åºK  ^X)§Ö³t¿ï³æ ");

  move(2, 0);  
  prints("%s  ½s¸¹ ", COLOR3);

  if (currmode & MODE_TINLIKE)
    outs ("SC½g ¼Æ");
  else
    outs ("SC¤é ´Á");
  outs ("  §@  ªÌ       ¤å  ³¹  ¼Ð  ÃD                                    [m");
}

void readfoot(char tag)
{
  move(b_lines, 0);
  clrtoeol();    
  switch(tag)
  {
    case 1:  //¤å³¹
    case 2:  //«H¥ó
      if ((currstat == RMAIL) || (currstat == READING))
      {
        prints("%s  %s  %s       =[]<>)¥DÃD¦¡¾\\Åª  ¡ö¡ô¡õ¡÷|PgUp|PgDn|Home|End)¾ÉÄý  h)»¡©ú \033[m",
                COLOR2, (tag==2) ? "«H¥ó¦Cªí" : "¤å³¹¿ïÅª", COLOR3);
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
  
  /*wildcat ¤å³¹µû¤À*/

  if(currstat == RMAIL && ent->filemode & FILE_REPLYOK) //hialan:§PÂ_'R'ªº¦a¤è
  							//¦]¬°RMAIL¨S¥Î¨ìµû¤À
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
    if (ent->filemode & FILE_REFUSE)  /* ¥[±Kªº¤å³¹«e­±¥X²{ X ¦r¼Ë */
      sprintf(type ,"[1;31m%c",(type[0] == ' ') ? 'x' : 'X');
  }
  else
  {
    usint tmpmode = ent->filemode;
    if (ent->filemode & FILE_REPLYOK)	//¦]¬°¥[¤F Reply ªº­È´N¤£¬O¨º­Ó¼Æ¦r¤F
      tmpmode ^= FILE_REPLYOK;
    sprintf(type,"%c","+ Mm"[tmpmode]); 
  }

  if (ent->filemode & FILE_TAGED)
    sprintf(type,"[1;32m%c",(type[0] == ' ') ? 't' : 'T');

  title = str_ttl (mark = ent->title);
  if (title == mark)
  {
    color = '6';
    mark = "¡¼";
  }
  else
  {
    color = '3';
    mark = "R:";
  }

  if (title[44])
    strcpy (title + 44, " ¡K");  /* §â¦h¾lªº string ¬å±¼ */

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
/* §ï¨} innbbsd Âà¥X«H¥ó¡B³s½u¬å«H¤§³B²zµ{§Ç             */
/* ----------------------------------------------------- */

outgo_post(fh, board)
  fileheader *fh;
  char *board;
{
  char buf[256];
  if(strcmp(fh->owner,cuser.userid))
    sprintf (buf, "%s\t%s\t%s\t%s\t%s", board,
      fh->filename, fh->owner, "Âà¿ý", fh->title);
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
/* µoªí¡B¦^À³¡B½s¿è¡BÂà¿ý¤å³¹                            */
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
  sprintf (genbuf, "±Ä¥Î­ì¼ÐÃD¡m%.60s¡n¶Ü?", save_title);
  genbuf2 = getans2(row, 0,genbuf, 0, 2,'y');
  if (genbuf2 == 'n')
    getdata (++row, 0, "¼ÐÃD¡G", save_title, TTLEN, DOECHO, 0);
}


static void
do_reply (fhdr)
     fileheader *fhdr;
{
  char genbuf;

// Ptt ¬ÝªO³s¸p¨t²Î
  if(!strcmp(currboard,VOTEBOARD))
    do_voteboardreply(fhdr);
//    DL_func("SO/votebrd.so:va_do_voteboardreply",fhdr);
  else
  {
    char *choose[4]={"fF)¬ÝªO","mM)§@ªÌ«H½c","bB)¨âªÌ¬Ò¬O", msg_choose_cancel};
      
    genbuf = getans2(b_lines - 1, 0,"¡¶ ¦^À³¦Ü ",choose,4,'f');
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


brdperm (char *brdname, char *userid)
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

#ifdef POSTNOTIFY
do_postnotify (char *fpath)
{
  fileheader mhdr;
  char title[256], buf1[80], buf[80];
  FILE *fp;
  char genbuf[256];

  sethomefile (genbuf, cuser.userid, "postnotify.ok");
  if (fp = fopen (genbuf, "r"))
    {
      char last_fname[80];
      boardheader *bp;
      boardheader *getbcache ();

      strcpy (last_fname, fpath);
      bp = getbcache (currboard);
      while (fgets (buf, 80, fp))
  if (brdperm (currboard, buf))
    {
      sethomefile(buf1, buf, "postlist");
      if (!belong_list(buf1, buf))
      {
        del_distinct(genbuf, cuser.userid);
        fclose(fp);
        return;
      }

      sethomepath (buf1, buf);
      stampfile (buf1, &mhdr);
      strcpy (mhdr.owner, cuser.userid);
      strcpy (mhdr.title, "[·s]");
      strncat (mhdr.title, save_title, TTLEN - 4);
      mhdr.savemode = 0;
      mhdr.filemode = 0;
      sethomedir (title, buf);
      rec_add (title, &mhdr, sizeof (mhdr));
      unlink (buf1);
      f_cp (last_fname, buf1, O_TRUNC);
      strcpy (last_fname, buf1);
    }
      fclose (fp);
    }
}
#endif

int
do_copy_post (board, fpath, filemode)   //½Æ»s¤å³¹¨ì¬ÝªO
  char *board, *fpath;
  uschar filemode;
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

extern long wordsnum;    /* ­pºâ¦r¼Æ */
#define PREFIXLEN 50	//¤å³¹Ãþ§O³Ì¤jªø«×

static char postprefix[10][PREFIXLEN];

static int b_load_class(char *bname) /*¤å³¹Ãþ§O by hialan 3.21.2002*/
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
        strcpy(postprefix[0],"[¤½§i]");
        strcpy(postprefix[1],"[·s»D]");
        strcpy(postprefix[2],"[¶¢²á]");
        strcpy(postprefix[3],"[¤å¥ó]");
        strcpy(postprefix[4],"[°ÝÃD]");
        strcpy(postprefix[5],"[³Ð§@]");
        strcpy(postprefix[6],"[ÀH«K]");
        strcpy(postprefix[7],"[´ú¸Õ]");
        strcpy(postprefix[8],"[¨ä¥L]");
      }
      return 0;
}

int make_cold(char *board, char *save_title, int money, char *fpath)
{
  int cold;

  /*­pºâ§N«×*/
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
    do_copy_post("ColdKing", fpath, 0); // Âà¿ý¤å³¹
    
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
    pressanykey("¥DÃD¦¡¾\\Åª½Ð¤Åµoªí¤å³¹");
    return RC_NONE;
  }
  
  bp = getbcache (currboard);
  if (!(currmode & MODE_POST) || !brdperm (currboard, cuser.userid))
  {
    pressanykey ("¹ï¤£°_¡A±z¥Ø«eµLªk¦b¦¹µoªí¤å³¹¡I");
    return RC_NONE;
  }

// Ptt ¬ÝªO³s¸p¨t²Î
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
      pressanykey ("¹ï¤£°_,¦¹ªO¥u­ã¬ÝªO¦n¤Í¤~¯àµoªí¤å³¹,½Ð¦VªO¥D¥Ó½Ð");
      return RC_FULL;
    }

  setbfile(genbuf, currboard, FN_POST_NOTE ); /* ychia.021212:¦Û­q¤å³¹µoªí­n»â */
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
    
    sprintf(win_title, "µoªí¤å³¹©ó¡i %s ¡j¬ÝªO", currboard);
    b_load_class(currboard);
    
    for(i = 0;i < 9;i++)
    {
      char tmp[50];
      
      sprintf(tmp, "%d%d)%s ", i+1, i+1, postprefix[i]);
      strcpy(postprefix[i], tmp);
      board_class[i] = postprefix[i];
    }
    board_class[9] = "wW)¦Û¦æ¿é¤J";
    board_class[10] = "qQ)¤£¿é¤J";
  
    memset (save_title, 0, TTLEN);
    
    clear();
    genbuf[0] = win_select(win_title, "½Ð¿ï¾Ü¤å³¹Ãþ§O", board_class, 11, '1');

    move(0,0);
    i = *genbuf - '0';
    if (i > 0 && i <= 9)  /* ¿é¤J¼Æ¦r¿ï¶µ */
      strncpy (save_title, board_class[i - 1]+3, strlen (board_class[i - 1]+3));
    else if (*genbuf == 'w')  /* ¦Û¦æ¿é¤J */
    {
      getdata(20, 0, "½Ð¿é¤J¤å³¹Ãþ§O: ", genbuf, 21, DOECHO, 0);
      strncpy(save_title, genbuf, strlen (genbuf));
      strcat(save_title," ");
    }
    else      /* ªÅ¥Õ¸õ¹L */
      save_title[0] = '\0';
           
    getdata (21, 0, "¼ÐÃD¡G", save_title, TTLEN, DOECHO, save_title);
    strip_ansi (save_title, save_title, ONLY_COLOR);
  }
  if (save_title[0] == '\0')
    return RC_FULL;

  curredit &= ~EDIT_MAIL;
  curredit &= ~EDIT_ITEM;
  setutmpmode (POSTING);

  /* ¥¼¨ã³Æ Internet Åv­­ªÌ¡A¥u¯à¦b¯¸¤ºµoªí¤å³¹ */

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
  if (local_article == 1)    /* local save */
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
    if (local_article != 1)// && !(currbrdattr & BRD_NOTRAN))
      outgo_post (&postfile, currboard);
    brc_addlist (postfile.filename);

    if (!(currbrdattr & BRD_NOCOUNT))
    {
      if (wordsnum <= WORLDSNUM)
        pressanykey ("©êºp¡A¤Óµuªº¤å³¹¤£¦C¤J¬ö¿ý¡C");
      else
      {
        int money = (wordsnum <= spendtime ? (wordsnum / 100) :
                    (spendtime / 100));
	int cold;
        
        money *= (float)(((rand () % 5) + 5) / 5);
        if (money < 1) money = 1;
	
        clear ();
        move (7, 0);
        update_data ();

        cold = make_cold(currboard,save_title,money,fpath);/*­pºâ§N«×*/
        if(cold == 9)
          money += 100;

        prints ("\
              [1;36m¡i[37m­p ºâ ½Z ¹S[36m¡j\n
              [37m ³o¬O±zªº[33m²Ä %d ½g[37m¤å³¹¡C
              [36m¡i¶O  ®É¡j[33m %d [37m¤À[33m % d [37m¬í¡C
              [36m¡i½Z  ¹S¡j[33m %d [37m(ª÷¹ô)
              [36m¡i§N  «×¡j[33m %d [37m ÂI",
        ++cuser.numposts, spendtime / 60, spendtime % 60, money, cold+1);
        if(cold == 9)
	{
	  prints ("\n\n[31m                  ®¥³ß§A¤¤¯S¼ú...¥[°e100ªTª÷¹ô^^[0m");
	  prints ("\n\n              ±zªº¤å³¹¤w¸g³QÂà¿ý¦Ü ColdKing ¬ÝªO¸Ì!!");
	}
        substitute_record (fn_passwd, &cuser, sizeof (userec), usernum);
        ingold (money);  // post§ïµoª÷¹ô by wildcat
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
          strncpy (mhdr.title, "POSTÀË¬d", TTLEN);
          mhdr.savemode = '\0';
          setbdir (fpath1, "Security");
          if (rec_add (fpath1, &mhdr, sizeof (mhdr)) == -1)
          {
            outs (err_uid);
            return 0;
          }
          if ((fp = fopen (genbuf1, "w")) != NULL)
          {
            fprintf (fp, "§@ªÌ: %s (%s)\n", cuser.userid, cuser.username);
            fprintf (fp, "¼ÐÃD: %s\n®É¶¡: %s\n", "POSTÀË¬d", ctime (&now));
            fprintf (fp,
"%s µoªí¤@½g %d ¦rªº¤å³¹©ó %s ªO\nªá¤F %d ¬í¡A±o¨ìª÷¹ô %d ¤¸"
              ,cuser.userid, wordsnum, currboard, spendtime, money);
            fclose (fp);
          }
        }
      }
    }
    else
      pressanykey ("¥»¬ÝªO¤å³¹¤£¦C¤J¬ö¿ý¡A·q½Ð¥]²[¡C");

    log_board3("POS", currboard, cuser.numposts);

  /* ¦^À³¨ì­ì§@ªÌ«H½c */
    if (curredit & EDIT_BOTH)
    {
      char *str, *msg = "¦^À³¦Ü§@ªÌ«H½c";

      if (str = strchr (quote_user, '.'))
      {
        if (bbs_sendmail (fpath, save_title, str + 1, NULL) < 0)
          msg = "§@ªÌµLªk¦¬«H";
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
    do_copy_post("All_Post", fpath, 0); // ¬ö¿ý©Ò¦³¯¸¤ºªº¶K¤å
    if (currbrdattr & BRD_ANONYMOUS)    // ¤Ï°Î¦W
      do_copy_post("UnAnonymous", fpath, 0);

#ifdef POSTNOTIFY    /* ·s¤å³¹³qª¾ */
    if (!(currbrdattr & BRD_ANONYMOUS) && !(currbrdattr & BRD_HIDE))
      do_postnotify (fpath);
#endif
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
    pressanykey("¥DÃD¦¡¾\\Åª½Ð¤Å¦^ÂÐ¤å³¹");
    return RC_NONE;  
  }
  if (!(currmode & MODE_POST))
    return RC_NONE;

//¥[±K¹Lªº¤å³¹¥u¦³ªO¥D¥i¥H¦^
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD) && !HAVE_PERM(PERM_SYSOP))
  {
    pressanykey("¥»¤å³¹¤w³Q¥[±K!!");
    return RC_NONE;
  }    
  setdirpath (quote_file, direct, fhdr->filename);
// Ptt ªº¬ÝªO³s¸p¨t²Î
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

  /* itoc.001203: ¥[±Kªº¤å³¹¤£¯à edit */
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD))
  {
    pressanykey("¥»¤å³¹¤w³Q¥[±K!!");
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
    {   // CityLion: SELECT®É¤]­n­×§ï­ì.DIR
       setbdir(genbuf,currboard);
       now = getindex(genbuf,fhdr->filename,sizeof(fileheader));
    }

    strcpy (fhdr->title, save_title);
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);

    if (currmode&MODE_SELECT)     // CityLion: SELECT®É¤]­n­×§ï­ì.DIR
       substitute_record(genbuf, fhdr, sizeof(*fhdr), now);

#ifdef POSTNOTIFY
    if (currbrdattr & BRD_ANONYMOUS)
      do_postnotify (fpath);
#endif       
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
  char *choose_save[3] = {"sS)¦sÀÉ","lL)¯¸¤º", msg_choose_cancel};
  fileheader xfile;
  FILE * xptr;
  int author = 0;
  char genbuf[256];

  /* itoc.001203: ¥[±Kªº¤å³¹¤£¯àÂà¿ý */
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD))
  {
    pressanykey("¥»¤å³¹¤w³Q¥[±K!!");
    return RC_NONE;
  }
  
  make_blist ();
  move (2, 0);
  clrtoeol ();
  move (3, 0);
  clrtoeol ();
  move (1, 0);
  namecomplete ("Âà¿ý¥»¤å³¹©ó¬ÝªO¡G", xboard);
  if (*xboard == '\0' || !haspostperm (xboard))
    return RC_FULL;

  ent = 1;
  if (HAS_PERM (PERM_SYSOP) || !strcmp (fhdr->owner, cuser.userid))
  {
    char *choose_repost[2] = {"11)­ì¤åÂà¸ü","22)ÂÂÂà¿ý®æ¦¡"};
    if (getans2(2, 0, "",choose_repost, 2,'1') != '2')
    {
      char inputbuf;

      ent = 0;
      inputbuf = getans2(2, 0, "«O¯d­ì§@ªÌ¦WºÙ¶Ü? ", 0, 2, 'y');
      if (inputbuf != 'n' && inputbuf != 'N') author = 1;
    }
  }

  if (ent)
    sprintf (xtitle, "[Âà¿ý]%.66s", fhdr->title);
  else
    strcpy (xtitle, fhdr->title);

  sprintf (genbuf, "±Ä¥Î­ì¼ÐÃD¡m%.60s¡n¶Ü? ", xtitle);

  if (getans2(2, 0, genbuf, 0, 2, 'y') == 'n')
  {
    if (getdata (2, 0, "¼ÐÃD¡G", genbuf, TTLEN, DOECHO, xtitle))
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

      fprintf (xptr, "¡° [¥»¤åÂà¿ý¦Û %s ¬ÝªO]\n\n", currboard);

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
    pressanykey ("¤å³¹Âà¿ý§¹¦¨");
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
  
  setdirpath (genbuf, direct, fhdr->filename);  /*¦]¬°¤å³¹¥[±K,·h¨ì«e­± hialan*/
  
  sprintf(buf, "%s.vis", genbuf);
  /* itoc.001203: ¥[±Kªº¤å³¹¥u¦³­ì§@ªÌ¥H¤ÎªO¥D¯à¾\Åª */
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD) &&
      strcmp(cuser.userid, fhdr->owner) && !belong_list(buf, cuser.userid))
  {
    pressanykey("¥»¤å³¹¤w³Q¥[±K!!");
    return RC_FULL;
  }

  if (fhdr->owner[0] == '-')
    return RC_NONE;

//  if(dashd(genbuf))
//    read_dir(genbuf,fhdr->title);

  /* yagami.010714: ½s¿è¥[±K¤å³¹¥i¬Ý¨£¦W³æ */
  if (fhdr->filemode & FILE_REFUSE && 
    ((currmode & MODE_BOARD) || !strcmp(cuser.userid, fhdr->owner)))
  {
    if(win_select("¥[±K¤å³¹", "¬O§_½s¿è¥i¬Ý¨£¦W³æ? ", 0, 2, 'n') == 'y')    
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
/* ±Ä¶°ºëµØ°Ï                                            */
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
    namecomplete ("¿é¤J¬Ýª©¦WºÙ (ª½±µEnter¶i¤J¨p¤H«H¥ó§¨)¡G", buf);
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
        a_menu (xboard, fpath, HAS_PERM (PERM_ALLBOARD) ? 2 : is_BM (bp->BM) ? 1 : 0);
    }
    else if(HAS_PERM(PERM_MAILLIMIT) || HAS_PERM(PERM_BM)) // wildcat : ¤§«e§Ñ°O¥[ PERM ­­¨î°Õ ^^;
    {
      int mode0 = currutmp->mode;
      int stat0 = currstat;
      sethomeman (buf, cuser.userid);
      sprintf (buf1, "%s ªº«H¥ó§¨", cuser.userid);
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
    pressanykey("¤£¥i½Æ»sÁôÂÃ¥Ø¿ý©Î¥[±KÀÉ®×!!");
    return RC_NONE;
  }
  strcpy (title, "¡º ");
  strncpy (title + 3, fhdr->title, TTLEN - 3);
  title[TTLEN] = '\0';
//  a_copyitem (fpath, title, fhdr->owner);
  a_copyitem (fpath, title, cuser.userid);
  /* shakalaca.990517: À³¨Ï¥ÎªÌ­n¨D, ½sªÌ¬°ªO¥D */
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
    pressanykey("¤£¥i½Æ»sÁôÂÃ¥Ø¿ý");
    return RC_NONE;
  }
  setbfile (fpath, currboard, fhdr->filename);
  sprintf (title, "%s%.72s",(currutmp->pager > 1) ? "" : "¡º ", fhdr->title);
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
    pressanykey("¤£¥i½Æ»sÁôÂÃ¥Ø¿ý");
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
/* shakalaca.990714: ±NÀÉ®×¿W¥ß
    unlink (newpath); */
    f_cp (fpath, newpath, O_TRUNC);
    strcpy (fh.owner, cuser.userid);
    sprintf (fh.title, "%s%.72s","¡º " , fhdr->title);
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
    
    if (getdata (b_lines - 1, 0, "¼ÐÃD¡G", genbuf, TTLEN, DOECHO, tmpfhdr.title))
    {
      strcpy (tmpfhdr.title, genbuf);
      dirty++;
    }

    if(HAS_PERM (PERM_SYSOP))
    {
      if (getdata (b_lines - 1, 0, "§@ªÌ¡G", genbuf, IDLEN + 2, DOECHO, tmpfhdr.owner))
      {
        strcpy (tmpfhdr.owner, genbuf);
        dirty++;
      }

      if (getdata (b_lines - 1, 0, "¤é´Á¡G", genbuf, 6, DOECHO, tmpfhdr.date))
      {
        sprintf (tmpfhdr.date, "%+5s", genbuf);
        dirty++;
      }
    }

    if (getdata (b_lines - 1, 0, "½T©w(Y/N)?[n] ", genbuf, 3, DOECHO, 0) &&
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
  
  /*hialan.020714 mark¹L©Î¥[±K¹Lªº¤å³¹ ¤£¯àtag*/
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
    if (getans2(1, 0, "½T©w§R°£¼Ð°O«H¥ó? ", 0, 2, 'y') != 'n')
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

    if (getans2(1, 0, "½T©w§R°£¼Ð°O¤å³¹? ", 0, 2, 'n') == 'y')
    {
      currfmode = FILE_TAGED;
      if (currmode & MODE_SELECT)
      { 
        char xfile[PATHLEN];
        
        sprintf(xfile,"%s.vis", direct);
        unlink (direct);
        unlink(xfile);  /*¥[±KÀÉ®×¦W³æ*/
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
  load_paste(); //Åª¤J paste_file ¨Ó©w¦ì
  if(!*paste_path)
  {
    pressanykey("©|¥¼©w¦ì,½Ð¶i¤JºëµØ°Ï¤¤§A·Q¦¬¿ýªº¥Ø¿ý«ö [P]");
    return RC_FOOT;
  }

  if (currstat == RMAIL)
  {
    if (getans2(1, 0, "½T©w¦¬¿ý¼Ð°O«H¥ó? ", 0, 2, 'y') != 'n')
    {
      currfmode = FILE_TAGED;
      if (gem_files (direct, cmpfmode))
        return RC_CHDIR;
    }
    return RC_FULL;
  }
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    if (getans2(1, 0, "½T©w¦¬¿ý¼Ð°O¤å³¹? ", 0, 2, 'n') == 'y')
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
score_note(char *prompt, fileheader *fhdr, char *direct)	//±ÀÂË¥[¤À!! ¾Ç ptt by hialan
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
      pressanykey("±ÀÂË¥¢±Ñ!!ÀÉ®×¦³°ÝÃD©Î¦³¤H¥¿¦b±ÀÂË, ½Ð­«·s±ÀÂË:)");
      return -1;
    }
          
    fp = fopen(fpath, "a");
    flock(fd, LOCK_EX);
    
    fprintf(fp, "\n\033[1;31m¡÷ \033[33m%s\033[0;33m:%-*s\033[m±À %14.14s %0d/%0d",
                cuser.userid, 51-strlen(cuser.userid), genbuf, fromhost, 
                ptime->tm_mon+1, ptime->tm_mday);
    flock(fd, LOCK_UN);
    close(fd);
    fclose(fp);
    
    pressanykey("±ÀÂË¦¨¥\\!!");
    return 1;
}

/*¤å³¹µû¤À*/
int
score (int ent, fileheader *fhdr, char *direct)
{
  char buf[128];
  time_t now = time(0);
  char *choose[3] = {"11)¥[¤À","22)¦©¤À", msg_choose_cancel};
  
  if (currstat == RMAIL)
    return RC_NONE;
  
  if (cuser.scoretimes <= 0 && !HAVE_PERM(PERM_SYSOP))
  {
    pressanykey("µû¤ÀÂI¼Æ¤£°÷µLªkµû¤À!!");
    return RC_FULL;
  }
  
  if (!strcmp("guest", cuser.userid))
  {
    pressanykey("guest ¤£´£¨Ñ¦¹¥\\¯à!!");
    return RC_FULL;
  }

  buf[0] = getans2(b_lines, 0, "½Ð°Ý­n ", choose, 3, 'q');
  if(!buf[0] || buf[0] < '1' || buf[0] > '3')
    return RC_DRAW;
  else if(buf[0] == '1' && fhdr->score < 99)
  {
    if(score_note("¥[¤À­ì¦]: ", fhdr, direct) < 0)
      return RC_FULL;
    else
      fhdr->score++;
  }
  else if(buf[0] == '2' && fhdr->score > -9)
  {
    if(score_note("¦©¤À­ì¦]: ", fhdr, direct) < 0)
      return RC_FULL;
    else
      fhdr->score--;
  }
  else if(buf[0] != 'q')
  {
    if(fhdr->score >= 99 || fhdr->score <= -9)                     // ¤À¼Æªº¤W
    {                                                              // ­­»P¤U­­
      sprintf(buf , "¤w¸g¬O³Ì%s¤À¤F!!", fhdr->score >= 99 ? "°ª" : "§C");    
      pressanykey(buf);
      return RC_DRAW;
    }  
  }
    
  if(!HAS_PERM(PERM_SYSOP))                                      // ¯¸ªø¤£¦©
  {                                                              // ¦¸¼Æ¦ý¬O
    ingold(1);                                                   // ¤£¥[¿ú
    cuser.scoretimes--;
  }

  if (currmode & MODE_SELECT)                                    // ¦b·j´M¤å³¹
  {                                                              // ±¡ªp¤U³B²z
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
                                                                                
  substitute_record (fn_passwd, &cuser, sizeof (userec), usernum); // ¦^¦s¦¸¼Æ
  sprintf(buf , "%-12.12s ´À %-12.12s ªO [%-40.40s] µû¤À [%s] %s",
    cuser.userid,currboard,fhdr->title,
    buf[0] == '1' ? "+1" : "-1",Etime(&now));
  f_cat(BBSHOME"/log/article_score.log",buf);                    // ¬ö¿ý
  if(!HAS_PERM(PERM_SYSOP))
  {
    sprintf(buf , "§Aªºµû¤À¦¸¼ÆÁÙ¦³ %d ¦¸ ...", cuser.scoretimes);
    pressanykey(buf);
  }
  return RC_DRAW;
}

/* itoc.001203: ¤å³¹¥[±K */
int
refusemark(ent, fhdr, direct)
  int ent;
   fileheader *fhdr;
   char *direct;
{
  char buf[256];
  
  if (currstat != READING)
    return RC_NONE;
                                                                                
  /*¦Û°Ê²M±¼¥[±K¦W³æ ... hialan.020714*/
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
    getdata (1, 0, "[³]©w§R°£½d³ò] °_ÂI¡G", num1, 5, DOECHO, 0);
    inum1 = atoi (num1);
    if (inum1 <= 0)
    {
      outz ("°_ÂI¦³»~");
      return RC_FOOT;
    }
    getdata (1, 28, "²×ÂI¡G", num2, 5, DOECHO, 0);
    inum2 = atoi (num2);
    if (inum2 < inum1)
    {
      outz ("²×ÂI¦³»~");
      return RC_FULL;
    }
    
    if (getans2(1, 48, msg_sure, 0, 2, 'n') == 'y')
    {
      outmsg ("³B²z¤¤,½Ðµy«á...");
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
// wildcat : ¯¸ªø¥i¥H³s½u¬å«H
  if(HAS_PERM(PERM_SYSOP) && answer("¬O§_­n³s½u¬å«H(y/N)") == 'y')
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
        pressanykey ("%s¡A±zªº¤å³¹´î¬° %d ½g¡A¤ä¥I²M¼ä¶O %d ª÷", msg_del_ok,
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
/* ¬ÝªO³Æ§Ñ¿ý¡B¤åºK¡BºëµØ°Ï                              */
/* ----------------------------------------------------- */

static int
board_edit ()
{
  boardheader bp;
  boardheader * getbcache ();
  int bid, mode = 0;
  time_t now = time(0);
  char *board_admin_menu[12] = {"00)¨ú®ø",
      			        "11)§ï¤¤¤å¦WºÙ",
      			        "22)¬ÝªO»¡©ú",
      			   	"33)¶iªOµe­±",
      				"44)¥i¨£¦W³æ",
      				"55)³]±K½X",
      				"66)§ï¤å³¹Ãþ§O",
      				"77)¶R¤W­­",
      				"88)¬ÝªOPO¤åª`·N¨Æ¶µ", 
      				"99)§ïÄÝ©Ê",
      				"aA)¬Ý¤µ¤é¬ö¿ý",
      				"bB)§ïÃþ§O"};  

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

    switch(win_select("¬ÝªOºÞ²z", "", board_admin_menu, (mode == 1) ? 12 : 9 ,'0'))
    {
      case '1':
        move (1, 0);
        clrtoeol ();
        getdata (1, 0, "½Ð¿é¤J¬ÝªO·s¤¤¤å±Ô­z:"
          ,genbuf, BTLEN - 16, DOECHO, bp.title + 7);
        if (!genbuf[0]) return 0;
        strip_ansi (genbuf, genbuf, 0);
        if(strcmp(bp.title+7,genbuf))
        {
          sprintf(buf,"%-13.13s §ó´«¬ÝªO %s ±Ô­z [%s] -> [%s] , %s",
            cuser.userid,bp.brdname,bp.title+7,genbuf,ctime(&now));
          f_cat(BBSHOME"/log/board_edit",buf);
          log_usies ("NameBoard", currboard);
          strcpy (bp.title + 7, genbuf);
        }
        break;

      case '2':

        change_bp(2, "¹ï¥»¬ÝªOªº´y­z (¦@¤T¦æ)", bp.desc);        
        sprintf(buf,"%-13.13s §ó´«¬ÝªO %s »¡©ú , %s",
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

          getdata (3, 0, "½Ð³]©w¦³®Ä´Á­­(0 - 9999)¤Ñ¡H", buf, 5, DOECHO, "9999");
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
          if(!getdata (1, 0, "½Ð¿é¤J­ì¥»ªº±K½X" ,genbuf, PASSLEN, PASS, 0) ||
             !chkpasswd(bp.passwd, genbuf))
          {
               pressanykey("±K½X¿ù»~");
               return -1;
          }
        }
        if (!getdata(1, 0, "½Ð³]©w·s±K½X¡G", genbuf, PASSLEN, PASS,0))
        {
          pressanykey("±K½X³]©w¨ú®ø, Ä~Äò¨Ï¥ÎÂÂ±K½X");
          return -1;
        }
        strncpy(buf, genbuf, PASSLEN);
 
        getdata(1, 0, "½ÐÀË¬d·s±K½X¡G", genbuf, PASSLEN, PASS,0);
        if (strncmp(genbuf, buf, PASSLEN)) 
        {
          pressanykey("·s±K½X½T»{¥¢±Ñ, µLªk³]©w·s±K½X");
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
        
        /*¤å³¹Ãþ§O by hialan 3.21.2002*/
        b_load_class(currboard);
        
        /*½Æ»s¨ì«ü¼Ð°}¦C*/
        for(i = 0;i < 11;i++)
          classpoint[i] = class[i];
   
        for(i = 0;i<9;i++)            
          sprintf(class[i],"%d%d.%s", i+1, i+1, postprefix[i]);
        
        sprintf(class[9],"dD.¦^´_¹w³]­È");
        sprintf(class[10],"qQ.Â÷¶}");          

        ch2 = '1';        

        do
        {
          ch2 = win_select("­×§ï¤å³¹Ãþ§O", "½Ð¿ï¾Ü­n­×§ïªºÃþ§O ", classpoint, 11, ch2);

          if(ch2 == 'd')
          {
            if(win_select("­×§ï¤å³¹Ãþ§O", "½T©w­n¦^ÂÐ¹w³]­È¶Ü? ", 0, 2, 'n')== 'y')
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
            getdata(b_lines-1, 0, "½Ð¿é¤J·sÃþ§O", buf, 21, DOECHO, postprefix[ch2-'1']);
            if(*buf != '\0')  /*¦pªG¨Ï¥ÎªÌ¨S¿é¤JªF¦è,«h¸õ¶}*/
 	    {
              strcpy(postprefix[ch2 - '1'], buf);
              sprintf(class[ch2 - '1'],"%c%c.%s", ch2, ch2, postprefix[ch2 - '1']);            
            }
            move(b_lines-1, 0);
            clrtobot(b_lines-1,0);
          }
        }while(ch2 != 'q');

        if(win_select("­×§ï¤å³¹Ãþ§O", "½T©w­n­×§ï¶Ü? ", 0, 2, 'y')== 'y') 
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
          pressanykey("¤å³¹Ãþ§O¨S¦³§ïÅÜ!!");
      }
      break;
      
      case '7':
      {
        clrchyiuan(1, 15);
        move(3, 0);
        prints("\
¥Ø«e¬ÝªOªº¤å³¹¤W­­¬° %-5d ½g
          «O¯d®É¶¡¬° %-5d ¤Ñ\n",bp.maxpost,bp.maxtime);
        outs("¤@­Ó³æ¦ì¬°[1;32m¤@¦Ê½g¤å³¹[m©Î¬O[1;32m¤T¤Q¤Ñ[m , ¤@­Ó³æ¦ì»Ý [1;33m3000 ª÷¹ô[m");
        getdata(7, 0, "§A­n (1)¶R¤å³¹¤W­­ (2)¶R«O¦s®É¶¡", buf, 2, DOECHO, 0);
        if (buf[0] == '1' || buf[0] == '2')
        {
          int num = 0;

              while (num <= 0)
              {
                getdata(9, 0, "§A­n¶R´X­Ó³æ¦ì", genbuf, 3, DOECHO, 0);
                num = atoi(genbuf);
              }

              if (check_money(num * 3000, GOLD))
                break;

              if (buf[0] == '1')
              {
                if (bp.maxpost >= 99999)
                {
                  pressanykey("¤å³¹¼Æ¤w¹F¤W­­");
                  break;
                }
                else
                {
                  bp.maxpost += num*100;
                  sprintf(buf, "%-13.13s ÁÊ¶R¬ÝªO %s ¤å³¹¤W­­ %d ½g , %s",
                    cuser.userid, bp.brdname,num*100, ctime(&now));
                  f_cat(BBSHOME"/log/board_edit", buf);
                  log_usies ("BuyPL", currboard);
                  pressanykey("¬ÝªO¤å³¹¤W­­¼W¥[¬° %d ½g", bp.maxpost);
                }
              }
              else
              {
                if (bp.maxtime >= 9999)
                {
                  pressanykey("«O¦s®É¶¡¤w¹F¤W­­");
                  break;
                }
                else
                {
                  bp.maxtime += num * 30;
                  sprintf(buf,"%-13.13s ÁÊ¶R¬ÝªO %s ¤å³¹«O¯d®É¶¡ %d ¤Ñ , %s",
                    cuser.userid,bp.brdname,num*30,ctime(&now));
                  f_cat(BBSHOME"/log/board_edit",buf);
                  log_usies ("BuyBT", currboard);
                  pressanykey("¬ÝªO¤å³¹«O¯d®É¶¡¼W¥[¬° %d ¤Ñ",bp.maxtime);
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
         if (win_select("¦Û­qpostª`·N¨Æ¶µ", "¬O§_¥Î¦Û­qpostª`·N¨Æ¶µ", 0, 2, 'n') == 'y')
           vedit(buf, NA, NULL);
         else
           unlink(buf);
       }
       break;


//¥H¤U¬O ¨p¤Hª©¤~¦³ªº¥\¯à!!
          case '9':
          {
            int oldattr=bp.brdattr;
            char *brd_type[3]={"oO)¶}©ñ","pP)¨p¤H","hH)ÁôÂÃ"};

            ans = getans2(1, 0,"¬ÝªOª¬ºA§ó§ï¬°", brd_type, 3, 'o');
    
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
              if(answer("¬O§_­nÅý guest ¬Ý¨ì§AªºªO? (Y/n)") == 'n')
                bp.brdattr &= ~BRD_POSTMASK;
              else
                bp.brdattr |= BRD_POSTMASK;
              bp.brdattr &= ~BRD_HIDE;
            }
            if(bp.brdattr != oldattr)
            {
              sprintf(buf,"%-13.13s §ó§ï¬ÝªO [%s] ¤§ÄÝ©Ê¬° %s , %s",
                cuser.userid,bp.brdname,
                ans == 'p' ? "¨p¤H" : ans == 'h' ? "ÁôÂÃ" : "¶}©ñ",ctime(&now));
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
            getdata (1, 0, "½Ð¿é¤J¬ÝªO·sÃþ§O:",genbuf, 5, DOECHO, bp.title );
            if (!genbuf[0]) return 0;
            strip_ansi (genbuf, genbuf, 0);
            if(strncmp(bp.title,genbuf,4))
            {
              sprintf(buf,"%-13.13s §ó´«¬ÝªO %s Ãþ§O [%-4.4s] -> [%s] , %s",
                cuser.userid,bp.brdname,bp.title,genbuf,ctime(&now));
              f_cat(BBSHOME"/log/board_edit",buf);
              log_usies("PREFIX",currboard);
              strncpy (bp.title , genbuf, 4);
            }
          break;

      default:
        pressanykey("©ñ±ó­×§ï");
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
  if (more (buf, YEA) == -1) // ¥i¥H¬Ý¦h­¶
  {
    clear ();
    pressanykey ("¥»¬ÝªO©|µL¡u³Æ§Ñ¿ý¡v¡C");
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

  /* shakalaca.000112: ¶W¹L 30min ¤~±N index §R°£, §@ cache ¥Î */
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
    pressanykey("¥[±K¤å³¹¤£¯à¦¬¤J¨ì¤åºK¸Ì!!");
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


/* hialan ¾ã²z from i_read_key*/
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

int
post_vote()
{
  if (currstat != ANNOUNCE)
    DL_func("SO/vote.so:b_vote");
  return RC_FULL;
}

int
post_b_results()
{
  if (currstat != ANNOUNCE)
    DL_func("SO/vote.so:b_results");
  return RC_FULL;
}
/* ----------------------------------------------------- */
/* ¬ÝªO¥\¯àªí                                            */
/* ----------------------------------------------------- */

struct one_key read_comms[] =
{
  KEY_TAB, board_digest,	0,"¶i¤J/°h¥X ¤åºK",0,
  'b', b_notes,        		0,"¬Ý¶iª©µe­±",0,
  'c', cite,              PERM_BM,"¦¬¿ýºëµØ",0,
  'r', read_post,        	0,"¾\\Åª¤å³¹",0,
  'z', man,         	 	0,"¶i¤JºëµØ°Ï",0,
  'D', del_range,	  PERM_BM,"¤j D ¬å«H",0,
  Ctrl ('S'), save_mail,	0,"¦s¤J«H½c",0,
  'E', edit_post,        	0,"­×§ï¤å³¹",0,
  'T', edit_title,        PERM_BM,"­×§ï¼ÐÃD",0,
  's', do_select,        	0,"¿ï¾Ü¬ÝªO",0,
  'B', board_edit,        PERM_BM,"¬ÝªO½s¿è",0,
  't', add_tag,           PERM_BM,"¼Ð°O¤å³¹",0,
  Ctrl ('D'), del_tag,    PERM_BM,"§R°£¼Ð°O¤å³¹",0,
  'x', cross_post,       	0,"Âà¶K",0,
  'g', good_post,         PERM_BM,"¦¬¨ì¤åºK¤¤",0,
  'y', reply_post,       	0,"¦^ÂÐ¤å³¹",0,
  'd', del_post,         	0,"§R°£¤å³¹",0,
  'm', mark,              PERM_BM,"Mark ¤å³¹",0,
  'X', refusemark,       	0,"¤å³¹¥[±K",0,
  Ctrl ('P'), do_post,   	0,"µoªí¤å³¹",0,
  'C', gem_tag,           PERM_BM,"¦¬¿ý¼Ð°O¤å³¹",0,
  Ctrl ('C'), Cite_posts,	0,"ª½±µ¦¬¿ý¤å³¹¦ÜºëµØ°Ï",0,
  '%', score,		 	0,"¤å³¹µû¤À",0,
  'v', v_board,		 	0,"ªO¤ºvªO",0,
  'w', write_msg,	 PERM_LOGINOK,"ªO¤º¥á¤ô²y",0,
  'F', post_mail,	 PERM_FORWARD,"±N¤å³¹±H¦^ Internet ¶l½c",0,
  'U', post_mail_uncode, PERM_FORWARD,"±N¤å³¹ uncode «á±H¦^ Internet ¶l½c",0,
  Ctrl ('Q'), post_query,	0,"ªO¤º q ¤H",0,
  'V', post_vote,	 	0,"°Ñ»P§ë²¼",0,
  'R', post_b_results,	 	0,"¬Ý§ë²¼µ²ªG",0,
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

  if ((fhdr->savemode == 'S') &&/* ¥~Âà«H¥ó */
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
/* Â÷¶} BBS ¯¸                                           */
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
    char *choose[3]={"sS)Àx¦s","eE)­«·s¨Ó¹L", msg_choose_cancel};

    change_bp(16, "½Ð¯d¨¥ (¦Ü¦h¤T¦æ)", myitem.buf);
    
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

  fputs("[1m                             "COLOR1" [33m¡» [37m¤ß ±¡ ¯d ¨¥ ªO [33m¡» [m \n\n",fp);
  collect = 1;

  while (total)
  {
    sprintf(buf, "[44m[1;36m¢z¢r [33m%s[37m(%s)",
      myitem.userid, myitem.username);
    len = strlen(buf);
    strcat(buf," [36m" + (len&1));

    for (i = len >> 1; i < 38; i++)
      strcat(buf, "¢w");
    sprintf(buf2, "¢w[33m %.14s  [36m¢r¢{[m\n",
      Etime(&(myitem.date)));
    strcat(buf, buf2);
    fputs(buf, fp);

    if (collect)
      fputs(buf, foo);

    sprintf(buf, "[1;44m[36m¢|¢{[37m%-70s[36m¢z¢}[m\n",myitem.buf[0]);
    if(*myitem.buf[1])
    {
      sprintf(buf2, "  [1;44m[36m¢x[37m%-70s[36m¢x[m\n",myitem.buf[1]);
      strcat(buf, buf2);
    }
    if(*myitem.buf[2])
    {
      sprintf(buf2, "  [1;44m[36m¢x[37m%-70s[36m¢x[m\n",myitem.buf[2]);
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
    prints("%16s   %-18sÅv³d¹º¤À\n\n", "½s¸¹", "¯¸ªø ID"/*, msg_seperator*/);

    for (i = 0; i < j; i++)
    {
      prints("%15d.   [1;%dm%-16s%s[0m\n",
        i + 1, 31 + i % 7, sysoplist[i].userid, sysoplist[i].duty);
    }
    prints("%-14s0.   [1;%dmÂ÷¶}[0m", "", 31 + j % 7);
    getdata(b_lines - 1, 0, "                   ½Ð¿é¤J¥N½X[0]¡G", genbuf, 4, DOECHO,"1");
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
  if(win_select("Â÷¶}" BOARDNAME, "±z½T©w­nÂ÷¶}¡i " BOARDNAME " ¡j¶Ü¡H", 0, 2, 'n') != 'y')
    return 0;

  movie(999);
  if (cuser.userlevel)
  {
    char *prompt[3]={"gG)ÀH­·¦Ó³u","mM)¦«¹Ú¯¸ªø","nN)§Ú­n§o³Û"};
    char ans = win_select("Â÷¶}" BOARDNAME, "", prompt, 3, 'g');

    if (ans == 'm')
      m_sysop();
    else if (ans == 'n')
      note();
  }

  t_display();
  clear();
  prints("[1;31m¿Ë·Rªº [31m%s([37m%s)[31m¡A§O§Ñ¤F¦A«×¥úÁ{"COLOR1
    " %s [40;33m¡I\n\n¥H¤U¬O±z¦b¯¸¤ºªºµù¥U¸ê®Æ:[m\n",
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

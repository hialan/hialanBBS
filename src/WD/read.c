/*-------------------------------------------------------*/
/* read.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : board/mail interactive reading routines      */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"
#include <sys/mman.h>

struct keeploc
{
  char *key;
  int top_ln;
  int crs_ln;
  struct keeploc *next;
};
typedef struct keeploc keeploc;


char currdirect[64];
static fileheader *headers = NULL;
/* static */ 
/* shakalaca.000117: unmarked for list.c */
int last_line;
static int hit_thread;

extern int search_num();
#define FHSZ    sizeof(fileheader)


/* ----------------------------------------------------- */
/* cursor & reading record position control              */
/* ----------------------------------------------------- */

keeploc *
getkeep(s, def_topline, def_cursline)
  char *s;
{
  static struct keeploc *keeplist = NULL;
  struct keeploc *p;
  void *malloc();

  if (def_cursline >= 0)
     for (p = keeplist; p; p = p->next)
     {
       if (!strcmp(s, p->key))
       {
         if (p->crs_ln < 1)
           p->crs_ln = 1;
         return p;
       }
     }
  else
     def_cursline = -def_cursline;
  p = (keeploc *) malloc(sizeof(keeploc));
  p->key = (char *) malloc(strlen(s) + 1);
  strcpy(p->key, s);
  p->top_ln = def_topline;
  p->crs_ln = def_cursline;
  p->next = keeplist;
  return (keeplist = p);
}


void
fixkeep(s, first)
  char *s;
  int first;
{
  keeploc *k;

  k = getkeep(s, 1, 1);
  if (k->crs_ln >= first)
  {
    k->crs_ln = (first == 1 ? 1 : first - 1);
    k->top_ln = (first < 11 ? 1 : first - 10);
  }
}


/* calc cursor pos and show cursor correctly */

static int
cursor_pos(locmem, val, from_top)
  struct keeploc *locmem;
  int val;
  int from_top;
{
  int top;

  if (val > last_line)
  {
    if(HAS_HABIT(HABIT_CYCLE) || (currstat == ANNOUNCE))
      val = 1;
    else
      val = last_line;
  }
  if (val <= 0)
  {
    if(HAS_HABIT(HABIT_CYCLE) || (currstat == ANNOUNCE))
      val = last_line;
    else
      val = 1;
  }

  top = locmem->top_ln;
  if (val >= top && val < top + p_lines)
  {
    cursor_clear(3 + locmem->crs_ln - top, 0);
    locmem->crs_ln = val;
    cursor_show(3 + val - top, 0);
    return RC_NONE;
  }
  locmem->top_ln = val - from_top;
  if (locmem->top_ln <= 0)
    locmem->top_ln = 1;
  locmem->crs_ln = val;
  return RC_BODY;
}


static int
move_cursor_line(locmem, mode)
  keeploc *locmem;
  int mode;
{
  int top, crs;
  int reload = 0;

  top = locmem->top_ln;
  crs = locmem->crs_ln;
  if (mode == RS_PREV)
  {
    if (crs <= top)
    {
      top -= p_lines - 1;
      if (top < 1)
        top = 1;
      reload = 1;
    }
    if (--crs < 1)
    {
      crs = 1;
      reload = -1;
    }
  }
  else if (mode == RS_NEXT)
  {
    if (crs >= top + p_lines - 1)
    {
      top += p_lines - 1;
      reload = 1;
    }
    if (++crs > last_line)
    {
      crs = last_line;
      reload = -1;
    }
  }
  locmem->top_ln = top;
  locmem->crs_ln = crs;
  return reload;
}

/* ----------------------------------------------------- */
/* Tag List 標籤					 */
/* ----------------------------------------------------- */


int TagNum;			/* tag's number */
TagItem TagList[MAXTAGS];	/* ascending list */


int
Tagger(chrono, recno, mode )
  time_t chrono;
  int recno;
  int mode;
{
  int head, tail, posi, comp;

  for (head = 0, tail = TagNum - 1, comp = 1; head <= tail;)
  {
    posi = (head + tail) >> 1;
    comp = TagList[posi].chrono - chrono;
    if (!comp)
      break;
    else if (comp < 0)
      head = posi + 1;
    else
      tail = posi - 1;
  }

  if (mode == TAG_COMP)
  {
    if (!comp && recno)		/* 絕對嚴謹：連 recno 一起比對 */
      comp = recno - TagList[posi].recno;
    return comp;
  }

  if (!comp)
  {
    if (mode != TAG_TOGGLE)
      return NA;

    TagNum--;
    memcpy(&TagList[posi], &TagList[posi + 1],
      (TagNum - posi) * sizeof(TagItem));
  }
  else if (TagNum < MAXTAGS)
  {
    TagItem *tagp, buf[MAXTAGS];

    tail = (TagNum - head) * sizeof(TagItem);
    tagp = &TagList[head];
    memcpy(buf, tagp, tail);
    tagp->chrono = chrono;
    tagp->recno = recno;
    memcpy(++tagp, buf, tail);
    TagNum++;
  }
  else
  {
    bell();
    return 0;			/* full */
  }
  return YEA;
}


void
EnumTagName(fname, locus)
  char *fname;
  int locus;
{
  sprintf(fname, "M.%d.A", TagList[locus].chrono);
}


void
EnumTagFhdr(fhdr, direct, locus)
  fileheader *fhdr;
  char *direct;
  int locus;
{
  rec_get(direct, fhdr, sizeof(fileheader), TagList[locus].recno);
}


/* -1 : 取消 */
/* 0 : single article */
/* ow: whole tag list */

int AskTag(char *msg)
{
  char buf[80];
  int num;
  char *choose[3] = {"aA)文章", "tT)標記", msg_choose_cancel};

  num = TagNum;
  sprintf(buf, "◆ %s ", msg);
  switch (getans2(b_lines, 0, buf, choose, 3, num ? 't' : 'a'))
  {
  case 'q':
    num = -1;
    break;
  case 'a':
    num = 0;
  }
  return num;
}                       

#define	BATCH_SIZE	65536

static int
TagThread(direct, search, type)
  char *direct;
  char *search;
  int type;     /* 0: title, 1: owner */
{
  caddr_t fimage, tail;
  off_t off;

  int fd, fsize, count;
  struct stat stbuf;
  fileheader *head;
  char *title;

  if ((fd = open(direct, O_RDONLY)) < 0)
    return RC_NONE;

  fstat(fd, &stbuf);
  fsize = stbuf.st_size;

  fimage = NULL;
  off = count = 0;
  do
  {
    fimage = mmap(fimage, BATCH_SIZE, PROT_READ, MAP_SHARED, fd, off);
    if (fimage == (char *) -1)
    {
      outs("MMAP ERROR!!!!");
      close(fd); //hialan:是不是缺少這個..@@""
      abort_bbs();
    }

    tail = fimage + BMIN(BATCH_SIZE, fsize - off);

    for (head = (fileheader *) fimage; (caddr_t) head < tail; head++)
    {
      int tmplen;

      count++;

      if (type == 1)
      {
        title = head->owner;
        tmplen = IDLEN+1;
      }
      else
      {
        title = str_ttl(head->title);
        tmplen = TTLEN;
      }

      if (!strncmp(search, title, tmplen))
      {
	if (!Tagger(atoi(head->filename + 2) , count, TAG_INSERT))
	{
	  off = fsize;
	  break;
	}
      }
    }

    off += BATCH_SIZE;
// wildcat : 會越吃越多記憶體?
  munmap(fimage, BATCH_SIZE);
  } while (off < fsize);
  close(fd);
  return RC_DRAW;
}


/* ----------------------------------------------------- */
/* 主題式閱讀						 */
/* ----------------------------------------------------- */

static int
thread(locmem, stype)
  keeploc *locmem;
  int stype;
{
  static char a_ans[32], t_ans[32];
  char ans[32], s_pmt[64];
  register char *tag, *query;
  register int now, pos, match, near;
  fileheader fh;
  int circulate_flag = 1; /* circulate at end or begin */


  match = hit_thread = 0;
  now = pos = locmem->crs_ln;

  if (stype == 'A') 
  {
     if (!*currowner)
        return RC_NONE;
     str_lower(a_ans, currowner);
     query = a_ans;
     circulate_flag = 0;
     stype = 0;
  }
  else if (stype == 'a') 
  {
     if (!*currowner)
        return RC_NONE;
     str_lower(a_ans, currowner);
     query = a_ans;
     circulate_flag = 0;
     stype = RS_FORWARD;
  }
  else if (stype == '/') 
  {
     if (!*t_ans)
        return RC_NONE;
     query = t_ans;
     circulate_flag = 0;
     stype = RS_TITLE | RS_FORWARD;
  }
  else if (stype == '?') 
  {
     if (!*t_ans)
        return RC_NONE;
     circulate_flag = 0;
     query = t_ans;
     stype = RS_TITLE;
  }
  else 

  if (stype & RS_RELATED)
  {
    tag = headers[pos - locmem->top_ln].title;
    if (stype & RS_CURRENT)
    {
      if (stype & RS_FIRST)
      {
        if (!strncmp(currtitle, tag, 40))
          return RC_NONE;
        near = 0;
      }
      query = currtitle;
    }
    else
    {
      query = str_ttl(tag);
      if (stype & RS_FIRST)
      {
        if (query == tag)
          return RC_NONE;
        near = 0;
      }
    }
  }
  else if (!(stype & RS_THREAD))
  {
    query = (stype & RS_TITLE) ? t_ans : a_ans;
    if (!*query && query == a_ans)
       if (*currowner)
          strcpy(a_ans, currowner);
       else if (*currauthor)
          strcpy(a_ans, currauthor);
    sprintf(s_pmt, "%s搜尋%s [%s] ",(stype & RS_FORWARD) ? "往後":"往前",
       (stype & RS_TITLE) ? "標題" : "作者", query);
    getdata(b_lines - 1, 0, s_pmt, ans, 30, DOECHO);
    if (*ans)
      strcpy(query, ans);
    else
    {
      if (*query == '\0')
        return RC_NONE;
    }
    str_lower(s_pmt, query);
    query = s_pmt;
  }

  tag = fh.owner;

  do
  {
    if (!circulate_flag || stype & RS_RELATED)
    {
      if (stype & RS_FORWARD)
      {
        if (++now > last_line)
          return RC_NONE;
      }
      else
      {
        if (--now <= 0)
        {
          if ((stype & RS_FIRST) && (near))
          {
            hit_thread = 1;
            return cursor_pos(locmem, near, 10);
          }
          return RC_NONE;
        }
      }
    }
    else
    {
      if (stype & RS_FORWARD)
      {
        if (++now > last_line)
          now = 1;
      }
      else
      {
        if (--now <= 0)
          now = last_line;
      }
    }

    rec_get(currdirect, &fh, sizeof(fileheader), now);

    if (fh.owner[0] == '-')
      continue;

    if (stype & RS_THREAD)
    {
      if (strncasecmp(fh.title, str_reply, 3))
      {
        hit_thread = 1;
        return cursor_pos(locmem, now, 10);
      }
      continue;
    }

    if (stype & RS_TITLE)
      tag = str_ttl(fh.title);

    if (((stype & RS_RELATED) && !strncmp(tag, query, 40)) ||
      (!(stype & RS_RELATED) && ((query == currowner) ? !strcmp(tag, query) : strstr_lower(tag, query))))
    {
      if (stype & RS_FIRST)
      {
        if (tag != fh.title)
        {
          near = now;
          continue;
        }
      }
      hit_thread = 1;
      match = cursor_pos(locmem, now, 10);
      if ((!(stype & RS_CURRENT)) && (stype & RS_RELATED) &&
        strncmp(currtitle, query, 40))
      {
        strncpy(currtitle, query, 40);
        match = RC_BODY;
      }
      break;
    }
  }
  while (now != pos);

  return match;
}


/* SiE: 系列式閱讀....使用mmap來加速搜尋 */

static int
select_read(locmem,sr_mode)
  keeploc *locmem;
  int sr_mode;
{
  register char *tag,*query;
  fileheader fh;
  char fpath[80], genbuf[128];
  char static t_ans[TTLEN+1]="";
  char static a_ans[IDLEN+1]="";
  int fd, fr, size = sizeof(fileheader);
  int fsize;
  char *fimage;
  fileheader *tail, *head;
  struct stat st;

  if( currmode & MODE_SELECT)
    return -1;
  if(sr_mode == RS_TITLE)
    query = str_ttl(headers[locmem->crs_ln - locmem->top_ln].title);
  else if (sr_mode == RS_FIRST) 
  {
    sprintf(genbuf,"SR.%s",cuser.userid);
    if(currstat == RMAIL)
      sethomefile(fpath,cuser.userid,genbuf);
    else      
      sprintf(fpath, "boards/%s/SR.thread", currboard);
    if (stat(fpath, &st) == 0 && st.st_mtime > time(0) - 60 * 30) 
    {
       currmode ^= (MODE_SELECT | MODE_TINLIKE);
       strcpy(currdirect,fpath);
       return st.st_size;
    }
    query = "Re:";
  }
  else if (sr_mode == RS_CURRENT)
     query = ".";
  else if (sr_mode == RS_THREAD)
     query = "m";
  else if (sr_mode == RS_SCORE)  /*文章評分*/
     query = "0";
  else 
  {
    query = (sr_mode == RS_RELATED) ? t_ans : a_ans;
    sprintf(fpath, "搜尋%s [%s] ",
        (sr_mode == RS_RELATED) ? "標題" : "作者", query);
    if(getdata(b_lines, 0, fpath, fpath, 30, DOECHO,0))
    {
      char buf[64];
      str_lower(buf,fpath);
      strcpy(query,buf);
    }
    if(!(*query))
     return RC_NONE;
  }
  outmsg("搜尋中,請稍後...");refresh();
  if ((fd = open(currdirect,  O_RDONLY)) >= 0)
  {
    if ( !fstat(fd, &st) && S_ISREG(st.st_mode) && (fsize = st.st_size) > 0)
      fimage = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);

    close(fd);
  }
  else fimage = (char *) -1;
  if (fimage == (char *) -1)
  {
    outmsg("索引檔開啟失敗");
    return RC_NONE;
  }

  head = (fileheader *) fimage;
  tail = (fileheader *) (fimage + fsize);

  sprintf(genbuf,"SR.%s",cuser.userid);

  if(currstat==RMAIL)
    sethomefile(fpath,cuser.userid,genbuf);
  else if (sr_mode == RS_FIRST)
  {
    fileheader *hdr, fhdr, *tmp;
    int num, j, count, current;
    FILE* fp;

    num = fsize / sizeof(fileheader);
    tmp = head;
    if(currstat==RMAIL)
      sethomefile(fpath,cuser.userid,genbuf);
    else
      sprintf(fpath, "boards/%s/SR.thread", currboard);

    current = 0;
    if (fp = fopen(fpath, "w")) 
    {
      do
      {
        if (strncmp(head->title, "Re:", 3))
        {
          memcpy(&fhdr, head, sizeof(fileheader));
          hdr = tmp;
          for (count = j = 0; j < num; j++)
          {
            if (!strncmp(fhdr.title, str_ttl(hdr->title), 40))
            {
              if (j < current)	/* 前面已有此篇, skip */
                break;
              ++count;
            }
            hdr++;
          }
          if (j > current)
          {
            sprintf(fhdr.date, "%5d", count);
            fwrite(&fhdr, sizeof(fileheader), 1, fp);
          }
        }
        current++;
      } while (++head < tail);
      fclose(fp);
    }                                             
    currmode ^= (MODE_SELECT | MODE_TINLIKE);
    strcpy(currdirect,fpath);
    
    munmap(fimage, fsize);	// sby: 記憶體會越吃越多 @@"
    
    return num;
  }
  else
    setbfile(fpath, currboard, genbuf);

  if(((fr = open(fpath,O_WRONLY | O_CREAT | O_TRUNC,0600)) != -1))
  {
    do
    {
      fh = *head;
      switch(sr_mode)
      {
        case RS_TITLE:
          tag = str_ttl(fh.title);
          if(!strncmp(tag, query, 40))
            write(fr,&fh,size);
          break;
        case RS_RELATED:
          tag = fh.title;
          if(str_str(tag,query))
            write(fr,&fh,size);
          break;
        case RS_AUTHOR:
          tag = fh.owner;
          if(str_str(tag,query))
            write(fr,&fh,size);
          break;
        case RS_CURRENT:
          tag = fh.owner;
          if(!strchr(tag, '.'))
            write(fr,&fh,size);
          break;
        case RS_THREAD:
          if(fh.filemode & (FILE_MARKED |  FILE_DIGEST))
            write(fr,&fh,size);
          break;
        case RS_SCORE:
          if(fh.score != 0)
            write(fr,&fh,size);
          break;
        }
      } while (++head < tail);
      fstat(fr,&st);
      close(fr);
    }
    munmap(fimage, fsize);

    if(st.st_size)
    {
      currmode ^= MODE_SELECT;
      strcpy(currdirect, fpath);
    }
  return st.st_size;
}


/*-----------------------------------------*/
/* i_read_helper() : i_read() 線上求助    */
/*-----------------------------------------*/

/* 基本指令集 */
struct one_key basic[]={
KEY_LEFT, NULL, 0, "離開。        其他相同的按鍵: q, e",0,
KEY_UP,   NULL, 0, "游標向上。    其他相同的按鍵: p, k",0,
KEY_DOWN, NULL, 0, "游標向下。    其他相同的按鍵: n, j",0,
KEY_PGDN, NULL, 0, "下一頁。      其他相同的按鍵: 空白鍵, N, Ctrl+f",0,
KEY_PGUP, NULL, 0, "上一頁。      其他相同的按鍵: P, Ctrl+b",0,
KEY_END,  NULL, 0, "跳到最後一項。其他相同的按鍵: $",0,
KEY_RIGHT,NULL, 0, "執行, 閱\讀。  其他相同的按鍵: Enter",0,
'\0', NULL, 0, NULL,0};


/* 主題式閱讀指令集 */
struct one_key sub_key[]={
'/',      NULL, 0, "找尋標題。              其他相同的按鍵: ?",0,
'S',      NULL, 0, "循序閱\讀新文章。",0,
'L',      NULL, 0, "閱\讀非轉信文章。",0,
'u',      NULL, 0, "標題式閱\讀。",0,
'=',      NULL, 0, "找尋首篇文章。",0,
'\\',     NULL, 0, "找尋游標該處之首篇文章。",0,
'[',	  NULL, 0, "向前搜尋相同標題之文章。其他相同的按鍵: +",0,
']',	  NULL, 0, "向後搜尋相同標題之文章。其他相同的按鍵: -",0,
'<',	  NULL, 0, "向前搜尋其他標題。      其他相同的按鍵: ,",0,
'>',	  NULL, 0, "向後搜尋其他標題。      其他相同的按鍵: .",0,
'A', 	  NULL, 0, "搜尋作者。              其他相同的按鍵: a",0,
'X',	  NULL, 0, "所有被加分過的文章。",0,
'G',	  NULL, 0, "所有被 mark 過的文章。",0,
'\0', NULL, 0, NULL,0};

static char *show_cmd_level(usint level)  //顯示權限 by spy
{
  register int i = -1;
  static char *none = "無限制";

  if (!level) return none;

  while (level)
  {
    level >>= 1;
    i++;
  }
  return permstrings[i];
}
                      

static int show_helplist_line(int row, struct one_key *cmd, char *barcolor)
{
  char buf[128];
  char key[10];
  int i;

  for(i='A';Ctrl(i) <= Ctrl('Z');i++)
  {
    if(Ctrl(i) == cmd->key) 
    {
      sprintf(key, "Ctrl+%c", i);
      break;
    }
  }
  if(i == 'Z'+1)
    sprintf(key, "%c", cmd->key);
  
  switch(cmd->key)
  {
    case KEY_UP:
      sprintf(key, "↑");
      break;
    case KEY_DOWN:
      sprintf(key, "↓");
      break;    
    case KEY_LEFT:
      sprintf(key, "←");
      break;    
    case KEY_RIGHT:
      sprintf(key, "→");
      break;    
    case KEY_PGUP:
      sprintf(key, "Page UP");
      break;    
    case KEY_PGDN:
      sprintf(key, "Page DOWN");
      break;    
    case KEY_END:
      sprintf(key, "End");
      break;    
    case KEY_TAB:
      sprintf(key, "Tab");
      break;
    case KEY_HOME:
      sprintf(key, "Home");    
      break;
  }
    
  sprintf(buf, "    %s %9s \033[m %-12s %s",  barcolor ? barcolor : "" , 
	       key, show_cmd_level(cmd->level), cmd->desc);
  move(3 + row, 0);
  clrtoeol();
  outs(buf);
}

int i_read_helper(struct one_key *rcmdlist)
{
  char cursor=0, i, draw=1, pos;
  char max_cursor;
  char barcolor[10];
  static char re_enter=0;
  int ch;
  char top, bottom;	//hialan: 一頁的最上面和最下面 , for 根據權限顯示

  get_lightbar_color(barcolor);
  top = bottom = cursor = pos = 0;
  for(max_cursor=0;rcmdlist[max_cursor].key;max_cursor++);

  while(1)
  {
    switch(draw)
    {
      case 1:
        clear();
  
        sprintf(tmpbuf,"%s [線上 %d 人]",BOARDNAME,count_ulist());
        showtitle("線上求助", tmpbuf);

        prints("←)上一頁 →)執行該指令 ↑|↓)選擇 TAB)本站說明手冊\n");
        prints("%s    指令/按鍵   權    限     說          明%36s\033[0m\n", COLOR3, "");

      case 2:
        bottom = top-1;
        ch=0;	//借用來記數
        for(i=0;i<p_lines && bottom<max_cursor;i++)
        {
          for(bottom++;bottom<max_cursor;bottom++)
            if(HAS_PERM(rcmdlist[bottom].level))
            {
              show_helplist_line(i, rcmdlist+bottom, 0);
              ch++;
              break;
            }
        }
        if(!ch) return -1;

      case 3:
        move(b_lines, 0);
        clrtoeol();
        prints("%s  選擇功\能  %s  b)基本指令集 %-40.40s  \033[m", COLOR2, COLOR3, "u)主題式閱\讀指令集");
        draw = 0;
        break;
    }
    
    if(HAS_HABIT(HABIT_LIGHTBAR))
    {
      show_helplist_line(pos, rcmdlist+cursor, barcolor);
      cursor_show(3+pos, 0);
      ch = igetkey();
      show_helplist_line(pos, rcmdlist+cursor, 0);
    }
    else
      ch = cursor_key(3 + pos, 0);
  
    switch(ch)
    {
      case KEY_LEFT:
      case 'q':
        return 0;

      case KEY_PGUP:
        cursor=top;
      case KEY_UP:
        cursor--;
        pos--;
        
        draw=0;
        if(cursor<0)
        {
          top = max_cursor;
          cursor = top-1;
        }
        if(cursor<top)
        {
          for(pos=0;pos<p_lines && top>0;pos++)
            for(top--;top>0 && !HAS_PERM(rcmdlist[top].level);top--);

          pos--;
          draw=1;
        }
        for(;!HAS_PERM(rcmdlist[cursor].level) && cursor>=0;cursor--);        
        break;

      case KEY_PGDN:
        cursor=bottom;
      case KEY_DOWN:
        cursor++;
        pos++;

        draw=0;
        for(;!HAS_PERM(rcmdlist[cursor].level) && cursor < max_cursor;cursor++);        

        if(cursor > bottom || cursor >= max_cursor)
        {
          for(top=bottom+1;!HAS_PERM(rcmdlist[top].level) && top < max_cursor;top++);

          if(top >= max_cursor)
          {
            top = 0;
            for(;!HAS_PERM(rcmdlist[top].level);top++);
          }
          cursor = top;
          pos=0;
          draw=1;
        }
        break;
      
      case '\r':
      case '\n':
      case KEY_RIGHT:
      {
        char buf[80];
        
        sprintf(buf,"請問你是否要執行 %s ?", rcmdlist[cursor].desc);
        if(getans2(b_lines, 0, buf, 0, 2, 'y') == 'y')
          return (ch = rcmdlist[cursor].key);
        draw = 3;
        break;
      }

      case 'b':
        if(!re_enter)
        {
          re_enter = 1;
          i_read_helper(basic);
          re_enter = 0;
          draw = 1;
        }
        break;
      
     case 'u':
       if(!re_enter)
       {
         re_enter = 1;
         i_read_helper(sub_key);
         re_enter = 0;
         draw = 1;
       }
       break;
     
     case KEY_TAB:
       HELP();
       draw=1;
       break;
    }
  }
  
  return RC_FULL;
}

int thread_title;
static int
i_read_key(struct one_key *rcmdlist, struct keeploc *locmem, int ch)
{
  int i;
//  static thread_title;
  
  if(ch == 'h')
  {
    screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));    

    vs_save(screen);    
    ch = i_read_helper(rcmdlist);
    vs_restore(screen);
    if(!ch)
      return RC_NONE;
  }

  switch (ch)
  {
    case KEY_LEFT:
    case 'q':
    case 'e':
    if ((currstat == RMAIL) || (currstat == READING) || (currstat == ANNOUNCE))
    {
        if (thread_title) 
        {
          --thread_title;
          if (thread_title) 
          {
            sprintf(currdirect, "boards/%s/SR.thread", currboard);
            return RC_NEWDIR;
          }
        }
        return (currmode & MODE_SELECT) ? board_select() :
        (currmode & MODE_DIGEST) ? board_digest() : QUIT;
    }
    else
      return QUIT;

  case 'p':
  case 'k':
  case KEY_UP:
    return cursor_pos(locmem, locmem->crs_ln - 1, p_lines - 2);

  case 'n':
  case 'j':
  case KEY_DOWN:
    return cursor_pos(locmem, locmem->crs_ln + 1, 1);

  case ' ':
  case KEY_PGDN:
  case 'N':
  case Ctrl('F'):
    if (last_line >= locmem->top_ln + p_lines)
    {
      if (last_line > locmem->top_ln + p_lines)
        locmem->top_ln += p_lines;
      else
        locmem->top_ln += p_lines - 1;
      locmem->crs_ln = locmem->top_ln;
      return RC_BODY;
    }
    cursor_clear(3 + locmem->crs_ln - locmem->top_ln, 0);
    locmem->crs_ln = last_line;
    cursor_show(3 + locmem->crs_ln - locmem->top_ln, 0);
    break;

  case 'P':
  case KEY_PGUP:
  case Ctrl('B'):
    if (locmem->top_ln > 1)
    {
      locmem->top_ln -= p_lines;
      if (locmem->top_ln <= 0)
        locmem->top_ln = 1;
      locmem->crs_ln = locmem->top_ln;
      return RC_BODY;
    }
    break;

  case KEY_END:
  case '$':
    if (last_line >= locmem->top_ln + p_lines)
    {
      locmem->top_ln = last_line - p_lines + 1;
      if (locmem->top_ln <= 0)
        locmem->top_ln = 1;
      locmem->crs_ln = last_line;
      return RC_BODY;
    }
    cursor_clear(3 + locmem->crs_ln - locmem->top_ln, 0);
    locmem->crs_ln = last_line;
    cursor_show(3 + locmem->crs_ln - locmem->top_ln, 0);
    break;
 
  case '\n':
  case '\r':
  case KEY_RIGHT:
    if ((thread_title == 1) && ((currstat == RMAIL) || (currstat == READING))) 
    {
      ++thread_title;
      currmode &= ~(MODE_SELECT | MODE_TINLIKE);
      setbdir(currdirect, currboard);
      select_read(locmem,RS_TITLE);
      return RC_NEWDIR;
    } 
    ch = 'r';

  default:
    for (i = 0; rcmdlist[i].key; i++)
    {
      if (rcmdlist[i].key == ch && rcmdlist[i].fptr)
      {
      /* shakalaca.000215: currdirect 為目前所在 root directory,
         如能善加修改, mail, post, anno 就可合而為一了, 這三者所差
         只在於 root dir. 
         相同: 轉寄 (F), 串列 (S), 搜尋 (/,A), 瀏覽 (r), 
               收錄 (c,C,^C,P), 暫存 (T), 查詢 (^Q),  
               移動 (上下左右鍵), tin-like read (u), 
         限制(not in mail):編修 (E), 發表 (^p)
       */
        if(!rcmdlist[i].level || HAS_PERM(rcmdlist[i].level))
        {
          return (*((int (*)())rcmdlist[i].fptr)) (locmem->crs_ln, 
                    &headers[locmem->crs_ln - locmem->top_ln], currdirect);
        }         
      }
    }
  }

  if ((currstat == RMAIL) || (currstat == READING) || (currstat == ANNOUNCE))
  {
    switch (ch)
    {
/* wildcat */
    case Ctrl('E'):
      if (HAS_PERM(PERM_SYSOP))
      {
        DL_func("SO/admin.so:m_user");
        return RC_FULL;
      }
      break;

    case '/':
    case '?':
      if (select_read(locmem,RS_RELATED))
        return RC_NEWDIR;
      else
        return RC_FOOT;

    case 'S':
      if (select_read(locmem,RS_TITLE))
        return RC_NEWDIR;
      else
        return RC_FOOT;

    case 'L':
    if (currstat != ANNOUNCE)
      if (select_read(locmem,RS_CURRENT)) /* local articles */
        return RC_NEWDIR;
      else
        return RC_FOOT;

    case 'u':
    if (currstat != ANNOUNCE)
      if (!thread_title && select_read(locmem,RS_FIRST)) 
      {
        thread_title = 1;
        return RC_NEWDIR;
      }
      else 
      {
        bell();
        return RC_NONE;
      }

    case '=':
      return thread(locmem, RELATE_FIRST);

    case '\\':
      return thread(locmem, CURSOR_FIRST);
 
    /* quick search title forword */
    case ']':
      return thread(locmem, RELATE_NEXT);

    case '+':
      return thread(locmem, CURSOR_NEXT);

    /* quick search title backword */
    case '[':
      return thread(locmem, RELATE_PREV);

    case '-':
      return thread(locmem, CURSOR_PREV);

    case '<':
    case ',':
      return thread(locmem, THREAD_PREV);

    case '.':
    case '>':
      return thread(locmem, THREAD_NEXT);

    case 'A':
    case 'a':
      if (select_read(locmem,RS_AUTHOR))
        return RC_NEWDIR;
      else
        return RC_FOOT;

    case 'X':
      if (select_read(locmem,RS_SCORE))
        return RC_NEWDIR;
      else
        return RC_FOOT;

    case 'G':
      if (select_read(locmem,RS_THREAD)) /* marked articles */
        return RC_NEWDIR;
      else
        return RC_FOOT;

    case Ctrl('X'):		/* terminator */
      if ((currstat == READING) && (HAS_PERM(PERM_ALLBOARD)) && (currstat != ANNOUNCE))
      {
        char buf[128], ans[4],mode[3];
        boardheader *bp;
        extern boardheader *bcache;
        extern int numboards;
  
        getdata(b_lines, 0,"刪除此 (1)作者 (2)標題 (q)取消",mode,2,LCECHO,0);
        if(mode[0] == '1')
          sprintf(buf, "終結所有看板中的 [%.40s] 嗎(Y/N)？[N] ",
            headers[locmem->crs_ln - locmem->top_ln].owner);
        else if(mode[0] == '2')
          sprintf(buf, "終結所有看板中的 [%.40s] 嗎(Y/N)？[N] ", 
            headers[locmem->crs_ln - locmem->top_ln].title);
        else
          return RC_FOOT; 
        getdata(b_lines, 0, buf, ans, 4, LCECHO,0);
        if (ans[0] != 'y')
          return RC_FOOT;
  
        resolve_boards();
        for (bp = bcache, i = 0; i < numboards; i++, bp++)
        {
          TagNum = 0;
          setbdir(buf, bp->brdname);
          outmsg(buf);
          refresh();
          if(mode[0] != '1') 
            TagThread(buf,headers[locmem->crs_ln - locmem->top_ln].title, 0);
          else
            TagThread(buf,headers[locmem->crs_ln - locmem->top_ln].owner, 1);
          if (TagNum)
          {
            delete_range2(buf, 0, 0); 
          }
        }
        if(mode[0] != '1') 
          log_usies("SPAM title ", currtitle);
        else
          log_usies("SPAM user", currtitle);
        TagNum = 0;
        return RC_CHDIR;
      }
      return RC_NONE;
    }
  }
}


void
i_read(cmdmode, direct, dotitle, doentry, rcmdlist, num_record)
  char *direct;
  void (*dotitle) ();
  void *(*doentry) ();
  int *num_record;
struct one_key *rcmdlist;
{
  keeploc *locmem;
  int recbase, mode, ch;
  int num, entries;
  int i;
  int jump = 0;
  char genbuf[4];
  char currdirect0[64];
  int last_line0 = last_line;
  int hit_thread0 = hit_thread;
  fileheader* headers0 = headers;
  char bar_color[50];
  
  strcpy(currdirect0 ,currdirect);
  
  get_lightbar_color(bar_color);

  headers = (fileheader *) calloc(p_lines, FHSZ);
  strcpy(currdirect, direct);
  mode = RC_NEWDIR;

  do
  {
    /* -------------------------------------------------------- */
    /* 依據 mode 顯示 fileheader                                 */
    /* -------------------------------------------------------- */

    setutmpmode(cmdmode);

    switch (mode)
    {
    case RC_NEWDIR:             /* 第一次載入此目錄 */
    case RC_CHDIR:
      last_line = rec_num(currdirect, FHSZ);
      if (num_record != NULL)
      {
         *num_record = last_line;
	 num_record = NULL;
      }
      
      if (mode == RC_NEWDIR)
      {
        if (last_line == 0)
        {
          if (currstat == VOTING)
          {
            if (currmode & MODE_BOARD)
            {
              getdata(b_lines - 1, 0, "尚未有投票 (V)舉辦投票 (Q)離開？[Q] ",
                genbuf, 4, LCECHO, 0);
              if (genbuf[0] == 'v')
                DL_func("SO/vote.so:make_vote");
            }
            else
              pressanykey("尚未有投票");

            goto return_i_read;
          }                       
          else if (currmode & MODE_DIGEST)
          {
            board_digest(); /* Kaede */
            pressanykey("尚未收錄文摘");
          }
          else if (currmode & MODE_SELECT)
          {
            board_select(); /* Leeym */
            pressanykey("沒有此系列的文章");
          }
          else if (curredit & EDIT_MAIL)
          {
            pressanykey("沒有來信");
            goto return_i_read;
          }
          else if (cmdmode == GAME)
            goto return_i_read;
          else
          {
            char *choose_post[2] = {"pP)發表文章", msg_choose_cancel};
            if (getans2(b_lines - 1, 0, "看板新成立 ", choose_post, 2, 'q') == 'p')
              do_post();
            goto return_i_read;
          }
        }
        num = last_line - p_lines + 1;
        locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
      }
      recbase = -1;

    case RC_FULL:
      clear();	//hialan.021129: 先清空螢幕再說..:)
      (void) (*dotitle) ();

    case RC_BODY:
      if (last_line < locmem->top_ln + p_lines) //當畫面有顯示最後一項
      {						//才更新 last_line
        num = rec_num(currdirect, FHSZ);

        if (last_line != num)
        {
          last_line = num;
          recbase = -1;
        }
      }

      if (last_line == 0)
         goto return_i_read;
      else if (recbase != locmem->top_ln) 
      {
        recbase = locmem->top_ln;
        if (recbase > last_line)
        {
          recbase = last_line - p_lines >> 1;
          if (recbase < 1)
            recbase = 1;
          locmem->top_ln = recbase;
        }
        entries = get_records(currdirect, headers, FHSZ, recbase, p_lines);

      }
      if (locmem->crs_ln > last_line)
        locmem->crs_ln = last_line;
      move(3, 0);
      clrtobot();

    case RC_DRAW:
      move(3, 0);
      for (i = 0; i < entries; i++)
      {
        /*Change For LighteBar by hialan*/
          (*doentry) (locmem->top_ln + i, &headers[i], i+3, 0);
      }

    case RC_FOOT:
      if(curredit & EDIT_MAIL)
        readfoot(2);
      else
        readfoot(1);
      break;
    case RS_PREV:
    case RS_NEXT:
    case RELATE_PREV:
    case RELATE_NEXT:
    case RELATE_FIRST:
    case POS_NEXT:
    case 'A':
    case 'a':
    case '/':
    case '?':
      jump = 1;
      break;
    }

    /* -------------------------------------------------------- */
    /* 讀取鍵盤，加以處理，設定 mode                             */
    /* -------------------------------------------------------- */

    if (!jump) 
    {
    /*Change For LightBar by hialan on 20020609*/
       int row;

       row=3 + locmem->crs_ln - locmem->top_ln;
       
       if(HAVE_HABIT(HABIT_LIGHTBAR))
         (*doentry) (locmem->crs_ln, &headers[locmem->crs_ln - locmem->top_ln], row, bar_color);
       cursor_show(row, 0);

       ch = igetkey();
       if(HAVE_HABIT(HABIT_LIGHTBAR))
         (*doentry) (locmem->crs_ln, &headers[locmem->crs_ln - locmem->top_ln], row, 0);
       
       mode = RC_NONE;
    }
    else
       ch = ' ';

    if (mode == POS_NEXT) 
    {
       mode = cursor_pos(locmem, locmem->crs_ln + 1, 1);
       if (mode == RC_NONE)
          mode = RC_DRAW;
       jump = 0;
    }
    else if (ch >= '0' && ch <= '9')
    {
      if ((i = search_num(ch, last_line)) != -1)
        mode = cursor_pos(locmem, i + 1, 10);
    }
    else
    {
      if (!jump)
         mode = i_read_key(rcmdlist, locmem, ch);

      while (mode == RS_NEXT || mode == RS_PREV ||
        mode == RELATE_FIRST || mode == RELATE_NEXT || mode == RELATE_PREV ||
        mode == THREAD_NEXT || mode == THREAD_PREV
          || mode == 'A' || mode == 'a' || mode == '/' || mode == '?')
      {
        int reload;

        if (mode == RS_NEXT || mode == RS_PREV)
        {
          reload = move_cursor_line(locmem, mode);
        }
        else
        {
          reload = thread(locmem, mode);
          if (!hit_thread)
          {
            mode = RC_FULL;
            break;
          }
        }

        if (reload == -1)
        {
          mode = RC_FULL;
          break;
        }
        else if (reload)
        {
          recbase = locmem->top_ln;
          entries = get_records(currdirect, headers, FHSZ, recbase, p_lines);
          if (entries <= 0)
          {
            last_line = -1;
            break;
          }
        }
        num = locmem->crs_ln - locmem->top_ln;
        if (headers[num].owner[0] != '-')
          mode = i_read_key(rcmdlist, locmem, ch);
      }
    }
  } while (mode != QUIT);

#undef  FHSZ
return_i_read:
   free(headers);
   last_line = last_line0;
   hit_thread = hit_thread0;
   headers = headers0;
   strcpy(currdirect ,currdirect0);
   return;
}

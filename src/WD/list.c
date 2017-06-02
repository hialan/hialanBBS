/*-------------------------------------------------------*/
/* list.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : lists' routines                              */
/* create : 00/01/12                                     */
/* update : 00/01/12                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

extern int cmpuname();
extern cmpfilename();
extern char currdirect[];
extern struct one_key list_comm[];
extern int last_line;
extern char msg_cc[];
void listtitle();
void listdoent();
void ListEdit();


int
belong_list(fname, userid)
  char *fname, *userid;
{
  int fd, can = 0;
  PAL pal;

  if ((fd = open(fname, O_RDONLY)) >= 0)
  {
    while (read(fd, &pal, sizeof(pal)) == sizeof(pal))
    {
      if (!strcmp(pal.userid, userid))
      {
        if (pal.ftype & M_BAD)
	  can = 2;
	else
	  can = 1;
	break;
      }
    }
    close(fd);
  }
  return can;
}


static void
list_desc(fhdr, echo)
  fileheader *fhdr;
  int echo;
{
  char buf[80];

  if (currstat == LISTMAIN)
    sprintf(buf, "對此名單的描述：");
  else
    sprintf(buf, "對他的描述：");

  if (echo == DOECHO)
    memset(fhdr, 0, sizeof(fileheader));
  getdata(1, 0, buf, fhdr->title, 60, DOECHO,
	  (echo != DOECHO) ? fhdr->title : 0);

  strcpy(buf, strrchr(currdirect, '/') + 1);
  if (!strcmp(buf, FN_PAL) || (currdirect[0] == 'b'))
  {
    buf[0] = getans2(1, 0,"壞人嗎? ", 0, 2,'n');
    if (*buf != 'y')
      fhdr->filemode |= M_PAL;
    else
      fhdr->filemode |= M_BAD;
  }
}


int
list_add()
{
  char listfile[80], fpath[80], ans[4];
  fileheader hdr;
  time_t now = time(NULL);
  struct tm *ptime = localtime(&now);

  if (currstat != LISTEDIT)
  {
    char *list_choose[3]={"11.一般名單","22.好友名單","qQ.離開"};
    
    strcpy(listfile, "list.0");
    /*
     * getdata(1, 0, "新增 1)一般名單 2)好友名單 3)文章通知 Q)uit？[1] ",
     * ans, 3, LCECHO, 0);
     */
    
    /*修改此檔會發生錯誤  所以還是不要用的好...||| */
    /*參考自exbbs                         by hialan*/
    
    /*
    getdata(1, 0, "新增 1)一般名單 2)好友名單 3)上下站通知 Q)uit？[1] ",
	    ans, 3, LCECHO, 0);
    */
    
    /*
    getdata(1, 0, "新增 1)一般名單 2)好友名單 Q)uit？[1] ",
	    ans, 3, LCECHO, 0);
    */
    
    ans[0] = getans2(1, 0, "新增 ", list_choose, 3, '1');

    if (ans[0] == '\0')
      ans[0] = '1';

    switch (ans[0])
      {
	case '2':
	  strcpy(listfile, FN_PAL);
	  break;

	  /*
	   * case '3': strcpy(listfile, "postlist"); break;
	   */
	
	/*  hialan */
	/*
	case '3':
	  strcpy(listfile, FN_ALOHA);
	  break;
	*/  
	case 'q':
	  return RC_FULL;

	default:
	  getdata(1, 0, "請選擇名單 (0-9)？[0] ", ans, 3, DOECHO, 0);

	  if (ans[0] == '\0')
	    ans[0] = '0';

	  if (ans[0] >= '0' && ans[0] <= '9')
	    listfile[5] = ans[0];
      }

    setuserfile(fpath, FN_LIST);
    if (belong_list(fpath, listfile))
    {
      pressanykey("已有此名單了 !");
      return RC_FULL;
    }
    list_desc(&hdr, DOECHO);
    strcpy(hdr.filename, listfile);
    sprintf(hdr.date, "%02d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
    setuserfile(fpath, FN_LIST);
    if (rec_add(fpath, &hdr, sizeof(fileheader)) == -1)
      pressanykey("系統發生錯誤! 請通知站長!");

    if (currstat != LISTMAIN)
      ListMain();
  } else
  {

    /* itoc.010529: 好友名單檢查人數上限 */
    if (strstr(currdirect, FN_PAL))
    {
      if (rec_num(currdirect, sizeof(fileheader)) >= MAX_FRIEND)
      {
        pressanykey("好友人數超過上限");
        return RC_FULL;
      }
    }

    move(1, 0);
    usercomplete(msg_uid, listfile);

    if (belong_list(currdirect, listfile))
    {
      pressanykey("已有這位站友了 !");
      return RC_FULL;
    }
    if (listfile[0] && searchuser(listfile))
    {
      hdr.filemode = 0;
      list_desc(&hdr, DOECHO);
      if (strcmp(listfile, cuser.userid) &&
	  !strstr(currdirect, "boards/") &&
	  strstr(currdirect, FN_PAL))
      {
	PAL aloha;
	int pos;

	sethomefile(fpath, listfile, FN_ALOHA);

	pos = rec_search(fpath, &aloha, sizeof(aloha), cmpuname, (int) cuser.userid);
	if (pos)
	  rec_del(fpath, sizeof(PAL), pos, NULL, NULL);

	//getdata(1, 0, "加入上站通知嗎 (Y/N) ? [Y]", ans, 3, DOECHO, 0);
	ans[0] = getans2(1, 0, "加入上站通知嗎? ", 0, 2, 'y');
	
	if (*ans != 'n')
	{
	  hdr.filemode |= M_ALOHA;
	  strcpy(aloha.userid, cuser.userid);
	  rec_add(fpath, &aloha, sizeof(aloha));
	}
      }
      strcpy(hdr.filename, listfile);
      sprintf(hdr.date, "%02d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
      if (rec_add(currdirect, &hdr, sizeof(fileheader)) == -1)
	pressanykey("系統發生錯誤! 請通知站長!");
    }
  }

  strcpy(fpath, strrchr(currdirect, '/') + 1);
  if (!strcmp(fpath, FN_PAL))
    friend_load();
  return RC_FULL;
}


static int
list_del(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char genbuf[100];

  getdata(1, 0, msg_del_ny, genbuf, 3, LCECHO, 0);
  if (genbuf[0] == 'y')
  {
    if (currstat == LISTMAIN)
    {
      setuserfile(genbuf, fhdr->filename);
      unlink(genbuf);
    }
    strcpy(genbuf, strrchr(currdirect, '/') + 1);
    if (!strcmp(genbuf, FN_PAL))
    {
      PAL aloha;
      int pos;

      sethomefile(genbuf, fhdr->filename, FN_ALOHA);
      pos = rec_search(genbuf, &aloha, sizeof(aloha), cmpuname, (int) cuser.userid);
      if (pos)
	rec_del(genbuf, sizeof(PAL), pos, NULL, NULL);
    }
    strcpy(currfile, fhdr->filename);
    delete_file(direct, sizeof(fileheader), ent, cmpfilename);
  }
  strcpy(genbuf, strrchr(currdirect, '/') + 1);
  if (!strcmp(genbuf, FN_PAL))
    friend_load();
  
  return RC_FULL;
}


/* shakalaca.000115: 應該用 i_read 再讀一遍的.. 慢改 */
static int
list_view(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char buf[80];

  if (currstat == LISTMAIN)
  {
    setuserfile(buf, fhdr->filename);
    ListEdit(buf);
  } else
    my_query(fhdr->filename);

  return RC_FULL;
}


static int
list_merge(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  /* 引入名單: 判斷要引入的名單中如果有 id 存在 target 則跳過 */
  int fd;
  PAL list;
  char buf[STRLEN], source[STRLEN];

  if (currstat == LISTEDIT)
  {
    /*
     * if (!getdata(b_lines, 0, "引入 1)一般名單 2)好友名單 3)文章通知 ？",
     * buf, 2, DOECHO, 0)) return RC_FOOT;
     */
    if (!getdata(b_lines, 0, "引入 1)一般名單 2)好友名單 ？", buf, 2, DOECHO, 0))
      return RC_FOOT;

    switch (buf[0])
      {
	case '1':
	  strcpy(source, "list.0");
	  getdata(1, 0, "請選擇名單 (0-9)？[0] ", buf, 3, DOECHO, 0);

	  if (buf[0] == '\0')
	    buf[0] = '0';

	  if (buf[0] >= '0' && buf[0] <= '9')
	    source[5] = buf[0];

	  break;

	case '2':
	  strcpy(source, "pal");
	  break;
	  /*
	   * case '3': strcpy(source, "postlist"); break;
	   */
	default:
	  return RC_FULL;
      }

   /* itoc.010529: 不可以引入到同一份名單 */
   if (strstr(currdirect, source))
     return RC_FULL;

    setuserfile(buf, source);
    if ((fd = open(buf, O_RDONLY)) >= 0)
    {
      while (read(fd, &list, sizeof(list)) == sizeof(list))
      {
	if (!belong_list(currdirect, list.userid))
	  /* 將 list.filename 加入 currdirect */
	  rec_add(currdirect, &list, sizeof(PAL));
      }
      close(fd);
    }
    return RC_NEWDIR;
  }
  return RC_NONE;
}


static int
list_multi(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  int reciper;

  if (currstat == LISTEDIT)
  {
    CreateNameList();
    LoadNameList(&reciper, currdirect, msg_cc);
    multi_send(NULL, 0);
    return RC_FULL;
  }
  return RC_NONE;
}


static int
list_edit(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  fileheader tmpfhdr = *fhdr;
  char fpath[80], ans[3];

  tmpfhdr.filemode = 0;
  list_desc(&tmpfhdr, GCARRY);

  /*
   * shakalaca.000117: 改描述而以嘛.. :p 用 enter 也可以改內容, so..
   * setuserfile(buf, fhdr->filename); if (currstat == LISTMAIN)
   * ListEdit(buf);
   */
  strcpy(fpath, strrchr(currdirect, '/') + 1);
  if (strcmp(fhdr->filename, cuser.userid) &&
      (currdirect[0] != 'b') &&
      !strcmp(fpath, FN_PAL))
  {
    PAL aloha;

    sethomefile(fpath, fhdr->filename, FN_ALOHA);
    {
      int pos;

      pos = rec_search(fpath, &aloha, sizeof(aloha), cmpuname, (int) cuser.userid);
      if (pos)
	rec_del(fpath, sizeof(PAL), pos, NULL, NULL);
    }

    ans[0] = getans2(1, 0, "加入上站通知嗎? ", 0, 2, 'y');
    if (*ans != 'n')
    {
      tmpfhdr.filemode |= M_ALOHA;
      strcpy(aloha.userid, cuser.userid);
      rec_add(fpath, &aloha, sizeof(aloha));
    }
  }
  *fhdr = tmpfhdr;
  substitute_record(direct, fhdr, sizeof(*fhdr), ent);

  return RC_FULL;
}


static int
list_move(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  fileheader *tmp;
  char newnum[5], buf[30];
  int num, max, min;
  int fail;

  sprintf(buf, "請輸入第 %d 選項的新次序：", ent);
  if (!getdata(b_lines, 0, buf, newnum, 5, DOECHO, 0))
    return RC_FOOT;

  if ((num = atoi(newnum) - 1) < 0)
    num = 0;
  else if (num >= last_line)
    num = last_line - 1;

  min = num < (ent - 1) ? num : (ent - 1);
  max = num > (ent - 1) ? num : (ent - 1);
  tmp = (fileheader *) calloc(max + 1, sizeof(fileheader));

  fail = 0;
  if (get_records(direct, tmp, sizeof(fileheader), 1, min) != min)
    fail = 1;

  if (num > (ent - 1))
  {
    if (get_records(direct, &tmp[min], sizeof(fileheader), ent + 1, max - min) != max - min)
      fail = 1;
    if (get_records(direct, &tmp[max], sizeof(fileheader), ent, 1) != 1)
      fail = 1;
  } else
  {
    if (get_records(direct, &tmp[min], sizeof(fileheader), ent, 1) != 1)
      fail = 1;
    if (get_records(direct, &tmp[min + 1], sizeof(fileheader), num + 1, max - min) != max - min)
      fail = 1;
  }
  if (!fail)
    substitute_record(direct, tmp, sizeof(fileheader) * (max + 1), 1);
  ent = num + 1;
  free(tmp);
  return RC_NEWDIR;
}


static int
list_help(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  move(3, 0);
  clrtobot();
  move(3, 0);
  prints("\n施工中.. ^^;");
  pressanykey(NULL);

  return RC_FULL;
}


struct one_key list_comm[] = {
  'r', list_view, 		// 進入/檢視
  'c', list_edit,		// 修改
  'a', list_add,		// 新增
  'd', list_del,		// 刪除   
/* 會發生問題的地方.. */
  'i', list_merge, 		// 引入其他名單
  's', list_multi,		// 群組寄信
  'm', list_move,		// 改變位置
  'h', list_help,  		// 使用說明
  '\0', NULL
};

/*Change For LightBar by hialan on 20020609*/
void
listdoent(num, ent, row, bar, bar_color)
  int num, row ,bar;
  fileheader *ent;
  char *bar_color;
{
  move(row, 0);
  clrtoeol();
  
  if (currstat != LISTMAIN)
  {
    prints(" %4d \033[1;33m%c\033[31m%c\033[36m%c\033[m  %-6s %s%-13s\033[m %-40s\n", num,
	   ent->filemode & M_PAL ? 'f' : ' ',
	   ent->filemode & M_BAD ? 'b' : ' ',
	   ent->filemode & M_ALOHA ? 'a' : ' ',
	   is_alnum(ent->date[0]) ? ent->date : "      ", 
	   (bar) ? bar_color : "", ent->filename, ent->title);
  } else
    prints(" %4d %-6s %s%-13s\033[m %-40s\n", 
                num, ent->date, (bar) ? bar_color : "", ent->filename, ent->title);
}


void
listtitle()
{
  char buf[256];
  sprintf(buf, "%s [線上 %d 人]", BOARDNAME, count_ulist());
  showtitle("群組名單", buf);
  if (currstat != LISTMAIN)
    outs("\
  [a]新增 [c]修改 [d]刪除 [m]移動 [i]引入名單 [s]群組寄信 [→]觀看 [h]elp\n\
" COLOR1 "\033[1m 編號 模式 日  期 名  稱        描        述                                   \033[0m");
  else
    outs("\
  [a]新增 [c]修改 [d]刪除 [m]移動 [→]觀看 [h]elp\n\
" COLOR1 "\033[1m 編號 日  期 名  稱        描        述                                        \033[0m");
}


int
ListMain()
{
  char buf[STRLEN];

  setuserfile(buf, FN_LIST);
  i_read(LISTMAIN, buf, listtitle, listdoent, list_comm, NULL);

  return RC_FULL;
}


void
ListEdit(fname)
  char *fname;
{
  char currdirect0[64];

  strcpy(currdirect0, currdirect);
  strcpy(currdirect, fname);
  i_read(LISTEDIT, fname, listtitle, listdoent, list_comm, NULL);
  strcpy(currdirect, currdirect0);
}

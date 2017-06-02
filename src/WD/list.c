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


int belong_list(char *fname, char *userid)
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


static void list_desc(fileheader *fhdr, int echo)
{
  char buf[80];

  if (currstat == LISTMAIN)
    sprintf(buf, "對此名單的描述：");
  else
    sprintf(buf, "對他的描述：");

  if (echo == DOECHO) memset(fhdr, 0, sizeof(fileheader));
  getdata(1, 0, buf, fhdr->title, 60, DOECHO,
	  (echo != DOECHO) ? fhdr->title : 0);

  strcpy(buf, strrchr(currdirect, '/') + 1);
  if (!strcmp(buf, FN_PAL) || (currdirect[0] == 'b'))
  {
    if (getans2(1, 0,"壞人嗎? ", 0, 2,'n') != 'y')
      fhdr->filemode |= M_PAL;
    else
      fhdr->filemode |= M_BAD;
  }
}


int list_add()
{
  char listfile[80], fpath[80], ans[4];
  fileheader hdr;
  time_t now = time(NULL);
  struct tm *ptime = localtime(&now);

  if (currstat != LISTEDIT)
  {
    char *list_choose[3]={"11)一般名單","22)好友名單", msg_choose_cancel};
    strcpy(listfile, "list.0");

    switch (getans2(1, 0, "新增 ", list_choose, 3, '1'))
    {
	case '2':
	  strcpy(listfile, FN_PAL);
	  break;

	case 'q':
	  return RC_FULL;

	default:
	  getdata(1, 0, "請選擇名單 (0-9)？[0] ", ans, 3, DOECHO, 0);
	  if (ans[0] == '\0') ans[0] = '0';
	  if (ans[0] >= '0' && ans[0] <= '9') listfile[5] = ans[0];
	  break;
    }

    sethomefile(fpath, cuser.userid, FN_LIST);
    if (belong_list(fpath, listfile))
    {
      pressanykey("已有此名單了 !");
      return RC_FULL;
    }
    list_desc(&hdr, DOECHO);
    strcpy(hdr.filename, listfile);
    sprintf(hdr.date, "%02d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
    sethomefile(fpath, cuser.userid, FN_LIST);
    if (rec_add(fpath, &hdr, sizeof(fileheader)) == -1)
      pressanykey("系統發生錯誤! 請通知站長!");

    if (currstat != LISTMAIN) ListMain();
  } 
  else
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
    
    if (!listfile[0])
      return RC_CHDIR;  //用 RC_CHDIR 來表示取消 for ListEdit()

    if (belong_list(currdirect, listfile))
    {
      pressanykey("已有這位站友了 !");
      return RC_CHDIR;
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

	if (getans2(1, 0, "加入上站通知嗎? ", 0, 2, 'y') != 'n')
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


static int list_del(int ent, fileheader *fhdr, char *direct)
{
  char genbuf[100];

  getdata(1, 0, msg_del_ny, genbuf, 3, LCECHO, 0);
  if (genbuf[0] == 'y')
  {
    if (currstat == LISTMAIN)
    {
      sethomefile(genbuf, cuser.userid, fhdr->filename);
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
static int list_view(int ent, fileheader *fhdr, char *direct)
{
  char buf[80];

  if (currstat == LISTMAIN)
  {
    sethomefile(buf, cuser.userid, fhdr->filename);
    ListEdit(buf);
  } 
  else
    my_query(fhdr->filename);

  return RC_FULL;
}


static int list_merge(int ent, fileheader *fhdr, char *direct)
{
  /* 引入名單: 判斷要引入的名單中如果有 id 存在 target 則跳過 */
  int fd;
  PAL list;
  char buf[STRLEN], source[STRLEN];

  if (currstat == LISTEDIT)
  {
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

	default:
	  return RC_FULL;
      }

   /* itoc.010529: 不可以引入到同一份名單 */
   if (strstr(currdirect, source))
     return RC_FULL;

    sethomefile(buf, cuser.userid, source);
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


static int list_multi(int ent, fileheader *fhdr, char *direct)
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


static int list_edit(int ent, fileheader *fhdr, char *direct)
{
  fileheader tmpfhdr = *fhdr;
  char fpath[80], ans[3];

  tmpfhdr.filemode = 0;
  list_desc(&tmpfhdr, GCARRY);

  /*
   * shakalaca.000117: 改描述而以嘛.. :p 用 enter 也可以改內容, so..
   * sethomefile(buf, cuser.userid, fhdr->filename); if (currstat == LISTMAIN)
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


static int list_move(int ent, fileheader *fhdr, char *direct)
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

static struct one_key list_comm[] = {
  'r', list_view, 0, "進入/檢視",0,
  'c', list_edit, 0, "修改",0,
  'a', list_add,  0, "新增",0,
  'd', list_del,  0, "刪除",0,
/* 會發生問題的地方.. */
  'i', list_merge,0, "引入其他名單",0,
  's', list_multi,0, "群組寄信",0,
  'm', list_move, 0, "改變位置",0,
  '\0', NULL, 0, NULL,0};

/*Change For LightBar by hialan on 20020609*/
static void listdoent(int num, fileheader *ent, int row, char *bar_color)
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
	   (bar_color) ? bar_color : "", ent->filename, ent->title);
  } 
  else
    prints(" %4d %-6s %s%-13s\033[m %-40s\n", 
                num, ent->date, (bar_color) ? bar_color : "", ent->filename, ent->title);
}


static void listtitle()
{
  char buf[256];
  sprintf(buf, "%s [線上 %d 人]", BOARDNAME, count_ulist());
  showtitle("群組名單", buf);
  if (currstat != LISTMAIN)
  {
    outs("[a]新增 [c]修改 [d]刪除 [m]移動 [i]引入名單 [s]群組寄信 [→]觀看 [h]elp\n");
    prints("%s 編號 模式 日  期 名  稱        描        述                                   \033[0m", COLOR3);
  }
  else
  {
    outs("[a]新增 [c]修改 [d]刪除 [m]移動 [→]觀看 [h]elp\n");
    prints("%s 編號 日  期 名  稱        描        述                                        \033[0m", COLOR3);
  }
}


int ListMain()
{
  char buf[STRLEN];

  sethomefile(buf, cuser.userid, FN_LIST);
  if(rec_num(buf, sizeof(fileheader)) == 0)
  {
    char *choose_list[2] = {"nN)新增", msg_choose_cancel};  
    
    if (getans2(1, 0, "尚未有名單 ", choose_list, 2, 'q') == 'n') 
    {
      usint currstat0 = currstat;
      char currdirect0[64];

      setutmpmode(LISTMAIN);
      strcpy(currdirect0, currdirect);
      strcpy(currdirect, buf);          
      list_add();
      strcpy(currdirect, currdirect0);
      setutmpmode(currstat0);
    }
    else
      return RC_FULL;
  }
  i_read(LISTMAIN, buf, listtitle, listdoent, list_comm, NULL);

  return RC_FULL;
}


void ListEdit(char *fname)
{
  char currdirect0[64];
  int tmp = 1;
  strcpy(currdirect0, currdirect);
  strcpy(currdirect, fname);    

  if(rec_num(fname, sizeof(fileheader)) == 0)
  {
    char *choose_list[2] = {"nN)新增", msg_choose_cancel};
    if (getans2(1, 0, "", choose_list, 2, 'q') == 'n') 
    {
      usint currstat0 = currstat;
      setutmpmode(LISTEDIT);
      tmp = list_add();
      setutmpmode(currstat0);
      if(tmp == RC_CHDIR) tmp = 0;  //RC_CHDIR 用來判斷是否為中途跳出?
    }
    else
      tmp = 0;
  }
  if(tmp) i_read(LISTEDIT, fname, listtitle, listdoent, list_comm, NULL);
  strcpy(currdirect, currdirect0);
  return;
}

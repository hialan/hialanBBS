/*-------------------------------------------------------*/
/* soman.c        ( AT-BBS/WD_hialan BBS    Ver 1.00 )   */
/*-------------------------------------------------------*/
/* target : so 外掛管理套件 (so manager)                 */
/* create : 2003/07/22                                   */
/* author : hialan					 */
/*-------------------------------------------------------*/
#include "bbs.h"

extern *currdirect;
extern int cmpfilename();

#if 0
struct fileheader
{
  char filename[FNLEN-1];       /* 檔案名稱	 	33 bytes*/
  char score;			/* 評分                  1 bytes*/
  char savemode;                /* file save mode 	 1 bytes*/
  char owner[IDLEN + 2];        /* 程式進入點 		14 bytes*/
  char date[6];                 /* [02/02] or space(5)   6 bytes*/
  char title[TTLEN + 1];	/* 程式說明		73 bytes*/
  uschar filemode;              /* must be last field @ boards.c 1 bytes*/
};
#endif

static void somantitle()
{
  char buf[256];
  
  sprintf(buf,"%s [線上 %d 人]",BOARDNAME, count_ulist());
  showtitle("外掛程式", buf);

  prints("[↑/↓]上下 [PgUp/PgDn]上下頁 [Home/End]首尾 [←][q]離開\n");
  prints("%s 編號    外  掛  程  式  敘  述                                                \033[m", COLOR3);
}

static void soman_doent(int num, fileheader *soman, int row, char *barcolor)
{
  char buf[31];
  
  sprintf(buf, "%s:%s", soman->filename, soman->owner);
  
  move(row, 0);
  clrtoeol();
  prints("%5d %s%s\033[m",
       num, (barcolor) ? barcolor : "", soman->title);
}

/*--------------------------------------*/
/*  SO Manager key function             */
/*--------------------------------------*/
static int soman_desc(fileheader *fhdr)
{
  int i=1;
  clear();
  getdata(i++, 0, "請輸入外掛檔案: ", fhdr->filename, 20, DOECHO, fhdr->filename);
  if(!fhdr->filename[0])
    return -1;
    
  getdata(i++, 0, "請輸入程式登入點: ", fhdr->owner, 13, DOECHO, fhdr->owner);
  if(!fhdr->owner[0])
    return -1;
  
  getdata(i++, 0, "請輸入本外掛的敘述: ", fhdr->title, 60, DOECHO, fhdr->title);
  return 0;
}

static int soman_add()
{
  fileheader hdr;
  char fname[PATHLEN];
  
  if(!HAS_PERM(PERM_SYSOP))
    return RC_NONE;
  
  hdr.filename[0]=hdr.owner[0]=hdr.title[0]='\0';
  if(soman_desc(&hdr) < 0)
    return RC_FULL;
  
  sprintf(fname, "etc/%s", FN_SOMAN);

  if (rec_add(fname, &hdr, sizeof(fileheader)) == -1)
    pressanykey("系統發生錯誤! 請通知站長!");
  return RC_FULL;
}

static int soman_exec(int ent, fileheader *fhdr, char *direct)
{
  if(dashf(fhdr->filename))
  {
    char buf[PATHLEN];
    void (*p)();
    
    sprintf(buf, "%s:%s", fhdr->filename, fhdr->owner);

    if((p = (void *) DL_get(buf)) == 0)
      return RC_FULL;
    
    (void)(p)();
  }
  else
    pressanykey("外掛程式不存在，請詢問站長!!");
  
  return RC_FULL;
}

static int soman_del(int ent, fileheader *fhdr, char *direct)
{
  if(!HAS_PERM(PERM_SYSOP))
    return RC_NONE;
  
  if(getans2(b_lines, 0, msg_sure, 0, 2, 'n') == 'y')
  {
    strcpy(currfile, fhdr->filename);
    delete_file(direct, sizeof(fileheader), ent, cmpfilename);    
  }
  return RC_FULL;
}

static int soman_edit(int ent, fileheader *fhdr, char *direct)
{
  if(getans2(b_lines, 0, msg_sure, 0, 2, 'n') == 'y')
  {
    soman_desc(fhdr);
    substitute_record(direct, fhdr, sizeof(*fhdr), ent);
  }
  return RC_FULL;
}

static int soman_detial(int ent, fileheader *fhdr, char *direct)
{
  int i=0;
  clear();
  move(i++, 0);
  outs("檔案位置: ");
  outs(fhdr->filename);
  
  move(i++, 0);
  outs("程式登入點: ");
  outs(fhdr->owner);
  
  move(i++, 0);
  outs("程式說明: ");
  outs(fhdr->title);
  
  pressanykey_old(NULL);
  return RC_FULL;
}

static struct one_key soman_comm[]={
  'r', soman_exec,  	     0, "執行外掛程式。", 0,
  'a', soman_add,   PERM_SYSOP, "增加外掛程式。", 0,
  'd', soman_del,   PERM_SYSOP, "刪除外掛程式。", 0, 
  'c', soman_edit,  PERM_SYSOP, "編輯外掛設定。", 0,
  'Q', soman_detial,PERM_SYSOP, "外掛程式詳細內容。", 0,
 '\0', NULL, 0, NULL, 0};

int soman()
{
  char fname[PATHLEN];
  
  sprintf(fname, "etc/%s", FN_SOMAN);
  if(dashf(fname))
  {
    if(rec_num(fname, sizeof(fileheader)) > 0)
      i_read(GAME, fname, somantitle, soman_doent, soman_comm, NULL);
    else
      soman_add();
  }
  else
    pressanykey("本站現不提供外掛程式!!");
  
  return 0;
}
/*hialan 給 spring, 轉換 id 用登記程式!!*/

#include "bbs.h"
extern struct BCACHE *brdshm;

#define FN_TRANB "tranb"

typedef struct{
  char brdname[IDLEN + 1];	//原本的看板名稱
  char title[BTLEN + 1];	//原本的看板標題
  char BM[IDLEN * 3 + 1];	//原本的看板板主
  
  char newname[IDLEN + 1];	//新的看板名稱
  char newBM[IDLEN * 3 + 1];	//新的看板板主
  
  time_t trantime;		//轉換申請時間
  
  char note[80];		//備註(如果以後要加什麼的話)
} tranbinfo;

#define TBI_ORI 0  //比對原始板名
#define TBI_NEW 1  //比對新的板名

static int belong_binfo(tranbinfo *ptr, int flag)
{
  // > 0	表示該申請的位置
  //<= 0	表示資料未申請
  char fname[80];
  int fd, size=sizeof(tranbinfo);
  tranbinfo tbinfo;
  int can=0, tag=0;
  
  sprintf(fname, "etc/%s", FN_TRANB);
  
  if((fd = open(fname, O_RDONLY)) >= 0)
  {
    while (read(fd, &tbinfo, size) == size)
    {
      can++;
      
      if(flag == TBI_ORI)
      {
        if(!strcmp(tbinfo.brdname, ptr->brdname))
        {
          tag = 1;
          break;
        }
      }
      else if(flag == TBI_NEW)
      {
        if(!strcmp(tbinfo.newname, ptr->newname) && strcmp(tbinfo.brdname, ptr->brdname))
        {
          tag = 1;
          break;
        }
      }
    }
    close(fd);
  }
  
  if (tag)
    return can;
  else
    return -1;
}

static void tranb_data(tranbinfo *ptr)
{
    move(2, 0);
    prints("看板名稱:  %s\n"
           "看板標題:  %s\n"
           "看板板主:  %s\n"
           "申請時間:  %s\n"
           "新看板名稱:%s\n"
           "新看板板主:%s", 
             ptr->brdname, ptr->title, ptr->BM, ctime(&ptr->trantime),
             ptr->newname, ptr->newBM);
}

static int tranb_add()
{
  char bname[IDLEN + 1];
  boardheader *bptr;
  tranbinfo newinfo;
  char buf[256];
  int pos;//弟幾筆資料!!
  
  clear();
  
  make_blist();
  namecomplete(MSG_SELECT_BOARD, bname);
  
  bptr=&brdshm->bcache[getbnum (bname)] - 1;
  if(!userid_is_BM(cuser.userid, bptr->BM))
  {
    pressanykey("你不是看板板主!");
    return -1;
  }

  strcpy(newinfo.brdname, bptr->brdname);
  pos = belong_binfo(&newinfo, TBI_ORI);
  if(pos > 0)
  {
    sprintf(buf, "etc/%s", FN_TRANB);
    rec_get(buf, &newinfo, sizeof(tranbinfo), pos);
    
    clear();
    prints("您已經申請過了, 是第%2d筆資料!!", pos);

    tranb_data(&newinfo);
           
    if(getans("請問是否修改申請(Y/N)？[N]") !='y')
      return -1;
  }
  else
  {
    strcpy(newinfo.newname, newinfo.brdname);
    strcpy(newinfo.newBM, newinfo.BM);
    strcpy(newinfo.title, bptr->title);
    strcpy(newinfo.BM, bptr->BM);
    newinfo.trantime = time(0);
  }
  
  clear();
  stand_title("看板基本資料");
  move(1, 0);
  prints("看板名稱:  %s\n看板標題:  %s\n看板板主:  %s\n申請時間:  %s", 
           newinfo.brdname, newinfo.title, newinfo.BM, ctime(&newinfo.trantime));
  
  if(getans("是否確定(Y/n)？[N]") != 'y')
  {
    pressanykey("使用者取消。");
    return 0;
  }
  
  do
  {
    getdata(5, 0, "請輸入新的看板名稱: ", newinfo.newname, IDLEN + 1, DOECHO, newinfo.newname);
  }while(!newinfo.newname[0] || belong_binfo(&newinfo, TBI_NEW) > 0);
  
  getdata(6, 0, "請輸入新的板主名稱: ", newinfo.newBM, IDLEN*3 + 1, DOECHO, newinfo.newBM);
  if(!newinfo.newBM[0])
  {
    pressanykey("輸入錯誤, 使用原設定!!");
    strcpy(newinfo.newBM, newinfo.BM);
  }
  
  sprintf(buf, "etc/%s", FN_TRANB);
  if(pos<=0)
    rec_add(buf, &newinfo, sizeof(tranbinfo));
  else
    substitute_record(buf, &newinfo, sizeof(tranbinfo), pos);
  
  return 0;
}

static int tranb_show()
{
  char buf[80];
  tranbinfo tinfo;
  int pos;
  
  clear();
  make_blist();
  namecomplete(MSG_SELECT_BOARD, buf);

  strcpy(tinfo.brdname, buf);
  pos = belong_binfo(&tinfo, TBI_ORI);
  
  if(pos <= 0)
  {
    pressanykey("該看板尚未申請!!");
    return -1;
  }
  
  sprintf(buf, "etc/%s", FN_TRANB);
  rec_get(buf, &tinfo, sizeof(tranbinfo), pos);
  
  tranb_data(&tinfo);
  pressanykey(NULL);
  
}

static void tranb_all_line(tranbinfo *ptr, int line)
{
  move(line, 0);
  clrtoeol();
  prints("%-4.4s %-13s %-13s %s", ptr->title, ptr->brdname, ptr->newname, ptr->newBM);
}

static tranb_all()
{
  char buf[80];
  tranbinfo binfo[MAXBOARD];
  int fd, size=sizeof(tranbinfo), i, pos;
  
  sprintf(buf, "etc/%s", FN_TRANB);

  if((fd = open(buf, O_RDONLY)) >= 0)
  {
    i = 0;
    
    while (read(fd, binfo+i, size) == size)
      i++;
    
    close(fd);
  }
  else
  {
    pressanykey("開檔錯誤!!");
    return -1;
  }
  
  if(i==0)
  {
    pressanykey("目前並沒有任何申請!!");
    return 0;
  }

  sprintf(buf,"%s [線上 %d 人]",BOARDNAME, count_ulist());
  showtitle("所有列表", buf);
  outs("    按任意鍵換頁!!\n");
  prints("%s類別 原始板名      新板名        新板主                                        \033[m", COLOR2);

  for(pos=0, i--;i>=0;i--, pos++)
  {
    if(pos == p_lines)
    {
      pressanykey(NULL);
      pos = 0;
    }
    tranb_all_line(binfo+i, 3+pos);
  }
  pressanykey(NULL);
}

int tranb()
{
  char buf[5];
  stand_title("看板轉換申請程式");
  
  getdata(1, 0, "請問要: [1]申請看板轉換 [2]查詢申請紀錄 [3]看所有申請 [Q]取消？[Q] ", buf, 5, LCECHO, 0);
  if(buf[0] == '1')
    tranb_add();
  else if(buf[0] == '2')
    tranb_show();
  else if(buf[0] == '3')
    tranb_all();
  
  return 0;
}
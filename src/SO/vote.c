/*-------------------------------------------------------*/
/* vote.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : boards' vote routines                        */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

extern cmpbnames();
extern int numboards;
// extern void friend_edit(int type);
extern cmpfilename();

#if 0
 .vch : vote control header, 套用 fileheader, 以 i_read 整合,
	  因此可以同一時間舉行多個投票. :)
struct fileheader
{
  char filename[FNLEN];          V.[closetime].A  投票目的
  char score;			 hialan: 目前無用
  char savemode;                 可投的最大票數
  char owner[IDLEN + 2];         votetime
  char date[6];                  opentime[xx/xx]
  char title[TTLEN + 1];         vote_title
  uschar filemode;               Vote_way : 一般, 私人, 打分, etc.
  					     1     2     3     4
  				VOTE_NORMAL	0x01	一般
  				VOTE_PRIVATE	0x02	私人
  				VOTE_SCORE	0x04	打分
  				VOTE_PAPER	0x08	問卷
};

#endif

#define FHSZ	sizeof(fileheader)
#define VOTE_NORMAL	0x01	/* 一般 */
#define VOTE_PRIVATE	0x02    /* 私人 */
#define VOTE_SCORE	0x04    /* 打分 */
#define VOTE_PAPER	0x08	/* 問卷 */

static char STR_bv_control[] = ".control";	/* 投票日期 選項 */
static char STR_bv_ballots[] = ".ballots";	/* 投過的票 */
static char STR_bv_results[] = ".results";	/* 投票結果 */
static char STR_bv_flags[] = ".flags";		/* 有無投過票 */
static char STR_fn_vote_polling[] = ".voting";
static char STR_bv_comments[] = ".comments";	/* 投票者的建意 */

boardheader *bcache;

void
b_closepolls()
{
  boardheader fh;
  struct stat st;
  FILE *cfp;
  time_t now;
  int fd, dirty;
  char currboard0[15];

  now = time(0);

  if (stat(STR_fn_vote_polling, &st) == -1/* || st.st_mtime > (now - 3600)*/)
  {
    pressanykey("No Such File : .voting !");
    return;
  }

  if ((cfp = fopen(STR_fn_vote_polling, "w")) == NULL)
  {
    return;
  }
  fprintf(cfp, ctime(&now));
  fclose(cfp);

  resolve_boards();
  if ((fd = open(fn_board, O_RDWR)) == -1)
  {
    pressanykey(".BOARDS 開啟錯誤");
    return;
  }

  flock(fd, LOCK_EX);

  outmsg("系統開票中, 請稍候...");
  sleep(1);

  strcpy(currboard0, currboard);
  dirty = 0;
  while (read(fd, &fh, sizeof(fh)) == sizeof(fh))
  {
    if (fh.bvote && b_close(&fh))
    {
      lseek(fd, (off_t) sizeof(fh) * (getbnum(fh.brdname) - 1), SEEK_SET);
      if (write(fd, &fh, sizeof(fh)) != sizeof(fh))
      {
        sprintf(currmsg, "\033[1;5;37;41mWarning!\033[m");
        kill(currpid, SIGUSR2);
        igetch();
        break;
      }
      dirty = 1;
    }
  }
  if (dirty) /* vote flag changed */
    touch_boards();

  flock(fd, LOCK_UN);
  close(fd);

  strcpy(currboard, currboard0);
  return;
}                       


time_t
next_closetime(direct, closetime)
  char *direct;
  time_t closetime;
{
  int num;
  fileheader vfh;

  if (!(num = rec_num(direct, FHSZ)))
    return closetime;
  else
  {
    num = open(direct, O_RDONLY);
    if (read(num, &vfh, FHSZ) == FHSZ)
    {
      closetime = atol(vfh.filename + 2);
      while(read(num, &vfh, FHSZ) == FHSZ)
      {
        if (closetime > atol(vfh.filename + 2))
          closetime = atol(vfh.filename + 2);
      }
    }
    close(num);                                              
    return closetime;
  }
}


void
delete_vote_files(ent, fhdr, direct, fpath)
  int ent;
  fileheader *fhdr;
  char *direct, *fpath;
{
  int len;

  setbfile(fpath, currboard, fhdr->filename);
  len = strlen(fpath);

  strcpy(fpath + len, STR_bv_ballots);
  unlink(fpath);
  strcpy(fpath + len, STR_bv_control);
  unlink(fpath);
  strcpy(fpath + len, STR_bv_flags);
  unlink(fpath);
  strcpy(fpath + len, STR_bv_comments);                                        
  unlink(fpath);
  *(fpath + len) = 0;
  unlink(fpath);

  strcpy(currfile, fhdr->filename);
  delete_file(direct, FHSZ, ent, cmpfilename);
}


void
result_post(fhdr, bname, fpath)
  fileheader *fhdr;
  char *bname, *fpath;
{
  char distpath[STRLEN];
  fileheader postfile;
  FILE *fp;

  setbpath(distpath, bname);
  memset(&postfile, 0, FHSZ);
  stampfile(distpath, &postfile);

  postfile.savemode = 'L';                               
  strcpy(postfile.owner,"[馬路探子]");
  snprintf(postfile.title, TTLEN, "[%s 板開票結果] <%s>",
    currboard, fhdr->title);

  fp = fopen(distpath, "a");
  b_suckinfile(fp, fpath);
  fclose(fp);

  setbdir(distpath, bname);
  rec_add(distpath, &postfile, FHSZ);
}                        


void
make_results(fhdr, counts, buf, fpath, interrupt)
  fileheader *fhdr;
  int *counts, interrupt;	/* 人為中止 */
  char *buf, *fpath;
{
  FILE *fout, *fin;
  int len, total = 0, num;
  time_t votetime, closetime;
  int fd, n;
  char inbuf[256];

  setbfile(fpath, currboard, fhdr->filename);
  strcpy(buf, fpath);
  len = strlen(fpath);

  strcpy(fpath + len, STR_bv_results);
  fout = fopen(fpath, "w");

  votetime = (time_t)atol(fhdr->owner);
  if (interrupt)
    time(&closetime);
  else
    closetime = atol(fhdr->filename + 2);
  fprintf(fout, "\n%s\n◆ 投票開始於: %s", MSG_SEPERATOR, Ctime(&votetime));
  fprintf(fout, "\n◆ %s於: %s\n◆ 票選主題/描述: %-41.40s\n\n",
    interrupt ? "提早開票":"投票終止", Ctime(&closetime), fhdr->title);

  b_suckinfile(fout, buf);	/* 吃入說明 */

  fprintf(fout, "\n◆ 投票結果:\n\n");

  strcpy(buf + len, STR_bv_ballots);
  total = dashs(buf) / 2;	/* 總票數 */

  strcpy(buf + len, STR_bv_control);
  if (fin = fopen(buf, "r"))
  {
    int count = 0;
    
    while (fgets(inbuf, STRLEN, fin))
    {
      inbuf[(strlen(inbuf) - 1)] = '\0';
      num = counts[count];
      count++;
      fprintf(fout, "   %-51.50s %3d %s (%-02.2f %%)\n", inbuf + 3, num,
        (fhdr->filemode & VOTE_SCORE) ? "分" : "票",
        total ? (float) 100 * num/total : 0 );
    }
    fclose(fin);
  }

  if (!(fhdr->filemode & VOTE_SCORE))
    fprintf(fout, "\n◆ 總票數 = %d 票", total);

  setbfile(buf, currboard, fhdr->filename);
  strcpy(buf + len, STR_bv_flags);

  if ((fd = open(buf, O_RDONLY)) > 0)
  {
    total = 0;
    while ((n = read(fd, inbuf, 256)) == 256)
    {
      while(n)
      {
        if (inbuf[n---1])
          total++;
      }
    }
    close(fd);
  }
  else
    total = -1;

  fprintf(fout, "\n◆ 總人數 = %d 人\n", total);

  fprintf(fout, "\n◆ 我有話要說:\n\n");
  strcpy(buf + len, STR_bv_comments);
  b_suckinfile(fout, buf);
  fprintf(fout, "\n\n");
  fclose(fout);

  result_post(fhdr, currboard, fpath);
  if (strcmp(currboard, "Record"))
    result_post(fhdr, "Record", fpath);

  fout = fopen(fpath, "a");
  setbfile(buf, currboard, STR_bv_results);
  b_suckinfile(fout, buf);
  fclose(fout);
  f_mv(fpath, buf);
}


int
b_close(bh)
  boardheader *bh;
{
  char buf[STRLEN], fpath[STRLEN], inchar[2];
  int counts[100], fd, ent=1;
  time_t now;
  fileheader vfh;

  now = time(NULL);

  if (bh->bvote == 2)
  {
    if (bh->vtime < now - 7 * 86400) /* 開票已一星期 */
    {
      bh->bvote = 0;
      return 1;
    }
    else
      return 0;
  }

  if (bh->vtime > now)
  {
    bh->bvote = 1;
    return 0;
  }

  setbfile(buf, bh->brdname, FN_VOTE);
  if ((fd = open(buf, O_RDONLY)) == -1)
    return 0;

  while (read(fd, &vfh, FHSZ) == FHSZ)
  {
    if (bh->vtime == atol(vfh.filename + 2))         /* read vote data(title... */
      break;
    else
      ent++;                            /* count entry */
  }

  close(fd);

  setbfile(fpath, bh->brdname, vfh.filename);
  memset(counts, 0, sizeof(counts));
  strcpy(fpath+strlen(fpath), STR_bv_ballots);
  if ((fd = open(fpath, O_RDONLY)) != -1)
  {
    while (read(fd, &inchar, 2) == 2)
      counts[atoi (inchar) - 1]++;
    close(fd);
  }
  strcpy(currboard, bh->brdname);
  make_results(&vfh, &counts, buf, fpath, 0);
  setbfile(buf, bh->brdname, FN_VOTE);
  delete_vote_files(ent, &vfh, buf, fpath);

  if ((bh->vtime = next_closetime(buf, atol(vfh.filename + 2))) == atol(vfh.filename + 2))
  {
    setbfile(fpath, bh->brdname, "V.*");
    snprintf(buf, STRLEN, "/bin/rm -f %s", fpath);
    system(buf);
    bh->bvote = 2;
  }
  return 1;
}


int
vote_flag(fhdr, val)
  fileheader *fhdr;
  char val;
{
  char buf[256], flag;
  int fd, num, size;

  setbfile(buf, currboard, fhdr->filename);
  strcpy(buf + strlen(buf), STR_bv_flags);

  if ((fd = open(buf, O_RDWR | O_CREAT, 0600)) == -1)
  {
    return -1;
  }
  num = usernum - 1;
  size = lseek(fd, 0, SEEK_END);
  memset(buf, 0, sizeof(buf));
  while (size <= num)
  {
    write(fd, buf, sizeof(buf));
    size += sizeof(buf);
  }
  lseek(fd, num, SEEK_SET);
  read(fd, &flag, 1);

  if (flag == 0 && val != 0)
  {
    lseek(fd, num, SEEK_SET);
    write(fd, &val, 1);
  }
  close(fd);
  return flag;
}


int
vote_results(bname)
  char *bname;
{
  char buf[STRLEN];

  setbfile(buf, bname, STR_bv_results);
  if (more(buf, YEA) == -1)
  {
    pressanykey("目前沒有任何投票的結果。");
    return RC_DRAW;
  }
  return RC_FULL;
}


int
same(compare, list, num, rep)
  int compare;
  int list[];
  int num;
  int rep;
{
  int n;
  int tmp = 0;

  for (n = 0; n < num; n++)
  {
    if (compare == list[n])
      tmp = 1;
    if (rep && tmp == 1)
      list[n] = list[n+1];
  }
  return tmp;
}                        


static int
do_vote(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  FILE *fp;
  char buf[STRLEN], fpath[STRLEN], vote[3], inbuf[STRLEN];
  int i = 0, len, choice = 0, page = 0, count = 0, invote, bufvote[100], score[100];

  setbfile(fpath, currboard, fhdr->filename);
  len = strlen(fpath);

  if (!HAS_PERM(PERM_BASIC))
  {
    pressanykey("對不起! 您還沒有投票權喔!");
    return RC_DRAW;
  }

  if (cuser.totaltime < 86400/6)  /* wildcat : 改成用totaltime算吧 */
  {
    pressanykey("對不起! 要上站超過 4 小時才能投票喔!");
    return RC_FULL;
  }              

  if (fhdr->filemode & VOTE_PRIVATE)
  {
    setbfile(buf, currboard, FN_CANVOTE);
    if (!belong(buf, cuser.userid))
    {
      pressanykey("對不起! 這是私人投票..你並沒有受邀唷!");
      return RC_FULL;
    }
    else
      pressanykey("恭喜你受邀此次私人投票....");
  }

  if (vote_flag(fhdr, NULL))
  {
    pressanykey("此次投票，您已投過了！一人一次，大家平等。");
    return RC_FULL;
  }

  strcpy(fpath + len, STR_bv_control);
  fp = fopen(fpath, "r");
  if (fp == NULL)
  {
    pressanykey("控制檔遺失! 請告知管理者.");
    fclose(fp);
    return RC_FOOT;
  }

  setutmpmode(VOTING);
  log_usies("VOTE",NULL);

  *(fpath + len) = 0;
  more(fpath, YEA);

  memset(bufvote, 0, sizeof(bufvote));
  memset(score, 0, sizeof(score));
  while (page < 3)
  {
    stand_title("投票箱");
    rewind(fp);

    count = 0;
    
    if (fhdr->filemode & VOTE_SCORE)
      prints("\033[1;32m計分方式：確定好您的選擇後，輸入其代碼(1, 2, 3...)即可給分數。\033[0m\n"
      "\033[1;37;44m 分數的範圍為 1~10 分 , 按 [F] 結束投票 \033[0m。");
    else
      prints("\033[1;32m計票方式：確定好您的選擇後，輸入其代碼(1, 2, 3...)即可。\033[0m\n"
      "\033[1;37;44m 按 [F] 結束投票 \033[0m，此次投票你可以投 %1d 票。", 
      fhdr->savemode);

    while (fgets(inbuf, sizeof(inbuf), fp))
    {
      count++;
      if (count > 34 * page && count <= 34 * (page + 1))
      {
        move(((count - 1 - 34 * page) % 17) + 4,((count-1-34*page)/17)*40);

        if (fhdr->filemode & VOTE_SCORE)
        {
          int n, num = -1;

          inbuf[strlen(inbuf) - 1] = 0;
          for (n = 0; n < i; n++)
          {
            if (count == score[n])
            {
              num = n;
              break;
            }
          }
          
          if (num >= 0)
            prints(" %-31.30s %2d 分", inbuf, bufvote[num]);
          else
            prints(" %-31.30s  0 分", inbuf);
        }
        else          
        {
          if (same(count, bufvote, i, 0))
            prints( "*%s", strtok(inbuf,"\n\0"));
          else
            prints( " %s", strtok(inbuf,"\n\0"));
        }
      }                                                            
    }

    while(1)
    {
      vote[0] = vote[1] = vote[2] = '\0';
      if (fhdr->filemode & VOTE_SCORE)
        sprintf(buf, "輕輸入您的選擇，總共 %d 頁%s%s (F)確認 (Q)取消",
              count % 34 ? count/34+1 : count/34,
             (page > 0) ? " (P)上頁" : "", (
             count > (page + 1) * 34) ? " (N)下頁" : "" );
      else
        sprintf(buf, "您還可以投 %d 票，總共 %d 頁%s%s (F)確認 (Q)取消",
          (int)fhdr->savemode - i, count % 34 ? count/34+1 : count/34,
             (page > 0) ? " (P)上頁" : "", (
             count > (page + 1) * 34) ? " (N)下頁" : "" );
      getdata(t_lines-2, 0, buf, vote, 3, DOECHO, 0);
      move(t_lines - 2, 0);
      invote = atoi(vote);
      *vote = tolower(*vote);
      if (vote[0] == 'q' || (!vote[0] && !i))
      {
        clrtoeol();
        prints("[5m記得再來投喔!!      [m");
        refresh();
        page = 3; /* 跳出 */
        break;
      }
      else if (vote[0] == 'f' && i)
         ;	/* 投完了 */
      else if (vote[0] == 'p' || vote[0] == 'n')
      {
        if (vote[0] == 'p' && page > 0)
          page--;

        if (vote[0] == 'n' && page < (count / 34))
          page++;

        break;
      }
      else if ( invote <= 34 * page || invote > 34 * (page + 1))
         continue;
      else if (invote > 34 * page && invote <= 34 * (page + 1) &&
       invote <= count)
      {
        if (fhdr->filemode & VOTE_SCORE)   /* shakalaca.991127: 拿 bufvote 來當計分,
        				   此時 bufvote[第幾票] 應該紀錄到 多少分,�,
        				   所以多一個 score[100] 來記第幾選項吧 ! */
        {
          char ans[3];
          if (!getdata(t_lines - 2, 0, "分數: ", ans, 3, DOECHO, 0))
            continue;
          bufvote[i] = atoi(ans);	/* 投下的第幾票 (i) 有多少分 (ans) */
          if (bufvote[i] < 0 || bufvote[i] > 10)
            bufvote[i] = 0;
          move(((invote-1-page*34)%17)+4, (((invote-1-page*34))/17)*40 + 33);
          prints("%2d", bufvote[i]);
          score[i] = invote;	/* 投下的第幾票為第幾個選項 */
          i++;
          continue;
        }                          
        else
        {
          if (same(invote, bufvote, i, 1))
          {
            move(((invote-1-page*34)%17)+4, (((invote-1-page*34))/17)*40);
            prints(" ");
            i--;
            continue;
          }
          else
          {
            if (i == fhdr->savemode)
               continue;
            bufvote[i] = invote;
            move(((invote-1-page*34)%17)+4, (((invote-1-page*34))/17)*40);
              prints("*");
            i++;
            continue;
          }
        }
      }
      else
        continue;

      if (vote_flag(fhdr, vote[0]) != 0)
        prints("重覆投票! 不予計票。");
      else
      {
        strcpy(fpath + len, STR_bv_ballots);
        if ((choice = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600)) == 0)
          outs("無法投入票匭\n");
        else
        {
          struct stat statb;

          flock(choice, LOCK_EX);
          if (fhdr->filemode & VOTE_SCORE)
          {
            for (count = 0; count < i;count ++) /* count沒用了，拿來計數 */
            {
              for (i = 0; i < bufvote[count]; i++)	/* shakalaca.991127: 多少分就多少個
              						   選項寫入 */
              {
                sprintf(buf, "%02d", score[count]);
                write(choice, buf, 2);
              }              
            }
          }
          else
          {
            for (count = 0; count < i;count ++) /* count沒用了，拿來計數 */
            {
              sprintf(buf, "%02d", bufvote[count]);
              write(choice, buf, 2);
            }
          }
          flock(choice, LOCK_UN);
          fstat(choice, &statb);
          close(choice);
                                                                               
          if (getdata(t_lines - 4, 0, "有話要說嗎 ? (一行)\n>", buf, 65, DOECHO, 0))
          {
            strcpy(fpath+len, STR_bv_comments);
            if(fp = fopen(fpath, "a"))
            {
              fprintf(fp, "%-12s: %s\n", cuser.userid, buf);
              fclose(fp);
            }
          }
  
          substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* 記錄 */
 
          ingold(1);
          clrtoeol();
          if (fhdr->filemode & VOTE_SCORE)
            pressanykey("已完成投票，車馬費1元金幣！");
          else            
            pressanykey("已完成投票，車馬費1元金幣！(目前已投票數: %d)", statb.st_size/2);
        }
      }
      page = 3; /* 跳出 */
      break;             
    }
  }

  fclose(fp);
  return RC_FULL;
}


static int
maintain_vote(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char buf[STRLEN], fpath[STRLEN], inbuf[STRLEN], inchar[2];
  int len, page = 0;
  FILE *fp;

  if (!(currmode & MODE_BOARD))
    return 0;

  while (page < 3)
  {
    log_usies("VOTE_Edit",NULL);
    stand_title("投票管理");

    setbfile(fpath, currboard, fhdr->filename);
    len = strlen(fpath);

    strcpy(fpath + len, STR_bv_control);
    if ((fp = fopen(fpath, "r")))
    {
      int counts[100], fd, total, count = 0;

      memset(counts, 0, sizeof(counts));
      strcpy(fpath + len, STR_bv_ballots);

      total = dashs(fpath) / 2;
      if ((fd = open(fpath, O_RDONLY)) != -1)
      {
        while (read(fd, &inchar, 2) == 2)
          counts[atoi (inchar) - 1]++;
        close(fd);
      }

      outs("\n◆ 預知投票紀事:\n\n");
      while (fgets(inbuf, sizeof(inbuf), fp))
      {
        inbuf[(strlen(inbuf) - 1)] = '\0';
        inbuf[31] = '\0';

        fd = counts[count];
        count++;
        if (count > 34 * page && count <= 34 * (page + 1))
        {
           move(((count-1 - 34 * page)%17)+4,((count-1 - 34 * page)/17)*40);
/*           prints(" %-31.30s %3d 票 (%-02.2f %%)", 
             buf, fd , total ? (float) 100 * fd/total : 0 ); */
           prints(" %-31.30s %3d %s", inbuf, fd, 
            (fhdr->filemode & VOTE_SCORE) ? "分": "票");
        }
      }
      fclose(fp);

      move(b_lines-2, 0);
      if (!(fhdr->filemode & VOTE_SCORE))
        prints("\n◆ 目前總票數 = %d 票", total);

      fd = 0;

      if (fhdr->filemode & VOTE_PRIVATE)
        buf[0] = answer("(A/B)取消/提早投開票 (C/D)編輯說明/名單 (R)閱\讀留言 (P/N)上/下頁 (E)繼續？[E]");
      else  
        buf[0] = answer("(A)取消投票 (B)提早開票 (C)編輯說明 (R)閱\讀留言 (P/N)上/下頁 (D)繼續？[D] ");

      if (buf[0] == 'a' || buf[0] == 'A')
      {
        delete_vote_files(ent, fhdr, direct, fpath);
        fd = 1;
      }
      else if(buf[0] == 'b' || buf[0] == 'B')
      {
        make_results(fhdr, &counts, buf, fpath, 1);
        delete_vote_files(ent, fhdr, direct, fpath);
        fd = 2;
      }
      else if(buf[0] == 'c'|| buf[0] == 'C')
      {
        *(fpath + len) = 0;
        if (vedit(fpath, NA) == -1)
        {
          outs("投票說明[未更新]");
          pressanykey(NULL);
        }
      }
/*
      else if ((buf[0] == 'd' || buf[0] == 'D') && (fhdr->filemode & VOTE_PRIVATE))
      {
        friend_edit(FRIEND_CANVOTE);
      }
*/
      else if (buf[0] == 'r' || buf[0] == 'R')
      {
        strcpy(fpath + len, STR_bv_comments);
        more(fpath);
      }
      else if (buf[0] == 'p' || buf[0] == 'n' || buf[0] == 'P' || buf[0] == 'N')
      {
        if ((buf[0] == 'p' || buf[0] == 'P') && page > 0)
          page--;

        if ((buf[0] == 'n' || buf[0] == 'N') && count > (page + 1) * 34)
          page++;

        continue;
      }
      else
        break;                       

      if (fd)
      {
        boardheader bh;
        int pos;
  
        pos = rec_search(fn_board, &bh, sizeof(bh), cmpbnames, (int) currboard);
        if ((bh.vtime = next_closetime(direct, atol(fhdr->filename + 2))) == atol(fhdr->filename + 2))
          bh.bvote = fd == 1 ? 0 : 2;
        else
          bh.bvote = 1;
        if (substitute_record(fn_board, &bh, sizeof(bh), pos, 0) == -1)
          outs(err_board_update);
        touch_boards();           /* vote flag changed */
        resolve_boards();
      }
    }
    break;
    return RC_FULL;
  }

  return RC_FULL;
}


int
make_vote()
{
  char buf[STRLEN], fpath[STRLEN];
  int num = 0, len, aborted = 0, page = 0;
  time_t closetime;
  fileheader vfh;
  struct tm *ptime;
  FILE *fp;

  if (!(currmode & MODE_BOARD))
    return 0;

  setbfile(buf, currboard, FN_VOTE);
  if (rec_num(buf,FHSZ) >= 20)
  {
    pressanykey("%s 板已經有太多投票了!!",currboard);
    return 0;
  }

  stand_title("舉辦投票");
  log_usies("VOTE_Make",NULL);
  memset(&vfh, 0, FHSZ);

  getdata(2, 0, "投票方式 (1)一般 (2)計分 [1] ", buf, 4, LCECHO, 0);

  if (atoi(buf) == 2)
    vfh.filemode = VOTE_SCORE;
  else if ((atoi(buf) == 3) && HAS_PERM(PERM_SYSOP))
    vfh.filemode = VOTE_PAPER;
  else 
    vfh.filemode = VOTE_NORMAL;

  getdata(3, 0, "此次投票進行幾天 (至少１天)？", buf, 4, DOECHO, 0);
  if ((num = atoi(buf)) < 1)
    num = 1;

  time(&closetime);
  ptime = localtime(&closetime);
  sprintf(vfh.date, "%2d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
  sprintf(vfh.title + 65, "/%02d", ptime->tm_year % 100);
  sprintf(vfh.owner, "%d", closetime);

  getdata(4, 0, "投票主題：", vfh.title, 40, DOECHO, 0);
  if (1)
  {
    FILE *fp;
    char buf[128];
    sprintf(buf,"boards/%s/title",currboard);
    fp = fopen(buf,"w");
    fputs(vfh.title, fp);                                       
    fclose(fp);                                          
  }

  closetime += num * 86400;
  do
  {
    sprintf(vfh.filename, "V.%d.A", ++closetime);
    setbfile(fpath, currboard, vfh.filename);
  }while(dashf(fpath));
  len = strlen(fpath);

  pressanykey("按任何鍵開始編輯此次 [投票說明/宗旨]");

  if (vedit(fpath, NA) == -1)
  {
    unlink(fpath);
    pressanykey("取消此次投票");
    return RC_FULL;
  }
/*
  getdata(2, 0, "投票要設限嗎？ (Y/N) [N] ", buf, 4, LCECHO, 0);

  if (*buf == 'y')
  {
    friend_edit(FRIEND_CANVOTE);
    vfh.filemode |= VOTE_PRIVATE;
  }
 wildcat : 為什麼 else 要把 V.xxx.A unlink 掉? 難怪都沒說明了 :p
  else
  {
    if (dashf(fpath)) 
      unlink(fpath);
  }
*/
  clear();
  strcpy(fpath + len, STR_bv_control);
  fp = fopen(fpath, "w");

  move(0, 0);
  outs("請依序輸入選項, 按 ENTER 完成設定");
  num = 0;
  /* 輸入 99 個選項 */
  while (!aborted)
  {
    sprintf(buf, "%02d) ", num + 1);
    getdata(((num - page * 34) % 17) + 2, ((num - page * 34) / 17) * 40,
      buf, buf, 33, DOECHO, 0);
    if (*buf)
    {
      fprintf(fp, "%02d) %s\n", num + 1, buf);
      num++;
    }
    
    if (*buf == '\0' || num == 99)
      aborted = 1;
    
    if (num % 34 == 0)
    {
      page++;
      clear();
      outs("請依序輸入選項, 按 ENTER 完成設定\n");
    }
  }
  fclose(fp);

  if (num == 0)
  {
    clear();
    unlink(fpath);
    *(fpath + len) = 0;
    unlink(fpath);
    prints( "取消此次投票\n" );
    pressanykey(NULL);
    return RC_FULL;
  }
  else
  {
    boardheader bh;

    if (!(vfh.filemode & VOTE_SCORE))
    {
      while(1)
      {
        move(t_lines - 3, 0);
        sprintf(buf, "請問可以投幾票 (1-%1d): ",num);
        getdata(t_lines - 3, 0, buf , buf, 3, DOECHO, 0);
       
        if (atoi(buf) <= 0 || atoi(buf) > num)
          continue;
        else
        {
          vfh.savemode = atoi(buf);
          break;
        }
      }
    }
    
    setbfile(fpath, currboard, FN_VOTE);
    if (rec_add(fpath, &vfh, FHSZ) == -1)
    {
      outs("系統發生錯誤! 投票取消!");
      setbfile(fpath, currboard, vfh.filename);
      unlink(fpath);
      strcpy(fpath + len, STR_bv_control);
      unlink(fpath);
      pressanykey(NULL);
      return RC_FULL;
    }

    num = rec_search(fn_board, &bh, sizeof(bh), cmpbnames, (int) currboard);
    if (bh.bvote == 1 && bh.vtime < closetime); /* voting & closetime is earlier */
    else
    {
      bh.bvote = 1;
      bh.vtime = closetime;
      if (substitute_record(fn_board, &bh, sizeof(bh), num, 0) == -1)
        outs(err_board_update);
      touch_boards();           /* vote flag changed */
    }
    move(t_lines - 2, 0);
    outs("開始投票了！");
  }
  pressanykey(NULL);
  return RC_FULL;

}

struct one_key vote_comms[] = {
  'r',         do_vote, 0, "投票",0,
  Ctrl('P'), make_vote, 0, "舉辦投票",0,
  'M',   maintain_vote, 0, "投票管理",0,
  '\0', NULL, 0, NULL,0};


void votedoent(int num, fileheader *ent, int row, char *bar_color)
{
  char closeday[9];
  struct tm *ptime;
  time_t dtime;

  dtime = atol(ent->filename + 2);
  ptime = localtime(&dtime);
  sprintf(closeday, "%02d/%02d/%02d",
          ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_year % 100);

  move(row, 0);
  clrtoeol();
  prints("%4d %c %-5s%-4.3s %-11s %s[%s] [m%s %s \033[m", 
    num, vote_flag(ent, NULL) ? ' ':'+', ent->date, ent->title + 65, closeday,
    (ent->filemode & VOTE_PRIVATE) ? "[1;31m" : "[1;36m",
    (ent->filemode & VOTE_PRIVATE) ? "私人" : "一般", 
    bar_color ? bar_color : "", ent->title);
}


static void
votetitle()
{
  showtitle("投票所", BoardName);
  outs("\
[←]離開 [→/ENTER]開始投票 板主專用鍵: [^P]舉辦一次投票 [M]取消投票/提早開票\n\
" COLOR1 "\033[1m編號   開始日期   結束日期   投  票  主  題                                   \033[0m");
}


int
ShowVote(bname)
  char *bname;
{
  char buf[STRLEN], currboard0[15];

  if (currmode & MODE_DIGEST)
    return RC_NONE;

  strcpy(currboard0, currboard);
  strcpy(currboard, bname);
  set_board();		/* alan.000415: 再check一次MODE_BOARD */
  setbfile(buf, currboard, FN_VOTE);
  getkeep(buf, 1, 1);

  i_read(VOTING, buf, votetitle, votedoent, vote_comms, NULL);

  strcpy(currboard, currboard0);
  return RC_FULL;
}


int
b_vote()
{
  return ShowVote(currboard);
}


int
b_results()
{
  return vote_results(currboard);
}

/* 投票中心 chyiuan */
int
all_vote()
{
  register int i;
  register boardheader *bhdr;
  int all,pass,ch,tmp;
  int boards;
  int redraw=1;
  int pageboard=15,pagenum=0;
  char buf[256];
//  fileheader vch;
  struct vdata
  {
    char pointer[5];
    char bname[20];
    char btitle[50];
    int num;
  } vdata[MAXBOARD];

  all = 0;
  pass = 1;
  resolve_boards();

  for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
    if (bhdr->bvote == 1 && Ben_Perm(bhdr))
    {
      strcpy(vdata[all].bname, bhdr->brdname);
      strcpy(vdata[all].btitle, bhdr->title);
      sprintf(buf, BBSHOME"/boards/%s/%s", bhdr->brdname, ".VCH");
      vdata[all].num = rec_num(buf, FHSZ);
      strcpy(vdata[all].pointer,"　");

/*
      int vnum = 0;
      
      sprintf(buf, BBSHOME"/boards/%s/%s", bhdr->brdname, ".VCH");
      for(vnum = rec_num(buf,FHSZ); vnum > 0 ; vnum--)
      {
        rec_get(buf, &vch , FHSZ , vnum);
        strcpy(vdata[all].bname, bhdr->brdname);
        strcpy(vdata[all].vtitle, vch.title);
        strcpy(vdata[all].pointer,"　");
        if(vch.filemode & VOTE_PRIVATE)
        {
          setbfile(buf, vdata[all].bname, FN_CANVOTE);
          if(!belong(buf, cuser.userid))
            strcpy(vdata[all].mode,"[1;33m[私][m不受邀");
          else
            strcpy(vdata[all].mode,"[1;33m[私][m受邀中");
        }
        else
          strcpy(vdata[all].mode,"[1;33m[公][m沒限制");
        all++;          
      }
*/
      all++;
    }

    if (!all)
    {
      pressanykey("目前站內並沒有任何投票...");
      return 0;
    }

    pass=0;
    pagenum=0;
    strcpy(vdata[pass].pointer,"●");
    do
    {
      if(redraw)
      {
        showtitle("聯合投票中心", BoardName);

        show_file(BBSHOME"/etc/votetitle",1,7,ONLY_COLOR);
        redraw=0;
      }
      pagenum=pass/pageboard;
      if(pagenum<(all/pageboard))
        boards=pageboard;
      else
        boards=all-(all/pageboard)*pageboard;
      clrchyiuan(8,9+pageboard);
      for (i = 0; i <boards; i++)
      {
        move(8+i,0);
        prints("[0m%s [37m%2d.  %-14s   %-44.44s %d[0m ",
        vdata[i+pagenum*pageboard].pointer,
        i+pagenum*pageboard + 1,
        vdata[i+pagenum*pageboard].bname,
        vdata[i+pagenum*pageboard].btitle,
        vdata[i+pagenum*pageboard].num);
//        vote_flag(vdata[i+pagenum*pageboard].bname,'\0')?"[1;36m已投[0m":"[1;31m未投[0m");
      }
      move(b_lines,0);
      prints(COLOR1
"[1m 投票選單 [33m(↑/↓)(p/n)[37m往上/下 [33m(PgUp/PgDn)(P/N)[37m上/下頁 [33m(→)(r)[37m投票 [33m(←)(q)[37m離開  [m");
      move(b_lines,79);
      ch=igetkey();
      strcpy(vdata[pass].pointer,"　");
      switch(ch)
      {
        case 'e':
        case KEY_LEFT:
          ch='q';
        case 'q':
          break;

        case KEY_PGUP:
        case 'P':
        case 'b':
          if(pass==0)
            pass=all-1;
          else
            pass-=pageboard;
          if(pass<0)
            pass=0;
          break;
       case KEY_PGDN:
       case ' ':
       case 'N':
       case Ctrl('F'):
         if(pass==all-1)
           pass=0;
         else
           pass+=pageboard;
         if(pass>=all-1)
           pass=all-1;
         break;
       case KEY_DOWN:
       case 'n':
       case 'j':
         if(pass++ >=all-1)
           pass=0;
         break;

       case KEY_UP:
       case 'p':
       case 'k':
         pass--;
         if(pass<0)
           pass=all-1;
         break;

       case '0':
       case KEY_HOME:
         pass= 0;
         break;
       case '$':
       case KEY_END:
         pass=all-1;
         break;

       case '1':
       case '2':
       case '3':
       case '4':
       case '5':
       case '6':
       case '7':
       case '8':
       case '9':
         if ((tmp = search_num(ch,all-1)) >= 0)
           pass = tmp;
         break;
       case KEY_RIGHT:
       case '\n':
       case '\r':
       case 'r':
         if (pass <= all-1 && pass >= 0)
         {
           strcpy(buf, currboard);
           strcpy(currboard, vdata[pass].bname);
           b_vote();
           strcpy(currboard, buf);
           redraw=1;
         }
         break;
       }
     strcpy(vdata[pass].pointer,"●");
   }while(ch!='q' && ch!='Q');
  return 0;
}

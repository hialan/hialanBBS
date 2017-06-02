/*-------------------------------------------------------*/
/* util/rmtrash.c                                        */
/*-------------------------------------------------------*/
/* target : 清除個人目錄中的垃圾檔案                     */
/* create : 01/09/25                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/

#define RM_ALLPOST  //刪除 allpost/ 目錄 by hialan

#if 0

  [目的] 清掉使用者目錄下的垃圾檔案

  [說明] 程式去刪除 談天記錄 talklog talk_xxxx
                    聊天記錄 chat_xxxx
                    舊版好友名單 overrides
                    賭盤記錄 bet_xxxx
                    沒有在 .DIR 中的信件
                    size = 0 的檔案

  [方法] 把這程式放在 src/util/rmtrash.c 下，並加入 src/util/Makefile 編譯
         執行 rmtrash SYSOP 可清除 SYSOP 目錄的垃圾檔案
         執行 rmtrash       可清除所有使用者目錄的垃圾檔案

  [備註] 通常要跑很久，如果垃圾檔案很多或使用者人數很多時，並請先備份 home
         可以改 find_dir() 來定義哪些檔案是垃圾
         可以不關站跑沒關係

#endif


#include "bbs.h"

static int
_rec_get(fpath, data, size, pos)
  char *fpath;
  void *data;
  int size, pos;
{
  int fd;
  int ret;

  ret = -1;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    if (lseek(fd, (off_t) (size * pos), SEEK_SET) >= 0)
    {
      if (read(fd, data, size) == size)
       ret = 0;
    }
    close(fd);
  }
  return ret;
}


static int                      /* 1: 在 .DIR 中  0: 不在 .DIR 中的垃圾信件 */
find_dir(userid, filename)
  char *userid;
  char *filename;
{
  char fpath[64];
  int pos;
  fileheader hdr;

  if (filename[0] == 't' ||                             /* talk_xxxx talklog */
    (filename[0] == 'c' && filename[2] == 'a') ||       /* chat_xxxx */
    (filename[0] == 'b' && filename[1] == 'e') ||       /* betxxx */
    filename[0] == 'o')                                 /* overrides */
  {
    return 0;                   /* 垃圾直接刪除 */
  }

  if (filename[0] != 'M')       /* 不是信件不檢查 */
    return 1;

  sprintf(fpath, BBSHOME "/home/%s/.DIR", userid);

  pos = 0;
  while (!_rec_get(fpath, &hdr, sizeof(fileheader), pos))
  {
    if (!strcmp(hdr.filename, filename))
      return 1;

    pos++;
  }

  return 0;
}


static void
reaper(userid, pos, fp)
  char *userid;
  int pos;
  FILE *fp;
{
  char fpath[64];
  int num;
  struct stat st;
  struct dirent *de;
  DIR *dirp;

  sprintf(fpath, BBSHOME "/home/%s", userid);

  if (!(dirp = opendir(fpath)))
  {
    printf("No such ID: %s\n", userid);
    return;
  }

  chdir(fpath);

  num = 0;

  while (de = readdir(dirp))
  {
    if (de->d_name[0] <= ' ' || de->d_name[0] == '.')
      continue;

    if ((!stat(de->d_name, &st) && (st.st_size == 0)) ||
      !find_dir(userid, de->d_name)) /* size == 0 或 不在 .DIR 中 */
    {
      printf("%s/%s is deleted.\n", fpath, de->d_name);
      unlink(de->d_name);
      num++;
    }
  }

#ifdef RM_ALLPOST
  strcat(fpath, "/allpost");
  if (dirp = opendir(fpath))
  {
    char cmd[80];
    
    printf("Size of %s:\n", fpath);
    system("ls -l | grep \"allpost\"");    
    printf("\n");
    sprintf(cmd, "rm -rf %s", fpath);
    system(cmd);
    
    printf("[ID %d] %s %s deleted.\n", pos, userid, fpath);
  }
#endif

  printf("[ID %d] %s  -- Total deleted : %d files\n", pos, userid, num);
  if(fp)
    fprintf(fp, "[ID %d] %s  -- Total deleted : %d files\n", pos, userid, num);
}


int
main(argc, argv)
  int argc;
  char **argv;
{
  int pos;
  userec cuser;

  if (argc > 2)
  {
    printf("Usage: %s [ID]\n", argv[0]);
  }
  else if (argc == 2)
  {
    reaper(argv[1], 0, 0);
  }
  else
  {
    /* 如果跑到一半暫停了或是中斷了，就看上次跑到哪個數字
       然後只要改程式 pos = xxx 重新編譯即可 */

    FILE *fp;	//log 檔案 by hialan
    fp = fopen(BBSHOME"/log/rmtrash.log", "w");
    
    pos = 0;        /* 供中途中斷使用 */
    while (!_rec_get(BBSHOME "/" FN_PASSWD, &cuser, sizeof(userec), pos))
    {
      reaper(cuser.userid, pos, fp);
      pos++;
    }
    printf("Total User: %d\n", pos);    /* 供中途中斷使用 */
    fprintf(fp, "Total User: %d\n", pos);
    fclose(fp);
  }
  exit(0);
}

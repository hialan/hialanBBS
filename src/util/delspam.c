/*
        想必大家今天都看到 [config] 版上的風暴了.
        交大資工站一天被灌了 5000 篇的廣告.
        小小的成大土木站也沒有倖免於難.最可惡的是連我都被灌到 :(

        看到 mfchen 大大放出來一個程式專砍廣告.
        可惜 Maple & Sob 的 .DIR 規格不合. file.owner 只有 13 characters.
        所以就改抓信件的第一行來判斷發信人

        寫了一個程式給大家玩玩. 這個是 BETA 版.
        如果有要更新我會在成大土木站 [sob] 精華區貼出來.
        Spam ? 肉品罐頭？ 我也不知道為什麼..那程式就叫 delspam 好了

        另外補充一下.我使用 /etc/sendmail.baduser
        來當 sendmail-8.8.x 擋 bad user 的設定檔.

        用法 delspam <boards|home> [spam mailer1] [spam mailer2]...

        用法：  delspam home
                找出 ~bbs/home 裡面發信人列在 /etc/sendmail.baduser 裡的

                delspam home journey@ms2.hinet.net franksha@ms4.hinet.net
                找出 ~bbs/home 裡面這兩個傢伙灌進來的廣告
                (光這兩個就在我這裡被灌了上百封)

                delspam boards dozer@netwizards.net
                找出 ~bbs/boards 下 dozer@netwizards.net 的信件
                (這個人今天在全球 news groups 上徵炮友)

        找出來以後看你是定義 define DELETE 還是 undef DELETE
        define  DELETE  的.會把信件砍掉.
        undef   DELETE  的.會把信件移到 ~bbs/tmp 下.

        要把 ~bbs/tmp 下的廣告信灌回去的請自己斟酌.後果自己負責.
        (嗯？嘿嘿...)

        以下程式碼開始
*/
/*-------------------------------------------------------*/
/* util/delspam.c       ( NTHU CS MapleBBS Ver 2.36.sob )*/
/*-------------------------------------------------------*/
/* target : 砍掉 Spam mailer 的垃圾信件 .DIR 重建        */
/* create : 96/01/10 <= 我的 20 歲生日前兩天             */
/* update : 隨時更新                                     */
/*-------------------------------------------------------*/
/* author : leeym@bbs.civil.ncku.edu.tw                  */
/*-------------------------------------------------------*/
/* syntex : delspam <board|home> [mailer1] [mailer2]..   */
/*-------------------------------------------------------*/

#define ERROR           /* 錯誤警告 */
#undef  DEBUG           /* 除錯模式 */
#undef  DELETE          /* 砍掉廣告信 */

        /* undef DELETE 則為搬移廣告信到 ~bbs/tmp , 以便嘿嘿.. */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "bbs.h"

#define SENDMAIL_BADUSER        "/etc/sendmail.baduser"
#define BUFSIZE                 256

#ifndef MAXPATHLEN
#define MAXPATHLEN      1024
#endif

int alphasort();
int summary = 0;
int visit = 0;
int junkmail = 0;
char *currfilename = "\0";

int
getindex(fpath,fname,size)
char *fpath;
char *fname;
int size;
{
    int fd, now=0;
    fileheader fhdr;

    if ((fd = open(fpath, O_RDONLY, 0)) != -1)
    {
      while((read(fd, &fhdr, size) == size)){
        now++;
        if(!strcmp(fhdr.filename,fname)){
          close(fd);
          return now;
        }
      }
      close(fd);
    }
    return 0;
}

int
delete_file(dirname, size, ent, filecheck)
  char *dirname;
  int size, ent;
  int (*filecheck) ();
{
  char abuf[BUFSIZE];
  int fd;
  struct stat st;
  long numents;

  if ((fd = open(dirname, O_RDWR)) == -1)
    return -1;
  flock(fd, LOCK_EX);
  fstat(fd, &st);
  numents = ((long) st.st_size) / size;
  if (((long) st.st_size) % size)
    fprintf(stderr, "align err\n");
  if (lseek(fd, size * (ent - 1), SEEK_SET) != -1)
  {
    if (read(fd, abuf, size) == size)
      if ((*filecheck) (abuf))
      {
        int i;

        for (i = ent; i < numents; i++)
        {
          if (lseek(fd, i * size, SEEK_SET) == -1)
            break;
          if (read(fd, abuf, size) != size)
            break;
          if (lseek(fd, (i - 1) * size, SEEK_SET) == -1)
            break;
          if (write(fd, abuf, size) != size)
            break;
        }
        ftruncate(fd, (off_t) size * (numents - 1));
        flock(fd, LOCK_UN);
        close(fd);
        return 0;
      }
  }
  lseek(fd, 0, SEEK_SET);
  ent = 1;
  while (read(fd, abuf, size) == size)
  {
    if ((*filecheck) (abuf))
    {
      int i;

      for (i = ent; i < numents; i++)
      {
        if (lseek(fd, (i + 1) * size, SEEK_SET) == -1)
          break;
        if (read(fd, abuf, size) != size)
          break;
        if (lseek(fd, i * size, SEEK_SET) == -1)
          break;
        if (write(fd, abuf, size) != size)
          break;
      }
      ftruncate(fd, (off_t) size * (numents - 1));
      flock(fd, LOCK_UN);
      close(fd);
      return 0;
    }
    ent++;
  }
  flock(fd, LOCK_UN);
  close(fd);
  return -2;
}


int
belong(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[STRLEN], *ptr;

    while (fgets(buf, STRLEN, fp))
    {
      if ((ptr = strtok(buf, " \t\n\r")) && !strcasecmp(ptr, key))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}

int
cmpfilename(fhdr)
  fileheader *fhdr;
{
  return (!strcmp(fhdr->filename, currfilename));
}

int
dashd(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}

int
dirselect(dir)
  struct direct *dir;
{
  register char *name = dir->d_name;
  return (strchr("MDSGH", *name) && name[1] == '.');
}

main(argc, argv)
  int argc;
  char *argv[];
{
  DIR *dirp;
  struct dirent *de;
  char *fname;
  time_t start, end;

  if (argc == 1)
   {
  printf("usage: %s <boards|home> [spam mailer1] [spam mailer2]..\n",argv[0]);
  printf("       if [spam mailer] exist, find out it.\n");
  printf("       else find spam mailer in sendmail setting file\n");
  exit(-1);
   }

  setuid(BBSUID);
  setgid(BBSGID);
  chdir(BBSHOME);
  chdir(argv[1]);

  /* visit the first hierarchy */

  if (!(dirp = opendir(".")))
  {
    printf("unable to enter %s\n",argv[1]);
    exit(-1);
  }

#ifdef  DEBUG
  printf("now entering %s\n",argv[1]);
#endif

  time(&start);

  while (de = readdir(dirp))
  {
    fname = de->d_name;
    if (fname[0] > ' ' && fname[0] != '.' && dashd(fname))
    {
     int fdir, i, j, bonus =0;
     struct stat st;
     char genbuf[512], path[256], *name, arthor[80], dirname[80];
     FILE *fp;
     int total, count;
     fileheader fhdr;
     struct direct **dirlist;
     time_t filetime;
     struct tm *ptime;

     sprintf(path, "%s/.DIR", fname);
     sprintf(dirname,BBSHOME"/%s/%s",argv[1],path);

     if (fdir = open(path, O_RDWR) == -1)
     {
#ifdef  ERROR
       printf("%s open error\n",path);
#endif
       continue;
     }


     total = scandir(fname, &dirlist, dirselect, alphasort);
     fname = strrchr(path, '.');

#ifdef  DEBUG
        printf("path: %s\n",path);
        printf("fname: %s\n",fname);
#endif

      for (count = 0; count < total; count++)
      {
       int junk = 0;
       name = dirlist[count]->d_name;
       strcpy(fname, name);
       currfilename = strrchr(path,'M');
       if (!stat(path, &st))
       {
         if (st.st_size && (fp = fopen(path, "r")))
         {
           fgets(genbuf, 256, fp);

           if (!strncmp(genbuf, "作者: ", 6) ||
             !strncmp(genbuf, "發信人: ", 8))
           {
             bzero(&fhdr, sizeof(fhdr));
             i = 5;
             while (genbuf[i] != ' ')
               i++;

             while (genbuf[i] == ' ')
               i++;

             j = i + 1;
             while (genbuf[j] != ' ' && genbuf[j] != '\n' &&
                   genbuf[j] != '\r' && genbuf[j] != '\t')
               j++;

           /* ban bad user e-mail */

             strncpy(arthor, &genbuf[i], j);
             arthor[j-i] = '\0';
             strcpy(fhdr.filename,name);

             j -= i;
             if(j > IDLEN +1)
              j = IDLEN+1;

             if (argc > 2) for(i= 2; i < argc; i++){
             if (!strcmp(arthor, argv[i]))
               {
#ifdef  DELETE
                 unlink(path);
#else
                 sprintf(genbuf,BBSHOME"/%s/%s",argv[1],path);
                 f_mv(genbuf,BBSHOME"/tmp");
#endif
#ifdef  DEBUG
                 printf("no:%d arthor:%s, %s deleted, file size:%d\n",
                  count, arthor, path, st.st_size);
#endif
              i=getindex(dirname,fhdr.filename,sizeof(fileheader));
              j=delete_file(dirname, sizeof(fileheader), i,cmpfilename);
#ifdef  ERROR
               if (j){
                printf("delete no:%d %s in %s error %d\n",
                        count, fhdr.filename, dirname,j);

                  printf("delete %s in %s error\n",name, dirname);
                  printf("fhdr.filename: %s, currfilename: %s\n",
                        fhdr.filename ,currfilename);
			break;
                }
#else
                 /* do nothing */ ;
#endif
                 bonus += st.st_size;
                 junkmail++;
                junk = 1;
               }
             }
             if (belong(SENDMAIL_BADUSER, arthor) && argc == 2)
               {
#ifdef  DELETE
                 unlink(path);
#else
                 sprintf(genbuf,BBSHOME"/%s/%s",argv[1],path);
                 f_mv(genbuf,BBSHOME"/tmp");
#endif
#ifdef  DEBUG
                 printf("no:%d arthor:%s, %s deleted, file size:%d\n",
                  count+1, arthor, path, st.st_size);
#endif
              i=getindex(dirname,fhdr.filename,sizeof(fileheader));
              j=delete_file(dirname, sizeof(fileheader), i,cmpfilename);
#ifdef  ERROR
               if (j){
                printf("delete no:%d %s in %s error %d\n",
                        count,fhdr.filename, dirname,j);

                  printf("delete %s in %s error\n",name, dirname);
                  printf("fhdr.filename: %s, currfilename: %s\n",
                        fhdr.filename ,currfilename);
			break;
                }
#else
                /* do nothing */ ;
#endif
                 bonus += st.st_size;
                junk = 1;
                junkmail++;
               }
#ifdef  DEBUG
             if(!junk)
                 printf("no:%d arthor:%s, %s kept\n",
                  count+1, arthor, path,st.st_size);
#endif
           }
         fclose(fp);
         }
        if (!st.st_size)
         {
#ifdef  DEBUG
          printf("%s size 0, deleted\n",path);
#endif
          unlink(path);
         }
       }
      }
     close(fdir);
     summary += bonus;
     visit++;
    }
  }
  closedir(dirp);

  time(&end);

  printf("\n# start: %s", ctime(&start));
  printf("# end  : %s", ctime(&end));
  end -= start;
  start = end % 60;
  end /= 60;
  printf("# time : %d:%d:%d\n", end / 60, end % 60, start);
  printf("# Visit:   %10d\n", visit);
  printf("# Number:  %10d\n", junkmail);
  printf("# Summary: %10d\n", summary);
  exit(0);
}

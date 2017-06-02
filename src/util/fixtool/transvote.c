/*-------------------------------------------------------*/
/* util/transvote.c          ( Wind & Dust Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : 新舊投票轉換                                 */
/* create : 99/11/24                                     */
/* update : 99/11/24                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

static char STR_bv_control[] = ".control";	/* 投票選項 */
static char STR_bv_ballots[] = ".ballots";	/* 投過的票 */

void
tran(bname)
  char *bname;
{
  FILE *fp1, *fp2;	/* 要轉兩個檔: .control 及 .ballots */
  int fd, fd1, fd2;
  char *fpath, fpath2[256], buf[256];
  fileheader vfh;
  int len;

  printf("transfering %s\n", bname);
  sprintf(buf, "%s/%s", bname, FN_VOTE);
  fpath = buf;
  len = strlen(bname) + 1;

  if ((fd = open(buf, O_RDONLY)) == -1)
    return;

  while (read(fd, &vfh, sizeof(fileheader)) == sizeof(fileheader))
  {
    int len2 = strlen(vfh.filename);

    strcpy(fpath + len, vfh.filename);
    strcat(fpath, STR_bv_control);
    sprintf(fpath2, "%s.new", fpath);
#if 0
    if ((fp1 = fopen(fpath, "r")) == NULL)
      return;
      
    if ((fp2 = fopen(fpath2, "w")) == NULL)
      return;        

    {
      char genbuf[256];
      int num = 1;

      while(fgets(genbuf, sizeof(genbuf), fp1))
      {
        fprintf(fp2, "%02d) %s", num, genbuf + 3);
        num++;
      }
      fclose(fp2);
      fclose(fp1);
      unlink(fpath);
      f_mv(fpath2, fpath);
    }
#endif
    
    strcpy(fpath + len + len2, STR_bv_ballots);
    sprintf(fpath2, "%s.new", fpath);

    if ((fd1 = open(fpath, O_RDONLY)) && 
        (fd2 = open(fpath2, O_WRONLY | O_CREAT | O_APPEND, 0600)))
    {
      char temp[10], inchar[2];

#if 0
      while (read(fd1, &inchar, 1) == 1)
      {
        sprintf(genbuf, "%02d", (int) (inchar - 'A'));
        write(fd2, genbuf, 2);
      }
#endif
      while (read(fd1, &inchar, 2) == 2)
      {
        printf("%s\n", inchar);
        printf("%02d\n", atoi(inchar) + 1);
        memset(temp, 0, sizeof(temp));
        sprintf(temp, "%02d", atoi(inchar) + 1);
        write(fd2, temp, 2);
      }
      close(fd1);
      close(fd2);
      unlink(fpath);
      f_mv(fpath2, fpath);
    }
  }
  close(fd);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  DIR *dirp;
  struct dirent *de;
  char *fname, fpath[80], brdname[20];
  int cc, count;

  count = 0;

  sprintf(fpath, "%s/boards", BBSHOME);
  dirp = opendir(fpath);

//  while (de = readdir(dirp))
  {
//    fname = de->d_name;                             
//    if (*fname != '.')
    {
//      sprintf(brdname, "%s/%s", fpath, fname);
      sprintf(brdname, "%s/SYSOP", fpath);
      tran(brdname);
      count++;
    }
    /* free(de); */
  }

  closedir(dirp);
  fflush(stdout);

  printf("## %d boards transformed\n", count);
  exit(0);
}
                     
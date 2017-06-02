#include "../include/bbs.h"
#include "record.c"

#define PAL	0x01
#define BAD	0x02
#define ALOHA	0x04

#define VISABLE	0x01
#define WATER	0x02
#define CANVOTE	0x04 

#define FN_REJECT       "reject"

dashf(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}


void
trans(userid)
  char *userid;
{
  FILE *fp;
  fileheader pal;
  char genbuf[200], fpath[80], *ptr, *desc;
  char *str_space         = " \t\n\r";

  sethomefile(fpath, userid, FN_PAL);
  if (dashf(fpath))
   unlink(fpath);

  sethomefile(genbuf, userid, FN_OVERRIDES);
  if (fp = fopen(genbuf, "r"))
  {
    while (fgets(genbuf, STRLEN, fp))
    {
      desc = genbuf + 13;
      if (ptr = strchr(desc, '\n'))
        ptr[0] = '\0';
      if (desc)                   
        strcpy(pal.title, desc);

      if (strtok(genbuf, str_space))
        strncpy(pal.filename, genbuf, IDLEN);
      pal.filemode = PAL;
      rec_add(fpath, &pal, sizeof(fileheader));
    }
    fclose(fp);
  }

  sethomefile(genbuf, userid, FN_REJECT);
  if (fp = fopen(genbuf, "r"))
  {
    while (fgets(genbuf, STRLEN, fp))
    {
      desc = genbuf + 13;
      if (ptr = strchr(desc, '\n'))
        ptr[0] = '\0';
      if (desc)                   
        strcpy(pal.title, desc);
      if (strtok(genbuf, str_space))
        strncpy(pal.filename, genbuf, IDLEN);
      pal.filemode = BAD;
      rec_add(fpath, &pal, sizeof(fileheader));
    }
    fclose(fp);
  }
}


void
main()
{
  DIR *dirp;
  struct dirent *de;
  char *fname, fpath[80], userid[20];
  
  sprintf(fpath, "%s/home/wildcat", BBSHOME);
  dirp = opendir(fpath);
  while (de = readdir(dirp))
  {
    fname = de->d_name;
    if (*fname != '.')
      trans(fname);
    printf("%s\n", fname);
  }
  closedir(dirp);
}
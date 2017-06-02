/*-------------------------------------------------------*/
/* util/clrdir.c	( NTHU CS MapleBBS Ver 2.36 )	 */
/*-------------------------------------------------------*/
/* target : Maple/Phoenix/Secret_Lover .DIR ²M²z¡BºûÅ@	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/
/* syntax : clrdir <boards | home>		 	 */
/*-------------------------------------------------------*/


#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include "config.h"
#include "struct.h"

void
clear_dir(dir)
  char *dir;
{
  char buf[24] = "M.";

  DIR *dirp;
  struct dirent *dp;

  int fin, fout;
  struct stat st;

  char table[5000][20];
  int i, total;
  int recsize;
  char *key, *hit;
  struct fileheader fh;


  printf("\n%s\n", dir);
  chdir(dir);

  /* build filename table & sort */

  dirp = opendir(".");
  for (dp = readdir(dirp), i = 0; dp; dp = readdir(dirp))
    if (!strncmp(dp->d_name, "M.", 2))
    {
      strcpy(table[i], dp->d_name + 2);
      table[i][19] = 0;
      i++;
    }
  closedir(dirp);
  total = i;

  qsort(table, total, 20, strcmp);

  recsize = sizeof(struct fileheader);
  key = &(fh.filename[2]);

  if ((fin = open(".DIR", O_RDONLY)) == -1)
  {
    printf("\t.DIR open error\n");
    return;
  }

  if ((fout = open(".dir", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
  {
    printf("\t.dir open error\n");
    return;
  }

  fstat(fin, &st);
  printf("\tTotal: %d records, %d files\n\tprune: ",
    st.st_size / recsize, total);

  /* read original .DIR */

  i = 0;

  while (read(fin, (char *) &fh, recsize) == recsize)
  {
    /* binary search : check consistency */

    if (hit = (char *) bsearch(key, table, total, 20, strcmp))
    {
#if 0
      stat(fh.filename, &st);
      if (st.st_size)
#endif
      {
	hit[19] = 'z';
	write(fout, &fh, recsize);
	continue;
      }
    }
    i++;
  }
  close(fin);
  close(fout);

  printf("%d records, ", i);

  /* prune dummy file */

  for (i = recsize = 0; recsize < total; recsize++)
  {
    if (table[recsize][19] != 'z')
    {
      strcpy(buf + 2, table[recsize]);
      unlink(buf);
      i++;
    }
  }
  printf("%d files\n", i);

  f_mv(".DIR", ".DIR.old");
  f_mv(".dir", ".DIR");
}


main(argc, argv)
  int argc;
  char *argv[];
{
  struct dirent *de;
  DIR *dirp;
  char buf[80], *ptr;
  char t_dir[80];

  if (argc != 2)
  {
    printf("Usage:\t%s <boards/home>\n", argv[0]);
    exit(-1);
  }

  sprintf(buf, BBSHOME "/%s", argv[1]);
  if ((dirp = opendir(buf)) == NULL)
  {
    printf(":Err: unable to open %s\n", buf);
    return;
  }

  ptr = buf + strlen(buf);
  *ptr++ = '/';
  getcwd(t_dir, 29);

  while (de = readdir(dirp))
  {
    if (de->d_name[0] > ' ' && de->d_name[0] != '.')
    {
      strcpy(ptr, de->d_name);
      clear_dir(buf);
    }
  }
  closedir(dirp);
  chdir(t_dir);
}

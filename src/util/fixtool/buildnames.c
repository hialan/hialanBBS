/*-------------------------------------------------------*/
/* util/buildir.c	( NTHU CS MapleBBS Ver 2.36 )	 */
/*-------------------------------------------------------*/
/* target : Maple/Phoenix/Secret_Lover .Names 重建	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "config.h"
#include "struct.h"


int alphasort();


int
dirselect(dir)
  struct direct *dir;
{
  register char *name = dir->d_name;
  return (name[0] == 'G' && name[1] == '.');
}


main(argc, argv)
  int argc;
  char **argv;
{
  int fdir, i, j, wrong;
  struct stat st;
  char genbuf[512], path[256], *ptr, *name;
  FILE *fp;
  int total, count;
  fileheader fhdr;
  struct direct **dirlist;
  time_t filetime;
  struct tm *ptime;

  if (argc != 2)
  {
    printf("Usage:\t%s <path>\n", argv[0]);
    exit(-1);
  }

  ptr = argv[1];
  sprintf(path, "%s/.Names", ptr);
  fdir = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fdir == -1)
  {
    printf(".DIR create error\n");
    exit(-1);
  }

  total = scandir(ptr, &dirlist, dirselect, alphasort);
  ptr = strrchr(path, '.');

  for (count = 0; count < total; count++)
  {
    name = dirlist[count]->d_name;
    strcpy(ptr, name);
    wrong = 0;
    if (!stat(path, &st))
    {
      wrong = 1;

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
	  while (genbuf[j] != ' ')
	    j++;

	  strcpy(fhdr.filename, name);
	  filetime = atoi(name + 2);
	  j -= i;
	  if(j > IDLEN +1)
	   j = IDLEN+1;
	  strncpy(fhdr.owner, &genbuf[i], j);
	  strtok(fhdr.owner, " .@\t\n\r");
	  if (strtok(NULL, " .@\t\n\r"))
	    strcat(fhdr.owner, ".");

	  while (fgets(genbuf, 256, fp))
	  {
	    if (!strncmp(genbuf, "標題: ", 6) ||
	      !strncmp(genbuf, "標  題: ", 8))
	    {
	      i = 5;
	      while (genbuf[i] != ' ')
		i++;

	      while (genbuf[i] == ' ')
		i++;

	      if (name = strchr(genbuf + i, '\n'))
		name[0] = '\0';
	      strncpy(fhdr.title, genbuf + i, TTLEN);

	      if (filetime > 740000000)
	      {
		ptime = localtime(&filetime);
		sprintf(fhdr.date, "%2d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
	      }
	      else
	      {
		strcpy(fhdr.date, "     ");
	      }

	      write(fdir, &fhdr, sizeof(fhdr));
	      wrong = 0;
	      break;
	    }
	  }
	}
	fclose(fp);
      }
    }

    if (wrong)
    {
      printf("rm %s", path);
      /* unlink(path); */
    }
  }
  close(fdir);
}

/*-------------------------------------------------------*/
/* util/buildir.c       ( NTHU CS MapleBBS Ver 2.36.sob )*/
/*-------------------------------------------------------*/
/* target : Maple/Phoenix/Secret_Lover .DIR ­««Ø         */
/* create : 95/03/29                                     */
/* update : 96/12/19                                     */
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

#ifndef MAXPATHLEN
#define MAXPATHLEN      1024
#endif

int alphasort();

int
dashl(fname)
  char *fname;
{
  struct stat st;

  return (lstat(fname, &st) == 0 && S_ISLNK(st.st_mode));
}


dashf(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}


int
dashd(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}

char*
mono(char* s)
{
   int i, j;
   static char ans[200];

   for (i = j = 0; s[i] && j < 199; i++)
      if (s[i] == 27)
         while (s[++i] && s[i] != 'm')
            ;
      else
         ans[j++] = s[i];
   ans[j] = 0;

   return ans;
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
  char **argv;
{
  int fdir, i, j, wrong;
  struct stat st;
  char genbuf[512], path[256], *ptr, *name, fname[MAXPATHLEN];
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
  sprintf(path, "%s/.DIR", ptr);
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
    wrong = 1;
    if (!stat(path, &st))
    {
      if (st.st_size && (fp = fopen(path, "r")))
      {
        fgets(genbuf, 256, fp);
        if (!strncmp(genbuf, "§@ªÌ: ", 6) ||
          !strncmp(genbuf, "µo«H¤H: ", 8))
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
            if (!strncmp(genbuf, "¼ÐÃD: ", 6) ||
              !strncmp(genbuf, "¼Ð  ÃD: ", 8))
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
                sprintf(fhdr.date, "%2d/%02d",
                        ptime->tm_mon + 1, ptime->tm_mday);
              }
              else
              {
                strcpy(fhdr.date, "     ");
              }
               fhdr.filemode = FILE_READ;
              if(!fhdr.title[0])
                strcpy(fhdr.title, name);
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
       bzero(&fhdr, sizeof(fhdr));

      if (st.st_size && (fp = fopen(path, "r")))
      {
        fgets(genbuf, 256, fp);
        if (!strncmp(genbuf, "¡ó Åwªï¥úÁ{", 11))
        {
        strcpy(fhdr.title, "·|Ä³[1;33m°O¿ý[m");
        }

          while (fgets(genbuf, 256, fp))
          {
            if (!strncmp(genbuf, "[1;35m", 7))
            {
            strtok(genbuf, " :");
            if (strtok(NULL, " :"))
              sprintf(fhdr.title, "¹ï¸Ü°O¿ý [1;36m(%s)[m",mono(genbuf));
            }
            if (!strncmp(genbuf, "[1;33;46m¡¹", 12) ||
                !strncmp(genbuf, "To", 2))
              strcpy(fhdr.title, "¼ö½u[41m°O¿ý[m");
          }
        fclose(fp);
      }

       strcpy(fhdr.filename, name);
       if (filetime > 740000000)
        {
          ptime = localtime(&filetime);
          sprintf(fhdr.date, "%2d/%02d",
          ptime->tm_mon + 1, ptime->tm_mday);
        }
       else
        {
          strcpy(fhdr.date, "     ");
        }
       fhdr.filemode = FILE_READ;
       strcpy(fhdr.owner, "[³Æ.§Ñ.¿ý]");
       if(!fhdr.title[0])
       {
         if(strstr(name,"M")!=NULL)
            sprintf(fhdr.title, "¡º %s",name);
         else if(strstr(name,"D")!=NULL)
            sprintf(fhdr.title, "¡» %s",name);
         else if(strstr(name,"S")!=NULL)
           {
            sprintf(fname,"%s/%s",argv[1],name);
            if ((i = readlink(fname, fname, MAXPATHLEN-1)) >= 0)
             {
             fname[i] = '\0';
            if(dashf(fname))
             {
              if(!strcmp(strrchr(fname,'.'),".index"))
               sprintf(fhdr.title, "¡· %s",name);
              else
               sprintf(fhdr.title, "¡³ %s",name);
             }
            else if(dashd(fname))
             sprintf(fhdr.title, "¡´ %s",name);
            else if(dashl(fname))
             sprintf(fhdr.title, "¡H %s",name);
            else
             sprintf(fhdr.title, "¡° %s",name);
             }
           }
         else strcpy(fhdr.title, name);
       }
       if(!fhdr.filename[0])
         strcpy(fhdr.filename, name);
       if(!fhdr.owner[0])
         strcpy(fhdr.owner, "Recoverd");

       write(fdir, &fhdr, sizeof(fhdr));
       printf("%s recovered\n", path);
    }
  }
  close(fdir);
}

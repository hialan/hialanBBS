/*-------------------------------------------------------*/
/* util/mandex.c        ( NTHU CS MapleBBS Ver 2.39 )    */
/*-------------------------------------------------------*/
/* target : ºëµØ°Ï¯Á¤Þµ{¦¡ (man index)                   */
/* create : 95/03/29                                     */
/* update : 95/08/08                                     */
/*-------------------------------------------------------*/
/* syntax : mandex [board]                               */
/*          [board] ¦³­È ==> ¥u¶]¸Ó board                */
/*                  ªÅªº ==> ©Ò¦³ªº boards ³£¶]          */
/*-------------------------------------------------------*/


#include        "bbs.h"
#include        "record.c"
#include        "cache.c"
#ifndef MAXPATHLEN
#define MAXPATHLEN  1024
#endif


extern int numboards;
extern boardheader *bcache;

char color[4][10]={"[1;33m","[1;32m","[1;36m","[1;37m"};
char fn_index[] = ".index";
char fn_new[] = ".index.new";
char index_title[] = "¡· ºëµØ°Ï¥Ø¿ý¯Á¤Þ";
FILE *fndx;
int ndir;
int nfile;
int index_pos;
char topdir[128],pgem[512], pndx[512];

int nb=0; /* board ¼Æ*/

struct boardinfo
{
  char bname[40];
  int  ndir;
  int  nfile;
  int  k;
};
typedef struct boardinfo boardinfo;

boardinfo
board[MAXBOARD];

int k_cmp(b, a)
boardinfo *b,*a;
{
  return ((a->k/100 + a->ndir + a->nfile) - (b->k/100 + b->ndir + b->nfile));
}

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
dashd(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}


/* visit the hierarchy recursively */


void
mandex(level, num_header, fpath)
  int level;
  char *fpath, *num_header;
{
  FILE *fgem;
  char *fname, *ptr, buf[256];
  struct stat st;
  int count;
  fileheader fhdr;

  fgem = fopen(fpath, "r+");
  if (fgem == NULL)
    return;

  fname = strrchr(fpath, '.');

  if (!level)
  {
    strcpy(pgem, fpath);

    strcpy(fname, fn_new);
    fndx = fopen(fpath, "w");
    if (fndx == NULL)
    {
      fclose(fgem);
      return;
    }
    fprintf(fndx, "[1;32m§Ç¸¹\t\t\tºëµØ°Ï¥DÃD[m\n"
"[36m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[m\n");
    strcpy(pndx, fpath);
    ndir = nfile = 0;
    index_pos = -1;
  }

  count = 0;
  while (fread(&fhdr, sizeof(fhdr), 1, fgem) == 1) {
     if (fhdr.filemode & FILE_REFUSE)
       continue;
     strcpy(fname, fhdr.filename);
        if (!level && !strncmp(fhdr.title, index_title, strlen(index_title))
           && index_pos < 0) {
        index_pos = count;
        unlink(fpath);
     }
     st.st_size = 0;
     stat(fpath, &st);
/*
     sprintf(buf, "*%.*s%3d. %s (%.1fk)\n",
        4 * level, num_header, ++count, fhdr.title, st.st_size / 1024.);
*/
     sprintf(buf, "%.*s%s%3d. %s \n",

11 * level, num_header,color[level%4], ++count, fhdr.title); /* Ptt */
     fputs(buf, fndx);
     if (dashd(fpath)) {
        ++ndir;
        if (*fhdr.title != '#' && level < 10 )
        {
           strcat(fpath, "/.DIR");
           mandex(level + 1, buf , fpath);
        }
     }
     else
        ++nfile;
  }

  if (!level)
  {
    char lpath[MAXPATHLEN];

    fclose(fndx);
    strcpy(fname, fn_index);
    f_mv(pndx, fpath);
    strcpy(pndx, fpath);

    sprintf(buf, "%s.new", pgem);
    if (index_pos >= 0 || (fndx = fopen(buf, "w")))
    {
      fname[-1] = 0;
      stamplink(fpath, &fhdr);
      unlink(fpath);
      strcpy(fhdr.owner, "¨C¤Ñ¦Û°Ê§ó·s");
      sprintf(lpath, "%s/%s", topdir, pndx);
      st.st_size = 0;
      stat(lpath, &st);
      sprintf(fhdr.title, "%s (%.1fk)", index_title, st.st_size / 1024.);
      board[nb].k = st.st_size;  /* Ptt */
      printf("(%d)[%dK]",nb,board[nb].k);
      symlink(lpath, fpath);
      if (index_pos < 0) {
         fwrite(&fhdr, sizeof(fhdr), 1, fndx);
         rewind(fgem);
         while (fread(&fhdr, sizeof(fhdr), 1, fgem) == 1)
            fwrite(&fhdr, sizeof(fhdr), 1, fndx);
         fclose(fndx);
         fclose(fgem);
         f_mv(buf, pgem);
      }
      else {
         fseek(fgem, index_pos * sizeof(fhdr), 0);
         fwrite(&fhdr, sizeof(fhdr), 1, fgem);
         fclose(fgem);
      }
      return;
    }
  }
  fclose(fgem);
}


void
main(argc, argv)
  int argc;
  char *argv[];
{
  boardheader *bptr;
  DIR *dirp;
  struct dirent *de;
  int ch,n;
  char *fname, fpath[MAXPATHLEN];

  resolve_boards();
  nb=0;
  if (argc == 1) {
     puts("Creating the whole index...");
     chdir(strcpy(topdir, BBSHOME));
     strcpy(fpath, "man/.DIR");
     mandex(0, "", fpath);
  }


  chdir(strcpy(topdir, BBSHOME "/man/boards"));

  if (argc > 1)
  {
    sprintf(fpath, "%s/.DIR", argv[1]);
    mandex(0, "", fpath);
    exit(0);
  }

  if (!(dirp = opendir(".")))
  {
    printf("## unable to enter [man/boards]\n");
    exit(-1);
  }

  while (de = readdir(dirp))
  {
    fname = de->d_name;
    ch = fname[0];
    if (ch != '.' && !belong(BBSHOME"/etc/nomandex",fname))
    {
      board[nb].k =0;
      strcpy(board[nb].bname,fname);
      sprintf(fpath, "%s/.DIR", fname);
      mandex(0, "", fpath);
      printf("%-14sd: %d\tf: %d\n", fname, ndir, nfile);        /* report */
      board[nb].ndir = ndir;
      board[nb].nfile = nfile;
      if(board[nb].k && !belong(BBSHOME"NoStatBoards",fname)) nb++;
    }
  }
  closedir(dirp);
  qsort(board, nb, sizeof(boardinfo),k_cmp);


  if(! (fndx=fopen(BBSHOME"/log/topboardman","w"))) exit(0);

  fprintf(fndx,"[1m[44m±Æ¦W[46;37m        ¬Ý ª©  ¥Ø¿ý¼Æ   ÀÉ®×¼Æ"
  "     byte¼Æ [33m  Á` ¤À     ª©   ¥D          [40;32m\n");

  for(ch=0;ch<nb;ch++)
        {
          for (n = 0; n < numboards; n++)
          {
                   bptr = &bcache[n];
                if(!strcmp(bptr->brdname,board[ch].bname)) break;
          }

          if(board[ch].ndir + board[ch].nfile <5) break;
          if(bptr->brdattr & BRD_GROUPBOARD || bptr->brdattr & BRD_CLASS) continue;
          fprintf(fndx,"[36m%3d.%15s[1m%5d %7d %10d   %6d [32m%-24.24s\n",ch+1,
                board[ch].bname,
                board[ch].ndir,board[ch].nfile, board[ch].k
                ,board[ch].k/100+board[ch].nfile+board[ch].ndir
                ,bptr->BM);
        }
  fclose(fndx);
  exit(0);
}



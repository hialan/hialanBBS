/*-------------------------------------------------------*/
/* gem.c   ( NTHU CS MapleBBS Ver 2.36 )                 */
/*-------------------------------------------------------*/
/* target : ºëµØ°Ï¾\Åª¡B½s¿ï                             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
/*
  4. ¤pªO¥D
*/
#include "bbs.h"

#define FHSZ            sizeof(fileheader)

#define MAXPATHLEN	1024


static char copyfile[MAXPATHLEN];

/* ---- enchanted announce.c ---- */
/* shakalaca.000525               */
extern int last_line;
extern int cmpfilename();
extern int TagNum;
extern char currdirect[];

char mandirect[256];
char dir_title[256];
char gem_mode;
char CurrSite[128];
char trans_buffer[256];

/* static */ char gem_level;		/* gem's depth */
/* static */ char gopher_level;	/* gopher's depth */
static fileheader copyfileheader;

#define MAX_LEVEL	16
#define GEM_DATA_SIZE	256

#define PROXY_HOME	BBSHOME "/net/"
#define FN_TITLE	"/.TITLE"
/* --------------------------------------------------------------------- */

static int
gem_article()
{
  char *ptr;
  fileheader fhdr;

  if (!(gem_mode & GEM_PERM) || gem_mode & GEM_NET)
  /* ¨S¦³Åv­­ & ¦b gopher ¤¤, ¤£±o¨Ï¥Î */
    return RC_NONE;

  strcpy(dir_title, mandirect);
  *(ptr = strrchr(dir_title, '/')) = 0;
  /* fhdr.filemode = */ stampfile(dir_title, &fhdr);
  strcpy(fhdr.title, "¡º ");

  if (!getdata(b_lines, 0, "½Ð¿é¤J¼ÐÃD¡G", fhdr.title + 3, 44, DOECHO, 0))
  {
    unlink(dir_title);
    return RC_FOOT;
  }

  if (vedit(dir_title, NA) == -1)
  {
    unlink(dir_title);
    return RC_FULL;
  }                           

//  fhdr.savemode = GEM_ARTICLE;
  strcpy(fhdr.owner, cuser.userid);

  if (rec_add(mandirect, &fhdr, FHSZ) == -1)
  {
    pressanykey("¤å³¹¯Á¤ÞÀÉ¼g¤J¥¢±Ñ!");
    return RC_FULL;
  }
  return RC_FULL;
}
                                          

static int
gem_group()
{
  char *ptr;
  fileheader fhdr;
  FILE *fp;

  if (!(gem_mode & GEM_PERM) || (gem_mode & GEM_NET))
    return RC_NONE;

  strcpy(dir_title, mandirect);
  *(ptr = strrchr(dir_title, '/')) = 0;
  /* fhdr.filemode = */ stampdir(dir_title, &fhdr);
  strcpy(fhdr.title, "¡» ");

  if (!getdata(b_lines, 0, "½Ð¿é¤J¼ÐÃD¡G", fhdr.title + 3, 44, DOECHO, 0))
  {
    unlink(dir_title);
    return RC_FOOT;
  }

  unlink(dir_title);
  mkdir(dir_title, 0755);

  /* write title to FN_TITLE */
  strcpy(dir_title + strlen(dir_title), FN_TITLE);
  if (fp = fopen(dir_title, "w"))
  {
    fprintf(fp, "%s", fhdr.title);
    fclose(fp);
  }

//  fhdr.savemode = GEM_GROUP;
  strcpy(fhdr.owner, cuser.userid);

  if (rec_add(mandirect, &fhdr, FHSZ) == -1)
  {
    pressanykey("¤å³¹¯Á¤ÞÀÉ¼g¤J¥¢±Ñ!");
    return RC_FULL;
  }
  return RC_FULL;
}


static int
gem_link()
{
  char *ptr, buf[MAXPATHLEN], lpath[MAXPATHLEN];
  fileheader fhdr;
  int d;

  if (!(gem_mode & GEM_PERM) || gem_mode & GEM_NET)
  /* ¨S¦³Åv­­ & ¦b gopher ¤¤, ¤£±o¨Ï¥Î */
    return RC_NONE;

  strcpy(dir_title, mandirect);
  *(ptr = strrchr(dir_title, '/')) = 0;
  stamplink(dir_title, &fhdr);
  unlink(dir_title);

  if (!getdata (b_lines, 0, "·s¼W³s½u¡G", buf, 61, DOECHO, 0))
    return RC_FOOT;

  fhdr.title[0] = 0;
  for (d = 0; d <= 4; d++)
  {
    switch (d)
    {
    case 0:
      sprintf (lpath, "%s%s%s/%s",
        BBSHOME, "/man/boards/", currboard, buf);
      break;
    case 1:
      sprintf (lpath, "%s%s%s", BBSHOME, "/man/boards/", buf);
      break;
    case 2:
      sprintf (lpath, "%s%s%s", BBSHOME, "/boards/", buf);
      break;
    case 3:
      sprintf (lpath, "%s%s%s", BBSHOME, "/etc/", buf);
      break;
    case 4:
      sprintf (lpath, "%s%s%s", BBSHOME, "/", buf);
      break;
    }
    if (dashf (lpath))
    {
      strcpy (fhdr.title, "¡¸ ");	/* A1B3 */
      break;
    }
    else if (dashd (lpath))
    {
      strcpy (fhdr.title, "¡¹ ");	/* A1B4 */
      break;
    }
    if (!HAS_PERM (PERM_SYSOP) && d == 1)
      break;
  } /* for (d = 0; d <= 4; d++) */

  if (!fhdr.title[0])
  {
    pressanykey("¥Øªº¦a¸ô®|¤£¦Xªk¡I");
    return RC_FOOT;
  }

  if (!getdata(b_lines, 0, "½Ð¿é¤J¼ÐÃD¡G", fhdr.title + 3, 44, DOECHO, 0))
    return RC_FOOT;

  if (symlink(lpath, dir_title) == -1)
  {
    pressanykey ("µLªk«Ø¥ß symbolic link");
    return RC_FOOT;
  }

  strcpy(fhdr.owner, cuser.userid);

  if (rec_add(mandirect, &fhdr, FHSZ) == -1)
  {
    pressanykey("¤å³¹¯Á¤ÞÀÉ¼g¤J¥¢±Ñ!");
    return RC_FULL;
  }
  return RC_FULL;
}


#ifdef HAVE_GEM_GOPHER

static int
gem_gopher()
{
  char *ptr;
  fileheader fh;

  if (!(gem_mode & GEM_PERM) || gem_mode & GEM_NET)
    return RC_NONE;

  strcpy(dir_title, mandirect);
  *(ptr = strrchr(dir_title, '/')) = 0;
  /* fh.filemode = */ stampfile(dir_title, &fh);
  unlink(dir_title);

  strcpy(fh.title, "¡¹ ");
  if (!getdata(b_lines, 0, "½Ð¿é¤J¼ÐÃD¡G", fh.title + 3 , 44, DOECHO, 0))
    return RC_FOOT;

  strcpy(fh.filename, "H.");
  if (!getdata(b_lines, 0, "½Ð¿é¤J³s½u¦ì§}¡G", fh.filename + 2, FNLEN - 2,  DOECHO, 0))
    return RC_FOOT;

  if (invalid_pname(fh.filename))
  {
    pressanykey("¤£¦Xªkªº¦ì§} !!");
    return RC_FOOT;
  }

#if 0 
  /* shakalaca.000601: ·Q·Q¦pªG¦³»Ý­n¦A¥[¤W¥h§a :) */
  if(!getdata(b_lines, 0, "½Ð¿é¤J³s½uport¡G", port, 5, DOECHO, "70"))
    strcpy(port, "70");
  strcpy(path, "1/");
  getdata(b_lines - 1, 0, "½Ð¿é¤J³s½u¸ô®|¡G", path+2, 60, DOECHO, 0);
#endif

//  fh.savemode = GEM_GOPHER;
  strcpy(fh.owner, cuser.userid);

  if (rec_add(mandirect, &fh, FHSZ) == -1)
  {
    pressanykey("¤å³¹¯Á¤ÞÀÉ¼g¤J¥¢±Ñ!");
    return RC_FOOT;
  }

  return RC_FULL;
}
#endif
      
static int
gem_delete(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char genbuf[3];

  if (!(gem_mode & GEM_PERM) || (gem_mode & GEM_NET))
    return RC_NONE;

  getdata(1, 0, msg_del_ny, genbuf, 3, LCECHO, 0);
  if (genbuf[0] == 'y')
  {
    strcpy(currfile, fhdr->filename);
    if (!delete_file(direct, FHSZ, ent, cmpfilename))
    {
      /* ­É¥Î dir_title */
      setdirpath(dir_title, direct, fhdr->filename);
//      if (fhdr->savemode == GEM_GROUP)	/* ¥Ø¿ý */
      if (dashd(dir_title))
      {
        char buf[256];

        snprintf(buf, 256, "/bin/rm -rf %s", dir_title);
        system(buf);
      }
//      else if (fhdr->savemode == GEM_GOPHER)	/* ³s½u */
      else if (fhdr->filename[0] == 'H')
      {
        strcpy(dir_title + strlen(dir_title), "~");
        unlink(dir_title);;
      }
      else	/* ¤å³¹ */
        unlink(dir_title);

      if (last_line == 1)
        return RC_NEWDIR;
      return RC_CHDIR;
    }
  }
  return RC_FULL;
}                                                 


static int
gem_move(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  if (gem_mode & GEM_PERM && !(gem_mode & GEM_NET))
  {
    fileheader *tmp;
    char newnum[6], buf[30];
    int num, success;

    sprintf (buf, "½Ð¿é¤J²Ä %d ¿ï¶µªº·s¦¸§Ç¡G", ent);
    if (!getdata(b_lines, 0, buf, newnum, 6, DOECHO, 0))
      return RC_FOOT;
    if ((num = atoi(newnum)) < 1)
      num = 1;
    else if (num > last_line)
      num = last_line;

    if (num == ent)
      return RC_FOOT;

    success = num < ent ? ent-num:num-ent;
    tmp = (fileheader *) calloc(success + 1, FHSZ);

    if (num < ent)
    {
      memcpy(tmp, fhdr, FHSZ);
      if (get_records(direct, tmp + 1, FHSZ, num, success) != success)
        success = 0;
    }
    else
    {
      if (get_records(direct, tmp, FHSZ, ent + 1, success) != success)
        success = 0;
      memcpy(tmp + success, fhdr, FHSZ);
    }
    if (success)
      substitute_record(direct, tmp, FHSZ * (success + 1), num < ent ? num : ent, FHSZ);
    free(tmp);
    return RC_NEWDIR;
  }
  return RC_NONE;
}


static int
gem_edit(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  int edit = 0;
  fileheader tmpfhdr = *fhdr;

  if (!(gem_mode & GEM_PERM) || (gem_mode & GEM_NET))
    return RC_NONE;

  /* change title */
  if (getdata(b_lines, 0, "¼ÐÃD¡G", tmpfhdr.title + 3, 44, DOECHO, fhdr->title + 3))
  {
    *fhdr = tmpfhdr;
    edit = 1;

    setdirpath(dir_title, mandirect, fhdr->filename);
//    if (fhdr->savemode == GEM_GROUP)
    if (dashd(dir_title))
    {
      FILE *fp;

      if (access(dir_title, X_OK | R_OK | W_OK))
        mkdir(dir_title, 0755);
      strcpy(dir_title + strlen(dir_title), FN_TITLE);
      if (fp = fopen(dir_title, "w"))
      {
        fprintf(fp, "%s", tmpfhdr.title);
        fclose(fp);
      }
    }
  }

  /* change editor */
  if (getdata(b_lines, 0, "§@ªÌ¡G", tmpfhdr.owner, 13, DOECHO, fhdr->owner))
  {
    *fhdr = tmpfhdr;
    edit = 1;
  }

  if (edit)
    substitute_record(direct, fhdr, FHSZ, ent);

  setdirpath(dir_title, mandirect, fhdr->filename);
  if (!dashd(dir_title))
  {
    if (fhdr->filename[0] == 'H')
    {
      char new[FNLEN + 1], buf[256];
    
      strcpy(new, "H.");
      if (!getdata(b_lines, 0, "½Ð¿é¤J³s½u¦ì§}¡G", new + 2, FNLEN - 2, DOECHO, fhdr->filename + 2))
        return RC_FOOT;      

      if (invalid_pname(new))
      {
        pressanykey("¤£¦Xªkªº¦ì§} !!");
        return RC_FOOT;
      }
      sprintf(buf, "/bin/mv %s/%s/%s %s/%s/%s", BBSHOME, PROXY_HOME, fhdr->filename + 2, 
        BBSHOME, PROXY_HOME, new);

      system(buf);    
      substitute_record(direct, fhdr, FHSZ, ent);
    }
    else
    {
      vedit(dir_title, NA);
      return RC_FULL;
    }
  }
  return RC_FOOT;
}


static int
gem_name(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  fileheader tmpfhdr = *fhdr;

  if (HAS_PERM(PERM_SYSOP) && !(gem_mode & GEM_NET))
  {
    if (getdata(b_lines, 0, "·sªºÀÉ¦W¡G", tmpfhdr.filename, FNLEN, DOECHO, fhdr->filename))
    {
      if (invalid_fname(tmpfhdr.filename))
      {
        pressanykey(err_filename);
        return RC_FOOT;
      }
      else
      {
        setdirpath(dir_title, mandirect, tmpfhdr.filename);
        if (dashf(dir_title) || dashd(dir_title))
        {
          pressanykey("¨t²Î¤¤¤w¦³¦P¦WÀÉ®×¦s¦b¤F¡I");
          return RC_FOOT;
        }
        else
        {
          char genbuf[256];
 
          setdirpath(genbuf, mandirect, fhdr->filename);
          if (rename(genbuf, dir_title) == -1)
          {
            pressanykey("ÀÉ¦W§ó§ï¥¢±Ñ¡I");
            return RC_FOOT;
          }
        }
      }
      *fhdr = tmpfhdr;
      substitute_record(direct, fhdr, FHSZ, ent);
      return RC_FULL;
    }
  }
  return RC_FOOT;
}
                                             

void
gem_copyitem(fpath, fhdr)
  char *fpath;
  fileheader *fhdr;
{
/*
  if ((currstat != READING) && (gem_mode & GEM_LOCK_PATH))
  {
    pressanykey("½Ð¥ý¸Ñ°£¸ô®|Âê©w");
    return;
  }
*/
  if ((currstat == READING) && !(gem_mode & GEM_LOCK_PATH))
    strcpy(copyfile, fpath);

  memcpy(&copyfileheader, fhdr, FHSZ);
//  copyfileheader.savemode = GEM_ARTICLE;
  copyfileheader.date[0] = 0;
  strcpy(copyfileheader.owner, cuser.userid);
/*
  outmsg("ÀÉ®×¼Ð°O§¹¦¨¡C[ª`·N] «þ¨©«á¤~¯à§R°£­ì¤å!");
  igetkey();
*/
}


static int
gem_copy(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  if (!(gem_mode & GEM_PERM))
    return RC_NONE;

  if ((gem_mode & GEM_NET) && fhdr->savemode)
    return RC_NONE;

  memcpy(&copyfileheader, fhdr, FHSZ);
  if (copyfileheader.owner[0] == '[')
    strcpy(copyfileheader.owner, "ºëµØ°Ï³s½u");
  setdirpath(copyfile, mandirect, fhdr->filename);
  pressanykey("¼Ð°O§¹¦¨¡C[ª`·N] ½Ð«þ¨©«á¦A§R°£­ì¤å!");
  return RC_FOOT;                                                        
}


int
cite_article(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char fpath[256], title[TTLEN + 1], *msg;
  fileheader tmp;
  int tag;
  
  if (!(HAS_PERM(PERM_LOGINOK)))
    return RC_NONE;
    
  memcpy(&tmp, fhdr, sizeof(fileheader));

  if (currstat == RMAIL)
    setuserfile(fpath, tmp.filename);
  else
    setbfile (fpath, currboard, tmp.filename);

  if (TagNum)
  {
    if (gem_mode & GEM_LOCK_PATH)
      msg = "¦¬¿ý¦ÜÂê©w¸ô®|";
    else
      msg = "¦¬¿ý";
    
    tag = AskTag(msg);
    
    if (tag < 0)
      return RC_FOOT;
  }

  sprintf(title, "¡º %s", tmp.title);
  strncpy(tmp.title, title, TTLEN);
  tmp.title[TTLEN] = '\0';
  gem_copyitem (fpath, &tmp);

  if (gem_mode & GEM_LOCK_PATH)
  {
    char genbuf[MAXPATHLEN], *ptr;
    struct stat st;

    ptr = strrchr(copyfile, '/');
    strcpy(ptr + 1, ".DIR");
    if (!stat(copyfile, &st) && (st.st_size / FHSZ) >= MAXITEMS)
    {
      pressanykey("¤å³¹¤Ó¦h©ñ¤£¤UÅo..");
      return RC_FOOT;
    }
    *ptr = 0;

    stampfile(copyfile, &tmp);
    strcpy(copyfileheader.filename, tmp.filename);
    memcpy(&tmp, &copyfileheader, FHSZ);
    sprintf(genbuf, "/bin/cp %s %s", fpath, copyfile);
    system(genbuf);
    
    strcpy(ptr + 1, ".DIR");
    if (rec_add(copyfile, &tmp, FHSZ) == -1)
    {
      pressanykey("¤å³¹¯Á¤ÞÀÉ¼g¤J¥¢±Ñ!");
      return RC_FOOT;
    }
    return POS_NEXT;
  }
  else
  {
    if (tag > 0)	/* ¼Ð°O¤å³¹ */
    {
      outmsg("¦¬¿ý§¹¦¨, ½Ð¦ÜºëµØ°Ï¤º¶K¤W");
      igetkey();
    }
    brd_man ();
    return RC_FULL;
  }
}


static int
gem_paste(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  if (!(gem_mode & GEM_PERM) || (gem_mode & GEM_NET))
    return RC_NONE;

  move(b_lines, 0);
  if (copyfile[0] && (dashf(copyfile) || dashd(copyfile)))
  {
    if (getans("­n (A)ªþ¥[©ó¦¹ÀÉ®×¤§«á (P)¶}·sÀÉ ? [P]") == 'a')
    {
      FILE *fout;

      if (last_line == 0)
      {
        pressanykey("¿ù»~: ½Ð¶}·sÀÉ«á¦Aªþ¥[");
        return RC_FOOT;
      }

      setdirpath(dir_title, mandirect, fhdr->filename);

//      if (copyfileheader.savemode != GEM_ARTICLE || 
//        fhdr->savemode != GEM_ARTICLE)
      if (dashd(dir_title) || fhdr->filename[0] == 'H' ||
          dashd(copyfile) || copyfileheader.filename[0] == 'H')
      {
        pressanykey("¥Ø¿ý©Î³s½u¤£±oªþ¥[©óÀÉ®×«á¡I");
        return RC_FOOT;
      }

      if (fout = fopen(dir_title, "a+"))
      {
        fprintf(fout, "\n%s\n\n", msg_seperator);
        b_suckinfile(fout, copyfile);
        fclose(fout);
        pressanykey("ªþ¥[§¹¦¨");
      }
      return RC_FOOT;
    }
    else
    {
      char genbuf[MAXPATHLEN], *ptr;
      fileheader tmp;

      if (last_line >= MAXITEMS)
      {
        pressanykey("¿ù»~: ¥Ø¿ý¤UÀÉ®×¤Ó¦h, ½Ð·s¼W¥Ø¿ý¦A¶K");
        return RC_FOOT;
      }

      strcpy(dir_title, mandirect);
      *(ptr = strrchr(dir_title, '/')) = 0;
      stampfile(dir_title, &tmp);
      strcpy(copyfileheader.filename, tmp.filename);
      if (!copyfileheader.date[0])
        strcpy(copyfileheader.date, tmp.date);

//      if (copyfileheader.savemode == GEM_GROUP)
      if (dashd(copyfile))
      {
        unlink(dir_title);
        sprintf(genbuf, "/bin/cp -r %s %s", copyfile, dir_title);
      }
      else
        sprintf(genbuf, "/bin/cp %s %s", copyfile, dir_title);

      system(genbuf);
      if (rec_add(mandirect, &copyfileheader, FHSZ) == -1)
      {
        pressanykey("¤å³¹¯Á¤ÞÀÉ¼g¤J¥¢±Ñ!");
        return RC_FOOT;
      }
      pressanykey("«þ¨©§¹¦¨");
      return RC_BODY;
    }
  }
  else
  {
    pressanykey("¿ù»~: ½Ð¥ý°õ¦æ«þ¨©©R¥O");
  }
  return RC_FOOT;
}

extern TagItem TagList[];	/* ascending list */


static int
gem_pastetag(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  int num;

  if (gem_mode & GEM_LOCK_PATH)
  {
    pressanykey("½Ð¥ý¸Ñ°£¸ô®|Âê©w");
    return RC_FOOT;
  }

  if (!(gem_mode & GEM_TAG))
  {
    pressanykey("±z¨S¦³¼Ð°O¤å³¹");
    return RC_FOOT;
  }

  if (gem_mode & GEM_NET)
    return RC_NONE;                                                          

  if (getans("½T©w­n«þ¨©©Îªþ¥[¦¹ªO©Ò¦³¼ÐÅÒ¹Lªº¤å³¹¶Ü(Y/N)¡H[N] ") != 'y')
    return RC_FOOT;

  if (getans("¿ï¾Ü (A)¥þ³¡ªþ¥[©ó¦¹ÀÉ®×¤§«á (P)¥þ³¡¶}·sÀÉ¨Ã«þ¨© ? [P]") == 'a')
  {
    FILE *fout;

    if (last_line == 0)
    {
      pressanykey("¿ù»~: ½Ð¶}¤@·sÀÉ«á¦Aªþ¥[");
      return RC_FOOT;
    }

    setdirpath(dir_title, mandirect, fhdr->filename);
//    if (fhdr->savemode != GEM_ARTICLE)
    if (dashd(dir_title) || fhdr->filename[0] == 'H')
    {
      pressanykey("¿ù»~: ¤£±oªþ¥[©ó¥Ø¿ý©Î³s½u!");
      return RC_FOOT;                            
    }

    if (!(fout = fopen(dir_title, "a+")))
    {
      pressanykey("¿ù»~: ÀÉ®×µLªk¶}±Ò");
      return RC_FOOT;
    }

    for (num = 0; num < TagNum; num++)
    {
      sprintf(copyfile, "boards/%s/M.%d.A", currboard, TagList[num]);
      if (dashf(copyfile))
      {
        fprintf(fout, "\n%s\n\n", msg_seperator);
        b_suckinfile(fout, copyfile);
      }
    }
    fclose(fout);
    pressanykey("ªþ¥[§¹¦¨");
    copyfile[0] = 0;
    return RC_FOOT;
  }                                                                            
  else
  {
    fileheader *fhdr;
    char genbuf[MAXPATHLEN], *ptr, *ptr1;

    if (TagNum > (MAXITEMS - last_line))
    {
      pressanykey("¿ù»~: ¤å³¹¼Æ¶W¹L¤W­­, ½Ð±N¤å³¹¤À§å¼ÐÅÒ©Î¶}·s¥Ø¿ý");
      return RC_FOOT;
    }

    fhdr = (fileheader *) calloc(TagNum, FHSZ);

    sprintf(copyfile, "boards/%s/.DIR", currboard);
    if (TagThread(copyfile, NULL, TAG_GET_RECORD) != RC_NONE)
    {
      strcpy(dir_title, mandirect);
      ptr  = strrchr(dir_title, '/');
      ptr1 = strrchr(copyfile, '/') + 1;

      for (num = 0; num < TagNum; num++)
      {
        strcpy(ptr1, fhdr[num].filename);                                     
        if (!dashf(copyfile))
          continue;
        *ptr = 0;
        stampfile(dir_title, &copyfileheader);
        strcpy(copyfileheader.title, "¡º ");
        strncpy(copyfileheader.title + 3, fhdr[num].title, TTLEN - 3);
//        copyfileheader.savemode = GEM_ARTICLE;
        strcpy(copyfileheader.owner, cuser.userid);

        sprintf(genbuf, "/bin/cp %s %s", copyfile, dir_title);
        system(genbuf);

        rec_add(mandirect, &copyfileheader, FHSZ);
      }
      pressanykey("«þ¨©§¹¦¨");
    }
    else
      pressanykey("¯Á¤ÞÀÉ¶}±Ò¥¢±Ñ");
    free(fhdr);
  }                                                                            
  copyfile[0] = 0; 
  return RC_FULL;
}    


static int
gem_lockpath()
{
  if (!(gem_mode & GEM_PERM) || gem_mode & GEM_NET)
    return RC_NONE;

  if (gem_mode & GEM_LOCK_PATH)
  {
    pressanykey("¸Ñ°£¸ô®|Âê©w");
  }
  else
  {
    strcpy(copyfile, mandirect);
    pressanykey("Âê©w¥Ø«e©Ò¦b¸ô®|");
  }

  gem_mode ^= GEM_LOCK_PATH;
  return RC_FOOT;
}


/* Gopher ³¡¥÷ */
#ifdef HAVE_GEM_GOPHER
static void
timeout() {}

int
net_cmd(site, sock)
  char *site;
  int *sock;
{
  struct sockaddr_in sin;
  struct hostent *host;

  (void) memset ((char *) &sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons (*sock);

  host = gethostbyname (site);
  if (host == NULL)
    sin.sin_addr.s_addr = inet_addr (site);
  else
    (void) memcpy (&sin.sin_addr.s_addr, host->h_addr, host->h_length);

  *sock = socket (AF_INET, SOCK_STREAM, 0);

  if (*sock < 0)
    return 1;

  signal(SIGALRM, timeout);
  alarm(10);
  if (connect (*sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
  {
    close (*sock);
    init_alarm();
    return 1;
  }
  init_alarm();
  return 0;
}


void
Transfer()
/* 
   dir_title : BBSHOME /man/boards/[brdname]/H.[path]~/.TEMP
   mandirect : BBSHOME /man/boards/[brdname]/H.[path]~/.DIR
*/   
{
  FILE *fin, *fout;
  char *ptr, *gem;

  if (fin = fopen(dir_title, "r"))
  {
    gem = strrchr(dir_title, '/');
    strcpy(gem + 1, ".GEM");

    if (fout = fopen(dir_title, "w"))
    {
      char *site, *path, *port, buf[512];
      fileheader fh;
      int pos = 0;

      memset(&fh, 0, FHSZ);
      unlink(mandirect);
      outmsg("[1;5;36m¸ê®ÆÂà´«¤¤...½Ðµy­Ô[0m");
      refresh();
      while(fgets(buf, sizeof(buf), fin))
      {
        if (*buf != '0' && *buf != '1')
          continue;

        if (!(path = strchr(buf + 1, '\t')))
          continue;
        *path = 0;

        if (!(site = strchr(++path, '\t')))
          continue;
        *site = 0;

        if (strstr(path, " "))
          continue;

        if (*path == 0 || *(path + 1) != '/')
          path = NULL;

        if (!(port = strchr(++site, '\t')))
          continue;
        *port = 0;

        if (!((ptr = strchr(++port, '\t')) || (ptr = strchr(port, '\n'))))
          continue;
        *ptr  = 0;

        if (path && (ptr = strrchr(path, '/')) && *++ptr != 0)
        {
          strcpy(fh.filename, ptr);
          strcpy(gem + 1, ptr);
        }
        else
        {
          *gem = 0;
          stampfile(dir_title, &fh);
        }
//        fh.savemode = *buf == '1' ? GEM_GOPHER : GEM_ARTICLE;
//        strcpy(fh.owner, *buf == '1'? "[¥Ø]":"[¤å]");
        strcpy (fh.title, "¡¼ ");
        if (*buf == '1')
          fh.title[1] = (char) 0xbd;
          
        strncpy(fh.title + 3, buf + 1, TTLEN - 3);
        fh.title[TTLEN] = 0;

        if (rec_add(mandirect, &fh, FHSZ) == -1)
          continue;

        if (!fseek(fout, pos, SEEK_SET))
        {
          fprintf(fout, "%s\n", path ? path : "1/");
          pos += GEM_DATA_SIZE;
        }
      }
      fclose(fout);
    }
    fclose(fin);
  }
}


/* ±N BBSHOME /man/boards/[brdname]/H.[path]~/ link ¨ì PROXY_HOME /H.[path]/ */
int
MakeProxyLink(path)
  char *path;
{
  char fpath[256], *ptr, *str, ch;

  str = CurrSite;
  strcpy(fpath, PROXY_HOME);
  ptr = fpath + strlen(fpath);

  while (ch = *str)
  {
    str++;
    if (ch == '.')
    {
      if (!strcmp(str, "edu.tw"))
        break;
    }
    else if ( ch >= 'A' && ch <= 'Z')
    {
      ch |= 0x20;
    }
    *ptr++ = ch;
  }
  *ptr = '\0';
  mkdir(fpath, 0755);

  *ptr++ = '/';
  if ((str = strrchr(path, '/')) && *++str == '/')
    *str = 0;

  str = path + 1;
  *(path - 1) = '|';
  if (*str++)
  {
    while (*str)
    {
      if (*str == '/')
      {
        *ptr = 0;
        mkdir(fpath, 0755);
      }
      *ptr++ = *str++;
    }
  }
  
  *ptr = 0;
  mkdir(fpath, 0755);
  symlink(fpath, dir_title);

  return 0;
}


int
GetSocketData(path, title)
  char *path, *title;
{
  char *ptr, buf[512];
  int sock;
  FILE *fsock, *fout;

  sock = 70;

  if (title)
  {
    ptr = dir_title + strlen(dir_title);
    strcpy(ptr, "/.TEMP");
  }

  if (net_cmd(CurrSite, &sock))
    return 1;

  if (!(fsock = fdopen(sock, "r+")))
  {
    close(sock);
    return 1;
  }

  if (!(fout = fopen(dir_title, "w")))
  {
    fclose(fsock);
    return 1;
  }

  outmsg("[1;5;37m¸ê®Æ¤U¸ü¤¤...½Ðµy­Ô[0m");
  move(b_lines, 0);
  clrtoeol();
  prints("¸ê®Æ¤U¸ü¤¤...");
  refresh();
  sock = 0;
  fprintf(fsock, "%s\r\n", path);
  fflush(fsock);

  while(fgets(buf, 100, fsock))
  {
    if (path = strstr(buf, "\r\n"))
      strcpy(path, "\n");
    fputs(buf, fout);
    move(b_lines, 13);
    sock += strlen(buf);
    prints("%-d", sock);
    refresh();
  }
  fclose(fsock);
  fclose(fout);

  if (title)
  {
    strcpy(ptr, FN_TITLE);
    if (!dashf(dir_title) && (fout = fopen(dir_title, "w")))
    {
      fprintf(fout, "%s", title);
      fclose(fout);
    }
    *ptr = 0;
  }

  return 0;
}

/* ¨ú±o proxy ¤ºªº¸ê®Æ */
int
GetData(ent, data)
  int ent;
  char *data;
{
  FILE *fp;

  strcpy(data, dir_title);
  if (gem_mode & GEM_NET)
  {
    char *ptr;

    ptr = strrchr(data, '/') + 1;
    strcpy(ptr, "/.GEM");
  }

  if (gopher_level >= 1)
  {
    if (fp = fopen(data, "r"))
    {
      if ((gem_mode & GEM_NET) && fseek(fp, GEM_DATA_SIZE * (ent - 1), SEEK_SET))
      {
        return 1;
      }
  
      fgets(data, GEM_DATA_SIZE, fp);
      fclose(fp);
      data[strlen(data) - 1] = 0;
      return 0;
    }
  }
  else
  {
    strcpy(data, "1/");
    data[strlen(data) - 1] = 0;
    return 0;
  } 
  return 1;
}


int
GopherRead(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char data[256];

  gem_mode &= ~GEM_RELOAD;

  if (GetData(ent, data))
    return 1;

  if (GetSocketData(data, NULL))
    return 2;

  if (!fhdr->filemode)
  {
    fhdr->filemode = 1;
    substitute_record(direct, fhdr, FHSZ, ent);
  }
  return 0;
}


int
CheckPath()
{
  char buf[256];

  if (dashl(dir_title))
  {
    buf[readlink(dir_title, buf, 256)] = 0;
    return dashd(buf) ? 0 : 1;
  }
  return 1;
}

#endif

/* ----------- */ 
int
gem_perm(fname,fhdr)
  char *fname;
  fileheader *fhdr;
{
  char buf[MAXPATHLEN];

  if (fhdr->filemode & FILE_REFUSE)
  {
    if (dashf(fname))
      sprintf(buf, "%s.vis",fname);
    else
      sprintf(buf, "%s/.vis",fname);

    if (gem_mode & GEM_PERM)
      return 2;

    if (belong_list(buf, cuser.userid))
      return 1;
    else
      return 0;
  }
  else
    return 1;
}


static int
gem_savemail(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  setdirpath(dir_title, mandirect, fhdr->filename);
  if (save_mail (0, fhdr, dir_title) == POS_NEXT)
    return POS_NEXT;
  else
    return RC_NONE;
}


static int
gem_read(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  int more_result, mode;
  
  setdirpath(dir_title, mandirect, fhdr->filename);
 
  mode = gem_perm(dir_title,fhdr);
  if (mode < 1)
  {
    pressanykey ("³o¬O¨p¤H­«¦a , §AµLªk¶i¤J , ¦³»Ý­n½Ð¦VªO¥D¥Ó½Ð");
    return RC_FOOT;
  }	      
  else if (mode >1)
  {
    if (getans("¬O§_½s¿è¥i¬Ý¨£¦W³æ? (y/N)") == 'y')
    {
      char buf[MAXPATHLEN];
      if (dashf(dir_title))
        sprintf(buf,"%s.vis",dir_title);
      else
	sprintf(buf,"%s/.vis",dir_title);
      ListEdit(buf);
    }
  }

//  else if (fhdr->savemode == GEM_GROUP)
  if (dashd(dir_title))
  {
    if (++gem_level > MAX_LEVEL)
    {
      gem_level--;
      return RC_NONE;
    }

    direct = strrchr(mandirect, '/');
    sprintf(direct + 1, "%s/.DIR", fhdr->filename);
    strcpy(currdirect, mandirect);
    getkeep(currdirect, 1, 1);

    return RC_NEWDIR;
  }    
//  else if (fhdr->savemode == GEM_GOPHER)
  else if (fhdr->filename[0] == 'H')
  {
    if (gopher_level == 0)
    {
      strcpy(CurrSite, fhdr->filename + 2);
    }
    
    if (++gem_level > MAX_LEVEL)
    {
      gem_level--;
      return RC_NONE;
    }

    /* ³]©w©Ò¦b¥Ø¿ý : man/boards/[brdname]/H.[path]~/ */
    setdirpath(dir_title, mandirect, fhdr->filename + 2);
    direct = dir_title + strlen(dir_title);
    strcpy(direct, "~");
    
    if (gem_mode & GEM_RELOAD)	/* §ó·s */
    {
      char buf[512];

      snprintf(buf, 512, "/bin/rm -f %s/.* %s/*", dir_title, dir_title);
      system(buf);
      gem_mode &= ~GEM_RELOAD;
    }

    if (dashl(dir_title) && (strcpy(direct + 1, "/.DIR") && dashf(dir_title)))
    {
      strcpy(mandirect, dir_title);
    }
    else	/* ¦pªG²Ä¤@¦¸³s½u or ¨S¦³¦¹¥Ø¿ý (reload) */
    {
      char data[256];

      *direct = 0;
      if (GetData(ent, data)	/* ¥ý¨ú±o¸ê®Æ */ 
        || (strcpy(direct, "~") && CheckPath() && MakeProxyLink(data))
        || (strcpy(direct + 1, "/.DIR") && !dashf(dir_title)
        && strcpy(direct, "~") && GetSocketData(data, fhdr->title)))
      {
        gem_level--;
        pressanykey("µLªk¹F¦¨³s½u, ½Ðµy­Ô¦A¹Á¸Õ.");
        return RC_FOOT;
      }
      strcpy(direct, "~");
      sprintf(mandirect, "%s/.DIR", dir_title);
      strcpy(direct + 1, "/.TEMP");
      Transfer();
    }
    if (!(gem_mode & GEM_TAG))
      TagNum = 0;
    strcpy(currdirect, mandirect);
    getkeep(currdirect, 1, 1);
    gem_mode |= GEM_NET;
    gopher_level++;
    return RC_NEWDIR;    
  }
  else
//  if (fhdr->savemode == GEM_ARTICLE)
  {
#ifdef HAVE_GEM_GOPHER
    if ((gem_mode & GEM_NET) && (gem_mode & GEM_RELOAD || fhdr->filemode == 0)
      && GopherRead(ent, fhdr, direct))
      return RC_NONE;
#endif
    if ((more_result = more(dir_title, YEA)) == -1)
      return RC_NONE;

    if (strstr (dir_title, "etc/editexp/") || strstr (dir_title, "etc/SONGBOOK/"))
    {
/*
      if (getans(strstr (dir_title, "etc/editexp/") ?
        "­n§â½d¨Ò Plugin ¨ì¤å³¹¶Ü?[y/N]" :
        "½T©w­nÂI³o­ººq¶Ü?[y/N]") == 'y')
*/
      if (getans2(b_lines, 0, 
            strstr (dir_title, "etc/editexp/") ?
            "­n§â½d¨Ò Plugin ¨ì¤å³¹¶Ü?[y/N]" :
            "½T©w­nÂI³o­ººq¶Ü?",
         0, 2, 'n') == 'y')           
      {
        strcpy (trans_buffer, dir_title);
        if (currstat == OSONG)
          f_cat (FN_USSONG, fhdr->title);
        return RC_FULL;
      }
    }
    
    strncpy(currtitle, str_ttl(fhdr->title), 40);
    strncpy(currowner, str_ttl(fhdr->owner), IDLEN + 2);

    switch (more_result)
    {
      case 1:
         return RS_PREV;
      case 2:
         return RELATE_PREV;
      case 3:
         return RS_NEXT;
      case 4:
         return RELATE_NEXT;
      case 5:
         return RELATE_FIRST;
      case 6:
      case 7:
      case 8:
         return RC_FULL;
      case 9:
         return 'A';
      case 10:
         return 'a';
      case 11:
         return '/';
      case 12:
         return '?';
    }

    return RC_FULL;
  }

  return RC_NONE;
}


#ifdef HAVE_GEM_GOPHER
static int
gem_reload(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  if (!(gem_mode & GEM_PERM) ||
     (!(gem_mode & GEM_NET) && fhdr->filename[0] != 'H'
     /* fhdr->savemode != GEM_GOPHER */))
    return RC_NONE;

  gem_mode |= GEM_RELOAD;
  return gem_read(ent, fhdr, direct);
}
#endif


static int
gem_fmode()
{
  if (HAS_PERM(PERM_SYSOP))
  {      
    gem_mode ^= GEM_FMODE;
    return RC_FULL;
  }               
  return RC_NONE;
}


static int
gem_refusemark (ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  if (!(gem_mode & GEM_PERM) || gem_mode & GEM_NET)
    return RC_NONE;

  fhdr->filemode ^= FILE_REFUSE;
  substitute_record(direct, fhdr, FHSZ, ent);

  return RC_DRAW;
}


static int
gem_delrange(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  return del_range (0, NULL, mandirect);
}


extern int cross_post();


struct one_key gem_comms[] = {
  'r', gem_read,	/* ¾\Åª */
  'a', gem_article,
  'd', gem_delete,
  'D', gem_delrange,
  'g', gem_group,
  'l', gem_link,
  'm', gem_move,
  'E', gem_edit,
  'n', gem_name,
  'p', gem_paste,
  'P', gem_pastetag,
  'c', gem_copy,
  'x', cross_post,
  'f', gem_fmode,
  'L', gem_lockpath,
  'o', gem_refusemark,
  'S', gem_savemail,
#ifdef HAVE_GEM_GOPHER
  'G', gem_gopher,
  'R', gem_reload,
#endif
  '\0', NULL
};


static void
gem_timestamp (buf, time)
  char *buf;	/* ¶Ç¦^ buf */
  time_t *time;
{
  struct tm *pt = localtime (time);
  sprintf (buf, "%02d/%02d/%02d", 
    pt->tm_mon + 1, pt->tm_mday, pt->tm_year % 100);
}


void
gem_doent(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char *mark, *title, color, buf[256], tag;
  time_t dtime;

  tag = ' ';  
  if (TagNum && !Tagger(atol(fhdr->filename + 2) , 0, TAG_COMP))
    tag = '*';
  mark = fhdr->title;
  title = str_ttl(mark);

  color = (title == mark) ? '1' : '3';

  if (gem_mode & GEM_FMODE)
    prints("%6d. %-46.46s[%22.22s]\n", ent, title, fhdr->filename);
  else
  {                                         
    if (gem_mode & GEM_NET)
      prints("%6d. %-70.70s\n", ent, title);
    else
    {
      setdirpath(buf, mandirect, fhdr->filename);
      dtime = dasht(buf);
      gem_timestamp(buf, &dtime);
      
/*
      if (strncmp(currtitle, title, 40))
        prints("%6d  %-45.45s %-12.12s [%-5.5s/%02d]\n",ent, 
          title, fhdr->owner, fhdr->date, fhdr->filemode);
      else
        prints("%6d  [1;3%cm%-45.45s %-12.12s [%-5.5s/%02d][0m\n", ent, 
          color, title, fhdr->owner, fhdr->date, fhdr->filemode);
*//* shakalaca.000601: ¤£¯à¥Î filemode ·í°µ¦~¥÷, ¦]¬°ÀÉ®×ªº¨Ï¥ÎÅv­­¦³¥Î¨ì.
     		       ¥H timestamp ¨M©w */
      if (strncmp(currtitle, title, 40))
        prints("%6d%c%c%-45.45s %-12.12s [%s]\n",ent, 
          (fhdr->filemode & FILE_REFUSE) ? ')' : '.', tag,
          title, fhdr->owner, buf);
      else
        prints("%6d%c%c[1;3%cm%-45.45s %-12.12s [%s][0m\n", ent, 
          (fhdr->filemode & FILE_REFUSE) ? ')' : '.', tag,
          color, title, fhdr->owner, buf);
    }
  }
}


static void
gem_title()
{
  FILE *fp;
  
  setdirpath(dir_title, mandirect, FN_TITLE);
  if (fp = fopen(dir_title, "r"))
  {
    if (!fgets(dir_title, 50, fp))
      strcpy(dir_title, BOARDNAME"ºëµØ°Ï");
    fclose(fp);
    showtitle("ºëµØ¤å³¹", dir_title);
  }
  else
    showtitle("ºëµØ¤å³¹", BOARDNAME"ºëµØ°Ï");

  outs("\
[¡ö]Â÷¶} [¡÷]¾\\Åª [F]Âà±H [t]¼ÐÅÒ  ¶}ÅP[a]·s¤å³¹ [g]·s¥Ø¿ý [G]·s³s½u [h]¨D§U\n\
" COLOR1 "[1m  ½s¸¹    ¼Ð      ÃD                                  ½s        ¿ï  ¤é    ´Á  [m");
}


int
gem_quit()
{
  char *ptr;

  if (!(gem_mode & GEM_TAG))
    TagNum = 0;

  if (!(--gopher_level))
  {
    memset(CurrSite, 0, sizeof(CurrSite));
    gem_mode &= ~GEM_NET;
  }
              
  if (!(--gem_level))	/* ¸õ¥XºëµØ°Ï */
  {
    gem_mode  &= ~GEM_TAG;
    return 0;
  }
  *(ptr = strrchr(mandirect, '/')) = 0;
    ptr = strrchr(mandirect, '/');
  strcpy(ptr + 1, ".DIR");
  strcpy(currdirect, mandirect);

  return gem_level;                   
}


int
gem_none()
{
  while(1)
  {
    gem_title();
    outs("\n\n  ¡mºëµØ°Ï¡n©|¦b§l¨ú¤Ñ¦a¶¡ªº¤é¤ëºëµØ¤¤...\r");
            
    switch(igetkey())
    {
      case 'q':
      case 'e':
      case KEY_LEFT:
	/* shakalaca.000525: ±o¦Ò¼{¥Ø¿ýªº°ÝÃD */
          return gem_quit();
      case 'a':
          gem_article();
          break;
      case 'g':
          gem_group();                                                         
          break;
      case 'l':
          gem_link();
          break;
      case 'p':
          gem_paste(NULL, NULL, mandirect);
          break;
      case 'G':
          gem_gopher();
          break;
      case 'L':
          gem_lockpath();
          break;
      case 'P':
          gem_pastetag(NULL, NULL, mandirect);
          break;
    }

    if ((last_line = rec_num(mandirect, FHSZ)) > 0)                     
      return last_line;
  }
}
                 

void
gem_menu(fpath, perm, mode)
  char *fpath;
  int perm, mode;
{
  gem_level = 1;
  gopher_level = 0;
  gem_mode = perm ? (gem_mode | GEM_PERM) : (gem_mode & ~GEM_PERM);
  if (TagNum && (gem_mode & GEM_PERM))
    gem_mode |= GEM_TAG;
  else
  {
    TagNum = 0;
    gem_mode &= ~GEM_TAG;
  }        
  strcpy(mandirect, fpath);
  getkeep(mandirect, 1, 1);
  i_read(mode, mandirect, gem_title, gem_doent, gem_comms, NULL);  
}


int
brd_man()
{
  char buf[64], xboard[20], fpath[256];
  boardheader * bp;
  boardheader * getbcache ();

  if (currstat == RMAIL)
  {
    move (1, 0); make_blist ();
    namecomplete ("¿é¤J¬Ýª©¦WºÙ (ª½±µEnter¶i¤J¨p¤H«H¥ó§¨)¡G", buf);
    if (*buf)
      strcpy (xboard, buf);
    else
      strcpy (xboard, "0");
    if (xboard && (bp = getbcache (xboard)))
    {
      sprintf(fpath, "man/boards/%s/.DIR", xboard);
      
      gem_menu(fpath, (HAS_PERM (PERM_ALLBOARD)|| is_BM (bp->BM)) ? 1 : 0, ANNOUNCE);
    }
    else if(HAS_PERM(PERM_MAILLIMIT) || HAS_PERM(PERM_BM)) // wildcat : ¤§«e§Ñ°O¥[ PERM ­­¨î°Õ ^^;
    {
      int mode0 = currutmp->mode;
      int stat0 = currstat;
      sethomeman (fpath, cuser.userid);
      gem_menu(fpath, 1, ANNOUNCE);
      currutmp->mode = mode0;
      currstat = stat0;
    }
  }
  else
  {
    sprintf(fpath, "man/boards/%s/.DIR", currboard);
    gem_menu(fpath, (currmode & MODE_BOARD) ? 1 : 0, ANNOUNCE);
  }
  return RC_FULL;
}



int
Announce()
{
  gem_menu("man/.DIR", HAS_PERM(PERM_SYSOP) ? 1 : 0, ANNOUNCE);
  return RC_FULL;
}


int
rpg_help ()
{
  gem_menu("game/rpg/help/.DIR", HAS_PERM(PERM_SYSOP) ? 1 : 0, ANNOUNCE);
  return RC_FULL;
}


int
Log ()
{
  gem_menu("man/log/.DIR", HAS_PERM(PERM_SYSOP) ? 1 : 0, ANNOUNCE);
  return RC_FULL;
}


int
XFile ()
{
  gem_menu("etc/xfile/.DIR", HAS_PERM(PERM_XFILE) ? 1 : 0, ANNOUNCE);
  return RC_FULL;  
}


int
HELP ()
{
  counter(BBSHOME"/log/counter/count-HELP","¨Ï¥Î HELP ¿ï³æ",0);
  gem_menu("etc/help/.DIR", HAS_PERM(PERM_XFILE) ? 1 : 0, ANNOUNCE);
  return RC_FULL;  
}


int
user_gem(char *uid)
{
  gem_level = 1;
  gopher_level = 0;
  sethomefile(mandirect, uid, "gem");
  if(!dashd(mandirect))
    mkdir(mandirect, 0755);
  getkeep(mandirect, 1, 1);
  if (HAS_PERM(PERM_SYSOP) || !strcmp(cuser.userid,uid))
    gem_mode |= GEM_PERM;
  else
    gem_mode &= ~GEM_PERM;
  i_read(ANNOUNCE, mandirect, gem_title, gem_doent, gem_comms, NULL);  
  return 0;
}


int
user_allpost(char *uid)
{
  gem_level = 1;
  gopher_level = 0;
  sethomefile(mandirect, uid, "allpost");
  if (!dashd(mandirect))
    mkdir(mandirect, 0755);
  if (HAS_PERM(PERM_SYSOP) || !strcmp(cuser.userid,uid))
  {
    getkeep(mandirect, 1, 1);
    gem_mode |= GEM_PERM;
    i_read(ANNOUNCE, mandirect, gem_title, gem_doent, gem_comms, NULL);  
  }
  else
    pressanykey("­­¨î¶i¤J");
  return 0;
}

void
my_gem()
{
  more(BBSHOME"/etc/my_gem");
  user_gem(cuser.userid);
}

void
my_allpost()
{
  more(BBSHOME"/etc/my_allpost");
  user_allpost(cuser.userid);
}

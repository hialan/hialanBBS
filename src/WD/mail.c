/*-------------------------------------------------------*/
/* mail.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : local/internet mail routines                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

extern int cmpfmode();

char currmaildir[32];
/* static */ char msg_cc[] = "[32m[¸s²Õ¦W³æ][0m\n";
static char listfile[] = "list.0";
static int mailkeep=0, mailsum=0;
static int mailsumlimit=0,mailmaxkeep=0;

int setforward() /* Ptt , modify by wildcat*/
{
  char buf[80],ip[50]="",yn[4];
// wildcat : bbs ©Ò¦³ domain name list
  char myhost[6][50] = {"wd.twbbs.org","wdbbs.net","wdbbs.org","wd.twbbs.org.tw","bbs.wdbbs.net","bbs.wdbbs.org"}; 
  FILE *fp;
  int i=0,allow=1;
   
  sethomepath(buf, cuser.userid);
  strcat(buf,"/.forward");
  if(fp = fopen(buf,"r"))
  {
    fscanf(fp,"%s",ip);
    fclose(fp);
  }
  getdata(b_lines-1,0,"½Ð¿é¤J«H½c¦Û°ÊÂà±Hªºemail¦a§}:",
    ip,41, DOECHO,ip);
// §PÂ_¬O§_Âà±Hµ¹¦Û¤v,©Î¬O±H¨ì¦P¤@¥x¾÷¾¹
  do
  {
    str_lower(myhost[i],myhost[i]);
    str_lower(ip,ip);
    if(strstr(ip,myhost[i])) allow=0;
  }while(!strlen(myhost[i++])); 
// ª½±µ¥Î not_addr ¨Ó§PÂ_¬O¤£¬O email §Y¥i
  if(allow && !not_addr(ip))
  {
    yn[0] = getans2(b_lines,0,"½T©w¶}±Ò¦Û°ÊÂà«H¥\\¯à? ",0,2,'y');
    if(yn[0] != 'n' &&  (fp=fopen(buf,"w")))
    {
      fprintf(fp,"%s",ip);
      fclose(fp);
      pressanykey("³]©w§¹¦¨!");
      refresh();
      return 0;
    }
  }
  unlink(buf);
  pressanykey("¨ú®ø¦Û°ÊÂà«H!");
  refresh();
  return 0;
}

#ifdef INTERNET_PRIVATE_EMAIL
int
m_internet()
{
  char receiver[60];

  getdata(20, 0, "¦¬«H¤H¡G", receiver, 60, DOECHO,0);
  if (strchr(receiver, '@') && !not_addr(receiver) &&
    getdata(21, 0, "¥D  ÃD¡G", save_title, TTLEN, DOECHO,0))
  {
    do_send(receiver, save_title);
  }
  else
  {
    move(22, 0);
    pressanykey("¦¬«H¤H©Î¥DÃD¤£¥¿½T, ½Ð­«·s¿ï¨ú«ü¥O");
  }
  return 0;
}

void
mail_forward(fhdr, direct, mode)
  fileheader *fhdr;
  char *direct;
  int mode;
{
  char buf[STRLEN];
  char *p;

  strncpy(buf, direct, sizeof(buf));
  if (p = strrchr(buf, '/'))
    *p = '\0';
  switch (doforward(buf, fhdr, mode))
  {
  case 0:
    outz(msg_fwd_ok);
    break;
  case -1:
    outz(msg_fwd_err1);
    break;
  case -2:
    outz(msg_fwd_err2);
  }
}
#endif

int
getmailnum(void)
{
  if (!HAVE_PERM(PERM_SYSOP) && !HAVE_PERM(PERM_MAILLIMIT))
  {
    if (HAS_PERM(PERM_BM))
       mailsumlimit = 300;
    else if (HAS_PERM(PERM_LOGINOK))
       mailsumlimit = 150;
    else
       mailsumlimit = 100;
    mailsumlimit += cuser.exmailbox;
    mailmaxkeep = MAXKEEPMAIL + cuser.exmailbox;
    
    sethomedir(currmaildir, cuser.userid);
    mailkeep = rec_num(currmaildir, sizeof(fileheader));
    mailsum = get_sum_records(currmaildir, sizeof(fileheader));
  }                
  return;
}

int
chkmailbox(void)
{
  if (!HAVE_PERM(PERM_SYSOP) && !HAVE_PERM(PERM_MAILLIMIT))
  {
    if (HAS_PERM(PERM_BM))
       mailsumlimit = 300;
    else if (HAS_PERM(PERM_LOGINOK))
       mailsumlimit = 150;
    else
       mailsumlimit = 100;
    mailsumlimit += cuser.exmailbox;
    mailmaxkeep = MAXKEEPMAIL + cuser.exmailbox;
    sethomedir(currmaildir, cuser.userid);
    if ((mailkeep = rec_num(currmaildir, sizeof(fileheader))) > mailmaxkeep)
    {
      move(b_lines, 0);
      clrtoeol();
      bell();
      prints("±z«O¦s«H¥ó¼Æ¥Ø %d ¶W¥X¤W­­ %d, ½Ð¾ã²z", mailkeep, mailmaxkeep);
      bell();
      refresh();
      igetch();
      return mailkeep;
    }
    if ((mailsum = get_sum_records(currmaildir, sizeof(fileheader))) >
                mailsumlimit)
    {
      move(b_lines, 0);
      clrtoeol();
      bell();
      prints("±z«O¦s«H¥ó®e¶q %d(k)¶W¥X¤W­­ %d(k), ½Ð¾ã²z", mailsum, mailsumlimit);
      bell();
      refresh();
      igetch();
      return mailkeep;
    }
  }
  return 0;
}


static void
do_hold_mail(fpath, receiver, holder)
  char *fpath;
  char *receiver;
  char *holder;
{
  char buf[80], title[128];

  fileheader mymail;

  sethomepath(buf, holder);
  stampfile(buf, &mymail);

  mymail.savemode = 'H';        /* hold-mail flag */
  mymail.filemode = FILE_READ;
  strcpy(mymail.owner, "[³Æ.§Ñ.¿ý]");
  if (receiver)
  {
    sprintf(title, "(%s) %s", receiver, save_title);
    strncpy(mymail.title, title, TTLEN);
  }
  else
    strcpy(mymail.title, save_title);

  sethomedir(title, holder);
  if (rec_add(title, &mymail, sizeof(mymail)) != -1)
  {
    unlink(buf);
    f_cp(fpath, buf, O_TRUNC);
  }
}


void
hold_mail(fpath, receiver)
  char *fpath;
  char *receiver;
{
  if(getans2(b_lines - 1,0,"¤w¶¶§Q±H¥X¡A¬O§_¦Û¦s©³½Z¡H",0,2,'n') == 'y')
    do_hold_mail(fpath, receiver, cuser.userid);
/*
  if (is_watched(cuser.userid))
    do_hold_mail(fpath, receiver, "SYSOP");
*/
}


int
do_send(userid, title)
  char *userid, *title;
{
  fileheader mhdr;
  char fpath[STRLEN];
  char receiver[IDLEN+1];
  char genbuf[200];
  char junbuf[40];  /*¾×«H¦W³æ*/

#ifdef INTERNET_PRIVATE_EMAIL
  int internet_mail;

  if (strchr(userid, '@'))
  {
    internet_mail = 1;
  }
  else
  {
    internet_mail = 0;
#endif

    if (!getuser(userid))
      return -1;
    if (!(xuser.userlevel & PERM_READMAIL))
      return -3;

    if (!title)
      getdata(2, 0, "¥DÃD¡G", save_title, TTLEN, DOECHO,0);
    curredit |= EDIT_MAIL;
    curredit &= ~EDIT_ITEM;
#ifdef INTERNET_PRIVATE_EMAIL
  }
#endif

  setutmpmode(SMAIL);

  fpath[0] = '\0';

#ifdef INTERNET_PRIVATE_EMAIL
  if (internet_mail)
  {
    int res, ch;

    if (vedit(fpath, NA) == -1)
    {
      unlink(fpath);
      clear();
      return -2;
    }
    clear();
    prints("«H¥ó§Y±N±Hµ¹ %s\n¼ÐÃD¬°¡G%s\n½T©w­n±H¥X¶Ü? (Y/N) [Y]",
      userid, title);
    ch = igetch();
    switch (ch)
    {
    case 'N':
    case 'n':
      outs("N\n«H¥ó¤w¨ú®ø");
      res = -2;
      break;

    default:
      outs("Y\n½Ðµy­Ô, «H¥ó¶Ç»¼¤¤...\n");
      res = bbs_sendmail(fpath, title, userid, NULL);
      hold_mail(fpath, userid);
    }
    unlink(fpath);
    return res;
  }
  else
  {
#endif
   strcpy(receiver, userid);
    if (vedit(fpath, YEA) == -1)
    {
      unlink(fpath);
      clear();
      return -2;
    }

/*  add by jungle for ¾×«H¦W³æ by hialan 20020527*/
     sethomefile(junbuf, userid, "spam-list");
//   sprintf(junbuf,BBSHOME"/home/%s/spam-list",userid);
//     if (belong(junbuf,cuser.userid) && !HAS_PERM(PERM_SYSOP))
     if (belong_list(junbuf, cuser.userid) && !HAS_PERM(PERM_SYSOP))
     {
       unlink(fpath);
       clear();
       return -3;
     }
/*  end  */

    clear();
   strcpy(userid, receiver);
    sethomepath(genbuf, userid);
    stampfile(genbuf, &mhdr);
    f_mv(fpath, genbuf);
    strcpy(mhdr.owner, cuser.userid);
    strncpy(mhdr.title, save_title, TTLEN);
    mhdr.savemode = '\0';
    sethomedir(fpath, userid);
    if (rec_add(fpath, &mhdr, sizeof(mhdr)) == -1)
      return -1;

    hold_mail(genbuf, userid);
    return 0;

#ifdef INTERNET_PRIVATE_EMAIL
  }
#endif
}


void
my_send(uident)
  char *uident;
{
  switch (do_send(uident, NULL))
  {
  case -1:
    outs(err_uid);
    break;
  case -2:
    outs(msg_cancel);
    break;
  case -3:
    prints("¨Ï¥ÎªÌ [%s] µLªk¦¬«H", uident);
    break;
  }
  pressanykey(NULL);
}


int
m_send()
{
  char uident[40];

  stand_title("¶®¨å¨º±a¨Óªº®ø®§");
  usercomplete(msg_uid, uident);
  if (uident[0])
    my_send(uident);
  return 0;
}


/* ------------------------------------------------------------ */
/* ¸s²Õ±H«H¡B¦^«H : multi_send, multi_reply                      */
/* ------------------------------------------------------------ */

extern struct word *toplev;

static void
multi_list(reciper)
  int *reciper;
{
  char uid[16];
  char genbuf[200];

  while (1)
  {
    stand_title("¸s²Õ±H«H¦W³æ");
    ShowNameList(3, 0, msg_cc);

    sprintf(genbuf, 
"(I)¤Þ¤J¦n¤Í (O)¤Þ¤J¤W½u³qª¾ (N)¤Þ¤J·s¤å³¹³qª¾ (0-9)¤Þ¤J¨ä¥L¯S§O¦W³æ\n"
"(A)¼W¥[     (D)§R°£         (M)½T»{±H«H¦W³æ  %s(Q)¨ú®ø ¡H[M]",
    HAS_PERM(PERM_SYSOP) ? " (B)©Ò¦³ªO¥D " : "");
    getdata(1, 0, genbuf, genbuf, 4, LCECHO,0);

    switch (genbuf[0])
    {
    case 'a':
      while (1)
      {
        move(2, 0);
        usercomplete("½Ð¿é¤J­n¼W¥[ªº¥N¸¹(¥u«ö ENTER µ²§ô·s¼W): ", uid);
        if (uid[0] == '\0')
          break;

        move(3, 0);

        if (!searchuser(uid))
          outs(err_uid);
        else if (!InNameList(uid))
        {
          AddNameList(uid);
          (*reciper)++;
        }
        ShowNameList(3, 0, msg_cc);
      }
      break;

    case 'd':
      while (*reciper)
      {
        move(1, 0);
        namecomplete("½Ð¿é¤J­n§R°£ªº¥N¸¹(¥u«ö ENTER µ²§ô§R°£): ", uid);
        if (uid[0] == '\0')
          break;
        if (RemoveNameList(uid))
        {
          (*reciper)--;
        }
        ShowNameList(3, 0, msg_cc);
      }
      break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      listfile[5] = genbuf[0];
      genbuf[0] = '1';

    case 'i':
/*[²{¶H] ¸s²Õ±H«H®ÉµLªk¤Þ¤J¦n¤Í¦W³æ*/
/*[»¡©ú] fn_override ¬°ÂÂª©ªº¦n¤Í¦W³æ*/
/*§ï¦¨ FN_PAL*/

      sethomefile(genbuf, cuser.userid, genbuf[0] == '1' ? listfile : FN_PAL);
      ToggleNameList(reciper, genbuf, msg_cc);
      break;

    case 'o':
      sethomefile(genbuf, cuser.userid, "alohaed");
      ToggleNameList(reciper, genbuf, msg_cc);
      break;

    case 'n':
      sethomefile(genbuf, cuser.userid, "postlist");
      ToggleNameList(reciper, genbuf, msg_cc);
      break;

    case 'm':
      *reciper = 0;
      return;
    break;

    case 'b':
      if (HAS_PERM(PERM_SYSOP)) {
         make_bmlist();
         *reciper = CountNameList();
         return;
      }
      break;

    default:
      return;
    }
  }
}



/* static */
/* shakalaca.000117: marked for list.c */
int
multi_send(title, inmail)
  char *title;
  int inmail;
{
  FILE *fp;
  struct word *p;
  fileheader mymail;
  char fpath[TTLEN], *ptr;
  int reciper, listing;
  char genbuf[256];
  char junbuf[40]; /*¾×«H¦W³æ*/

  if (inmail)
  {
    CreateNameList();
    listing = reciper = 0;

    if (*quote_file)
    {
      AddNameList(quote_user);
      reciper = 1;
      fp = fopen(quote_file, "r");
      while (fgets(genbuf, 256, fp))
      {
        if (strncmp(genbuf, "¡° ", 3))
        {
          if (listing)
            break;
        }  
        else
        {
          if (listing)
          {
            strtok(ptr = genbuf + 3, " \n\r");
            do
            {
              if (searchuser(ptr) && !InNameList(ptr) && strcmp(cuser.userid, ptr))
              {
                AddNameList(ptr);
                reciper++;
              }
            } while (ptr = (char *) strtok(NULL, " \n\r"));
          }
          else if (!strncmp(genbuf + 3, "[³q§i]", 6))
            listing = 1;
        }
      }
      ShowNameList(3, 0, msg_cc);
    }  

    multi_list(&reciper);
  }
  else
  {
    reciper = CountNameList();
  }
  
  move(1, 0);
  clrtobot();

  if (reciper)
  {
    setutmpmode(SMAIL);

    if (title)
    {
      do_reply_title(2, title);
    }
    else
    { 
      getdata(2, 0, "¥DÃD¡G", fpath, 64, DOECHO,0);
      sprintf(save_title, "[³q§i] %s", fpath);
    } 

    sethomefile(fpath, cuser.userid, fn_notes);

    if (fp = fopen(fpath, "w"))
    {
      fprintf(fp, "¡° [³q§i] ¦@ %d ¤H¦¬¥ó", reciper);
      listing = 80;

      for (p = toplev; p; p = p->next)
      {
        reciper = strlen(p->word) + 1;
        if (listing + reciper > 75)
        {
          listing = reciper;
          fprintf(fp, "\n¡°");
        }
        else
          listing += reciper;

        fprintf(fp, " %s", p->word);
      }
      memset(genbuf, '-', 75);
      genbuf[75] = '\0';
      fprintf(fp, "\n%s\n\n", genbuf);
      fclose(fp);
    }

    curredit |= EDIT_LIST;

    if (vedit(fpath, YEA) == -1)
    {
      unlink(fpath);
      curredit = 0;
      pressanykey(msg_cancel);
      return;
    }

    stand_title("±H«H¤¤...");
    refresh();

    listing = 80;

    for (p = toplev; p; p = p->next)
    {
      reciper = strlen(p->word) + 1;
      if (listing + reciper > 75)
      {
        listing = reciper;
        outc('\n');
      }
      else
      {
        listing += reciper;
        outc(' ');
      }
      outs(p->word);

/* add by jungle for ¾×«H¦W³æ by hialan 20020527*/
      sethomefile(junbuf, p->word, "spam-list");
//      sprintf(junbuf,BBSHOME"/home/%s/spam-list",p->word);
      if (belong_list(junbuf, cuser.userid) && !HAS_PERM(PERM_SYSOP))
//      if (belong(junbuf,cuser.userid) && !HAS_PERM(PERM_SYSOP))
      {
       clear();
       return;
      }
/*  end  */

      if (searchuser(p->word) && strcmp(STR_GUEST, p->word) )
        sethomepath(genbuf, p->word);
      else
        continue;
      stampfile(genbuf, &mymail);
      unlink(genbuf);
      f_cp(fpath, genbuf, O_TRUNC);

      strcpy(mymail.owner, cuser.userid);
      strcpy(mymail.title, save_title);
      mymail.savemode = 'M';    /* multi-send flag */
      sethomedir(genbuf, p->word);
      if (rec_add(genbuf, &mymail, sizeof(mymail)) == -1)
        outs(err_uid);
    }
    hold_mail(fpath, NULL);
    unlink(fpath);
    curredit = 0;
  }
  else
  {
    outs(msg_cancel);
  }
  pressanykey(NULL);
}


static int
multi_reply(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  if (fhdr->savemode != 'M')
    return mail_reply(ent, fhdr, direct);

  stand_title("¸s²Õ¦^«H");
  strcpy(quote_user, fhdr->owner);
  sethomefile(quote_file, cuser.userid, fhdr->filename);
  multi_send(fhdr->title, 1);
  return 0;
}


int
mail_list()
{
  stand_title("¸s²Õ§@·~");
  multi_send(NULL, 1);
  return 0;
}

extern int
bad_user_id(char userid[]);

int
mail_all()
{
   FILE *fp;
   fileheader mymail;
   char fpath[TTLEN];
   char genbuf[200];
   extern struct UCACHE *uidshm;
   int i, unum;
   char* userid;

   stand_title("µ¹©Ò¦³¨Ï¥ÎªÌªº¨t²Î³q§i");
   setutmpmode(SMAIL);
/*
   if(answer("¬O§_­nµ¹¿ú¡H (y/N)") == 'y')
   {
     while(money <= 0)
     {
       getdata(2, 0, "­nµ¹¦h¤Öª÷¹ô?",fpath, 4, DOECHO, 0);
       money = atoi(fpath);
       if(money <=0) return RC_FULL;
       mode = 1;
     }
   }
*/
   getdata(2, 0, "¥DÃD¡G", fpath, 64, DOECHO,0);
   sprintf(save_title, "[¨t²Î³q§i][1;32m %s[m", fpath);

   sethomefile(fpath, cuser.userid, fn_notes);

   if (fp = fopen(fpath, "w")) {
      fprintf(fp, "¡° [[1m¨t²Î³q§i[m] ³o¬O«Êµ¹©Ò¦³¨Ï¥ÎªÌªº«H\n");
      fprintf(fp, "---------------------------------------------------------------------------\n");
      fclose(fp);
    }

   *quote_file = 0;

   curredit |= EDIT_MAIL;
   curredit &= ~EDIT_ITEM;
   if (vedit(fpath, YEA) == -1) {
      curredit = 0;
      unlink(fpath);
      pressanykey(msg_cancel);
      return;
   }
   curredit = 0;

   setutmpmode(MAILALL);
   stand_title("±H«H¤¤...");

   sethomepath(genbuf, cuser.userid);
   stampfile(genbuf, &mymail);
   unlink(genbuf);
   f_cp(fpath, genbuf, O_TRUNC);
   unlink(fpath);
   strcpy(fpath, genbuf);

   strcpy(mymail.owner, cuser.userid);  /*¯¸ªø ID*/
   strcpy(mymail.title, save_title);
   mymail.savemode = 0;

   sethomedir(genbuf, cuser.userid);
   if (rec_add(genbuf, &mymail, sizeof(mymail)) == -1)
      outs(err_uid);

   for (unum = uidshm->number, i = 0; i < unum; i++) {

      if(bad_user_id(uidshm->userid[i])) continue; /* Ptt */

      userid = uidshm->userid[i];
      if (strcmp(userid, "guest") && strcmp(userid, "new") && strcmp(userid, cuser.userid)) {
         sethomepath(genbuf, userid);
         stampfile(genbuf, &mymail);
         unlink(genbuf);
         f_cp(fpath, genbuf, O_TRUNC);

         strcpy(mymail.owner, cuser.userid);
         strcpy(mymail.title, save_title);
         mymail.savemode = 0;
         sethomedir(genbuf, userid);
         if (rec_add(genbuf, &mymail, sizeof(mymail)) == -1)
            outs(err_uid);
         sprintf(genbuf, "%*s %5d / %5d", IDLEN + 1, userid, i + 1, unum);
/*
         if(mode)
           inugold(userid,money);
*/
         outmsg(genbuf);
         refresh();
      }
   }
   return;
}


mail_mbox()
{
   char cmd[100];
   fileheader fhdr;

   sprintf(cmd, "/tmp/%s.tgz", cuser.userid);
   sprintf(fhdr.title, "%s ¨p¤H¸ê®Æ", cuser.userid);
   doforward(cmd, &fhdr, 'Z');
}

static int
m_forward(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char uid[STRLEN];

  stand_title("Âà¹F«H¥ó");
  usercomplete(msg_uid, uid);
  if (uid[0] == '\0')
  {
    return RC_FULL;
  }

  strcpy(quote_user, fhdr->owner);
  sethomefile(quote_file, cuser.userid, fhdr->filename);
  sprintf(save_title, "%.64s (fwd)", fhdr->title);
  move(1, 0);
  clrtobot();
  prints("Âà«Hµ¹: %s\n¼Ð  ÃD: %s\n", uid, save_title);

  switch (do_send(uid, save_title))
  {
  case -1:
    outs(err_uid);
    break;
  case -2:
    outs(msg_cancel);
    break;
  case -3:
    prints("¨Ï¥ÎªÌ [%s] µLªk¦¬«H", uid);
    break;
  }
  pressanykey(NULL);
  return RC_FULL;
}


/* JhLin: At most 128 mail */

int delmsgs[128];
int delcnt;
int mrd;


static int
read_new_mail(fptr)
  fileheader *fptr;
{
  static int idc;
  char done = NA, delete_it;
  char fname[256];
  char genbuf[4];

  if (fptr == NULL)
  {
    delcnt = 0;
    idc = 0;
    return 0;
  }
  idc++;
  if (fptr->filemode)
    return 0;
  clear();
  move(10, 0);
  prints("±z­nÅª¨Ó¦Û[%s]ªº°T®§(%s)¶Ü¡H", fptr->owner, fptr->title);
  genbuf[0] = getans2(11, 0,"½Ð±z½T©w? ",0,3,'y');
  if (genbuf[0] == 'q')
    return QUIT;
  if (genbuf[0] == 'n')
    return 0;

  sethomefile(fname, cuser.userid, fptr->filename);
  fptr->filemode |= FILE_READ;
  if (substitute_record(currmaildir, fptr, sizeof(*fptr), idc))
    return -1;

  mrd = 1;
  delete_it = NA;
  while (!done)
  {
    int more_result = more(fname, YEA);
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
       return 0;
    case 7:
      mail_reply(idc, fptr, currmaildir);
      return RC_FULL;
    case 8:
      multi_reply(idc, fptr, currmaildir);
      return RC_FULL;
    }
    readfoot(2);
    refresh();

    switch (igetkey())
    {
    case 'r':
    case 'R':
      mail_reply(idc, fptr, currmaildir);
      break;
    case 'x':
      m_forward(idc, fptr, currmaildir);
      break;
    case 'y':
      multi_reply(idc, fptr, currmaildir);
      break;
    case 'd':
    case 'D':
      delete_it = YEA;
    default:
      done = YEA;
    }
  }
  if (delete_it)
  {
    clear();
    prints("§R°£«H¥ó¡m%s¡n?", fptr->title);
    genbuf[0] = getans2(1, 0, msg_sure, 0, 2, 'n');
    if (genbuf[0] == 'y')
    {
      unlink(fname);
      delmsgs[delcnt++] = idc;
    }
  }
  clear();
  return 0;
}


int
m_new()
{
  clear();
  mrd = 0;
  setutmpmode(RMAIL);
  read_new_mail(NULL);
  clear();
  curredit |= EDIT_MAIL;
  curredit &= ~EDIT_ITEM;
  if (rec_apply(currmaildir, read_new_mail, sizeof(fileheader)) == -1)
  {
    pressanykey("¨S¦³·s«H¥ó¤F");
    return -1;
  }
  curredit = 0;
  if (delcnt)
  {
    while (delcnt--)
      rec_del(currmaildir, sizeof(fileheader), delmsgs[delcnt]);
  }
  outs(mrd ? "«H¤w¾\\²¦" : "¨S¦³·s«H¥ó¤F");
  pressanykey(NULL);
  return -1;
}


static void mailtitle()
{
  char buf[100]="";

  sprintf(tmpbuf,"%s [½u¤W %d ¤H]",BOARDNAME,count_ulist());
  showtitle("\0¶l¥ó¿ï³æ", tmpbuf);
  outs("  y)¦^«H  x)Âà¹Fµ¹§O¤H  ^X)Âà¿ý¨ì¬ÝªO  m)«O¯d  d)§R°£  D)§R°£³sÄò¦h½g  F)Âà±H\n");
  prints("%s  ½s¸¹¦^ ¤é ´Á §@ ªÌ        «H  ¥ó  ¼Ð  ÃD", COLOR3);

  getmailnum();
  if(mailsumlimit)
    sprintf(buf,"(®e¶q:%d/%dk %d/%d½g)",mailsum, mailsumlimit ,mailkeep,mailmaxkeep);
    
  prints("\033[34m%34.34s   [m",buf);
}

static int
mail_del(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char genbuf[200];

  if (fhdr->filemode & FILE_MARKED)
    return RC_NONE;

  genbuf[0] = getans2(1, 0, msg_del_ny, 0, 2, 'n');
  if (genbuf[0] == 'y')
  {
    extern int cmpfilename();

    strcpy(currfile, fhdr->filename);
    if (!delete_file(direct, sizeof(*fhdr), ent, cmpfilename))
    {
      setdirpath(genbuf, direct, fhdr->filename);
      unlink(genbuf);
      if( currmode & MODE_SELECT ){
         int now;
         sethomedir(genbuf,cuser.userid);
         now=getindex(genbuf,fhdr->filename,sizeof(fileheader));
         delete_file (genbuf, sizeof(fileheader),now,cmpfilename);
      }
      return RC_CHDIR;
    }
  }
  return RC_FULL;
}


static int
mail_read(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char buf[64];
  int more_result;

  clear();
  setdirpath(buf, direct, fhdr->filename);
  strncpy(currtitle, str_ttl(fhdr->title), 40);
  
  more_result =  more(buf, YEA);
  if (more_result != -1) 
  {  
     fhdr->filemode |= FILE_READ;
     if (!strncmp("[·s]", fhdr->title, 4) && !(fhdr->filemode & FILE_MARKED))
        fhdr->filemode |= FILE_TAGED;
     if ( currmode & MODE_SELECT )
     {
        int now;
        now = getindex(currmaildir, fhdr->filename, sizeof(*fhdr));
        substitute_record(currmaildir, fhdr, sizeof(*fhdr), now);
     }
     else
       substitute_record(currmaildir, fhdr, sizeof(*fhdr), ent);
  }
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
       return RC_FULL;
    case 7:
      mail_reply(ent, fhdr, direct);
      return RC_FULL;
    case 8:
      multi_reply(ent, fhdr, direct);
      return RC_FULL;
    default:
      return RC_FULL;
  }
}

/* ---------------------------------------------- */
/* in boards/mail ¦^«Hµ¹­ì§@ªÌ¡AÂà«H¯¸¥ç¥i        */
/* ---------------------------------------------- */

int
mail_reply(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char uid[STRLEN];
  char *t;
  FILE *fp;
  char genbuf[256];
  int enttmp=ent;

  stand_title("¦^  «H");

  /* §PÂ_¬O boards ©Î mail */

  if (curredit & EDIT_MAIL)
    sethomefile(quote_file, cuser.userid, fhdr->filename);
  else
    setbfile(quote_file, currboard, fhdr->filename);

  /* find the author */

  strcpy(quote_user, fhdr->owner);

  if (strchr(quote_user, '.'))
  {
    genbuf[0] = '\0';
    if (fp = fopen(quote_file, "r"))
    {
      fgets(genbuf, sizeof(genbuf), fp);
      fclose(fp);
    }

    t = strtok(genbuf, str_space);
    if (!strcmp(t, str_author1) || !strcmp(t, str_author2))
    {
      strcpy(uid, strtok(NULL, str_space));
    }
    else
    {
      pressanykey("¿ù»~: §ä¤£¨ì§@ªÌ¡C");
      return RC_FULL;
    }
  }
  else
    strcpy(uid, quote_user);

  /* make the title */

  do_reply_title(3, fhdr->title);
  prints("\n¦¬«H¤H: %s\n¼Ð  ÃD: %s\n", uid, save_title);

  /* edit, then send the mail */

  ent = curredit;
  switch (do_send(uid, save_title))
  {
  case -1:
    outs(err_uid);
    break;
  case -2:
    outs(msg_cancel);
    break;
  case -3:
    prints("¨Ï¥ÎªÌ [%s] µLªk¦¬«H", uid);
    break;
  default:
    fhdr->filemode |= FILE_REPLYOK;
    substitute_record(currmaildir, fhdr, sizeof(*fhdr), enttmp);
    break;
  }
  curredit = ent;
  pressanykey(NULL);
  return RC_FULL;
}

static int
mail_cross_post(ent, fhdr, direct)
  int ent;
  fileheader *fhdr;
  char *direct;
{
  char xboard[20], fname[80], xfpath[80], xtitle[80]; 
  fileheader xfile;
  FILE *xptr;
  int author = 0;
  char genbuf[200];

  make_blist();
  move(2, 0);
  clrtoeol();
  move(3, 0);
  clrtoeol();
  move(1, 0);
  namecomplete("Âà¿ý¥»¤å³¹©ó¬ÝªO¡G", xboard);
  if (*xboard == '\0' || !haspostperm(xboard))
    return RC_FULL;

  ent = 1;
  if (HAS_PERM(PERM_SYSOP) || !strcmp(fhdr->owner, cuser.userid))
  {
    char *choose[2]={"11.­ì¤åÂà¸ü","22.ÂÂÂà¿ý®æ¦¡"};

    if (getans2(2, 0, "",choose,2,'1') != '2')
    {
      ent = 0;
      if (getans2(2,0,"«O¯d­ì§@ªÌ¦WºÙ¶Ü?",0,2,'y') != 'n') author = 1;
    }
  }

  if (ent)
    sprintf(xtitle, "[Âà¿ý]%.66s", fhdr->title);
  else
    strcpy(xtitle, fhdr->title);

  sprintf(genbuf, "±Ä¥Î­ì¼ÐÃD¡m%.60s¡n¶Ü? ", xtitle);
  if (getans2(2, 0, genbuf,0,2,'y') == 'n')
  {
    if (getdata(2, 0, "¼ÐÃD¡G", genbuf, TTLEN, DOECHO,0))
      strcpy(xtitle, genbuf);
  }

  {
    char *choose_save[3]={"sS.¦sÀÉ","lL.¯¸¤º","qQ.¨ú®ø"};
    genbuf[0] = getans2(2, 0, "", choose_save, 3, 's');
  }
  if (genbuf[0] == 'l' || genbuf[0] == 's')
  {
    int currmode0 = currmode;

    currmode = 0;
    setbpath(xfpath, xboard);
    stampfile(xfpath, &xfile);
    if (author)
      strcpy(xfile.owner, fhdr->owner);
    else
      strcpy(xfile.owner, cuser.userid);
    strcpy(xfile.title, xtitle);
    if (genbuf[0] == 'l')
    {
      xfile.savemode = 'L';
      xfile.filemode = FILE_LOCAL;
    }
    else
      xfile.savemode = 'S';

    sethomefile(fname, cuser.userid, fhdr->filename);
    if (ent)
    {
      xptr = fopen(xfpath, "w");

      strcpy(save_title, xfile.title);
      strcpy(xfpath, currboard);
      strcpy(currboard, xboard);
      write_header(xptr);
      strcpy(currboard, xfpath);

      fprintf(xptr, "¡° [¥»¤åÂà¿ý¦Û %s «H½c]\n\n", cuser.userid);

      b_suckinfile(xptr, fname);
      addsignature(xptr);
      fclose(xptr);
    }
    else
    {
      unlink(xfpath);
      f_cp(fname, xfpath, O_TRUNC);
    }

    setbdir(fname, xboard);
    rec_add(fname, (char *) &xfile, sizeof(xfile));
    if (!xfile.filemode)
      outgo_post(&xfile, xboard);
    update_data();
    cuser.numposts++;
    substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
    pressanykey("¤å³¹Âà¿ý§¹¦¨");
    currmode = currmode0;
  }
  return RC_FULL;
}



mail_save(int ent, fileheader* fhdr, char* direct)
{
   char fpath[256];
   char title[TTLEN+1];

   if (HAS_PERM(PERM_MAILLIMIT)) 
   {
      sethomefile(fpath, cuser.userid, fhdr->filename);
      strcpy(title, "¡º ");
      strncpy(title+3, fhdr->title, TTLEN-3);
      title[TTLEN] = '\0';
      a_copyitem(fpath, title, fhdr->owner);
      sethomeman(fpath, cuser.userid);
      a_menu(cuser.userid, fpath, belong("etc/sysop", cuser.userid) ? 2 : 1);
      return RC_FULL;
   }
   return RC_NONE;
}

extern int cite();
extern int man();
extern int add_tag();
extern int del_tag();
extern int gem_tag();
extern int edit_post();
extern int mark();
extern int del_range();
extern int edit_title();
extern int post_mail();
extern int post_query();

static struct one_key mail_comms[] = {
  'z', man,		 0, "¶i¤J¨p¤H«H¥ó§¨",0,
  'c', cite,		 0, "±N¦¹«H¥ó¶i¤J¨p¤H«H¥ó§¨",0,
  'C', gem_tag,		 0, "gem_tag",0,
  's', mail_save,	 0, "mail_save",0,
  'd', mail_del,	 0, "±þ±¼¦¹«H",0,
  'D', del_range,	 0, "±þ±¼«ü©w½d³òªº«H",0,
  'r', mail_read,	 0, "Åª«H",0,
  'R', mail_reply,	 0, "¦^ÂÐ«H¥ó",0,
  'E', edit_post,	 0, "½s¿è¤å¥ó",0,
  'm', mark,		 0, "±N«H¥ó¼Ð°O, ¥H¨¾«H¥ó³Q²M°£",0,
  't', add_tag,		 0, "¼Ð°O¸Ó«H¥ó",0,
  'T', edit_title,	 0, "½s¿è¼ÐÃD",0,
  'x', m_forward,	 0, "Âà¹F«H¥ó",0,
  'X', mail_cross_post,	 0, "Âà¿ý¤å³¹¨ì¨ä¥L¬ÝªO",0,
  Ctrl('D'), del_tag,	 0, "§R°£¤w¼Ð°O«H¥ó",0,
  'y', multi_reply,  	 0, "¸s²Õ¦^«H",0,
  'F', post_mail,	 0, "±N«H¶Ç°e¦^±zªº¹q¤l«H½c",0,
  Ctrl('Q'), post_query, 0, "q ¸Ó«H¤H",0,
  '\0', NULL, 0, NULL,0};


int
m_read()
{
  if (rec_num(currmaildir, sizeof(fileheader)))
  {
    curredit = EDIT_MAIL;
    curredit &= ~EDIT_ITEM;
    i_read(RMAIL, currmaildir, mailtitle, doent, mail_comms,NULL);
    currfmode = FILE_TAGED;
    if (search_rec(currmaildir, cmpfmode))
      del_tag(0, 0, currmaildir);
    curredit = 0;
    return 0;
  }
  else
  {
    pressanykey("±z¨S¦³¨Ó«H");
    return 0;
  }
}


#ifdef INTERNET_EMAIL
#include <pwd.h>

int
bbs_sendmail(fpath, title, receiver, key)
  char *fpath, *title, *receiver, *key;
{
  static int configured = 0;
  static char myhostname[STRLEN];
  static char myusername[20];
  struct hostent *hbuf;
  struct passwd *pbuf;
  char *ptr, genbuf[200];

  FILE *fin, *fout;

  if (ptr = strchr(receiver, ';'))
    *ptr = '\0';

  if ((ptr = strstr(receiver, str_mail_address)) || !strchr(receiver, '@'))
  {
    fileheader mymail;
    char hacker[20];
    int len;

    if (strchr(receiver, '@'))
    {
      len = ptr - receiver;
      memcpy(hacker, receiver, len);
      hacker[len] = '\0';
    } 
    else
      strcpy(hacker, receiver);

    if (!searchuser(hacker))
      return -2;

    sethomepath(genbuf, hacker);
    stampfile(genbuf, &mymail);
    if (!strcmp(hacker, cuser.userid))
    {
      strcpy(mymail.owner, BOARDNAME);
      mymail.filemode = FILE_READ;
    } else
      strcpy(mymail.owner, cuser.userid);
    strncpy(mymail.title, title, TTLEN);
    f_rm(genbuf);
    f_cp(fpath, genbuf, O_TRUNC);
    sethomefile(genbuf, hacker, ".DIR");
    return rec_add(genbuf, &mymail, sizeof(mymail));
  }
  /* setup the hostname and username */
  if (!configured)
  {
    /* get host name */
    gethostname(myhostname, STRLEN);
    hbuf = gethostbyname(myhostname);
    if (hbuf)
      strncpy(myhostname, hbuf->h_name, STRLEN);

    /* get bbs uident */
    pbuf = getpwuid(getuid());
    if (pbuf)
      strncpy(myusername, pbuf->pw_name, 20);
    if (hbuf && pbuf)
      configured = 1;
    else
      return -1;
  }
  /* Running the sendmail */

#ifdef  INTERNET_PRIVATE_EMAIL
  if (fpath == NULL)
  {
    sprintf(genbuf, "/usr/sbin/sendmail %s > /dev/null", receiver);
    fin = fopen("etc/confirm", "r");
  } else
  {
    sprintf(genbuf, "/usr/sbin/sendmail -f %s%s %s > /dev/null"
	    ,cuser.userid, str_mail_address, receiver);
    fin = fopen(fpath, "r");
  }
  fout = popen(genbuf, "w");
  if (fin == NULL || fout == NULL)
    return -1;

  if (fpath)
    fprintf(fout, "Reply-To: %s%s\nFrom: %s%s\n",
	    cuser.userid, str_mail_address, cuser.userid, str_mail_address);
#else
  sprintf(genbuf, "/usr/sbin/sendmail %s > /dev/null", receiver);
  fout = popen(genbuf, "w");
  fin = fopen(fpath ? fpath : "etc/confirm", "r");
  if (fin == NULL || fout == NULL)
    return -1;

  if (fpath)
    fprintf(fout, "From: %s@%s (%s)\n", myusername, myhostname, BBSNAME);
#endif

  fprintf(fout, "To: %s\nSubject: %s\n", receiver, title);
  fprintf(fout, "X-Disclaimer: " BOARDNAME "¹ï¥»«H¤º®e®¤¤£­t³d¡C\n\n");

  while (fgets(genbuf, sizeof(genbuf), fin))
  {
    if (genbuf[0] == '.' && genbuf[1] == '\n')
      fputs(". \n", fout);
    else
    {
#ifdef REG_MAGICKEY
      char *po;
      while (po = strstr(genbuf, "<Magic>"))
      {
	char buf[128];

	po[0] = 0;
	sprintf(buf, "%s%s%s", genbuf, key, po + 7);
	strcpy(genbuf, buf);
      }
#endif
      fputs(genbuf, fout);
    }
  }
  fclose(fin);
  fprintf(fout, ".\n");
  pclose(fout);
  return 0;
}


int
doforward(direct, fh, mode)
  char *direct;
  fileheader *fh;
  int mode;                     /* ¬O§_ uuencode */
{
  static char address[60];
  char fname[PATHLEN];
  int return_no;
  char genbuf[200];
  
  /* itoc.001203: ¥[±Kªº¤å³¹¤£¯àÂà±H */
  if ((currstat == READING) && (fh->filemode & FILE_REFUSE) &&
     !(currmode & MODE_BOARD))
    return RC_NONE;

  if (!address[0])
    strcpy(address, cuser.email);

  if (address[0])
  {
    sprintf(genbuf, "½T©wÂà±Hµ¹ [%s] ¶Ü¡H ", address);
    fname[0] = getans2(b_lines - 1, 0,genbuf,0,3,'y');
    if (fname[0] == 'q')
    {
      outz("¨ú®øÂà±H");
      return 1;
    }
    if (fname[0] == 'n')
      address[0] = '\0';
  }

  if (!address[0])
  {
    getdata(b_lines - 1, 0, "½Ð¿é¤JÂà±H¦a§}¡G", fname, 60, DOECHO,0);
    if (fname[0])
    {
      if (strchr(fname, '.'))
        strcpy(address, fname);
      else
        sprintf(address, "%s.bbs@%s", fname, MYHOSTNAME);
    }
    else
    {
      outz("¨ú®øÂà±H");
      return 1;
    }
  }

  if (not_addr(address))
    return -2;

  sprintf(fname, "¥¿Âà±Hµ¹ %s, ½Ðµy­Ô...", address);
  outz(fname);

  if (mode == 'Z') {
     FILE* fp;
     int address_ok = valid_ident(address);

     if (fp = fopen("mbox_sent", "a")) {
        time_t now = time(0);

        fprintf(fp, "%c%-12s %s => %s\n",
           address_ok ? ' ' : '-', cuser.userid, Cdatelite(&now), address);
        fclose(fp);
     }
     if (!address_ok) {
         sprintf(fname, "µL®Äªº¤u§@¯¸¦ì§} %s", address);
         outz(fname);
         return -2;
      }
     sprintf(fname, "cd home; tar cfz - %s | uuencode %s.tgz > /tmp/%s.tgz",
        cuser.userid, cuser.userid, cuser.userid);
     system(fname);
     strcpy(fname, direct);
  }
  else if (mode == 'U')
  {
    char tmp_buf[128];

    sprintf(fname, "/tmp/bbs.uu%05d", currpid);
    sprintf(tmp_buf, "/usr/bin/uuencode %s/%s uu.%05d > %s",
      direct, fh->filename, currpid, fname);
    system(tmp_buf);
    sleep(1);
  }
  else
    sprintf(fname, "%s/%s", direct, fh->filename);

  return_no = bbs_sendmail(fname, fh->title, address, NULL);

    if (mode != 'F')

  return (return_no);
}

#endif


int
chkmail(rechk)
  int rechk;
{
  static time_t lasttime = 0;
  static int ismail = 0;
  struct stat st;
  int fd;
  register numfiles;
  unsigned char ch;

  if (!HAS_PERM(PERM_BASIC))
    return 0;

  if (stat(currmaildir, &st) < 0)
    return (ismail = 0);

  if ((lasttime >= st.st_mtime) && !rechk)
    return ismail;

  lasttime = st.st_mtime;
  numfiles = st.st_size / sizeof(fileheader);
  if (numfiles <= 0)
    return (ismail = 0);

  /* ------------------------------------------------ */
  /* ¬Ý¬Ý¦³¨S¦³«H¥óÁÙ¨SÅª¹L¡H±qÀÉ§À¦^ÀYÀË¬d¡A®Ä²v¸û°ª */
  /* ------------------------------------------------ */

  if ((fd = open(currmaildir, O_RDONLY)) > 0)
  {
    lseek(fd, (off_t)(st.st_size - 1), SEEK_SET);
    while (numfiles--)
    {
      read(fd, &ch, 1);
      if (!(ch & FILE_READ))
      {
        close(fd);
        return (ismail = 1);
      }
      lseek(fd, -(off_t)(sizeof(fileheader) + 1), SEEK_CUR);
    }
    close(fd);
  }
  return (ismail = 0);
}

#ifdef  REG_MAGICKEY
void mail_justify(userec muser)
{
  fileheader mhdr;
  char title[128], buf1[80];

  sethomepath(buf1, muser.userid);
  stampfile(buf1, &mhdr);
  unlink(buf1);
  strcpy(mhdr.owner, "¯¸ªø");
  strncpy(mhdr.title, "[EMail»{ÃÒ]", TTLEN);
  mhdr.savemode = 0;
  mhdr.filemode = 0;

  if (valid_ident(muser.email) && !not_addr(muser.email))
  {
    FILE *fp;
    char title[80], *ptr, ch, MagicKey[9];
    ushort checksum;            /* 16-bit is enough */
                
    checksum = getuser(muser.userid);
    ptr = muser.email;
    while (ch = *ptr++)
    {
      if (ch <= ' ')
        break;
      if (ch >= 'A' && ch <= 'Z')
        ch |= 0x20;
      checksum = (checksum << 1) ^ ch;
    }
    /* shakalaca.990725: ¨­¥÷»{ÃÒ¥Îªº MagicKey */
    sprintf(title, "%d", checksum + time(0));
    strncpy(MagicKey, title, 8);
    MagicKey[8] = '\0';
    sethomefile(title, cuser.userid, fn_magickey);
    if((fp = fopen(title, "w")) == NULL)
    {
      pressanykey("¿ù»~!!¶}±ÒÀÉ®×¥¢±Ñ, ½Ð³qª¾¯¸ªø!!");
      return;
    }
    fprintf(fp, "%s\n", MagicKey);
    fclose(fp);

    sprintf(title, "%s To %s: [­«­n ! ½Ð¾\\Åª]", TAG_VALID, muser.userid);

    if (bbs_sendmail(NULL, title, muser.email, MagicKey) < 0)
      f_cp("etc/bademail", buf1, O_TRUNC);
    else
      f_cp("etc/justify", buf1, O_TRUNC);
  }
  else
    f_cp("etc/bademail", buf1, O_TRUNC);

  sethomedir(title, muser.userid);
  rec_add(title, &mhdr, sizeof(mhdr));
}
#endif                          /* REG_MAGICKEY */

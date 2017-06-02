/*-------------------------------------------------------*/
/* do_talk.c             ( AT-bbs/WD_hialan )            */
/*-------------------------------------------------------*/
/* target : Âù¤H²á¤Ñ                                     */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include<stdarg.h>
#include "bbs.h"


static FILE* flog;
extern char save_page_requestor[40];
extern char *getuserid();

struct talk_win
{
  int curcol, curln;
  int sline, eline;
};

/*---------------------------------------------*/
/* do_talk				       */
/*---------------------------------------------*/
static void do_talk_nextline(struct talk_win *twin)
{
   twin->curcol = 0;
   if (twin->curln < twin->eline)
      ++(twin->curln);
   else
      region_scroll_up(twin->sline, twin->eline);
   move(twin->curln, twin->curcol);
}


static void do_talk_char(struct talk_win *twin, int ch)
{
  extern int dumb_term;
  extern screenline* big_picture;
  screenline* line;
  int i;
  char ch0, buf[81];

  if (isprint2(ch))
  {
    ch0 = big_picture[twin->curln].data[twin->curcol];
    if (big_picture[twin->curln].len < 79)
       move(twin->curln, twin->curcol);
    else
       do_talk_nextline(twin);
    outc(ch);
    ++(twin->curcol);
    line =  big_picture + twin->curln;
    if (twin->curcol < line->len) {      /* insert */
       ++(line->len);
       memcpy(buf, line->data + twin->curcol, 80);
       save_cursor();
       do_move(twin->curcol, twin->curln);
       ochar(line->data[twin->curcol] = ch0);
       for (i = twin->curcol + 1; i < line->len; i++)
          ochar(line->data[i] = buf[i - twin->curcol - 1]);
       restore_cursor();
    }
    line->data[line->len] = 0;
    return;
  }

  switch (ch)
  {
  case Ctrl('H'):
  case '\177':
    if (twin->curcol == 0)
    {
      return;
    }
    line =  big_picture + twin->curln;
    --(twin->curcol);
    if (twin->curcol < line->len) {
       --(line->len);
       save_cursor();
       do_move(twin->curcol, twin->curln);
       for (i = twin->curcol; i < line->len; i++)
          ochar(line->data[i] = line->data[i + 1]);
       line->data[i] = 0;
       ochar(' ');
       restore_cursor();
    }
    move(twin->curln, twin->curcol);
    return;

  case Ctrl('D'):
     line =  big_picture + twin->curln;
     if (twin->curcol < line->len) {
        --(line->len);
        save_cursor();
        do_move(twin->curcol, twin->curln);
        for (i = twin->curcol; i < line->len; i++)
           ochar(line->data[i] = line->data[i + 1]);
        line->data[i] = 0;
        ochar(' ');
        restore_cursor();
     }
     return;
  case Ctrl('G'):
    bell();
    return;
  case Ctrl('B'):
     if (twin->curcol > 0) {
        --(twin->curcol);
        move(twin->curln, twin->curcol);
     }
     return;
  case Ctrl('F'):
     if (twin->curcol < 79) {
        ++(twin->curcol);
        move(twin->curln, twin->curcol);
     }
     return;
  case Ctrl('A'):
     twin->curcol = 0;
     move(twin->curln, twin->curcol);
     return;
  case Ctrl('K'):
     clrtoeol();
     return;
  case Ctrl('Y'):
     twin->curcol = 0;
     move(twin->curln, twin->curcol);
     clrtoeol();
     return;
  case Ctrl('E'):
     twin->curcol = big_picture[twin->curln].len;
     move(twin->curln, twin->curcol);
     return;
  case Ctrl('M'):
  case Ctrl('J'):
     line =  big_picture + twin->curln;
     strncpy(buf, line->data, line->len);
     buf[line->len] = 0;
     if (dumb_term)
       outc('\n');
     do_talk_nextline(twin);
     break;
  case Ctrl('P'):
     line =  big_picture + twin->curln;
     strncpy(buf, line->data, line->len);
     buf[line->len] = 0;
     if (twin->curln > twin->sline) {
        --(twin->curln);
        move(twin->curln, twin->curcol);
     }
     break;
  case Ctrl('N'):
     line =  big_picture + twin->curln;
     strncpy(buf, line->data, line->len);
     buf[line->len] = 0;
     if (twin->curln < twin->eline) {
        ++(twin->curln);
        move(twin->curln, twin->curcol);
     }
     break;
  }
  str_trim(buf);
  if (*buf)
     fprintf(flog, "%s%s: %s%s\n",
        (twin->eline == b_lines - 1) ? "[1;33m" : "",
        (twin->eline == b_lines - 1) ?
        getuserid(currutmp->destuid) : cuser.userid, buf,
        (ch == Ctrl('P')) ? "[37;45m(Up)[m" : "[m");
}


static void do_talk(int fd)
{
   struct talk_win mywin, itswin;
   time_t talkstart;
   usint myword = 0,itword = 0;
   char mid_line[128], data[200];
   int i, ch, datac;
   int im_leaving = 0;
   struct tm *ptime;
   time_t now;
   char genbuf[200], fpath[100];

   time(&now);
   ptime = localtime(&now);

   sethomepath(fpath, cuser.userid);
   strcpy(fpath, tempnam(fpath, "talk_"));
   flog = fopen(fpath, "w");

   setutmpmode(TALK);

   ch = 58 - strlen(save_page_requestor);
   sprintf(genbuf, "%s¡i%s", cuser.userid, cuser.username);
   i = ch - strlen(genbuf);
   if (i >= 0)
      i = (i >> 1) + 1;
   else
   {
     genbuf[ch] = '\0';
     i = 1;
   }
   memset(data, ' ', i);
   data[i] = '\0';

   sprintf(mid_line, COLOR2"  ¯«·µ¹ï¸Ü  [1m"COLOR1"%s%s¡j [37m»P  "COLOR1"%s%s[m",
     data, genbuf, save_page_requestor,  data);

   memset(&mywin, 0, sizeof(mywin));
   memset(&itswin, 0, sizeof(itswin));

   i = b_lines >> 1;
   mywin.eline = i - 1;
   itswin.curln = itswin.sline = i + 1;
   itswin.eline = b_lines - 1;

   clear();
   move(i, 0);
   outs(mid_line);
   move(0, 0);

   add_io(fd, 0);
   talkstart = time(0);

   while (1)
   {
     ch = igetkey();

     if (ch == I_OTHERDATA)
     {
       datac = recv(fd, data, sizeof(data), 0);
       if (datac <= 0)
         break;
       itword++;
       for (i = 0; i < datac; i++)
         do_talk_char(&itswin, data[i]);
     }
     else
     {
       if (ch == Ctrl('C'))
       {
         if (im_leaving)
           break;
         move(b_lines, 0);
         clrtoeol();
         outs("¦A«ö¤@¦¸ Ctrl-C ´N¥¿¦¡¤¤¤î½Í¸ÜÅo¡I");
         im_leaving = 1;
         continue;
       }
       if (im_leaving)
       {
         move(b_lines, 0);
         clrtoeol();
         im_leaving = 0;
       }
       switch(ch)
       {
        case KEY_LEFT:
          ch = Ctrl('B');
          break;
        case KEY_RIGHT:
          ch = Ctrl('F');
          break;
        case KEY_UP:
          ch = Ctrl('P');
          break;
        case KEY_DOWN:
          ch = Ctrl('N');
          break;
       }
       myword++;
       data[0] = (char) ch;
       if (send(fd, data, 1, 0) != 1)
         break;
       do_talk_char(&mywin, *data);
     }
   }

   add_io(0, 0);
   close(fd);

   if (flog) 
   {
#if 0
     extern screenline *big_picture;
     extern uschar scr_lns;
#endif
     char buf[128];

     time(&now);
#if 0
     fprintf(flog, "\n[33;44mÂ÷§Oµe­± [%s] ...     [m\n", Cdatelite(&now));
     for (i = 0; i < scr_lns; i++)
       fprintf(flog, "%.*s\n", big_picture[i].len, big_picture[i].data);
#endif
     fclose(flog);

     pressanykey("§A¥´¤F %d ¦r,¹ï¤è %d ¦r¡C",myword,itword);
     sprintf(buf,"%s»P%s²á¤Ñ, %d ¬í, %d ¦r, %s",
       cuser.userid,save_page_requestor,now - talkstart,myword,Ctime(&now));
     f_cat(BBSHOME"/log/talk.log",buf);
     more(fpath, NA);

     sprintf(buf, "¹ï¸Ü°O¿ý [1;36m(%s)[m", getuserid(currutmp->destuid));
          
     if (getans("²M°£(C) ²¾¦Ü³Æ§Ñ¿ý(M) (C/M)?[C]") == 'm')
       mail2user(cuser.userid, buf, fpath, 0);
     unlink(fpath);
     flog = 0;
   }
   setutmpmode(XINFO);
}

/* ----------------------------------------------------- */
/* For va_args DL_func                                   */
/* ----------------------------------------------------- */

int va_do_talk (va_list pvar)
{
  int fd;
  fd = va_arg(pvar, int);
  do_talk(fd);
  return;
}
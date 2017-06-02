/*-------------------------------------------------------*/
/* screen.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : ANSI/Chinese screen display routines 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/

#include <varargs.h>
#include "bbs.h"

extern char clearbuf[];
extern char cleolbuf[];
extern char scrollrev[];
extern char strtstandout[];
extern char endstandout[];
extern int clearbuflen;
extern int cleolbuflen;
extern int scrollrevlen;
extern int strtstandoutlen;
extern int endstandoutlen;
extern int automargins;
extern int dumb_term;
extern void ochar();
extern void output();

#define o_clear()     output(clearbuf,clearbuflen)
#define o_cleol()     output(cleolbuf,cleolbuflen)
#define o_scrollrev() output(scrollrev,scrollrevlen)
#define o_standup()   output(strtstandout,strtstandoutlen)
#define o_standdown() output(endstandout,endstandoutlen)


uschar scr_lns, scr_cols;
uschar cur_ln = 0, cur_col = 0;
uschar docls;
uschar standing = NA;
char roll = 0;
int scrollcnt, tc_col, tc_line;

screenline *big_picture = NULL;
// int old_roll;

void
initscr()
{
  if (!dumb_term && !big_picture)
  {
    extern void *calloc();

    scr_lns = t_lines;
    scr_cols = t_columns = ANSILINELEN;
    /* scr_cols = BMIN(t_columns, ANSILINELEN); */
    big_picture = (screenline *) calloc(scr_lns, sizeof(screenline));
    docls = YEA;
  }
}


void
move(y, x)
{
  cur_col = x;
  cur_ln = y;
}


void
getyx(y, x)
  int *y, *x;
{
  *y = cur_ln;
  *x = cur_col;
}


static void
rel_move(was_col, was_ln, new_col, new_ln)
{
  extern char *BC;

  if (new_ln >= t_lines || new_col >= t_columns)
    return;

  tc_col = new_col;
  tc_line = new_ln;
  if (new_col == 0)
  {
    if (new_ln == was_ln)
    {
      if (was_col)
	ochar('\r');
      return;
    }
    else if (new_ln == was_ln + 1)
    {
      ochar('\n');
      if (was_col)
	ochar('\r');
      return;
    }
  }

  if (new_ln == was_ln)
  {
    if (was_col == new_col)
      return;

    if (new_col == was_col - 1)
    {
      if (BC)
	tputs(BC, 1, ochar);
      else
	ochar(Ctrl('H'));
      return;
    }
  }
  do_move(new_col, new_ln);
}


static void
standoutput(buf, ds, de, sso, eso)
  char *buf;
  int ds, de, sso, eso;
{
  if (eso <= ds || sso >= de)
  {
    output(buf + ds, de - ds);
  }
  else
  {
    int st_start, st_end;

    st_start = BMAX(sso, ds);
    st_end = BMIN(eso, de);
    if (sso > ds)
      output(buf + ds, sso - ds);
    o_standup();
    output(buf + st_start, st_end - st_start);
    o_standdown();
    if (de > eso)
      output(buf + eso, de - eso);
  }
}


void
redoscr()
{
  register screenline *bp;
  register int i, j, len;

  if (dumb_term)
    return;

  o_clear();
  for (tc_col = tc_line = i = 0, j = roll; i < scr_lns; i++, j++)
  {
    if (j >= scr_lns)
      j = 0;
    bp = &big_picture[j];
    if (len = bp->len)
    {
      rel_move(tc_col, tc_line, 0, i);

      if (bp->mode & STANDOUT)
	standoutput(bp->data, 0, len, bp->sso, bp->eso);
      else
	output(bp->data, len);
      tc_col += len;
      if (tc_col >= t_columns)
      {
	if (automargins)
	  tc_col = t_columns - 1;
	else
	{
	  tc_col -= t_columns;
	  tc_line++;
	  if (tc_line >= t_lines)
	    tc_line = b_lines;
	}
      }
      bp->mode &= ~(MODIFIED);
      bp->oldlen = len;
    }
  }
  rel_move(tc_col, tc_line, cur_col, cur_ln);
  docls = scrollcnt = 0;
  oflush();
}


void
refresh()
{
  register screenline *bp = big_picture;
  register int i, j, len;
  extern int automargins;
  extern int scrollrevlen;

  if (dumb_term)
    return;
  if (num_in_buf())
    return;

  if ((docls) || (abs(scrollcnt) >= (scr_lns - 3)))
  {
    redoscr();
    return;
  }

  if (scrollcnt < 0)
  {
    if (!scrollrevlen)
    {
      redoscr();
      return;
    }
    rel_move(tc_col, tc_line, 0, 0);
    do
    {
      o_scrollrev();
    } while (++scrollcnt);
  }
  else if (scrollcnt > 0)
  {
    rel_move(tc_col, tc_line, 0, b_lines);
    do
    {
      ochar('\n');
    } while (--scrollcnt);
  }

  for (i = 0, j = roll; i < scr_lns; i++, j++)
  {
    if (j >= scr_lns)
      j = 0;
    bp = &big_picture[j];
    len = bp->len;
    if (bp->mode & MODIFIED && bp->smod < len)
    {
      bp->mode &= ~(MODIFIED);
      if (bp->emod >= len)
	bp->emod = len - 1;
      rel_move(tc_col, tc_line, bp->smod, i);

      if (bp->mode & STANDOUT)
	standoutput(bp->data, bp->smod, bp->emod + 1, bp->sso, bp->eso);
      else
	output(&bp->data[bp->smod], bp->emod - bp->smod + 1);
      tc_col = bp->emod + 1;
      if (tc_col >= t_columns)
      {
	if (automargins)
	{
	  tc_col -= t_columns;
	  if (++tc_line >= t_lines)
	    tc_line = b_lines;
	}
	else
	  tc_col = t_columns - 1;
      }
    }

    if (bp->oldlen > len)
    {
      rel_move(tc_col, tc_line, len, i);
      o_cleol();
    }
    bp->oldlen = len;
  }
  rel_move(tc_col, tc_line, cur_col, cur_ln);
  oflush();
}

void
clear()
{
  int i;
  screenline *slp;

  docls = YEA;
  cur_ln = cur_col = roll = i = 0;
  slp = big_picture;
  while (i++ < t_lines)
    memset(slp++, 0, 9);
}

void
clrtobot()
{
  if (!dumb_term)
  {
    register screenline *slp;
    register int i, j;

    for (i = cur_ln, j = i + roll; i < scr_lns; i++, j++)
    {
      if (j >= scr_lns)
	j -= scr_lns;
      slp = &big_picture[j];
      slp->mode = slp->len = 0;
      if (slp->oldlen)
	slp->oldlen = 255;
    }
  }
}

void
clrtoeol()
{
  if (!dumb_term)
  {
    register screenline *slp;
    register int ln;

    standing = NA;
    if ((ln = cur_ln + roll) >= scr_lns)
      ln -= scr_lns;
    slp = &big_picture[ln];
    if (cur_col <= slp->sso)
      slp->mode &= ~STANDOUT;

    if (cur_col > slp->oldlen)
    {
      for (ln = slp->len; ln <= cur_col; ln++)
	slp->data[ln] = ' ';
    }

    if (cur_col < slp->oldlen)
    {
      for (ln = slp->len; ln >= cur_col; ln--)
	slp->data[ln] = ' ';
    }

    slp->len = cur_col;
  }
}


void
clrchyiuan(int x,int y)
{
  if (!dumb_term)
  {
    register screenline *slp;
    register int i, j;

    for (i = x, j = i + roll; i < y; i++, j++)
    {
      if (j >= scr_lns)
        j -= scr_lns;
      slp = &big_picture[j];
      slp->mode = slp->len = 0;
      if (slp->oldlen)
        slp->oldlen = 255;
    }
  }
}


#if 0
void
clrstandout()
{
  if (!dumb_term)
  {
    register int i;

    for (i = 0; i < scr_lns; i++)
      big_picture[i].mode &= ~(STANDOUT);
  }
}
#endif


void
outch(c)
  register uschar c;
{
  register screenline *slp;
  register int i;

#ifndef BIT8
  c &= 0x7f;
#endif

  if (dumb_term)
  {

#ifdef BIT8
    if ((c != '') && !isprint2(c))
#else
    if (!isprint(c))
#endif

    {
      if (c == '\n')
	ochar('\r');
      else
	c = '*';
    }
    ochar(c);
    return;
  }

  if ((i = cur_ln + roll) >= scr_lns)
    i -= scr_lns;
  slp = &big_picture[i];

#ifdef BIT8
  if ((c != '') && !isprint2(c))
#else
  if (!isprint(c))
#endif

  {
    if (c == '\n' || c == '\r')
    {
      if (standing)
      {
	slp->eso = BMAX(slp->eso, cur_col);
	standing = NA;
      }

#if 1
      if ((i = cur_col - slp->len) > 0)
	memset(&slp->data[slp->len], ' ', i + 1);
#else
      if (cur_col > slp->len)
      {
	for (i = slp->len; i <= cur_col; i++)
	  slp->data[i] = ' ';
      }
#endif

      slp->len = cur_col;
      cur_col = 0;
      if (cur_ln < scr_lns)
	cur_ln++;
      return;
    }
    c = '*';			/* substitute a '*' for non-printable */
  }

  if (cur_col >= slp->len)
  {
    for (i = slp->len; i < cur_col; i++)
      slp->data[i] = ' ';
    slp->data[cur_col] = '\0';
    slp->len = cur_col + 1;
  }

  if (slp->data[cur_col] != c)
  {
    slp->data[cur_col] = c;
    if ((slp->mode & MODIFIED) != MODIFIED)
      slp->smod = slp->emod = cur_col;
    slp->mode |= MODIFIED;
    if (cur_col > slp->emod)
      slp->emod = cur_col;
    if (cur_col < slp->smod)
      slp->smod = cur_col;
  }

  if (++cur_col >= scr_cols)
  {
    if (standing && (slp->mode & STANDOUT))
    {
      standing = 0;
      slp->eso = BMAX(slp->eso, cur_col);
    }
    cur_col = 0;
    if (cur_ln < scr_lns)
      cur_ln++;
  }
}


static void
parsecolor(buf)
  char *buf;
{
  char *val;
  char data[24];

  data[0] = '\0';
  val = (char *) strtok(buf, ";");

  while (val)
  {
    if (atoi(val) < 30)
    {
      if (data[0])
	strcat(data, ";");
      strcat(data, val);
    }
    val = (char *) strtok(NULL, ";");
  }
  strcpy(buf, data);
}


#define NORMAL (00)
#define ESCAPE (01)
#define VTKEYS (02)


void
outc(ch)
  register unsigned char ch;
{
  if (showansi)
    outch(ch);
  else
  {
    static char buf[24];
    static int p = 0;
    static int mode = NORMAL;
    int i;

    switch (mode)
    {
    case NORMAL:
      if (ch == '\033')
	mode = ESCAPE;
      else
	outch(ch);
      return;

    case ESCAPE:
      if (ch == '[')
	mode = VTKEYS;
      else
      {
	mode = NORMAL;
	outch('');
	if (ch != ']')
  	  outch(ch);
      }
      return;

    case VTKEYS:
      if (ch == 'm')
      {
	buf[p++] = '\0';
	parsecolor(buf);
      }
      else if ((p < 24) && (not_alpha(ch)))
      {
	buf[p++] = ch;
	return;
      }

      if (buf[0])
      {
	outch('');
	outch('[');

	for (i = 0; p = buf[i]; i++)
	  outch(p);
	outch(ch);
      }
      p = 0;
      mode = NORMAL;
    }
  }
}


void
outs(str)
  register char *str;
{
/*
  if(HAS_HABIT(HABIT_BIG5GB))
  {
    big2gb(str, strlen(str), 0);
  }
*/
  while (*str)
    outc(*str++);
}


void
outmsg(msg)
  register char *msg;
{
  move(b_lines, 0);
  clrtoeol();
  while (*msg)
    outc(*msg++);
}

/* From exbbs for lightbar in main menu

void
outmsgline(msg,line)
  register char *msg;
  int line;
{
  move(line, 0);
  clrtoeol();
  while (*msg)
    outc(*msg++);
}

*/

void
outz(msg)
  register char *msg;
{
  outmsg(msg);
  refresh();
  sleep(1);
}


void
prints(va_alist)
va_dcl
{
  va_list args;
  char buff[1024], *fmt;

  va_start(args);
  fmt = va_arg(args, char *);
  vsprintf(buff, fmt, args);
  va_end(args);
  outs(buff);
}


void
scroll()
{
  if (dumb_term)
  {
    outc('\n');
  }
  else
  {
    scrollcnt++;
    if (++roll >= scr_lns)
      roll = 0;
    move(b_lines, 0);
    clrtoeol();
  }
}


void
rscroll()
{
  if (dumb_term)
  {
    outs("\n\n");
  }
  else
  {
    scrollcnt--;
    if (--roll < 0)
      roll = b_lines;
    move(0, 0);
    clrtoeol();
  }
}

region_scroll_up(int top, int bottom)
{
   int i;

   if (top > bottom) {
      i = top; 
      top = bottom;
      bottom = i;
   }

   if (top < 0 || bottom >= scr_lns)
     return;

   for (i = top; i < bottom; i++)
      big_picture[i] = big_picture[i + 1];
   memset(big_picture + i, 0, sizeof(*big_picture));
   memset(big_picture[i].data, ' ', scr_cols);
   save_cursor();
   change_scroll_range(top, bottom);
   do_move(0, bottom);
   scroll_forward();
   change_scroll_range(0, scr_lns - 1);
   restore_cursor();
   refresh();
}


region_scroll_down(int top, int bottom)
{
}

void
standout()
{
  if (!standing && !dumb_term && strtstandoutlen)
  {
    register screenline *slp;

    slp = &big_picture[((cur_ln + roll) % scr_lns)];
    standing = YEA;
    slp->sso = slp->eso = cur_col;
    slp->mode |= STANDOUT;
  }
}


void
standend()
{
  if (standing && !dumb_term && strtstandoutlen)
  {
    register screenline *slp;

    slp = &big_picture[((cur_ln + roll) % scr_lns)];
    standing = NA;
    slp->eso = BMAX(slp->eso, cur_col);
  }
}


#if 0
void
outns(str, n)
  register char *str;
  register int n;
{
  while (n--)
    outc(*str++);
}


void
chcxy(type)
  int type;
{
  static backup_x;
  static backup_y;

  if (type == 0)
  {				/* backup */
    backup_x = cur_col;
    backup_y = cur_ln;
  }
  else
  {				/* restore */
    cur_col = backup_x;
    cur_ln = backup_y;
  }
}
#endif

#define VS_STACK_SIZE 5                                        
int vs_stack_ptr = -1;          /* CityLion */                 
int old_roll[VS_STACK_SIZE];                                   
int my_newfd[VS_STACK_SIZE],x[VS_STACK_SIZE],y[VS_STACK_SIZE]; 
extern int i_newfd;                                            
int mode0[VS_STACK_SIZE],stat0[VS_STACK_SIZE];                 
                                                               
void                                                           
vs_save(screen)                                                
  screenline *screen;                                          
{                                                              
  vs_stack_ptr++;                                              
  old_roll[vs_stack_ptr] = roll;
  
  if(currutmp)
  {
    mode0[vs_stack_ptr] = currutmp->mode;                        
    stat0[vs_stack_ptr] = currstat;                              
  }
  getyx(&y[vs_stack_ptr],&x[vs_stack_ptr]);                    
  memcpy(screen, big_picture, t_lines * sizeof(screenline));   
  my_newfd[vs_stack_ptr] = i_newfd;                            
  i_newfd = 0;                                              
//  save_cursor();                                            
}                                                           
                                                            
void                                                        
vs_restore(screen)                                          
  screenline *screen;                                       
{                                                           
  roll = old_roll[vs_stack_ptr];                            
  i_newfd = my_newfd[vs_stack_ptr];
  if(currutmp)
  {
    currstat = stat0[vs_stack_ptr];
    currutmp->mode = mode0[vs_stack_ptr];
  }
  memcpy(big_picture, screen, t_lines * sizeof(screenline));
  move(y[vs_stack_ptr],x[vs_stack_ptr]);                    
  vs_stack_ptr--;                                           
  free(screen);                                             
  redoscr();                                                
  refresh();                                                
//  restore_cursor();                                         
}                                                           


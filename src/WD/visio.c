/*-------------------------------------------------------*/
/* visio.c           ( AT-BBS/WD_hialan BBS )            */
/*-------------------------------------------------------*/
/* target : VIrtual Screen Input Output routines         */
/*	    term.c + screen.c + io.c			 */
/* ¦X  ¨Ö : hialan					 */
/*-------------------------------------------------------*/
#include "bbs.h"

/* ----------------------------------------------------- */
/* output routines  (io.c)                               */
/* ----------------------------------------------------- */
#ifdef  LINUX
#define OBUFSIZE  (2048)
#define IBUFSIZE  (128)
#else
#define OBUFSIZE  (4096)
#define IBUFSIZE  (512)
#endif

static char outbuf[OBUFSIZE];
static int obufsize = 0;

void oflush()
{
  if (obufsize)
  {
     write(1, outbuf, obufsize);
     obufsize = 0;
  }
}


static void output(char *s, int len)
{
  /* Invalid if len >= OBUFSIZE */

  if (obufsize + len > OBUFSIZE)
  {
    write(1, outbuf, obufsize);
    obufsize = 0;
  }
  memcpy(outbuf + obufsize, s, len);
  obufsize += len;
}


void ochar(char c)
{
  if (obufsize > OBUFSIZE - 1)
  {
    write(1, outbuf, obufsize);
    obufsize = 0;
  }
  outbuf[obufsize++] = c;
}

/*-------------------------------------------------------*/
/* term.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : termcap I/O control routines                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include <sys/ioctl.h>

#ifdef HP_UX
#define O_HUPCL 01
#define O_XTABS 02
#endif

#ifdef LINUX
// shakalaca patch , 990818
// #include <linux/termios.h>
#include <termios.h>

#define stty(fd, data) tcsetattr( fd, TCSETS, data )
#define gtty(fd, data) tcgetattr( fd, data )
#endif

#ifndef TANDEM
#define TANDEM  0x00000001
#endif

#ifndef CBREAK
#define CBREAK  0x00000002
#endif

#ifdef LINUX
struct termios tty_state, tty_new;
#else
struct sgttyb tty_state, tty_new;
#endif


#ifndef BSD44
/*
 * tparm.c
 *
 * By Ross Ridge
 * Public Domain
 * 92/02/01 07:30:36
 *
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#if defined(SunOS) || defined(SOLARIS)
#include "termcap.h_SunOS"
#else
#include "termcap.h"
#endif


#ifdef USE_SCCS_IDS
static const char SCCSid[] = "@(#) mytinfo tparm.c 3.2 92/02/01 public domain, By Ross Ridge";
#endif

#ifndef MAX_PUSHED
#define MAX_PUSHED      32
#endif

#define ARG     1
#define NUM     2

#define INTEGER 1
#define STRING  2

// #define MAX_LINE 640
// wildcat ½Õ¤j¤@ÂI¬Ý¬Ý
#define MAX_LINE 65536

typedef void* anyptr;

typedef struct stack_str {
        int     type;
        int     argnum;
        int     value;
} stack;

static stack S[MAX_PUSHED];
static stack vars['z'-'a'+1];
static int pos = 0;

static struct arg_str {
        int type;
        int     integer;
        char    *string;
} arg_list[10];

static int argcnt;

static va_list tparm_args;

static int
pusharg(arg)
int arg; {
        if (pos == MAX_PUSHED)
                return 1;
        S[pos].type = ARG;
        S[pos++].argnum = arg;
        return 0;
}

static int
pushnum(num)
int num; {
        if (pos == MAX_PUSHED)
                return 1;
        S[pos].type = NUM;
        S[pos++].value = num;
        return 0;
}

/* VARARGS2 */
static int
getarg(argnum, type, p)
int argnum, type;
anyptr p; {
        while (argcnt < argnum) {
                arg_list[argcnt].type = INTEGER;
                arg_list[argcnt++].integer = (int) va_arg(tparm_args, int);
        }
        if (argcnt > argnum) {
                if (arg_list[argnum].type != type)
                        return 1;
                else if (type == STRING)
                        *(char **)p = arg_list[argnum].string;
                else
                        *(int *)p = arg_list[argnum].integer;
        } else {
                arg_list[argcnt].type = type;
                if (type == STRING)
                        *(char **)p = arg_list[argcnt++].string
                                = (char *) va_arg(tparm_args, char *);
                else
                        *(int *)p = arg_list[argcnt++].integer = (int) va_arg(tparm_args, int);
        }
        return 0;
}


static int
popstring(str)
char **str; {
        if (pos-- == 0)
                return 1;
        if (S[pos].type != ARG)
                return 1;
        return(getarg(S[pos].argnum, STRING, (anyptr) str));
}

static int
popnum(num)
int *num; {
        if (pos-- == 0)
                return 1;
        switch (S[pos].type) {
        case ARG:
                return (getarg(S[pos].argnum, INTEGER, (anyptr) num));
        case NUM:
                *num = S[pos].value;
                return 0;
        }
        return 1;
}

static int
cvtchar(sp, c)
register char *sp, *c; {
        switch(*sp) {
        case '\\':
                switch(*++sp) {
                case '\'':
                case '$':
                case '\\':
                case '%':
                        *c = *sp;
                        return 2;
                case '\0':
                        *c = '\\';
                        return 1;
                case '0':
                        if (sp[1] == '0' && sp[2] == '0') {
                                *c = '\0';
                                return 4;
                        }
                        *c = '\200'; /* '\0' ???? */
                        return 2;
                default:
                        *c = *sp;
                        return 2;
                }
        default:
                *c = *sp;
                return 1;
        }
}

static int termcap;

/* sigh... this has got to be the ugliest code I've ever written.
   Trying to handle everything has its cost, I guess.

   It actually isn't to hard to figure out if a given % code is supposed
   to be interpeted with its termcap or terminfo meaning since almost
   all terminfo codes are invalid unless something has been pushed on
   the stack and termcap strings will never push things on the stack
   (%p isn't used by termcap). So where we have a choice we make the
   decision by wether or not somthing has been pushed on the stack.
   The static variable termcap keeps track of this; it starts out set
   to 1 and is incremented as each argument processed by a termcap % code,
   however if something is pushed on the stack it's set to 0 and the
   rest of the % codes are interpeted as terminfo % codes. Another way
   of putting it is that if termcap equals one we haven't decided either
   way yet, if it equals zero we're looking for terminfo codes, and if
   its greater than 1 we're looking for termcap codes.

   Terminfo % codes:

        %%      output a '%'
        %[[:][-+# ][width][.precision]][doxXs]
                output pop according to the printf format
        %c      output pop as a char
        %'c'    push character constant c.
        %{n}    push decimal constant n.
        %p[1-9] push paramter [1-9]
        %g[a-z] push variable [a-z]
        %P[a-z] put pop in variable [a-z]
        %l      push the length of pop (a string)
        %+      add pop to pop and push the result
        %-      subtract pop from pop and push the result
        %*      multiply pop and pop and push the result
        %&      bitwise and pop and pop and push the result
        %|      bitwise or pop and pop and push the result
        %^      bitwise xor pop and pop and push the result
        %~      push the bitwise not of pop
        %=      compare if pop and pop are equal and push the result
        %>      compare if pop is less than pop and push the result
        %<      compare if pop is greater than pop and push the result
        %A      logical and pop and pop and push the result
        %O      logical or pop and pop and push the result
        %!      push the logical not of pop
        %? condition %t if_true [%e if_false] %;
                if condtion evaulates as true then evaluate if_true,
                else evaluate if_false. elseif's can be done:
%? cond %t true [%e cond2 %t true2] ... [%e condN %t trueN] [%e false] %;
        %i      add one to parameters 1 and 2. (ANSI)

  Termcap Codes:

        %%      output a %
        %.      output parameter as a character
        %d      output parameter as a decimal number
        %2      output parameter in printf format %02d
        %3      output parameter in printf format %03d
        %+x     add the character x to parameter and output it as a character
(UW)    %-x     subtract parameter FROM the character x and output it as a char
(UW)    %ax     add the character x to parameter
(GNU)   %a[+*-/=][cp]x
                GNU arithmetic.
(UW)    %sx     subtract parameter FROM the character x
        %>xy    if parameter > character x then add character y to parameter
        %B      convert to BCD (parameter = (parameter/10)*16 + parameter%16)
        %D      Delta Data encode (parameter = parameter - 2*(paramter%16))
        %i      increment the first two parameters by one
        %n      xor the first two parameters by 0140
(GNU)   %m      xor the first two parameters by 0177
        %r      swap the first two parameters
(GNU)   %b      backup to previous parameter
(GNU)   %f      skip this parameter

  Note the two definitions of %a, the GNU defintion is used if the characters
  after the 'a' are valid, otherwise the UW definition is used.

  (GNU) used by GNU Emacs termcap libraries
  (UW) used by the University of Waterloo (MFCF) termcap libraries

*/

static char *tparm(const char *str, ...) 
{
        static char OOPS[] = "OOPS";
        static char buf[MAX_LINE];
        register const char *sp;
        register char *dp;
        register char *fmt;
        char conv_char;
        char scan_for;
        int scan_depth = 0, if_depth;
        static int i, j;
        static char *s, c;
        char fmt_buf[MAX_LINE];
        char sbuf[MAX_LINE];

        va_start(tparm_args, str);

        sp = str;
        dp = buf;
        scan_for = 0;
        if_depth = 0;
        argcnt = 0;
        pos = 0;
        termcap = 1;
        while(*sp != '\0') {
                switch(*sp) {
                case '\\':
                        if (scan_for) {
                                if (*++sp != '\0')
                                        sp++;
                                break;
                        }
                        *dp++ = *sp++;
                        if (*sp != '\0')
                                *dp++ = *sp++;
                        break;
                case '%':
                        sp++;
                        if (scan_for) {
                                if (*sp == scan_for && if_depth == scan_depth) {
                                        if (scan_for == ';')
                                                if_depth--;
                                        scan_for = 0;
                                } else if (*sp == '?')
                                        if_depth++;
                                else if (*sp == ';') {
                                        if (if_depth == 0)
                                                return OOPS;
                                        else
                                                if_depth--;
                                }
                                sp++;
                                break;
                        }
                        fmt = NULL;
                        switch(*sp) {
                        case '%':
                                *dp++ = *sp++;
                                break;
                        case '+':
                                if (!termcap) {
                                        if (popnum(&j) || popnum(&i))
                                                return OOPS;
                                        i += j;
                                        if (pushnum(i))
                                                return OOPS;
                                        sp++;
                                        break;
                                }
                                ;/* FALLTHROUGH */
                        case 'C':
                                if (*sp == 'C') {
                                        if (getarg(termcap - 1, INTEGER, &i))
                                                return OOPS;
                                        if (i >= 96) {
                                                i /= 96;
                                                if (i == '$')
                                                        *dp++ = '\\';
                                                *dp++ = i;
                                        }
                                }
                                fmt = "%c";
                                /* FALLTHROUGH */
                        case 'a':
                                if (!termcap)
                                        return OOPS;
                                if (getarg(termcap - 1, INTEGER, (anyptr) &i))
                                        return OOPS;
                                if (*++sp == '\0')
                                        return OOPS;
                                if ((sp[1] == 'p' || sp[1] == 'c')
                                    && sp[2] != '\0' && fmt == NULL) {
                                        /* GNU aritmitic parameter, what they
                                           realy need is terminfo.            */
                                        int val, lc;
                                        if (sp[1] == 'p'
                                            && getarg(termcap - 1 + sp[2] - '@',
                                                      INTEGER, (anyptr) &val))
                                                return OOPS;
                                        if (sp[1] == 'c') {
                                                lc = cvtchar(sp + 2, &c) + 2;
                                        /* Mask out 8th bit so \200 can be
                                           used for \0 as per GNU doc's    */
                                                val = c & 0177;
                                        } else
                                                lc = 2;
                                        switch(sp[0]) {
                                        case '=':
                                                break;
                                        case '+':
                                                val = i + val;
                                                break;
                                        case '-':
                                                val = i - val;
                                                break;
                                        case '*':
                                                val = i * val;
                                                break;
                                        case '/':
                                                val = i / val;
                                                break;
                                        default:
                                        /* Not really GNU's %a after all... */
                                                lc = cvtchar(sp, &c);
                                                val = c + i;
                                                break;
                                        }
                                        arg_list[termcap - 1].integer = val;
                                        sp += lc;
                                        break;
                                }
                                sp += cvtchar(sp, &c);
                                arg_list[termcap - 1].integer = c + i;
                                if (fmt == NULL)
                                        break;
                                sp--;
                                /* FALLTHROUGH */
                        case '-':
                                if (!termcap) {
                                        if (popnum(&j) || popnum(&i))
                                                return OOPS;
                                        i -= j;
                                        if (pushnum(i))
                                                return OOPS;
                                        sp++;
                                        break;
                                }
                                fmt = "%c";
                                /* FALLTHROUGH */
                        case 's':
                                if (termcap && (fmt == NULL || *sp == '-')) {
                                        if (getarg(termcap - 1, INTEGER, &i))
                                                return OOPS;
                                        if (*++sp == '\0')
                                                return OOPS;
                                        sp += cvtchar(sp, &c);
                                        arg_list[termcap - 1].integer = c - i;
                                        if (fmt == NULL)
                                                break;
                                        sp--;
                                }
                                if (!termcap)
                                        return OOPS;
                                ;/* FALLTHROUGH */
                        case '.':
                                if (termcap && fmt == NULL)
                                        fmt = "%c";
                                ;/* FALLTHROUGH */
                        case 'd':
                                if (termcap && fmt == NULL)
                                        fmt = "%d";
                                ;/* FALLTHROUGH */
                        case '2':
                                if (termcap && fmt == NULL)
                                        fmt = "%02d";
                                ;/* FALLTHROUGH */
                        case '3':
                                if (termcap && fmt == NULL)
                                        fmt = "%03d";
                                ;/* FALLTHROUGH */
                        case ':': case ' ': case '#': case 'u':
                        case 'x': case 'X': case 'o': case 'c':
                        case '0': case '1': case '4': case '5':
                        case '6': case '7': case '8': case '9':
                                if (fmt == NULL) {
                                        if (termcap)
                                                return OOPS;
                                        if (*sp == ':')
                                                sp++;
                                        fmt = fmt_buf;
                                        *fmt++ = '%';
                                        while(*sp != 's' && *sp != 'x' && *sp != 'X' && *sp != 'd' && *sp != 'o' && *sp != 'c' && *sp != 'u') {
                                                if (*sp == '\0')
                                                        return OOPS;
                                                *fmt++ = *sp++;
                                        }
                                        *fmt++ = *sp;
                                        *fmt = '\0';
                                        fmt = fmt_buf;
                                }
                                conv_char = fmt[strlen(fmt) - 1];
                                if (conv_char == 's') {
                                        if (popstring(&s))
                                                return OOPS;
                                        sprintf(sbuf, fmt, s);
                                } else {
                                        if (termcap) {
                                                if (getarg(termcap++ - 1,
                                                           INTEGER, &i))
                                                        return OOPS;
                                        } else
                                                if (popnum(&i))
                                                        return OOPS;
                                        if (i == 0 && conv_char == 'c')
                                                strcpy(sbuf, "\000");
                                        else
                                                sprintf(sbuf, fmt, i);
                                }
                                sp++;
                                fmt = sbuf;
                                while(*fmt != '\0') {
                                        if (*fmt == '$')
                                                *dp++ = '\\';
                                        *dp++ = *fmt++;
                                }
                                break;
                        case 'r':
                                if (!termcap || getarg(1, INTEGER, &i))
                                        return OOPS;
                                arg_list[1].integer = arg_list[0].integer;
                                arg_list[0].integer = i;
                                sp++;
                                break;
                        case 'i':
                                if (getarg(1, INTEGER, &i)
                                    || arg_list[0].type != INTEGER)
                                        return OOPS;
                                arg_list[1].integer++;
                                arg_list[0].integer++;
                                sp++;
                                break;
                        case 'n':
                                if (!termcap || getarg(1, INTEGER, &i))
                                        return OOPS;
                                arg_list[0].integer ^= 0140;
                                arg_list[1].integer ^= 0140;
                                sp++;
                                break;
                        case '>':
                                if (!termcap) {
                                        if (popnum(&j) || popnum(&i))
                                                return OOPS;
                                        i = (i > j);
                                        if (pushnum(i))
                                                return OOPS;
                                        sp++;
                                        break;
                                }
                                if (getarg(termcap-1, INTEGER, &i))
                                        return OOPS;
                                sp += cvtchar(sp, &c);
                                if (i > c) {
                                        sp += cvtchar(sp, &c);
                                        arg_list[termcap-1].integer += c;
                                } else
                                        sp += cvtchar(sp, &c);
                                sp++;
                                break;
                        case 'B':
                                if (!termcap || getarg(termcap-1, INTEGER, &i))
                                        return OOPS;
                                arg_list[termcap-1].integer = 16*(i/10)+i%10;
                                sp++;
                                break;
                        case 'D':
                                if (!termcap || getarg(termcap-1, INTEGER, &i))
                                        return OOPS;
                                arg_list[termcap-1].integer = i - 2 * (i % 16);
                                sp++;
                                break;
                        case 'p':
                                if (termcap > 1)
                                        return OOPS;
                                if (*++sp == '\0')
                                        return OOPS;
                                if (*sp == '0')
                                        i = 9;
                                else
                                        i = *sp - '1';
                                if (i < 0 || i > 9)
                                        return OOPS;
                                if (pusharg(i))
                                        return OOPS;
                                termcap = 0;
                                sp++;
                                break;
                        case 'P':
                                if (termcap || *++sp == '\0')
                                        return OOPS;
                                i = *sp++ - 'a';
                                if (i < 0 || i > 25)
                                        return OOPS;
                                if (pos-- == 0)
                                        return OOPS;
                                switch(vars[i].type = S[pos].type) {
                                case ARG:
                                        vars[i].argnum = S[pos].argnum;
                                        break;
                                case NUM:
                                        vars[i].value = S[pos].value;
                                        break;
                                }
                                break;
                        case 'g':
                                if (termcap || *++sp == '\0')
                                        return OOPS;
                                i = *sp++ - 'a';
                                if (i < 0 || i > 25)
                                        return OOPS;
                                switch(vars[i].type) {
                                case ARG:
                                        if (pusharg(vars[i].argnum))
                                                return OOPS;
                                        break;
                                case NUM:
                                        if (pushnum(vars[i].value))
                                                return OOPS;
                                        break;
                                }
                                break;
                        case '\'':
                                if (termcap > 1)
                                        return OOPS;
                                if (*++sp == '\0')
                                        return OOPS;
                                sp += cvtchar(sp, &c);
                                if (pushnum(c) || *sp++ != '\'')
                                        return OOPS;
                                termcap = 0;
                                break;
                        case '{':
                                if (termcap > 1)
                                        return OOPS;
                                i = 0;
                                sp++;
                                while(isdigit(*sp))
                                        i = 10 * i + *sp++ - '0';
                                if (*sp++ != '}' || pushnum(i))
                                        return OOPS;
                                termcap = 0;
                                break;
                        case 'l':
                                if (termcap || popstring(&s))
                                        return OOPS;
                                i = strlen(s);
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '*':
                                if (termcap || popnum(&j) || popnum(&i))
                                        return OOPS;
                                i *= j;
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '/':
                                if (termcap || popnum(&j) || popnum(&i))
                                        return OOPS;
                                i /= j;
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case 'm':
                                if (termcap) {
                                        if (getarg(1, INTEGER, &i))
                                                return OOPS;
                                        arg_list[0].integer ^= 0177;
                                        arg_list[1].integer ^= 0177;
                                        sp++;
                                        break;
                                }
                                if (popnum(&j) || popnum(&i))
                                        return OOPS;
                                i %= j;
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '&':
                                if (popnum(&j) || popnum(&i))
                                        return OOPS;
                                i &= j;
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '|':
                                if (popnum(&j) || popnum(&i))
                                        return OOPS;
                                i |= j;
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '^':
                                if (popnum(&j) || popnum(&i))
                                        return OOPS;
                                i ^= j;
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '=':
                                if (popnum(&j) || popnum(&i))
                                        return OOPS;
                                i = (i == j);
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '<':
                                if (popnum(&j) || popnum(&i))
                                        return OOPS;
                                i = (i < j);
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case 'A':
                                if (popnum(&j) || popnum(&i))
                                        return OOPS;
                                i = (i && j);
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case 'O':
                                if (popnum(&j) || popnum(&i))
                                        return OOPS;
                                i = (i || j);
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '!':
                                if (popnum(&i))
                                        return OOPS;
                                i = !i;
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '~':
                                if (popnum(&i))
                                        return OOPS;
                                i = ~i;
                                if (pushnum(i))
                                        return OOPS;
                                sp++;
                                break;
                        case '?':
                                if (termcap > 1)
                                        return OOPS;
                                termcap = 0;
                                if_depth++;
                                sp++;
                                break;
                        case 't':
                                if (popnum(&i) || if_depth == 0)
                                        return OOPS;
                                if (!i) {
                                        scan_for = 'e';
                                        scan_depth = if_depth;
                                }
                                sp++;
                                break;
                        case 'e':
                                if (if_depth == 0)
                                        return OOPS;
                                scan_for = ';';
                                scan_depth = if_depth;
                                sp++;
                                break;
                        case ';':
                                if (if_depth-- == 0)
                                        return OOPS;
                                sp++;
                                break;
                        case 'b':
                                if (--termcap < 1)
                                        return OOPS;
                                sp++;
                                break;
                        case 'f':
                                if (!termcap++)
                                        return OOPS;
                                sp++;
                                break;
                        }
                        break;
                default:
                        if (scan_for)
                                sp++;
                        else
                                *dp++ = *sp++;
                        break;
                }
        }
        va_end(tparm_args);
        *dp = '\0';
        return buf;
}
#endif

/* ----------------------------------------------------- */
/* basic tty control                                     */
/* ----------------------------------------------------- */
void init_tty()
{
  if (gtty(1, &tty_state) < 0)
  {
    fprintf(stderr, "gtty failed\n");
    exit(-1);
  }
  memcpy(&tty_new, &tty_state, sizeof(tty_new));

#ifdef  LINUX

//  tty_new.c_lflag &= ~(ICANON | ECHO | RAW | ISIG);
// shakalaca patch , 990818
  tty_new.c_lflag &= ~(ICANON | ECHO | ISIG);
  tcsetattr(1, TCSANOW, &tty_new);
  restore_tty();

#else

  tty_new.sg_flags |= RAW;

#ifdef  HP_UX
  tty_new.sg_flags &= ~(O_HUPCL | O_XTABS | LCASE | ECHO | CRMOD);
#else
  tty_new.sg_flags &= ~(TANDEM | CBREAK | LCASE | ECHO | CRMOD);
#endif

  stty(1, &tty_new);
#endif
}


#ifdef LINUX
reset_tty()
{
   system("stty -raw echo");
}
restore_tty()
{
   system("stty raw -echo");
}
#else
void
reset_tty()
{
  stty(1, &tty_state);
}
void
restore_tty()
{
  stty(1, &tty_new);
}

#endif


/* ----------------------------------------------------- */
/* init tty control code                                 */
/* ----------------------------------------------------- */
#define TERMCOMSIZE (40)

int dumb_term = YEA;

char clearbuf[TERMCOMSIZE];
int clearbuflen;

char cleolbuf[TERMCOMSIZE];
int cleolbuflen;

char cursorm[TERMCOMSIZE];
char *cm;

char changescroll[TERMCOMSIZE];
char *cs;

char savecursor[TERMCOMSIZE];
char *sc;

char restorecursor[TERMCOMSIZE];
char *rc;

char scrollforward[TERMCOMSIZE];
char *sf;

char scrollreverse[TERMCOMSIZE];
char *sr;

char scrollrev[TERMCOMSIZE];
int scrollrevlen;

char scrollset[TERMCOMSIZE];
int scrollsetlen;

char strtstandout[TERMCOMSIZE];
int strtstandoutlen;

char endstandout[TERMCOMSIZE];
int endstandoutlen;

int t_lines = 24;
int b_lines = 23;
int p_lines = 20;
int t_columns = 80;

int automargins;

char *outp;
int *outlp;


static void outcf(char ch)
{
  if (*outlp < TERMCOMSIZE)
  {
    (*outlp)++;
    *outp++ = ch;
  }
}


int term_init(char *term)
{
  extern char PC, *UP, *BC;
#ifndef LINUX
  extern short ospeed;
#endif
  static char UPbuf[TERMCOMSIZE];
  static char BCbuf[TERMCOMSIZE];
  static char buf[1024];
  char sbuf[2048];
  char *sbp, *s;
  char *tgetstr();

#ifdef LINUX
  ospeed = cfgetospeed(&tty_state);
#else
  ospeed = tty_state.sg_ospeed;
#endif

  if (tgetent(buf, term) != 1)
    return NA;

  sbp = sbuf;
  s = tgetstr("pc", &sbp);      /* get pad character */
  if (s)
    PC = *s;

  t_lines = tgetnum("li");
  t_columns = tgetnum("co");
  automargins = tgetflag("am");

  outp = clearbuf;              /* fill clearbuf with clear screen command */
  outlp = &clearbuflen;
  clearbuflen = 0;
  sbp = sbuf;
  s = tgetstr("cl", &sbp);
  if (s)
    tputs(s, t_lines, outcf);

  outp = cleolbuf;              /* fill cleolbuf with clear to eol command */
  outlp = &cleolbuflen;
  cleolbuflen = 0;
  sbp = sbuf;
  s = tgetstr("ce", &sbp);
  if (s)
    tputs(s, 1, outcf);

  outp = scrollrev;
  outlp = &scrollrevlen;
  scrollrevlen = 0;
  sbp = sbuf;
  s = tgetstr("sr", &sbp);
  if (s)
    tputs(s, 1, outcf);

  outp = strtstandout;
  outlp = &strtstandoutlen;
  strtstandoutlen = 0;
  sbp = sbuf;
  s = tgetstr("so", &sbp);
  if (s)
    tputs(s, 1, outcf);

  outp = endstandout;
  outlp = &endstandoutlen;
  endstandoutlen = 0;
  sbp = sbuf;
  s = tgetstr("se", &sbp);
  if (s)
    tputs(s, 1, outcf);

  sbp = cursorm;
  cm = tgetstr("cm", &sbp);
  if (cm)
    dumb_term = NA;
  else
  {
    dumb_term = YEA;
    t_lines = 24;
    t_columns = 80;
  }

  sbp = changescroll;
  cs = tgetstr("cs", &sbp);

  sbp = scrollforward;
  sf = tgetstr("sf", &sbp);

  sbp = scrollreverse;
  sr = tgetstr("sr", &sbp);

  sbp = savecursor;
  sc = tgetstr("sc", &sbp);

  sbp = restorecursor;
  rc = tgetstr("rc", &sbp);

  sbp = UPbuf;
  UP = tgetstr("up", &sbp);
  sbp = BCbuf;
  BC = tgetstr("bc", &sbp);

  b_lines = t_lines - 1;
  p_lines = t_lines - 4;
  return YEA;
}



do_move(destcol, destline)
  int destcol, destline;
{
  tputs(tgoto(cm, destcol, destline), 0, ochar);
}


save_cursor()
{
  tputs(sc, 0, ochar);
}

restore_cursor()
{
  tputs(rc, 0, ochar);
}

/*
woju
*/
change_scroll_range(int top, int bottom)
{
  tputs(tparm(cs, top, bottom), 0, ochar);
}

scroll_forward()
{
  tputs(sf, 0, ochar);
}

scroll_reverse()
{
  tputs(sr, 0, ochar);
}

/*-------------------------------------------------------*/
/* screen.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : ANSI/Chinese screen display routines 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/
#include <varargs.h>

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

void initscr()
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

void move(int y, int x)
{
  cur_col = x;
  cur_ln = y;
}

void getyx(int *y, int *x)
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


void redoscr()
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


void refresh()
{
  register screenline *bp = big_picture;
  register int i, j, len;

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

void clear()
{
  int i;
  screenline *slp;

  docls = YEA;
  cur_ln = cur_col = roll = i = 0;
  slp = big_picture;
  while (i++ < t_lines)
    memset(slp++, 0, 9);
}

void clrtobot()
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

void clrtoeol()
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


void clrchyiuan(int x,int y)
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


void outs(str)
  register char *str;
{
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


void scroll()
{
  if (dumb_term)
    outc('\n');
  else
  {
    scrollcnt++;
    if (++roll >= scr_lns)
      roll = 0;
    move(b_lines, 0);
    clrtoeol();
  }
}


void rscroll()
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

void standout()
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


void standend()
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
}                                                           

/*-------------------------------------------------------*/
/* io.c         ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : basic console/screen/keyboard I/O routines   */
/* create : 95/02/28                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#ifdef AIX
#include <sys/select.h>
#endif

#define INPUT_ACTIVE    0
#define INPUT_IDLE      1

static char inbuf[IBUFSIZE];
static int ibufsize = 0;
static int icurrchar = 0;

static int i_mode = INPUT_ACTIVE;

passwd_outs(text)
  char *text;
{
  register int column = 0;
  register char ch;
  while ((ch = *text++) && (++column < 80))
  {
    outch('*');
  }
}

/* ----------------------------------------------------- */
/* ©w®ÉÅã¥Ü°ÊºA¬ÝªO                                      */
/* ----------------------------------------------------- */
#define STAY_TIMEOUT    (30*60)

static void hit_alarm_clock()
{
  static int stay_time = 0;
  static int idle_time = 0;

  time_t now = time(0);
  char buf[100]="\0";

  if(currutmp->pid != currpid)
    setup_utmp(XMODE);   /* ­«·s°t¸m shm */

  if((idle_time = now - currutmp->lastact) > IDLE_TIMEOUT && !HAS_PERM(PERM_RESEARCH))
  {
    pressanykey("¶W¹L¶¢¸m®É¶¡¡I½ð¥X¥hÅo¡K¡K");
    abort_bbs();
  }

  if (HAS_HABIT(HABIT_MOVIE) && (currstat && (currstat < CLASS || currstat == MAILALL)))
    movie(0);

  alarm(MOVIE_INT);
  stay_time += MOVIE_INT;

  if(idle_time > IDLE_TIMEOUT - 60 && !HAS_PERM(PERM_RESEARCH)) 
    sprintf(buf, "[1;5;37;41mÄµ§i¡G±z¤w¶¢¸m¹L¤[¡A­YµL¦^À³¡A¨t²Î§Y±N¤ÁÂ÷¡I¡I[m");
  else if(stay_time > 10 * 60 && chkmail(0)) 
  {
    sprintf(buf, "\033[1;33;41m[%s] «H½cùØÁÙ¦³¨S¬Ý¹Lªº«H­ò\033[m",
      Etime(&now));
    stay_time = 0 ;
  }
  else if(stay_time > STAY_TIMEOUT && HAS_HABIT(HABIT_ALARM))
  {
    /* ¦b³o¸Ì´£¥Ü user ¥ð®§¤@¤U */
    char *msg[10] = {
    "¦ù¦ù¸y, ´|´|²´, ³Ü¤f¯ù....³Ý¤f®ð...¦AÄ~Äò...!",
    "¤@Ãä¬O¤Í±¡ ¤@Ãä¬O·R±¡ ¥ª¥kªº¬G¨Æ¬°ÃøµÛ¦Û¤v...",
    "¬O§_¦³¤HÁA¸Ñ±z¤º¤ßªº©t±I? ¤j®a¨Ótalk talk§a.. ",
    "¥ª¤T°é,¥k¤T°é,²ä¤l§á§á§¾ªÑ§á§á ¤j®a¨Ó§@¹B°Ê­ò~",
    "§ÚÄé..§ÚÄé..§ÚÄéÄéÄé! Äé¨ìµwºÐÃz±¼...",
    "¥Î¡E¥\\¡E°á¡E®Ñ",
    "®Ñ©À§¹¤F¨S°Ú....^.^",
    "©ú¤Ñ¦³¨S¦³¦Ò¸Õ°Ú...©À®Ñ­«­n­ò...!",
    "¬Ý¡ã¬y¬P¡I",
    "¡E®Ñ¦b¤ß¤¤®ð¦Û¬Ó¡EÅª®Ñ¥h¡E"};
    int i = rand() % 10;

    sprintf(buf, "[1;33;41m[%s] %s[m", Etime(&now), msg[i]);
    stay_time = 0 ;
  }

  if(buf[0]) 
  {
    outmsg(buf);
    refresh();
    bell();
  }
}

void init_alarm()
{
  alarm(0);
  signal(SIGALRM, hit_alarm_clock);
  alarm(MOVIE_INT);
}

/* ----------------------------------------------------- */
/* input routines                                        */
/* ----------------------------------------------------- */
int i_newfd = 0;
static struct timeval i_to, *i_top = NULL;
static int (*flushf) () = NULL;


void add_io(fd, timeout)
  int fd;
  int timeout;
{
  i_newfd = fd;
  if (timeout)
  {
    i_to.tv_sec = timeout;
    i_to.tv_usec = 0;
    i_top = &i_to;
  }
  else
    i_top = NULL;
}


void add_flush(flushfunc)
  int (*flushfunc) ();
{
  flushf = flushfunc;
}


int
num_in_buf()
{
  return icurrchar - ibufsize;
}

static int dogetch()
{
  int ch;

  if(currutmp) time(&currutmp->lastact);

  for (;;)
  {
    if (ibufsize == icurrchar)
    {
      fd_set readfds;
      struct timeval to;

      to.tv_sec = to.tv_usec = 0;
      FD_ZERO(&readfds);
      FD_SET(0, &readfds);
      if (i_newfd)
        FD_SET(i_newfd, &readfds);
      if ((ch = select(FD_SETSIZE, &readfds, NULL, NULL, &to)) <= 0)
      {
        if (flushf)
          (*flushf) ();

        if (dumb_term)
          oflush();
        else
          refresh();

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        if (i_newfd)
          FD_SET(i_newfd, &readfds);

        while ((ch = select(FD_SETSIZE, &readfds, NULL, NULL, i_top)) < 0)
        {
          if (errno == EINTR)
            continue;
          else
          {
            perror("select");
            return -1;
          }
        }
        if (ch == 0)
          return I_TIMEOUT;
      }
      if (i_newfd && FD_ISSET(i_newfd, &readfds))
        return I_OTHERDATA;

      while ((ibufsize = read(0, inbuf, IBUFSIZE)) <= 0)
      {
        if (ibufsize == 0)
          longjmp(byebye, -1);
        if (ibufsize < 0 && errno != EINTR)
          longjmp(byebye, -1);
      }
      icurrchar = 0;
    }

    i_mode = INPUT_ACTIVE;
    
    ch = inbuf[icurrchar++];
    return (ch);
  }
}

extern char oldmsg_count;            /* pointer */    
extern char watermode;

int igetch()
{
    register int ch;
    while(ch = dogetch())
    {
     switch (ch)
      {
       case Ctrl('L'):
         redoscr();
         continue;
       case Ctrl('I'):
         if(currutmp != NULL && currutmp->mode == MMENU)
         {
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
           vs_save(screen);
           t_idle();
           vs_restore(screen);
           continue;
         }
         else return(ch);

       case Ctrl('W'):
         if(currutmp != NULL && currutmp->mode)
         {
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
           vs_save(screen);
           DL_func("SO/dreye.so:main_dreye");
           vs_restore(screen);
           continue;
         }

       case Ctrl('Q'):  // wildcat : §Ö³tÂ÷¯¸ :p
         if(currutmp->mode && currutmp->mode != READING)
         {
           if(answer("½T©w­nÂ÷¯¸?? (y/N)") != 'y')
             return(ch);
           update_data();
           u_exit("ABORT");
           pressanykey("ÁÂÁÂ¥úÁ{, °O±o±`¨Ó³á !");
           exit(0);
         }
         else return (ch);

       case Ctrl('Z'):   /* wildcat:help everywhere */
       {
         static short re_entry = 0; /* CityLion: ¨¾­«¤Jªº... */
         if(currutmp && !re_entry && currutmp->mode != IDLE)
         {
           int mode0 = currutmp->mode;
           int stat0 = currstat;
           int more0 = inmore;
           int i;
           int old_roll = roll;
           int my_newfd = i_newfd;
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

           re_entry = 1;
           vs_save(screen);
           i = show_help(currutmp->mode);

           currutmp->mode = mode0;
           currstat = stat0;
           inmore = more0;
           roll = old_roll;
           i_newfd = my_newfd;
           vs_restore(screen);
           re_entry = 0;
           continue;
         }
         else return (ch);
       }
       case Ctrl('U'):
         resetutmpent();
         if(currutmp != NULL && currutmp->mode != EDITING &&
            currutmp->mode != LUSERS && currutmp->mode)
         {
           int mode0 = currutmp->mode;
           int stat0 = currstat;
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

           vs_save(screen);
           t_users();
           vs_restore(screen);

           currutmp->mode = mode0;
           currstat = stat0;

           continue;
         }
         else return (ch);

        case Ctrl('R'):
        {
          if(currutmp == NULL) return (ch);
          else if(watermode > 0)
          {
            watermode = (watermode + oldmsg_count)% oldmsg_count + 1;
            t_display_new(0);
            continue;
          }
          else if (!currutmp->mode && (currutmp->chatid[0] == 2 ||
               currutmp->chatid[0] == 3) && oldmsg_count && !watermode)
          {
            watermode=1;
            t_display_new(1);
            continue;
          }
          else if (currutmp->msgs[0].last_pid && currutmp->mode)
          {
            screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
            vs_save(screen);
            watermode=1;
            t_display_new(1);
            my_write(currutmp->msgs[0].last_pid, "¤ô²y¥á¦^¥h¡G");
            vs_restore(screen);
            continue;
          }
          else return (ch);
        }


        case '\n':   /* Ptt§â \n®³±¼ */
           continue;
        case Ctrl('T'):
          if(watermode > 0 )
          {
            watermode = (watermode + oldmsg_count - 2 )% oldmsg_count + 1;
            t_display_new(0);
            continue;
          }

        default:
          return (ch);
       }
    }
}

int
offset_count(char *prompt)   // Robert Liu 20010813
{
  int i=0, j=0, off=0;
  for(i=0 ; i<strlen(prompt) ; i++)
  {
    if(prompt[i]==27) off=1;
    if(off==0) j++;
    if(prompt[i]=='m' && off==1) off=0;
  }
  return (strlen(prompt)-j);
}

int
check_ctrlword(buf,len)
  char *buf;
  int len;
{
  int i;
  
  for(i = 0;i < len - 1;i++)
  {
    if(buf[i]=='\033')
    {
      return 1;
    }
    if(buf[i]=='\0')
      return 0;
  }
  return 0;
}

getdata(line, col, prompt, buf, len, echo, ans)
  int line, col;
  char *prompt, *buf, *ans;
  int len, echo;
{
  /*int ctrlword = 0;*/  /*¹w¨¾µo¥Í±¡ªp,©Ò¥H¯dµÛ...*/
		         /*¦pªG¦³¤H¥i¥H¯}¸Ñ,¦A§â¥L¥´¶}*/
		         /*³s¦P¾ú¥v¬ö¿ýªº¦a¤è¤]¥´¶}  by hialan*/
  register int ch;
  int clen;
  int x, y;
  int off_set = 0; /*add color*/
  extern unsigned char scr_cols;
#define MAXLASTCMD 6
  static char lastcmd[MAXLASTCMD][80];

  if (prompt)
  {
    move(line, col);
    clrtoeol();
    outs(prompt);
    off_set=offset_count(prompt); /*add color*/
  }
  else
    clrtoeol();

  if (dumb_term || !echo || /* echo == PASS || */echo == 9)
  {                     /* shakalaca.990422: ¬°¤F¿é¤J passwd ®É¦³¤Ï¥Õ */
    len--;              /* ¤U­±³o¬qµ{¦¡½X¬O¨S¦³¤Ï¥Õ (!echo) */
    clen = 0;
    while ((ch = igetch()) != '\r')
    {
      if (ch == '\n')
        break;
      if (ch == '\177' || ch == Ctrl('H'))
      {
        if (!clen)
        {
          bell();
          continue;
        }
        clen--;
        if (echo)
        {
          ochar(Ctrl('H'));
          ochar(' ');
          ochar(Ctrl('H'));
        }
        continue;
      }

#ifdef BIT8
      if (!isprint2(ch))
#else
      if (!isprint(ch))
#endif

      {
        if (echo)
          bell();
        continue;
      }
      if (clen >= len)
      {
        if (echo)
          bell();
        continue;
      }
      buf[clen++] = ch;
      if (echo && echo != 9)
        ochar(/* echo == PASS ? '-' : */ch); /* shakalaca.990422: ¬°¤F passwd */
    }
    buf[clen] = '\0';
    outc('\n');
    oflush();
  }
  else
  {
   int cmdpos = MAXLASTCMD -1;
   int currchar = 0;
   int keydown;
   int dirty;

    getyx(&y, &x);
    x=x-off_set;  /*add color by hialan*/
    standout();
    for (clen = len--; clen; clen--)
      outc(' ');
    standend();

    if (ans && check_ctrlword(ans,strlen(ans)) != 1) {
       int i;

       strncpy(buf, ans, len);
       buf[len] = 0;
       for (i = strlen(buf) + 1; i < len; i++)
          buf[i] = 0;
       
       move(y, x);
       edit_outs(buf);
       clen = currchar = strlen(buf);

    }
    else
       memset(buf, 0, len);

    dirty = 0;
    while (move(y, x + currchar), (ch = igetkey()) != '\r')
    {
/*
woju
*/
       keydown = 0;
       switch (ch) {
       case Ctrl('Y'): {
          int i;

          if (clen && dirty) {
             for (i = MAXLASTCMD - 1; i; i--)
                strcpy(lastcmd[i], lastcmd[i - 1]);
             strncpy(lastcmd[0], buf, len);
          }

          move(y, x);
          for (clen = len--; clen; clen--)
            outc(' ');
          memset(buf, '\0', strlen(buf));
          clen = currchar = strlen(buf);
          continue;
          }

       case KEY_DOWN:
       case Ctrl('N'):
          keydown = 1;
       case Ctrl('P'):
       case KEY_UP: {
          int i;

          if (clen && dirty) {
             for (i = MAXLASTCMD - 1; i; i--)
                strcpy(lastcmd[i], lastcmd[i - 1]);
             strncpy(lastcmd[0], buf, len);
          }

          i = cmdpos;
          do {
             if (keydown)
                --cmdpos;
             else
                ++cmdpos;
             if (cmdpos < 0)
                cmdpos = MAXLASTCMD - 1;
             else if (cmdpos == MAXLASTCMD)
                cmdpos = 0;
          } while (cmdpos != i && (!*lastcmd[cmdpos]
                   || !strncmp(buf, lastcmd[cmdpos], len)));
          if (cmdpos == i)
             continue;

          strncpy(buf, lastcmd[cmdpos], len);
          buf[len] = 0;

          move(y, x);                   /* clrtoeof */
          for (i = 0; i <= clen; i++)
             outc(' ');
          move(y, x);

          if (echo == PASS)
            passwd_outs(buf);
          else
            edit_outs(buf);
          clen = currchar = strlen(buf);
          dirty = 0;
          continue;
       }
       case KEY_ESC:
         if (KEY_ESC_arg == 'c')
            capture_screen();
            
         /*¦b¿é¤J±b¸¹±K½X®É«öEsc+n·|³Q½ð*/
         /*©Ò¥H§ï¦¨¿é¤Jpassword®É¤£¦æedit_note()*/
         /*³Ì«áÁÙ¬O©ñ±ó,§â¥¦ª`¸Ñ±¼....>"<    by hialan*/
/*
         if (KEY_ESC_arg == 'n')
         {
           if (echo != PASS)
            edit_note();
	 }
*/
         if (ch == 'U' && currstat != IDLE  &&
           !(currutmp->mode == 0 &&
           (currutmp->chatid[0] == 2 || currutmp->chatid[0] == 3)))
            t_users();
            continue;

/* yagami.000504 : ´å¼Ð¥i¨ì³Ì«e©Î³Ì«á */ 
/* wildcat : ¨ä¹ê«ö ctrl-a , ctrl-e ¤]¬O¤@¼Ëªº°Õ :p */
       case KEY_HOME:
         currchar = 0;
         break;
       case KEY_END:
         currchar = strlen(buf);
         break;

       case KEY_LEFT:
          if (currchar)
             --currchar;
          continue;
       case KEY_RIGHT:
          if (buf[currchar])
             ++currchar;
          continue;
       }

      if (ch == '\n' || ch == '\r')
         break;

      if (ch == Ctrl('I') && currstat != IDLE &&
          !(currutmp->mode == 0 &&
            (currutmp->chatid[0] == 2 || currutmp->chatid[0] == 3))) {
         t_idle();
         continue;
      }
      if (ch == '\177' || ch == Ctrl('H'))
      {
        if (currchar) {
           int i;

           currchar--;
           clen--;
           for (i = currchar; i <= clen; i++)
              buf[i] = buf[i + 1];
           move(y, x + clen);
           outc(' ');
           move(y, x);
           if (echo == PASS)
             passwd_outs(buf);
           else
             edit_outs(buf);
           dirty = 1;
        }
        continue;
      }
      if (ch == Ctrl('D')) {
        if (buf[currchar]) {
           int i;

           clen--;
           for (i = currchar; i <= clen; i++)
              buf[i] = buf[i + 1];
           move(y, x + clen);
           outc(' ');
           move(y, x);
           if (echo == PASS)
             passwd_outs(buf);
           else
             edit_outs(buf);
           dirty = 1;
        }
        continue;
      }
      if (ch == Ctrl('K')) {
         int i;

         buf[currchar] = 0;
         move(y, x + currchar);
         for (i = currchar; i < clen; i++)
            outc(' ');
         clen = currchar;
         dirty = 1;
         continue;
      }
      if (ch == Ctrl('A')) {
         currchar = 0;
         continue;
      }
      if (ch == Ctrl('E')) {
         currchar = clen;
         continue;
      }


      if (!(isprint2(ch)))
      {
        continue;
      }
      if (clen >= len || x + clen >= scr_cols)
      {
        continue;
      }
/*
woju
*/
      if (buf[currchar]) {               /* insert */
         int i;

         for (i = currchar; buf[i] && i < len && i < 80; i++)
            ;
         buf[i + 1] = 0;
         for (; i > currchar; i--)
            buf[i] = buf[i - 1];
      }
      else                              /* append */
         buf[currchar + 1] = '\0';

      buf[currchar] = ch;
      move(y, x + currchar);
      if (echo == PASS)
        passwd_outs(buf + currchar);
      else
      /* shakalaca.990422: ­ì¥»¥u¦³¤U­±¨º¦æ, ³o¬O¬°¤F¿é¤J passwd ¦³¤Ï¥Õ */
        edit_outs(buf + currchar);
      currchar++;
      clen++;
      dirty = 1;
    }
    buf[clen] = '\0';

//    ctrlword = check_ctrlword(buf,clen);
    
    if (clen > 1 && echo != PASS /*&& ctrlword != 1*/) {
    /* shaklaaca.990514: ^^^^^^^ ¤£Åý¿é¤Jªº password ¯d¤U¬ö¿ý */
    /* ¤£Åý¿é¤Jªº±±¨î½X¯d¤U¬ö¿ý ~~ by hialan */
       for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
          strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
       strncpy(lastcmd[0], buf, len);
    }
    if (echo) {
      move(y, x + clen);
      outc('\n');
    }
    refresh();
    

  }
  if ((echo == LCECHO) && ((ch = buf[0]) >= 'A') && (ch <= 'Z'))
    buf[0] = ch | 32;

  return clen;
}


char
getans(prompt)
  char *prompt;
{
  char ans[5];

  getdata(b_lines,0,prompt,ans,4,LCECHO,0);

  return ans[0];
}

/*
woju
*/
#define TRAP_ESC

#ifdef  TRAP_ESC
int KEY_ESC_arg;

int igetkey()
{
  int mode;
  int ch, last;

  mode = last = 0;
  while (1)
  {
    ch = igetch();
    if (mode == 0)
    {
      if (ch == KEY_ESC)
        mode = 1;
      else
        return ch;              /* Normal Key */
    }
    else if (mode == 1)
    {                           /* Escape sequence */
      if (ch == '[' || ch == 'O')
        mode = 2;
      else if (ch == '1' || ch == '4')
        mode = 3;
      else
      {
        KEY_ESC_arg = ch;
        return KEY_ESC;
      }
    }
    else if (mode == 2)
    {                           /* Cursor key */
      if (ch >= 'A' && ch <= 'D')
        return KEY_UP + (ch - 'A');
      else if (ch >= '1' && ch <= '6')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 3)
    {                           /* Ins Del Home End PgUp PgDn */
      if (ch == '~')
        return KEY_HOME + (last - '1');
      else
        return ch;
    }
    last = ch;
  }
}

#else                           /* TRAP_ESC */

int igetkey(void)
{
  int mode;
  int ch, last;

  mode = last = 0;
  while (1)
  {
    ch = igetch();
    if (ch == KEY_ESC)
      mode = 1;
    else if (mode == 0)         /* Normal Key */
      return ch;
    else if (mode == 1)
    {                           /* Escape sequence */
      if (ch == '[' || ch == 'O')
        mode = 2;
      else if (ch == '1' || ch == '4')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 2)
    {                           /* Cursor key */
      if (ch >= 'A' && ch <= 'D')
        return KEY_UP + (ch - 'A');
      else if (ch >= '1' && ch <= '6')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 3)
    {                           /* Ins Del Home End PgUp PgDn */
      if (ch == '~')
        return KEY_HOME + (last - '1');
      else
        return ch;
    }
    last = ch;
  }
}
#endif                          /* TRAP_ESC */



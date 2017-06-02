/*-------------------------------------------------------*/
/* term.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : termcap I/O control routines                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"
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
// wildcat 調大一點看看
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

char *tparm(const char *str, ...) {
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


void
init_tty()
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

extern ochar();

int t_lines = 24;
int b_lines = 23;
int p_lines = 20;
int t_columns = 80;

int automargins;

char *outp;
int *outlp;


static
outcf(ch)
  char ch;
{
  if (*outlp < TERMCOMSIZE)
  {
    (*outlp)++;
    *outp++ = ch;
  }
}


int
term_init(term)
  char *term;
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

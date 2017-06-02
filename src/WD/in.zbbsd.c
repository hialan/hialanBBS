/*-------------------------------------------------------*/
/* in.zbbsd.c   ( NTHU CS MapleBBS Ver 2.39 )            */
/*-------------------------------------------------------*/
/* target : super telnet server for BBS                  */
/* create : 95/03/29                                     */
/* update : 96/10/10 (modified by woju)                  */
/*-------------------------------------------------------*/
/* syntax : (1) login as root                            */
/*          (2) in.zbbsd {ports} [bbsuser] [bbs-public-user-without-passwd]
*/
/*-------------------------------------------------------*/
/* (1) standalone telnet daemon                          */
/* (2) CPU/system loading daemon                         */
/* (3) replace /bin/login                                */
/* (4) replace ~bbs/bin/bbsrf                            */
/* (5) speed up pty/tty search by shared memory          */
/*-------------------------------------------------------*/

#include <pwd.h>
#include "config.h"
#undef MAXPATHLEN

#define BAD_HOST BBSHOME"/etc/bad_host"

char bbsprog[80];
char *bbsuser2, *bbsuser, *bbshome, *bbsshell;
int bbsuid, bbsgid;


#define HASHING
#define HAVE_CHKLOAD
#define NUMTTYS         256
#define QLEN            5
#define MSG_WORKING     "\r\n[1;33m³s½u¤¤...[m"
#define SYNC_PERIOD     ( 8 * 60 + 17 )
#define BUSY_PERIOD     55      /* less than timeout = 75 */


#define TH_LOW          36
#define TH_HIGH         40


#define PTYSIZ          2048    /* ¨Ó¦Û pty ¤§ buffer size */
#define NETSIZ          2048    /* °e©¹ net ¤§ buffer size */


#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/telnet.h>

#ifndef LINUX
   #include <ttyent.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <netdb.h>
#include <syslog.h>
#include <ctype.h>

#include <unistd.h>
#include <stdlib.h>

#ifdef STREAM_PTY
#include <sys/stropts.h>
#include "../lib/tiocpkt.h"
extern char *ptsname();

#define PTY_PKT_READ    pty_pkt_read
#else
#define PTY_PKT_READ    read
#endif

#ifndef O_RDWR
#include <sys/fcntl.h>
#endif

#ifndef FIONBIO
#include <sys/filio.h>
#endif

#ifndef FIONBIO
#include <sys/ioctl.h>
#endif

#if !defined(STREAM_PTY) && !defined(TIOCPKT)
#include <sys/pty.h>
#endif

#include <string.h>
#define bcopy(s,d,l)    memcpy(d,s,l)

#ifdef __STDC__
#define puts            xputs   /* prototype conflict */
#endif

/* Ultrix syslog(3) has no facility stuff. */

#ifndef LOG_DAEMON
#define LOG_DAEMON      0
#define LOG_ODELAY      0
#endif

#ifdef ultrix
#define setsid()        setpgrp(0,0)
#endif

#define OPT_NO                  0       /* won't do this option */
#define OPT_YES                 1       /* will do this option */
#define OPT_YES_BUT_ALWAYS_LOOK 2
#define OPT_NO_BUT_ALWAYS_LOOK  3
char hisopts[256];
char myopts[256];

char doopt[] = {IAC, DO, '%', 'c', 0};
char dont[] = {IAC, DONT, '%', 'c', 0};
char will[] = {IAC, WILL, '%', 'c', 0};
char wont[] = {IAC, WONT, '%', 'c', 0};

/* I/O data buffers, pointers, and counters. */

char maple[64];

char ptyibuf[PTYSIZ], *ptyip = ptyibuf;

char ptyobuf[BUFSIZ], *pfrontp = ptyobuf, *pbackp = ptyobuf;

char netibuf[BUFSIZ], *netip = netibuf;

#define NIACCUM(c)      {   *netip++ = c; \
                            ncc++; \
                        }

char netobuf[NETSIZ], *nfrontp = netobuf, *nbackp = netobuf;
char *neturg = 0;               /* one past last bye of urgent data */

/* the remote system seems to NOT be an old 4.2 */
int not42 = 1;


//#ifdef  HAVE_CHKLOAD
//#define BANNER  "\r\n¡i%s¡j¡· ¹q¤l¥¬§iÄæ¨t²Î ¡· (%s)\r\nPowered by FreeBSD (¨t²Î­t¸ü %s) [%s:%s]\r\n\n"
//#else
#define BANNER  "\r\n¡i"BOARDNAME"¡j¡· ¹q¤l¥¬§iÄæ¨t²Î ¡· ("MYIP")\r\nPowered by FreeBSD [%s]\r\n\n"
//#endif

/* buffer for sub-options */
char subbuffer[128], *subpointer = subbuffer, *subend = subbuffer;

#define SB_CLEAR()      subpointer = subbuffer;
#define SB_TERM()       { subend = subpointer; SB_CLEAR(); }
#define SB_ACCUM(c)     if (subpointer < (subbuffer+sizeof subbuffer)) { \
                                *subpointer++ = (c); \
                        }
#define SB_GET()        ((*subpointer++)&0xff)
#define SB_EOF()        (subpointer >= subend)

int pcc, ncc;
int pty, net;

#ifdef STREAM_PTY
int pts;
#endif
int inter;

#ifndef SYSV_ENV
extern char **environ;
#endif
extern int errno;

static char line[] = "/dev/ptyXX";
char *terminaltype = 0;
int SYNCHing = 0;               /* we are in TELNET SYNCH mode */

#ifdef  HAVE_UTMP
int utmp_slot;
#endif

gid_t mygid;


/*
 * The following are some clocks used to decide how to interpret the
 * relationship between various variables.
 */

struct
{
  int
      system,                   /* what the current time is */
      echotoggle,               /* last time user entered echo character */
      modenegotiated,           /* last time operating mode negotiated */
      didnetreceive,            /* last time we read data from network */
      ttypeopt,                 /* ttype will/won't received */
      ttypesubopt,              /* ttype subopt is received */
      getterminal,              /* time started to get terminal information */
      gotDM;                    /* when did we last see a data mark */
}      clocks;

#define settimer(x)     (clocks.x = ++clocks.system)
#define sequenceIs(x,y) (clocks.x < clocks.y)
time_t time();


/* ----------------------------------------------------- */


static int rfds;                /* read file descriptor set */
static struct sockaddr_in xsin;


#include <sys/ipc.h>
#include <sys/shm.h>


#define PPT_SHMKEY      889


struct PPT_SHM
{
  time_t uptime;
  int visit;
  int busy;
  int load;
  pid_t ppt[NUMTTYS];
}      *ppt_shm = NULL;


int slot;


static void
ipc_err(name)
  char *name;
{
  fprintf(stderr, "[%s] error\n", name);
  exit(1);
}


static void
resolve_pptshm()
{
  register int i;
  register struct PPT_SHM *shm;

  if (ppt_shm == NULL)
  {
    i = shmget(PPT_SHMKEY, sizeof(struct PPT_SHM), 0);
    if (i < 0)
    {
      i = shmget(PPT_SHMKEY, sizeof(struct PPT_SHM), IPC_CREAT | 0600);
      if (i < 0)
        ipc_err("shmget");
      shm = (struct PPT_SHM *) shmat(i, NULL, 0);
      if (shm < 0)
        ipc_err("shmat");
      memset(shm, 0, sizeof(struct PPT_SHM));
      (void) time(&shm->uptime);
    }
    else
    {
      shm = (struct PPT_SHM *) shmat(i, NULL, 0);
      if (shm < 0)
        ipc_err("shmat");
    }
    ppt_shm = shm;
  }
}


/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol       */
/* ----------------------------------------------------- */

void ptyflush();
void dooption();
void telrcv();

void
cleanup()
{
  char *p;

  p = line + sizeof("/dev/") - 1;

#ifdef SYSV_UTMP
  UTMP_LOGOUT(p);
  (void) chown(line, 0, 0);
  (void) chmod(line, 0644);
#else                           /* SYSV_UTMP */

#ifdef  HAVE_UTMP
  logout(p);
#endif

  (void) chmod(line, 0666);
  (void) chown(line, 0, 0);

  *p = 'p';
  (void) chmod(line, 0666);
  (void) chown(line, 0, 0);
#endif                          /* SYSV_UTMP */

  resolve_pptshm();
  ppt_shm->ppt[slot] = 0;
  ppt_shm->visit--;

#if 0
  visit = ppt_shm->visit - 1;
  if (visit >= 0)
    ppt_shm->visit = visit;
#endif

  shutdown(net, 2);
  exit(0);
}


void
fatal(f, msg)
  int f;
  char *msg;
{
  char buf[256];

  (void) sprintf(buf, "%s\r\n", msg);
  (void) write(f, buf, strlen(buf));
  exit(1);
}


void
fatalperror(f, msg)
  int f;
  char *msg;
{
  char buf[256];
#ifndef BSD44
  extern char *sys_errlist[];
#endif

  (void) sprintf(buf, "%s: %s\r\n", msg, sys_errlist[errno]);
  fatal(f, buf);
}


#if 0
mode(on, off)
  int on, off;
{
  struct sgttyb b;

#ifdef STREAM_PTY
  int pty = pts;                /* XXX apply ioctl()s at slave end */
#endif

  ptyflush();
  ioctl(pty, TIOCGETP, &b);
  b.sg_flags |= on;
  b.sg_flags &= ~off;
  ioctl(pty, TIOCSETP, &b);
}

#else

void
telopt_echo(on)
  int on;
{
  struct termios b;

/*
woju
*/
  on = 0;
#ifdef STREAM_PTY
  int pty = pts;
#endif

  ptyflush();
  tcgetattr(pty, &b);
  if (on)
  {
    b.c_lflag |= ECHO;
  }
  else
  {
    b.c_lflag &= ~ECHO;
  }
  tcsetattr(pty, TCSANOW, &b);
}


void
telopt_binary(on)
  int on;
{
  struct termios b;

/*
woju
*/
  on = 1;
#ifdef STREAM_PTY
  int pty = pts;
#endif

  ptyflush();
  tcgetattr(pty, &b);
  if (on)
  {
    b.c_oflag &= ~OPOST;
  }
  else
  {
    b.c_oflag |= OPOST;
  }
  tcsetattr(pty, TCSANOW, &b);
}
#endif

/*
 * Send interrupt to process on other side of pty. If it is in raw mode, just
 * write NULL; otherwise, write intr char.
 */

void
interrupt()
{
  struct termios b;

#ifdef STREAM_PTY
  int pty = pts;                /* XXX apply ioctl()s at slave end */
#endif

  ptyflush();                   /* half-hearted */
  tcgetattr(pty, &b);
  if ((b.c_lflag & ICANON) == 0)
  {
    *pfrontp++ = '\0';
    return;
  }
  if (b.c_cc[VINTR] != 0377)
    *pfrontp++ = b.c_cc[VINTR];
}

/*
 * Send quit to process on other side of pty. If it is in raw mode, just
 * write NULL; otherwise, write quit char.
 */

void
sendbrk()
{
  struct termios b;

#ifdef STREAM_PTY
  int pty = pts;                /* XXX apply ioctl()s at slave end */
#endif

  ptyflush();                   /* half-hearted */
  tcgetattr(pty, &b);
  if ((b.c_lflag & ICANON) == 0)
  {
    *pfrontp++ = '\0';
    return;
  }
  if (b.c_cc[VQUIT] != 0377)
    *pfrontp++ = b.c_cc[VQUIT];
}

void
ptyflush()
{
  register int n;

  if ((n = pfrontp - pbackp) > 0)
    n = write(pty, pbackp, n);
  if (n >= 0)
  {
    pbackp += n;
    if (pbackp == pfrontp)
      pbackp = pfrontp = ptyobuf;
  }
}


/*
 * nextitem()
 *
 * Return the address of the next "item" in the TELNET data stream.  This will
 * be the address of the next character if the current address is a user data
 * character, or it will be the address of the character following the TELNET
 * command if the current address is a TELNET IAC ("I Am a Command")
 * character.
 */

char *
nextitem(current)
  char *current;
{
  if ((*current & 0xff) != IAC)
  {
    return current + 1;
  }
  switch (*(current + 1) & 0xff)
  {
  case DO:
  case DONT:
  case WILL:
  case WONT:
    return current + 3;
  case SB:                      /* loop forever looking for the SE */
    {
      register char *look = current + 2;

      for (;;)
      {
        if ((*look++ & 0xff) == IAC)
        {
          if ((*look++ & 0xff) == SE)
          {
            return look;
          }
        }
      }
    }
  default:
    return current + 2;
  }
}


/*
 * netclear()
 *
 * We are about to do a TELNET SYNCH operation.  Clear the path to the network.
 *
 * Things are a bit tricky since we may have sent the first byte or so of a
 * previous TELNET command into the network. So, we have to scan the network
 * buffer from the beginning until we are up to where we want to be.
 *
 * A side effect of what we do, just to keep things simple, is to clear the
 * urgent data pointer.  The principal caller should be setting the urgent
 * data pointer AFTER calling us in any case.
 */

void
netclear()
{
  register char *thisitem, *next;
  char *good;

#define wewant(p)       ((nfrontp > p) && ((*p&0xff) == IAC) && \
                                ((*(p+1)&0xff) != EC) && ((*(p+1)&0xff) != EL))

  thisitem = netobuf;

  while ((next = nextitem(thisitem)) <= nbackp)
  {
    thisitem = next;
  }

  /* Now, thisitem is first before/at boundary. */

  good = netobuf;               /* where the good bytes go */

  while (nfrontp > thisitem)
  {
    if (wewant(thisitem))
    {
      int length;

      next = thisitem;
      do
      {
        next = nextitem(next);
      } while (wewant(next) && (nfrontp > next));
      length = next - thisitem;
      bcopy(thisitem, good, length);
      good += length;
      thisitem = next;
    }
    else
    {
      thisitem = nextitem(thisitem);
    }
  }

  nbackp = netobuf;
  nfrontp = good;               /* next byte to be sent */
  neturg = 0;
}


/*
 * netflush Send as much data as possible to the network, handling requests
 * for urgent data.
 */

void
netflush()
{
  register int n;

  if ((n = nfrontp - nbackp) > 0)
  {
    /*
     * if no urgent data, or if the other side appears to be an old 4.2
     * client (and thus unable to survive TCP urgent data), write the entire
     * buffer in non-OOB mode.
     */
    if ((neturg == 0) || (not42 == 0))
    {
      n = write(net, nbackp, n);/* normal write */
    }
    else
    {
      n = neturg - nbackp;
      /*
       * In 4.2 (and 4.3) systems, there is some question about what byte in
       * a sendOOB operation is the "OOB" data. To make ourselves compatible,
       * we only send ONE byte out of band, the one WE THINK should be OOB
       * (though we really have more the TCP philosophy of urgent data rather
       * than the Unix philosophy of OOB data).
       */
      if (n > 1)
      {
        n = send(net, nbackp, n - 1, 0);        /* send URGENT all by itself */
      }
      else
      {
        n = send(net, nbackp, n, MSG_OOB);      /* URGENT data */
      }
    }
  }
  if (n < 0)
  {

#if 0
    if (errno == EWOULDBLOCK)
      return;
#endif

    /* should blow this guy away... */
    return;
  }
  nbackp += n;
  if (nbackp >= neturg)
  {
    neturg = 0;
  }
  if (nbackp == nfrontp)
  {
    nbackp = nfrontp = netobuf;
  }
}


void
willoption(option)
  int option;
{
  char *fmt;

  switch (option)
  {
  case TELOPT_BINARY:
    telopt_binary(1);
    fmt = doopt;
    break;

  case TELOPT_ECHO:
    not42 = 0;                  /* looks like a 4.2 system */
    /*
     * Now, in a 4.2 system, to break them out of ECHOing (to the terminal)
     * mode, we need to send a "WILL ECHO". Kludge upon kludge!
     */
    if (myopts[TELOPT_ECHO] == OPT_YES)
    {
      dooption(TELOPT_ECHO);
    }
    fmt = dont;
    break;

  case TELOPT_TTYPE:
    settimer(ttypeopt);
    if (hisopts[TELOPT_TTYPE] == OPT_YES_BUT_ALWAYS_LOOK)
    {
      hisopts[TELOPT_TTYPE] = OPT_YES;
      return;
    }
    fmt = doopt;
    break;

  case TELOPT_SGA:
    fmt = doopt;
    break;

  case TELOPT_TM:
    fmt = dont;
    break;

  default:
    fmt = dont;
    break;
  }
  if (fmt == doopt)
  {
    hisopts[option] = OPT_YES;
  }
  else
  {
    hisopts[option] = OPT_NO;
  }
  (void) sprintf(nfrontp, fmt, option);
  nfrontp += sizeof(dont) - 2;
}


void
wontoption(option)
  int option;
{
  char *fmt;

  switch (option)
  {
  case TELOPT_ECHO:
    not42 = 1;                  /* doesn't seem to be a 4.2 system */
    break;

  case TELOPT_BINARY:
    telopt_binary(0);
    break;

  case TELOPT_TTYPE:
    settimer(ttypeopt);
    break;
  }

  fmt = dont;
  hisopts[option] = OPT_NO;
  (void) sprintf(nfrontp, fmt, option);
  nfrontp += sizeof(doopt) - 2;
}


void
dooption(option)
  int option;
{
  char *fmt;

  switch (option)
  {
  case TELOPT_ECHO:
    telopt_echo(1);
    fmt = will;
    break;

  case TELOPT_BINARY:
    telopt_binary(1);
    fmt = will;
    break;

  case TELOPT_SGA:
    fmt = will;
    break;

  case TELOPT_TM:
  default:
    fmt = wont;
  }
  if (fmt == will)
  {
    myopts[option] = OPT_YES;
  }
  else
  {
    myopts[option] = OPT_NO;
  }
  (void) sprintf(nfrontp, fmt, option);
  nfrontp += sizeof(doopt) - 2;
}


void
dontoption(option)
  int option;
{
  char *fmt;

  switch (option)
  {
  case TELOPT_ECHO:             /* we should stop echoing */
    telopt_echo(0);
  default:
    fmt = wont;
  }

  if (fmt = wont)
  {
    myopts[option] = OPT_NO;
  }
  else
  {
    myopts[option] = OPT_YES;
  }
  (void) sprintf(nfrontp, fmt, option);
  nfrontp += sizeof(wont) - 2;
}


/*
 * suboption()
 *
 * Look at the sub-option buffer, and try to be helpful to the other side.
 *
 * Currently we recognize:
 *
 * Terminal type is
 */

void
suboption()
{
  switch (SB_GET())
  {
    case TELOPT_TTYPE:
    {                           /* Yaaaay! */
      static char terminalname[5 + 41] = "TERM=";

      settimer(ttypesubopt);

      if (SB_GET() != TELQUAL_IS)
      {
        return;                 /* ??? XXX but, this is the most robust */
      }

      terminaltype = terminalname + strlen(terminalname);

      while ((terminaltype < (terminalname + sizeof terminalname - 1)) &&
        !SB_EOF())
      {
        register int c;

        c = SB_GET();
        if (isupper(c))
        {
          c = tolower(c);
        }
        *terminaltype++ = c;    /* accumulate name */
      }
      *terminaltype = 0;
      terminaltype = terminalname;
      break;
    }
  }
}


/*
 * ttloop
 *
 * A small subroutine to flush the network output buffer, get some data from the
 * network, and pass it through the telnet state machine.  We also flush the
 * pty input buffer (by dropping its data) if it becomes too full.
 */

void
ttloop()
{
  if (nfrontp - nbackp)
  {
    netflush();
  }
  ncc = read(net, netibuf, sizeof netibuf);
  if (ncc < 0)
  {
    syslog(LOG_INFO, "ttloop: read: %m\n");
    cleanup();                  /* exit(1); */
  }
  else if (ncc == 0)
  {
    syslog(LOG_INFO, "ttloop: peer died: %m\n");
    cleanup();                  /* exit(1); */
  }
  netip = netibuf;
  telrcv();                     /* state machine */
  if (ncc > 0)
  {
    pfrontp = pbackp = ptyobuf;
    telrcv();
  }
}


/* Check a descriptor to see if out of band data exists on it. */


int
stilloob(s)
  int s;                        /* socket number */
{
  static struct timeval timeout = {0};
  fd_set excepts;
  int value;

  do
  {
    FD_ZERO(&excepts);
    FD_SET(s, &excepts);
    value = select(s + 1, (fd_set *) 0, (fd_set *) 0, &excepts, &timeout);
  } while ((value == -1) && (errno == EINTR));

  if (value < 0)
  {
    fatalperror(pty, "select");
  }
  return (FD_ISSET(s, &excepts));
}


/*
 * Main loop.  Select from pty and network, and hand data to telnet receiver
 * finite state machine.
 */


void
telnet(f, p)
{
  int on = 1;

#ifdef  VERBOSE
  char hostname[MAXHOSTNAMELEN];

#define TABBUFSIZ       512
  char defent[TABBUFSIZ];
  char defstrs[TABBUFSIZ];
#undef  TABBUFSIZ

  char *HE;
  char *HN;
  char *IM;
#endif

  ioctl(f, FIONBIO, &on);
  ioctl(p, FIONBIO, &on);

#ifndef STREAM_PTY
  ioctl(p, TIOCPKT, &on);
#endif

#if     defined(SO_OOBINLINE)
  setsockopt(net, SOL_SOCKET, SO_OOBINLINE, (char *) &on, sizeof on);
#endif                          /* defined(SO_OOBINLINE) */

  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  /*
   * Ignoring SIGTTOU keeps the kernel from blocking us in ttioctl() in
   * /sys/tty.c.
   */
  signal(SIGTTOU, SIG_IGN);
  signal(SIGCHLD, cleanup);
  setsid();

  /* Request to do remote echo and to suppress go ahead. */
  if (!myopts[TELOPT_ECHO])
  {
    dooption(TELOPT_ECHO);
  }
  if (!myopts[TELOPT_SGA])
  {
    dooption(TELOPT_SGA);
  }
  /*
   * Is the client side a 4.2 (NOT 4.3) system?  We need to know this because
   * 4.2 clients are unable to deal with TCP urgent data.
   *
   * To find out, we send out a "DO ECHO".  If the remote system answers "WILL
   * ECHO" it is probably a 4.2 client, and we note that fact ("WILL ECHO"
   * ==> that the client will echo what WE, the server, sends it; it does NOT
   * mean that the client will echo the terminal input).
   */
  (void) sprintf(nfrontp, doopt, TELOPT_ECHO);
  nfrontp += sizeof doopt - 2;
  hisopts[TELOPT_ECHO] = OPT_YES_BUT_ALWAYS_LOOK;

  /*
   * Show banner that getty never gave.
   *
   * We put the banner in the pty input buffer.  This way, it gets carriage
   * return null processing, etc., just like all other pty --> client data.
   */

#ifdef  VERBOSE
  gethostname(hostname, sizeof(hostname));

  if (getent(defent, "default") == 1)
  {
    char *getstr();
    char *p = defstrs;

    HE = getstr("he", &p);
    HN = getstr("hn", &p);
    IM = getstr("im", &p);
    if (HN && *HN)
      strcpy(hostname, HN);
    edithost(HE, hostname);
    if (IM && *IM)
      putf(IM, ptyibuf + 1);
  }
  else
#endif

#ifdef  HAVE_CHKLOAD
    sprintf(ptyibuf + 1, BANNER, maple, line + 5);
#else
    sprintf(ptyibuf + 1, BANNER);
#endif

  ptyip = ptyibuf + 1;          /* Prime the pump */
  pcc = strlen(ptyip);          /* ditto */

  /* Clear ptybuf[0] - where the packet information is received */
  ptyibuf[0] = 0;

  /*
   * Call telrcv() once to pick up anything received during terminal type
   * negotiation.
   */
  telrcv();

  for (;;)
  {
    fd_set ibits, obits, xbits;
    register int c;

    if (ncc < 0 && pcc < 0)
      break;

    FD_ZERO(&ibits);
    FD_ZERO(&obits);
    FD_ZERO(&xbits);
    /*
     * Never look for input if there's still stuff in the corresponding
     * output buffer
     */
    if (nfrontp - nbackp || pcc > 0)
    {
      FD_SET(f, &obits);
      FD_SET(p, &xbits);
    }
    else
    {
      FD_SET(p, &ibits);
    }
    if (pfrontp - pbackp || ncc > 0)
    {
      FD_SET(p, &obits);
    }
    else
    {
      FD_SET(f, &ibits);
    }
    if (!SYNCHing)
    {
      FD_SET(f, &xbits);
    }
    if ((c = select(8, &ibits, &obits, &xbits,
          (struct timeval *) 0)) < 1)
    {
      if (c == -1)
      {
        if (errno == EINTR)
        {
          continue;
        }
      }
      sleep(3);
      continue;
    }

    /* Any urgent data? */
    if (FD_ISSET(net, &xbits))
    {
      SYNCHing = 1;
    }

    /* Something to read from the network... */
    if (FD_ISSET(net, &ibits))
    {

#if     !defined(SO_OOBINLINE)
      /*
       * In 4.2 (and 4.3 beta) systems, the OOB indication and data handling
       * in the kernel is such that if two separate TCP Urgent requests come
       * in, one byte of TCP data will be overlaid. This is fatal for Telnet,
       * but we try to live with it.
       *
       * In addition, in 4.2 (and...), a special protocol is needed to pick up
       * the TCP Urgent data in the correct sequence.
       *
       * What we do is:  if we think we are in urgent mode, we look to see if we
       * are "at the mark". If we are, we do an OOB receive.  If we run this
       * twice, we will do the OOB receive twice, but the second will fail,
       * since the second time we were "at the mark", but there wasn't any
       * data there (the kernel doesn't reset "at the mark" until we do a
       * normal read). Once we've read the OOB data, we go ahead and do
       * normal reads.
       *
       * There is also another problem, which is that since the OOB byte we read
       * doesn't put us out of OOB state, and since that byte is most likely
       * the TELNET DM (data mark), we would stay in the TELNET SYNCH
       * (SYNCHing) state. So, clocks to the rescue.  If we've "just"
       * received a DM, then we test for the presence of OOB data when the
       * receive OOB fails (and AFTER we did the normal mode read to clear
       * "at the mark").
       */
      if (SYNCHing)
      {
        int atmark;

        ioctl(net, SIOCATMARK, (char *) &atmark);
        if (atmark)
        {
          ncc = recv(net, netibuf, sizeof(netibuf), MSG_OOB);
          if ((ncc == -1) && (errno == EINVAL))
          {
            ncc = read(net, netibuf, sizeof(netibuf));
            if (sequenceIs(didnetreceive, gotDM))
            {
              SYNCHing = stilloob(net);
            }
          }
        }
        else
        {
          ncc = read(net, netibuf, sizeof(netibuf));
        }
      }
      else
      {
        ncc = read(net, netibuf, sizeof(netibuf));
      }
      settimer(didnetreceive);
#else                           /* !defined(SO_OOBINLINE)) */
      ncc = read(net, netibuf, sizeof(netibuf));
#endif                          /* !defined(SO_OOBINLINE)) */

      if (ncc < 0 && errno == EWOULDBLOCK)
        ncc = 0;
      else
      {
        if (ncc <= 0)
        {
          break;
        }
        netip = netibuf;
      }
    }

    /* Something to read from the pty... */
    if (FD_ISSET(p, &xbits))
    {
      if (PTY_PKT_READ(p, ptyibuf, 1) != 1)
      {
        break;
      }
    }
    if (FD_ISSET(p, &ibits))
    {
      pcc = PTY_PKT_READ(p, ptyibuf, PTYSIZ);
      if (pcc < 0 && errno == EWOULDBLOCK)
        pcc = 0;
      else
      {
        if (pcc <= 0)
          break;
        /* Skip past "packet" */
        pcc--;
        ptyip = ptyibuf + 1;
      }
    }
    if (ptyibuf[0] & TIOCPKT_FLUSHWRITE)
    {
      netclear();               /* clear buffer back */
      *nfrontp++ = IAC;
      *nfrontp++ = DM;
      neturg = nfrontp - 1;     /* off by one XXX */
      ptyibuf[0] = 0;
    }

    while (pcc > 0)
    {
      if ((&netobuf[NETSIZ] - nfrontp) < 2)
        break;
      c = *ptyip++ & 0377, pcc--;
      if (c == IAC)
        *nfrontp++ = c;
      *nfrontp++ = c;
      /* Don't do CR-NUL if we are in binary mode */
      if ((c == '\r') && (myopts[TELOPT_BINARY] == OPT_NO))
      {
        if (pcc > 0 && ((*ptyip & 0377) == '\n'))
        {
          *nfrontp++ = *ptyip++ & 0377;
          pcc--;
        }
        else
          *nfrontp++ = '\0';
      }
    }
    if (FD_ISSET(f, &obits) && (nfrontp - nbackp) > 0)
      netflush();
    if (ncc > 0)
      telrcv();
    if (FD_ISSET(p, &obits) && (pfrontp - pbackp) > 0)
      ptyflush();
  }
  cleanup();
}

/* State for recv fsm */

#define TS_DATA         0       /* base state */
#define TS_IAC          1       /* look for double IAC's */
#define TS_CR           2       /* CR-LF ->'s CR */
#define TS_SB           3       /* throw away begin's... */
#define TS_SE           4       /* ...end's (suboption negotiation) */
#define TS_WILL         5       /* will option negotiation */
#define TS_WONT         6       /* wont " */
#define TS_DO           7       /* do " */
#define TS_DONT         8       /* dont " */


void
telrcv()
{
  register int c;
  static int state = TS_DATA;

#ifdef STREAM_PTY
  int pty = pts;                /* XXX apply ioctl()s at slave end */
#endif

  while (ncc > 0)
  {
    if ((&ptyobuf[BUFSIZ] - pfrontp) < 2)
      return;
    c = *netip++ & 0377, ncc--;
    switch (state)
    {

    case TS_CR:
      state = TS_DATA;
      /* Strip off \n or \0 after a \r */
      if ((c == 0) || (c == '\n'))
      {
        break;
      }
      /* FALL THROUGH */

    case TS_DATA:
      if (c == IAC)
      {
        state = TS_IAC;
        break;
      }
      if (inter > 0)
        break;
      /*
       * We now map \r\n ==> \r for pragmatic reasons. Many client
       * implementations send \r\n when the user hits the CarriageReturn key.
       *
       * We USED to map \r\n ==> \n, since \r\n says that we want to be in
       * column 1 of the next printable line, and \n is the standard unix way
       * of saying that (\r is only good if CRMOD is set, which it normally
       * is).
       */
      if ((c == '\r') && (hisopts[TELOPT_BINARY] == OPT_NO))
      {
        state = TS_CR;
      }
      *pfrontp++ = c;
      break;

    case TS_IAC:
      switch (c)
      {

        /*
         * Send the process on the pty side an interrupt.  Do this with a
         * NULL or interrupt char; depending on the tty mode.
         */
      case IP:
        interrupt();
        break;

      case BREAK:
        sendbrk();
        break;

        /* Are You There? */
      case AYT:
        strcpy(nfrontp, "\r\n[Yes]\r\n");
        nfrontp += 9;
        break;

        /* Abort Output */
      case AO:
        {
          ptyflush();           /* half-hearted */
          tcflush(pty, TCOFLUSH);
          netclear();           /* clear buffer back */
          *nfrontp++ = IAC;
          *nfrontp++ = DM;
          neturg = nfrontp - 1; /* off by one XXX */
          break;
        }

        /* Erase Character and Erase Line */
      case EC:
      case EL:
        {
          struct termios b;
          char ch;

          ptyflush();           /* half-hearted */
          tcgetattr(pty, &b);
          ch = (c == EC) ?
            b.c_cc[VERASE] : b.c_cc[VKILL];
          if (ch != '\377')
          {
            *pfrontp++ = ch;
          }
          break;
        }

        /* Check for urgent data... */
      case DM:
        SYNCHing = stilloob(net);
        settimer(gotDM);
        break;


        /* Begin option subnegotiation... */
      case SB:
        state = TS_SB;
        continue;

      case WILL:
        state = TS_WILL;
        continue;

      case WONT:
        state = TS_WONT;
        continue;

      case DO:
        state = TS_DO;
        continue;

      case DONT:
        state = TS_DONT;
        continue;

      case IAC:
        *pfrontp++ = c;
        break;
      }
      state = TS_DATA;
      break;

    case TS_SB:
      if (c == IAC)
      {
        state = TS_SE;
      }
      else
      {
        SB_ACCUM(c);
      }
      break;

    case TS_SE:
      if (c != SE)
      {
        if (c != IAC)
        {
          SB_ACCUM(IAC);
        }
        SB_ACCUM(c);
        state = TS_SB;
      }
      else
      {
        SB_TERM();
        suboption();            /* handle sub-option */
        state = TS_DATA;
      }
      break;

    case TS_WILL:
      if (hisopts[c] != OPT_YES)
        willoption(c);
      state = TS_DATA;
      continue;

    case TS_WONT:
      if (hisopts[c] != OPT_NO)
        wontoption(c);
      state = TS_DATA;
      continue;

    case TS_DO:
      if (myopts[c] != OPT_YES)
        dooption(c);
      state = TS_DATA;
      continue;

    case TS_DONT:
      if (myopts[c] != OPT_NO)
      {
        dontoption(c);
      }
      state = TS_DATA;
      continue;

    default:
      syslog(LOG_ERR, "panic state=%d\n", state);
      cleanup();                /* exit(1) */
    }
  }
}


/*
 * getterminaltype
 *
 * Ask the other end to send along its terminal type. Output is the variable
 * terminaltype filled in.
 */

void
getterminaltype()
{
  static char sbuf[] = {IAC, DO, TELOPT_TTYPE};

  settimer(getterminal);
  bcopy(sbuf, nfrontp, sizeof sbuf);
  nfrontp += sizeof sbuf;
  hisopts[TELOPT_TTYPE] = OPT_YES_BUT_ALWAYS_LOOK;
  while (sequenceIs(ttypeopt, getterminal))
  {
    ttloop();
  }
  if (hisopts[TELOPT_TTYPE] == OPT_YES)
  {
    static char sbbuf[] = {IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE};

    bcopy(sbbuf, nfrontp, sizeof sbbuf);
    nfrontp += sizeof sbbuf;
    while (sequenceIs(ttypesubopt, getterminal))
    {
      ttloop();
    }
  }
}


/* --------------------------------- */
/* log in BBS                        */
/* --------------------------------- */


#include <sys/types.h>
#include <grp.h>
#include <sys/resource.h>
#include <utmp.h>

#ifndef _PATH_UTMP
#define _PATH_UTMP      "/etc/utmp"
#endif

/* HP-UX 9.0 termios doesn't define these */

#ifndef FLUSHO
#define FLUSHO  0
#define XTABS   0
#endif

#ifndef OXTABS
#define OXTABS  XTABS
#endif

#ifndef LINUX
#define _PATH_DEFPATH   "/usr/ucb:/bin:/usr/bin:"
#endif

#ifdef SunOS
char* getpass(const char* prompt);
char* crypt(const char *key, const char *setting);

void
setenv(name, value, overwrite)
  const char *name;
  const char *value;
  int overwrite;
{
  if (overwrite || (getenv(name) == 0))
  {
    char *p;

    if ((p = malloc(strlen(name) + strlen(value) + 2)) == 0)
    {
      syslog(LOG_ERR, "out of memory\n");
      exit(1);
    }
    sprintf(p, "%s=%s", name, value);
    putenv(p);
  }
}
#endif


void
slave_termios()
{
  struct termios termios;

  /*
   * Finalize the terminal settings. Some systems default to 8 bits, others
   * to 7, so we should leave that alone.
   */
  tcgetattr(0, &termios);

  termios.c_iflag |= (BRKINT | IGNPAR | ICRNL | IXON | IMAXBEL);
  termios.c_iflag &= ~IXANY;
  termios.c_lflag |= (ISIG | IEXTEN | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL |
    ECHOKE);
  termios.c_lflag &= ~(ECHOPRT | TOSTOP | FLUSHO);
  termios.c_oflag |= (OPOST | ONLCR);
  termios.c_oflag &= ~OXTABS;
  termios.c_cc[VEOF] = 4;
  (void) tcsetattr(0, TCSANOW, &termios);
}


#ifdef  HAVE_UTMP
/* --------------------------------- */
/* log out process                   */
/* --------------------------------- */

/* 0 on failure, 1 on success */

void
logout(line)
  register char *line;
{
  int fd;

  if ((fd = open(_PATH_UTMP, O_RDWR, 0)) >= 0)
  {
    struct utmp ut;

    (void) lseek(fd, utmp_slot, L_SET);
    (void) read(fd, &ut, sizeof(struct utmp));
    if (ut.ut_name[0] && !strncmp(ut.ut_line, line, sizeof(ut.ut_line)))
    {
      bzero(ut.ut_name, sizeof(ut.ut_name));
      bzero(ut.ut_host, sizeof(ut.ut_host));
      (void) time(&ut.ut_time);
      (void) lseek(fd, -(off_t)sizeof(struct utmp), L_INCR);
      (void) write(fd, &ut, sizeof(struct utmp));
    }
    (void) close(fd);
  }
}
#endif


/* ----------------------------------------------- */
/* ¨ú±o remote user name ¥H§P©w¨­¥÷                */
/* ----------------------------------------------- */

/*
 * rfc931() speaks a common subset of the RFC 931, AUTH, TAP, IDENT and RFC
 * 1413 protocols. It queries an RFC 931 etc. compatible daemon on a remote
 * host to look up the owner of a connection. The information should not be
 * used for authentication purposes. This routine intercepts alarm signals.
 *
 * Diagnostics are reported through syslog(3).
 *
 * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
 */

#include <setjmp.h>

#define STRN_CPY(d,s,l) { strncpy((d),(s),(l)); (d)[(l)-1] = 0; }
#define STRING_LENGTH    60
#define RFC931_TIMEOUT   10
#define RFC931_PORT     113     /* Semi-well-known port */
#define ANY_PORT        0       /* Any old port will do */


/* ------------------------- */
/* timeout - handle timeouts */
/* ------------------------- */


static jmp_buf timebuf;

static void
gtimeout(sig)
  int sig;
{
  longjmp(timebuf, sig);
}


void
getremotename(from, rhost, rname)
  struct sockaddr_in *from;
  char *rhost;
  char *rname;
{
  struct sockaddr_in our_sin;
  struct sockaddr_in rmt_sin;
  unsigned rmt_port, rmt_pt;
  unsigned our_port, our_pt;
  FILE *fp;
  char buffer[512], user[40], *cp;
  int s;
  struct hostent *hp;

  /* get remote host name */

  hp = NULL;
  if (setjmp(timebuf) == 0)
  {
    signal(SIGALRM, gtimeout);
    alarm(3);
    hp = gethostbyaddr((char *) &from->sin_addr, sizeof(struct in_addr),
      from->sin_family);
    alarm(0);
  }
  strncpy(rhost, hp ? hp->h_name : (char *) inet_ntoa(from->sin_addr), 39);
  rhost[39] = 0;

  /*
   * Use one unbuffered stdio stream for writing to and for reading from the
   * RFC931 etc. server. This is done because of a bug in the SunOS 4.1.x
   * stdio library. The bug may live in other stdio implementations, too.
   * When we use a single, buffered, bidirectional stdio stream ("r+" or "w+"
   * mode) we read our own output. Such behaviour would make sense with
   * resources that support random-access operations, but not with sockets.
   */

  s = sizeof our_sin;
  if (getsockname(0, (struct sockaddr*)&our_sin, &s) < 0)
    return;

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("bbsd: socket in rfc931");
    return;
  }

  if (!(fp = fdopen(s, "r+")))
  {
    close(s);
    perror("bbsd:fdopen");
    return;
  }

  /*
   * Set up a timer so we won't get stuck while waiting for the server.
   */

  if (setjmp(timebuf) == 0)
  {
    signal(SIGALRM, gtimeout);
    alarm(RFC931_TIMEOUT);

    /*
     * Bind the local and remote ends of the query socket to the same IP
     * addresses as the connection under investigation. We go through all
     * this trouble because the local or remote system might have more than
     * one network address. The RFC931 etc. client sends only port numbers;
     * the server takes the IP addresses from the query socket.
     */

    our_pt = ntohs(our_sin.sin_port);
    our_sin.sin_port = htons(ANY_PORT);

    rmt_sin = *from;
    rmt_pt = ntohs(rmt_sin.sin_port);
    rmt_sin.sin_port = htons(RFC931_PORT);

    setbuf(fp, (char *) 0);
    s = fileno(fp);

    if (bind(s, (struct sockaddr *) & our_sin, sizeof(our_sin)) >= 0 &&
      connect(s, (struct sockaddr *) & rmt_sin, sizeof(rmt_sin)) >= 0)
    {

      /*
       * Send query to server. Neglect the risk that a 13-byte write would
       * have to be fragmented by the local system and cause trouble with
       * buggy System V stdio libraries.
       */

      fprintf(fp, "%u,%u\r\n", rmt_pt, our_pt);
      fflush(fp);

      /*
       * Read response from server. Use fgets()/sscanf() so we can work
       * around System V stdio libraries that incorrectly assume EOF when a
       * read from a socket returns less than requested.
       */

      if (fgets(buffer, sizeof(buffer), fp) && !ferror(fp) && !feof(fp)
        && sscanf(buffer, "%u , %u : USERID :%*[^:]:%39s",
          &rmt_port, &our_port, user) == 3
        && rmt_pt == rmt_port && our_pt == our_port)
      {

        /*
         * Strip trailing carriage return. It is part of the protocol, not
         * part of the data.
         */

        if (cp = strchr(user, '\r'))
          *cp = 0;
        strcpy(rname, user);
      }
    }
    alarm(0);
  }
  fclose(fp);
}

bad_host(char* name)
{
   FILE* list;
   char buf[40];

  if (list = fopen(BAD_HOST, "r")) {
     while (fgets(buf, 40, list)) {
        buf[strlen(buf) - 1] = '\0';
        if (!strcmp(buf, name))
           return 1;
        if (buf[strlen(buf) - 1] == '.' && !strncmp(buf, name, strlen(buf)))
           return 1;
        if (*buf == '.' && strlen(buf) < strlen(name) &&
            !strcmp(buf, name + strlen(name) - strlen(buf)))
           return 1;
     }
     fclose(list);
  }
  return 0;
}


int
get_tty()
{
  int i, p, t, now, visit;
  int *busy;
  pid_t *pid;

  resolve_pptshm();
  busy = &ppt_shm->busy;
  i = BUSY_PERIOD;
  while (*busy && i)
  {
    sleep(1);
    i--;
  }
  *busy = 1;

  now = time(0) - SYNC_PERIOD;
  if (ppt_shm->uptime <= now)
  {
    for (i = visit = 0, pid = ppt_shm->ppt; i < NUMTTYS; i++, pid++)
    {
      if (p = *pid)
      {
        errno = 0;
        if ((kill(p, 0) < 0) && (errno == ESRCH))
        {
          *pid = 0;
        }
        else
          visit++;
      }
    }
    ppt_shm->visit = visit;
    (void) time(&ppt_shm->uptime);
  }
  else
    visit = ppt_shm->visit;

  if (visit < 500)
  {
    now &= NUMTTYS - 1;
    i = now;
    pid = &ppt_shm->ppt[i];

    do
    {
      if (!*pid)
      {
#ifdef BSD44
        line[sizeof("/dev/ptyp") - 2] =
          "pqrsPQRS"[(i / 32) % 8];
        line[sizeof("/dev/ptyp") - 1] = "0123456789abcdefghijklmnopqrstuv"[i % 32];
#else
        line[sizeof("/dev/ptyp") - 2] =
          "klmnopqrstuvwxyzKLMNOPQRSTUVWXYZ"[(i >> 4) & 31];
        line[sizeof("/dev/ptyp") - 1] = "0123456789abcdef"[i & 15];
#endif
        p = open(line, O_RDWR | O_NOCTTY);
        if (p > 0)
        {
          line[sizeof("/dev/") - 1] = 't';
          t = open(line, O_RDWR | O_NOCTTY);
          if (t > 0)
          {
            *pid = getpid();
            ppt_shm->visit = visit + 1;
            *busy = 0;
            slot = i;
            pty = p;
            return (t);
          }
          close(p);
          line[sizeof("/dev/") - 1] = 'p';
        }
      }
      if (i == NUMTTYS - 1)
      {
        i = 0;
        pid = ppt_shm->ppt;
      }
      else
      {
        i++;
        pid++;
      }
    } while (i != now);
  }
  *busy = 0;


#if 0
  sleep(5);
  sprintf(maple, "«Èº¡¤F¡A½Ðµy«á¦A¨Ó¡A¥Ø«e½u¤W¤H¼Æ [%d] ¤H", visit);
  fatal(net, maple);
#else
  fatal(net, "«Èº¡¤F¡A½Ðµy«á¦A¨Ó¡C(visit)");
#endif
}


void
start_client()
{
  char rhost[40];
  char rname[40] = "?";

  int i, t;

  openlog("bbsd", LOG_PID | LOG_ODELAY, LOG_DAEMON);

#ifdef __linux
  setpgrp();
#else
  setpgrp(0, 0);
#endif

  net = 0;

  getremotename(&xsin, rhost, rname);   /* RFC931 */

  /* ban ±¼ bad host / bad user */
  if (bad_host(rhost) && !strcmp(rname, "?"))
    exit(1);

  /* ---------------------------- */
  /* Get a pty, scan input lines. */
  /* ---------------------------- */

  t = get_tty();

#ifdef  NO_SHM

#ifdef STREAM_PTY
  if ((p = open("/dev/ptmx", O_RDWR)) < 0)
    fatal(net, "All network ports in use");
  if (grantpt(p) < 0 || unlockpt(p) < 0)
    fatal(net, "Cannot initialize pty slave");
  dup2(net, 0);
  if ((line = ptsname(p)) == 0 || (t = open(line, O_RDWR)) < 0)
    fatal(net, "Cannot find  pty slave");
  if (ioctl(t, I_PUSH, "ptem") < 0 || ioctl(t, I_PUSH, "ldterm") < 0
    || ioctl(t, I_PUSH, "ttcompat") < 0 || ioctl(p, I_PUSH, "pckt") < 0)
    fatal(net, "Cannot push streams modules onto pty");
#else                           /* STREAM_PTY */


  /* ----------------------------------------- */
  /* hashing : increase the seaching pty speed */
  /* ----------------------------------------- */

#ifdef  HASHING
  {
    static char ptyname[] = "/dev/ptyXX";
    line = ptyname;
  }
  i = getpid();
#ifdef BSD44
  for (t = i + 256; i < t; i++)
  {
    line[sizeof("/dev/ptyp") - 2] =
      "pqrsPQRS"[(i / 32) % 8];
    line[sizeof("/dev/ptyp") - 1] = "0123456789abcdefghijklmnopqrstuv"[i % 32];
#else
  for (t = i + 512; i < t; i++)
  {
    line[sizeof("/dev/ptyp") - 2] =
      "klmnopqrstuvwxyzKLMNOPQRSTUVWXYZ"[(i >> 4) & 31];
    line[sizeof("/dev/ptyp") - 1] = "0123456789abcdef"[i & 15];
#endif

    p = open(line, O_RDWR | O_NOCTTY);

    if (p > 0)
      goto gotpty;
  }
  if (p <= 0)
  {
    sleep(5);
    fatal(net, "«Èº¡¤F¡A½Ðµy«á¦A¨Ó(tty)");
  }

#else

  for (t = 'P'; t <= 'T'; t++)
  {
    struct stat stb;
    static char ptyname[] = "/dev/ptyXX";

    line = ptyname;
    line[strlen("/dev/pty")] = t;
    line[strlen("/dev/ptyp")] = '0';
    if (stat(line, &stb) < 0)
      break;
    for (i = 0; i < 16; i++)
    {
      line[sizeof("/dev/ptyp") - 1] = "0123456789abcdef"[i];
      close(open(line, O_RDWR | O_NOCTTY));
      p = open(line, O_RDWR | O_NOCTTY);
      if (p > 0)
        break;
    }
  }
  if (p <= 0)
    fatal(net, "All network ports in use");
#endif                          /* HASHING */

  /* NOTREACHED */

gotpty:

  /* dup2(net, 0); */

  line[sizeof("/dev/") - 1] = 't';
  /* close(open(line, O_RDWR)); */
  t = open(line, O_RDWR | O_NOCTTY);
  if (t < 0)
  {
    fatalperror(net, line);
  }

#ifdef ultrix
  {
    struct termios b;

    tcgetattr(t, &b);
    b.c_iflag |= ICRNL;
    b.c_oflag |= ONLCR;
    tcsetattr(t, TCSANOW, &b);
  }
#endif                          /* ultrix */

#endif                          /* STREAM_PTY */

  pty = p;

#ifdef STREAM_PTY
  pts = t;
#endif

#endif  NO_SHM

  /* get terminal type. */
  getterminaltype();

  i = fork();
  if (i < 0)
    fatalperror(net, "fork");
  else if (i)
    telnet(net, pty);

  /* Acquire a controlling terminal */

  setsid();

#if defined(TIOCSCTTY) && !defined(BROKEN_TIOCSCTTY)
  ioctl(t, TIOCSCTTY, (caddr_t) 0);
#else                           /* TIOCSCTTY */
  i = t;
  if ((t = open(line, O_RDWR)) < 0)
    _exit(1);
  close(i);
#endif                          /* TIOCSCTTY */

  close(net);
  close(pty);

  fchmod(t, 0620);
  fchown(t, bbsuid, mygid);
  dup2(t, 0);
  dup2(t, 1);
  close(t);

  /* --------------- */
  /* shortcut to BBS */
  /* --------------- */

#if 0
  for (i = getdtablesize(); i > 2; i--)
    close(i);
#endif

  (void) setpriority(PRIO_PROCESS, 0, 0);

{
   char *salt;
   struct passwd* pwd;

   if (pwd = getpwnam(bbsuser))
      salt = pwd->pw_passwd;
   else
      salt = "xx";
/*
   if (*pwd->pw_passwd) {
      p = getpass(PASSWD);
      if (strcmp(crypt(p, salt), pwd->pw_passwd)) {
         if (*bbsuser2) {
            bbsuser = bbsuser2;
            if (pwd = getpwnam(bbsuser)) {
               bbsuid = pwd->pw_uid;
               bbsgid = pwd->pw_gid;
               bbshome =pwd->pw_dir;
               bbsshell = pwd->pw_shell;
               sprintf(bbsprog, "%s/bin/bbs", bbshome);
              }
             else {
                sleep(5);
                exit(1);
             }
          }
          else {
             sleep(5);
             exit(1);
          }
      }
   }
*/
}

  slave_termios();

#ifdef  HAVE_UTMP
  {
    register int fd;
    struct utmp utmp;

    utmp_slot = ttyslot() * sizeof(struct utmp);
    if (utmp_slot > 0 && (fd = open(_PATH_UTMP, O_WRONLY, 0)) >= 0)
    {
      memset((char *) &utmp, 0, sizeof(utmp));
      (void) time(&utmp.ut_time);
      strncpy(utmp.ut_name, BBSUSER, sizeof(utmp.ut_name));
      strncpy(utmp.ut_host, rhost, sizeof(utmp.ut_host));
      strncpy(utmp.ut_line, line + sizeof("/dev/") - 1, sizeof(utmp.ut_line));

      (void) lseek(fd, utmp_slot, L_SET);
      (void) write(fd, &utmp, sizeof(struct utmp));
      (void) close(fd);
    }
  }
#endif

  /* Give up root privileges: no way back from here. */

  if (setgid(bbsgid))
    exit(1);
  initgroups(bbsuser, bbsgid);

  if (setuid(bbsuid))
    exit(1);

  /*
   * Now that we have given up root privilege do the stuff that must be done
   * as the real user: Kerberos or Secure RPC authentication, entering the
   * (possibly remote) home directory.
   */

  /* Change to home directory */

  if (chdir(bbshome))
    exit(1);

  /*
   * Set up a new environment. With SYSV, some variables are always
   * preserved; some varables are never preserved, and some variables are
   * always clobbered. With BSD, nothing is always preserved, and some
   * variables are always clobbered. We add code to make sure that LD_* and
   * IFS are never preserved.
   */

  environ[0] = 0;
  if (terminaltype)
    putenv(terminaltype);

  (void) setenv("HOME", bbshome, 1);
  (void) setenv("SHELL", bbsshell, 1);
  (void) setenv("REMOTEHOST", rhost, 1);
  (void) setenv("REMOTEUSERNAME", rname, 1);
  (void) setenv("USER", bbsuser, 1);
  (void) setenv("PATH", _PATH_DEFPATH, 0);
  {
    char RFC931[80];
    sprintf(RFC931, "%s@%s", rname, rhost);
    (void) setenv("RFC931", RFC931, 1);
  }

  execl(bbsprog, "bbs", rhost, line, rname, NULL);

  syslog(LOG_ERR, "%s: %m", bbsprog);
  exit(1);
}


/* --------------------------------- */
/* stand-alone daemon                */
/* --------------------------------- */


void
reapchild()
{
  int state, pid;

  /* signal(SIGCHLD, reapchild); */
  while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0);
}


/* ----------------------------------- */
/* check system / memory / CPU loading */
/* ----------------------------------- */

#ifdef  HAVE_CHKLOAD
int
chkload(limit)
  int limit;
{
  double cpu_load[3];
  register int i;

#if defined(LINUX)
  FILE *fp;

  fp = fopen("/proc/loadavg", "r");
  if (!fp)
    cpu_load[0] = cpu_load[1] = cpu_load[2] = 0;
  else
  {
    float av[3];

    fscanf(fp, "%g %g %g", av, av + 1, av + 2);
    fclose(fp);
    cpu_load[0] = av[0];
    cpu_load[1] = av[1];
    cpu_load[2] = av[2];
  }
#elif defined(BSD44)
  getloadavg(cpu_load, 3);
#else

#include <nlist.h>
#define VMUNIX  "/vmunix"
#define KMEM    "/dev/kmem"

  static struct nlist nlst[] = {
    {"_avenrun"},
    {0}
  };
  static long offset = -1;

  int kmem;

  if ((kmem = open(KMEM, O_RDONLY)) == -1)
    return (1);

  if (offset < 0)
  {
    (void) nlist(VMUNIX, nlst);
    if (nlst[0].n_type == 0)
      return (1);
    offset = (long) nlst[0].n_value;
  }
  if (lseek(kmem, offset, L_SET) == -1)
  {
    close(kmem);
    return (1);
  }
  if (read(kmem, (char *) avenrun, sizeof(avenrun)) == -1)
  {
    close(kmem);
    return (1);
  }
  close(kmem);
#define loaddouble(la) ((double)(la) / (1 << 8))

  for (i = 0; i < 3; i++)
    cpu_load[i] = loaddouble(avenrun[i]);
#endif

  i = cpu_load[0];
  if (i < limit)
    i = 0;
  sprintf(maple, "¨t²Î­t¸ü %.2f %.2f %.2f%s",
    cpu_load[0], cpu_load[1], cpu_load[2],
    (i ? "¡A½Ðµy«á¦A¨Ó\n" : ""));

  return i;
}
#endif


void
dokill()
{
  kill(0, SIGKILL);
}


void
start_daemon()
{
  int n, fp;
  struct group *gr;
  char buf[80];

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time_t dummy = time(NULL);
  struct tm *dummy_time = localtime(&dummy);
  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", dummy_time);

  gr = (struct group *) getgrnam("tty");
  mygid = gr ? gr->gr_gid : bbsgid;

  n = getdtablesize();
  if (fork())
    exit(0);

#ifdef SunOS
  sprintf(buf, "/var/tmp/bbsd.pid");
#else
  sprintf(buf, BBSHOME"/run/bbsd.pid");
#endif
  if ((fp = open(buf, O_WRONLY | O_CREAT | O_TRUNC, 0644)) >= 0)
  {
    sprintf(buf, "%5d\n", getpid());
    write(fp, buf, 6);
    close(fp);
  }
  else
  {
    fprintf(stderr, "bbsd: cant log PID file [%s]\n", buf);
    exit(-1);
  }

  while (n)
    (void) close(--n);

#if 0
  chdir("/");
  (void) open("/", O_RDONLY);
#endif

  (void) open("/dev/null", O_RDONLY);
  (void) dup2(0, 1);

  n = open("/dev/tty", O_RDWR);
  if (n > 0)
  {
    ioctl(n, TIOCNOTTY, (char *) 0);
    (void) close(n);
  }

  openlog("bbsd", LOG_PID, LOG_AUTH);
  syslog(LOG_NOTICE, "start\n");
}


int
bind_port(port)
  int port;
{
  int sock, on;
  struct linger onn;
  onn.l_onoff = 0;
  onn.l_linger = 0;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0)
  {
    syslog(LOG_NOTICE, "socket\n");
    exit(1);
  }

  on = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0)
    syslog(LOG_ERR, "(SO_REUSEADDR): %m");
  if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof(on)) < 0)
    syslog(LOG_WARNING, "(SO_KEEPALIVE): %m");

  on = 0;
  if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (void *) &onn, sizeof(onn)) < 0)
    syslog(LOG_ERR, "(SO_LINGER): %m");

#if 0                           /* 0825 */
  on = 4096;
  setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *) &on, sizeof(on));
  setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &on, sizeof(on));
#endif

  xsin.sin_port = htons(port);
  if (bind(sock, (struct sockaddr *)&xsin, sizeof xsin) < 0) {
    syslog(LOG_INFO, "bbsd bind_port can't bind to %d",port);
    exit(1);
  }

  if (listen(sock, QLEN) < 0) {
    syslog(LOG_INFO, "bbsd bind_port can't listen to %d",port);
    exit(1);
  }

  FD_SET(sock, (fd_set *) & rfds);
  return sock;
}


int
main(argc, argv)
  char *argv[];
{
  int msock, csock;             /* socket for Master and Child */
  int ofds, nfds;
  int th_low, th_high, overload;
  pid_t pid;
  time_t uptime;
  struct timeval tv;

  struct passwd* pwd;

  /* --------------------------------------------------- */
  /* setup standalone                                    */
  /* --------------------------------------------------- */

  start_daemon();

  signal(SIGCHLD, reapchild);

  /* --------------------------------------------------- */
  /* port binding                                        */
  /* --------------------------------------------------- */

  memset(&xsin, 0, sizeof(xsin));
  xsin.sin_family = AF_INET;
  rfds = 0;

  bbsuser = BBSUSER;
  bbsuser2 = BBSUSER;
  if (argc > 1)
  {
    msock = -1;
    for (nfds = 1; nfds < argc; nfds++)
    {
      csock = atoi(argv[nfds]);
      if (csock > 0)
         msock = bind_port(csock);
      else
         break;
    }
    if (msock < 0) {
      syslog(LOG_INFO, "bbsd started with invalid arguments (no port)");
      exit(1);
    }
    if (nfds < argc)
      bbsuser = argv[nfds++];
    if (nfds < argc)
      bbsuser2 = argv[nfds++];
  }
  else
  {
    static int ports[] = {23, 3000, 3001, 3002, 3003, 3004, 3005};

    for (nfds = 0; nfds < sizeof(ports) / sizeof(int); nfds++)
    {
      msock = bind_port(ports[nfds]);
    }
  }
  if (!(pwd = getpwnam(bbsuser))) {
     fprintf(stderr, "No such user: %s\n", bbsuser);
     exit(1);
  }
  bbsuid = pwd->pw_uid;
  bbsgid = pwd->pw_gid;
  bbshome =pwd->pw_dir;
  bbsshell = pwd->pw_shell;
  sprintf(bbsprog, "%s/bin/bbs", bbshome);


  ofds = rfds;
  nfds = msock + 1;

  /* --------------------------------------------------- */
  /* main loop                                           */
  /* --------------------------------------------------- */

  th_low =  TH_LOW;
  th_high = TH_HIGH;

  overload = uptime = 0;

  for (;;)
  {
forever:

#ifdef  HAVE_CHKLOAD
    pid = time(0);
    if (pid > uptime)
    {
      overload = chkload(overload ? th_low : th_high);
      uptime = pid + overload + 60;     /* µu®É¶¡¤º¤£¦AÀË¬d system load */
    }
#endif

    rfds = ofds;
    tv.tv_sec = 60 * 30;
    tv.tv_usec = 0;
    msock = select(nfds, (fd_set *) & rfds, NULL, NULL, &tv);

    if (msock < 0)
    {
      if (errno != EINTR)
        sleep(5);
      continue;
    }
    else if (msock == 0)        /* No network traffic */
    {
      continue;
    }

#if 0
    if (rfds & ofds == 0)
      continue;
#endif

    msock = 0;
    csock = 1;
    for (;;)
    {
      if (csock & rfds)
        break;
      if (++msock >= nfds)
        goto forever;
      csock <<= 1;
    }

    slot = sizeof xsin;
    do
    {
      csock = accept(msock, (struct sockaddr *)&xsin, &slot);
    } while (csock < 0 && errno == EINTR);

    if (csock < 0)
      continue;

#ifdef  HAVE_CHKLOAD
    if (overload)
    {
      (void) write(csock, maple, strlen(maple));
      close(csock);
      continue;
    }
#endif

    pid = fork();

    if (!pid)
    {
      /* ¥ýÅã¥Ü°T®§¡A§K±o user ¥¢¥h­@¤ß */

      (void) write(csock, MSG_WORKING, sizeof(MSG_WORKING) - 1);

      while (--nfds >= 0)
        close(nfds);

      dup2(csock, 0);
      close(msock);
      start_client();
    }
    else
    {

#if 0
      if (pid < 0)
      {
        perror("fork");
      }
#endif

      close(csock);
    }
  }
}


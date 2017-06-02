/*-------------------------------------------------------*/
/* util/blookd.c        ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : Simple LOOK server for BBS user              */
/* create : 00/11/07                                     */
/*-------------------------------------------------------*/
/* notice : single process concurrent server             */
/*-------------------------------------------------------*/

#define HAVE_MMAP

#include "bbs.h"
#include "record.c"
#include "cache.c"

#define DEBUG

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>

#define LOOK_PORT       3333
#define LOOK_TIMEOUT    (60 * 20)

#define LOOK_LOGFILE    BBSHOME "/log/blookd.log"
#define LOOK_PIDFILE    BBSHOME "/log/blookd.pid"

#define LOOK_FREEDOM    2

#define SOCK_QLEN       5
#define SOCK_TIMEOUT    (30 * 60)


#define SND_TIMEOUT     120
#define RCV_TIMEOUT     120


#define SNDBUFSIZ       4096
#define RCVBUFSIZ       1024
#define MAXBUFSIZ       (SNDBUFSIZ > RCVBUFSIZ ? SNDBUFSIZ : RCVBUFSIZ)


/* ----------------------------------------------------- */
/* MapleBBS lookd message strings                        */
/* ----------------------------------------------------- */


#define LOOK_HELLO_MSG  "+OK WD-BBS-LOOK server ready\r\n"
#define LOOK_HELLO_LEN  (sizeof(LOOK_HELLO_MSG) - 1)


#define LOOK_BYE_MSG    "+OK WD-BBS-LOOK server terminated\r\n"
#define LOOK_BYE_LEN    (sizeof(LOOK_BYE_MSG) - 1)


#define LOOK_ERRCMD_MSG "-ERR invalid command"
#define LOOK_ERRARG_MSG "-ERR invalid argument"

/* ----------------------------------------------------- */
/* client connection structure                           */
/* ----------------------------------------------------- */


struct Client
{
  int sock;
  int serial;
  int totalnum;
  int totalbyte;
  time_t birth;
  time_t uptime;
  struct sockaddr_in cin;
  char state;
  char mode;

  FILE *fin;                    /* input file */
  char pool[MAXBUFSIZ];         /* output pool */
  int locus;

  struct Client *next;
};
typedef struct Client Client;

Client *cnlist;

struct UTMPFILE *utmpshm;

char *fn_passwd = BBSHOME "/.PASSWDS";
time_t start_time;
userec cuser;
char buf[512];


/* ----------------------------------------------------- */
/* connection state                                      */
/* ----------------------------------------------------- */


#define CS_FREE         0x00
#define CS_CLOSING      0x01
#define CS_READING      0x02
#define CS_WRITING      0x03

#define CS_BLOCKED      0x40


#define CM_LOGIN        1
#define CM_DIRTY        2       /* 有刪除信件 */


/* ----------------------------------------------------- */
/* operation log and debug information                   */
/* ----------------------------------------------------- */


int flog;                       /* log file descriptor */

int cn_serial;                  /* client's serial number */

/*-------------------------------------------------------*/
/* .PASSWDS cache                                        */
/*-------------------------------------------------------*/

void
logit(key, msg)
  char *key;
  char *msg;
{
  time_t now;
  struct tm *p;
  char buf[256];

  time(&now);
  p = localtime(&now);
  sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d %-8s%s\n",
    p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, key, msg);
  write(flog, buf, strlen(buf));
}

void
log_init()
{
  flog = open(LOOK_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logit("START", "bbs-look daemon");
}

void
log_close()
{
  close(flog);
}


/* ----------------------------------------------------- */
/* parse string token                                    */
/* ----------------------------------------------------- */


#define LOWER   1


static char *trail_token;


char *
parse_token(str, lower)
  char *str;
  int lower;
{
  char *token;
  int ch;

  if (str == NULL)
  {
    str = trail_token;
    if (str == NULL)
      return NULL;
  }

  token = NULL;
  while (ch = *str)
  {
    if (ch == ' ' /* || ch == '\t' || ch == '\r' || ch == '\n' */ )
    {
      if (token)
      {
        *str++ = '\0';
        break;
      }
    }
    else
    {
      if (token == NULL)
        token = str;

      if (lower && ch >= 'A' && ch <= 'Z')
        *str = ch | 0x20;
    }
    str++;
  }

  trail_token = str;
  return token;
}


/* ----------------------------------------------------- */
/* exclusively create file [*.new]                       */
/* ----------------------------------------------------- */


int
fexcl_creat(fold, fnew)
  char *fold;
  char *fnew;
{
  register int fd, i;
  extern int errno;

  sprintf(fnew, "%s.new", fold);
  i = 0;

  for (;;)
  {
    fd = open(fnew, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if ((fd >= 0) || (errno != EEXIST))
      break;

    if (i == 0)
    {
      struct stat st;

      if (stat(fnew, &st) < 0)
        break;
      if (st.st_mtime < time(NULL) - 20 * 60);  /* 假設 20 分鐘內應該處理完 */
      unlink(fnew);
    }
    else
    {
      if (i > 12)               /* 等待 60 秒鐘 */
        break;
      sleep(5);
    }
    i++;
  }
  return fd;
}


/* ----------------------------------------------------- */
/* 取得 remote user name 以判定身份                      */
/* ----------------------------------------------------- */


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
timeout(sig)
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
  char buffer[512], user[80], *cp;
  int s;
  struct hostent *hp;

  /* get remote host name */

  hp = NULL;
  if (setjmp(timebuf) == 0)
  {
    signal(SIGALRM, timeout);
    alarm(3);
    hp = gethostbyaddr((char *) &from->sin_addr, sizeof(struct in_addr),
      from->sin_family);
    alarm(0);
  }
  strcpy(rhost, hp ? hp->h_name : (char *) inet_ntoa(from->sin_addr));

  /*
   * Use one unbuffered stdio stream for writing to and for reading from the
   * RFC931 etc. server. This is done because of a bug in the SunOS 4.1.x
   * stdio library. The bug may live in other stdio implementations, too.
   * When we use a single, buffered, bidirectional stdio stream ("r+" or "w+"
   * mode) we read our own output. Such behaviour would make sense with
   * resources that support random-access operations, but not with sockets.
   */

  *rname = '\0';

  s = sizeof our_sin;
  if (getsockname(0, (struct sockaddr*)&our_sin, &s) < 0)
    return;

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    logit("ERROR", "socket in rfc931");
    return;
  }

  if (!(fp = fdopen(s, "r+")))
  {
    close(s);
    return;
  }

  /*
   * Set up a timer so we won't get stuck while waiting for the server.
   */

  if (setjmp(timebuf) == 0)
  {
    signal(SIGALRM, timeout);
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
        && sscanf(buffer, "%u , %u : USERID :%*[^:]:%79s",
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


/* ----------------------------------------------------- */
/* supporting commands                                   */
/* ----------------------------------------------------- */


void cmd_xxxx();


#define do_errcmd(x)    cmd_xxxx(x)


void
do_argument(cn)
  Client *cn;
{
  client_flush(cn, LOOK_ERRARG_MSG);
}


/* return value : -1(all), 0, n */


int
do_number(cn, n)
  Client *cn;
  int n;                        /* 0 ==> exact 1 arg  -1 ==> 1 or 0 arg */
{
  char *cmd;

  if (cn->mode < CM_LOGIN)
  {
    do_errcmd(cn);
    return 0;
  }

  cmd = parse_token(NULL, 0);

  if (!cmd || !*cmd)
  {
    if (n == 0)
      do_argument(cn);
    return n;
  }

  n = atoi(cmd);
  if (n <= 0 || n > cn->totalnum)
  {
    client_flush(cn, "-ERR message number out of range");
    return 0;
  }

  return n;
}

/* ----------------------------------------------------- */
/* stuff commands                                        */
/* ----------------------------------------------------- */

int
bad_user_id()
{
  register char ch;
  int j;

  if (strlen(cuser.userid) < 2 || !isalpha(cuser.userid[0]))
    return 1;

  if (cuser.numlogins==0)
    return 1;

  for(j=1;ch = cuser.userid[j];j++)
  {
    if (!isalnum(ch))
      return 1;
  }

  return 0;
}

void
resolve_utmp()
{
  if (utmpshm == NULL)
  {
    utmpshm = shm_new(UTMPSHM_KEY, sizeof(*utmpshm));
    if (utmpshm->uptime == 0)
      utmpshm->uptime = utmpshm->number = 1;
  }
}

/* ----------------------------------------------------- */
/* main commands                                         */
/* ----------------------------------------------------- */


void
cmd_xxxx(cn)
  Client *cn;
{
  client_flush(cn, LOOK_ERRCMD_MSG);
}


void
cmd_noop(cn)
  Client *cn;
{
  client_flush(cn, "+OK");
}

void
cmd_quit(cn)
  Client *cn;
{
  int n;

  n = cn->sock;
  write(n, LOOK_BYE_MSG, LOOK_BYE_LEN);
  close(n);

  cn->sock = -1;
  cn->state = CS_FREE;
  logit("BYE",NULL);
}

void
cmd_online_user(cn)
  Client *cn;
{
  register user_info *uentp;
  register int i,user_num;

  resolve_utmp();
  for (i = user_num = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (uentp->userid[0])
    {
      user_num++;
    }
  }
  sprintf(buf,"%d 人在線上",user_num);
  client_flush(cn, buf);
}

void
cmd_total_user(cn)
  Client *cn;
{
  FILE *fp1=fopen(fn_passwd,"r");
  usint total_user=0;

  while((fread(&cuser, sizeof(cuser), 1, fp1)) > 0)
  {
    if(!bad_user_id())
      total_user++;
  }
  fclose(fp1);
  sprintf(buf,"%d",total_user);
  client_flush(cn, buf);
}

struct
{
  char *cmd;
  void (*fun) ();
}      cmdlist[] =
{
  "online_user", cmd_online_user,
  "total_user", cmd_total_user,
  "noop", cmd_noop,
  "quit", cmd_quit,
  NULL, cmd_xxxx
};


/* ----------------------------------------------------- */
/* put a line into client's buffer, padding with "\r\n"  */
/* ----------------------------------------------------- */


/* return value = bytes sent , 0 ==> error */


int
client_puts(cn, msg)
  Client *cn;
  char *msg;
{
  int pos, len;
  char *str;

  len = strlen(msg);
  pos = cn->locus;
  str = cn->pool;

  while (pos + len + 2 > SNDBUFSIZ)
  {
    int n = write(cn->sock, str, pos);
    if (n <= 0)
    {
      cn->state = CS_WRITING | CS_BLOCKED;
      return 0;
    }
    (void) time(&cn->uptime);
    pos -= n;
    if (pos == 0)
      break;
    memcpy(str, str + n, pos);
  }
  str += pos;
  memcpy(str, msg, len);
  memcpy(str + len, "\r\n", 2);
  return cn->locus = pos + len + 2;
}


int
client_flush(cn, msg)
  Client *cn;
  char *msg;
{
  int len, n, sock;

  if (msg != NULL)
  {
    len = client_puts(cn, msg);
    if (len == 0)
      return 0;
  }

  len = cn->locus;

  if (len == 0)
  {
    cn->state = CS_READING;
    return 0;
  }

  sock = cn->sock;
  msg = cn->pool;

  for (;;)
  {
    n = write(sock, msg, len);
    if (n <= 0)
    {
      cn->state = CS_WRITING | CS_BLOCKED;
      return 0;
    }
    len -= n;
    (void) time(&cn->uptime);
    if (len == 0)
    {
      cn->locus = 0;
      cn->state = CS_READING;
      return n;
    }
    msg += n;
  }
}


int
client_write(cn)
  Client *cn;
{
  int len, n, sock;
  char *str;
  FILE *fin;

  /* send out a text file in MIME mode */

  if (fin = cn->fin)
  {
    char data[512], *ptr;

    ptr = data;
    while (fgets(ptr, sizeof(data), fin))
    {
      if (str = strchr(ptr, '\n'))
      {
        *str = '\0';
      }
      len = client_puts(cn,
        (*ptr == '.' && ptr[1] == '\0') ? ".." : ptr);
      if (len == 0)
      {
        return 0;
      }
    }
    fclose(fin);
    cn->fin = NULL;
    str = ".";                  /* ==> [end of file] */
  }
  else
    str = NULL;

  return client_flush(cn, str);
}


/* ----------------------------------------------------- */
/* client's service dispatcher                           */
/* ----------------------------------------------------- */


void
client_serve(cn)
  Client *cn;
{
  char *cmd, *str;
  int code;

  cn->locus = 0;                /* reset buffer pool position */

  cmd = parse_token(cn->pool, LOWER);

  if (!cmd || !*cmd)
  {
    cn->state = CS_READING;     /* retry reading command */
    return;
  }

  for (code = 0; str = cmdlist[code].cmd; code++)
  {
    if (!strcmp(cmd, str))
      break;
  }

#if 0
  logit(cmd, cn->userid);
#endif

  (*cmdlist[code].fun) (cn);

#if 0
  logit(str, "END");
#endif
}


/* ----------------------------------------------------- */
/* receive command from client                           */
/* ----------------------------------------------------- */


void
client_read(cn)
  Client *cn;
{
  int pos, bytes;
  char *ip;

  pos = cn->locus;
  ip = &cn->pool[pos];
  bytes = read(cn->sock, ip, RCVBUFSIZ - pos);
  if (bytes <= 0)
  {
    cn->state = CS_CLOSING;
  }

#if 0
  else if (bytes == 0)
  {
    cn->state = CS_READING | CS_BLOCKED;
  }
#endif

  else
  {
    ip[bytes] = '\0';

    while (pos = *ip)
    {
      switch (pos)
      {
      case '\r':
      case '\n':
        *ip = '\0';
        return client_serve(cn);

      case '\t':
        *ip = ' ';
      }
      ip++;
    }

    cn->locus += bytes;
    cn->state = CS_READING;     /* keep on reading command */
  }
}


/* ----------------------------------------------------- */
/* release idle/timeout connections                      */
/* ----------------------------------------------------- */


void
client_terminate(cn, msg)
  Client *cn;
  char *msg;
{
  logit(msg, NULL);
  cmd_quit(cn);
}


void
client_free()
{
  Client *cn, *prev, *next;
  int state, freedom;
  time_t timeout;

  freedom = 0;

  for (cn = cnlist; cn; cn = next)
  {
    if (state = cn->state)
    {
      switch (state)
      {
      case CS_CLOSING:
        timeout = 0;
        break;

      case (CS_READING | CS_BLOCKED):
        timeout = RCV_TIMEOUT;
        break;

      case (CS_WRITING | CS_BLOCKED):
        timeout = SND_TIMEOUT;
        break;

      default:
        timeout = LOOK_TIMEOUT;
      }
      if (cn->uptime <= time(NULL) - timeout)
      {
        client_terminate(cn, "timeout");
      }
    }

    /* release free client */

    next = cn->next;

    if (cn->state == CS_FREE)
    {
      if (++freedom >= LOOK_FREEDOM)
      {
        prev->next = next;
        (void) free(cn);
        continue;
      }
    }

    prev = cn;
  }
}


/* ----------------------------------------------------- */
/* accept a new connection                               */
/* ----------------------------------------------------- */

int i_client;

void
client_new(n)
  int n;
{
  register Client *cn;
  register int csock;
  int value;
  struct sockaddr_in sin;

  value = sizeof sin;
  for (;;)
  {
    csock = accept(n, (struct sockaddr *)&sin, &value);
    if (csock >= 0)
      break;
    csock = errno;
    if (csock != EINTR)
    {
#ifndef BSD44
      extern char *sys_errlist[];
#endif

      logit("accept", sys_errlist[csock]);
      return;
    }
  }

  n = write(csock, LOOK_HELLO_MSG, LOOK_HELLO_LEN);
  if (n != LOOK_HELLO_LEN)
  {
    close(csock);
    return;
  }

  for (cn = cnlist; cn; cn = cn->next)
  {
    if (cn->state == CS_CLOSING)
    {
      client_terminate(cn, "close");
    }

    if (cn->state == CS_FREE)
    {
      break;
    }
  }

  if (cn == NULL)
  {
    cn = (Client *) malloc(sizeof(Client));

#if 111
    (void) memset(cn, 0, sizeof(Client));
#endif

    cn->next = cnlist;
    cnlist = cn;
    i_client++;
  }

  cn->sock = csock;
  cn->cin = sin;
  cn->serial = cn_serial++;
  cn->state = CS_READING;
  cn->birth = time(&cn->uptime);

#if 111
  cn->locus = 0;
  cn->mode = 0;
  cn->fin = NULL;
#endif
}


/* ---------------------------------------------------- */
/* server core routines                                 */
/* ---------------------------------------------------- */


int
start_daemon()
{
  int fd, value;
  char buf[80];
  struct sockaddr_in fsin;

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time_t dummy = time(NULL);
  struct tm *dummy_time = localtime(&dummy);
  struct tm *other_dummy_time = gmtime(&dummy);
  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", dummy_time);

  fd = getdtablesize();

  if (fork())
    exit(0);

  while (fd)
    (void) close(--fd);

  fd = open(LOOK_PIDFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd >= 0)
  {
    sprintf(buf, "%5d\n", getpid());
    write(fd, buf, 6);
    close(fd);
  }

#if 0
  (void) open("/dev/null", O_RDONLY);
  (void) dup2(0, 1);
  (void) dup2(0, 2);
#endif

  if ((fd = open("/dev/tty", O_RDWR)) > 0)
  {
    ioctl(fd, TIOCNOTTY, 0);
    close(fd);
  }

  for (fd = 1; fd < NSIG; fd++)
    (void) signal(fd, SIG_IGN);

  fd = socket(AF_INET, SOCK_STREAM, 0);
  /* fcntl(fd, F_SETFL, O_NDELAY); */

  value = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));
  setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &value, sizeof(value));

  value = SNDBUFSIZ;
  setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &value, sizeof(value));

#if 0
  value = RCVBUFSIZ;
  setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &value, sizeof(value));
#endif

  memset((char *) &fsin, 0, sizeof(fsin));
  fsin.sin_family = AF_INET;
  fsin.sin_addr.s_addr = htonl(INADDR_ANY);
  fsin.sin_port = htons(LOOK_PORT);

  if (bind(fd, (struct sockaddr *) & fsin, sizeof(fsin)) < 0) {
    logit("EXIT", "bind error");
    syslog(LOG_NOTICE, "bind error\n");
    exit(1);
  }

  openlog("blookd", LOG_PID, LOG_AUTH);
  syslog(LOG_NOTICE, "start\n");
  return fd;
}


void
abort_server()
{
  if (time(0) - start_time < 60) {
     syslog(LOG_NOTICE, "Got SIGHUP at first 60 secs, ignore");
     return;
  }
  syslog(LOG_NOTICE, "abort");
  log_close();
  exit(1);
}


int
main()
{
  register int msock, csock, nfds;
  register Client *cn;
  fd_set rset, wset;
  struct timeval tv;
  int iteration, i_free;

  start_time = time(0);
  msock = start_daemon();

  (void) setgid(BBSGID);
  (void) setuid(BBSUID);

  log_init();

  (void) signal(SIGHUP, abort_server);

  listen(msock, SOCK_QLEN);

  /* initialize resource */

  cn_serial = 1;

  iteration = i_free = 0;

  for (;;)
  {
    int gotactive;

    /* Set up the fdsets. */

    FD_ZERO(&rset);
    FD_ZERO(&wset);

    FD_SET(msock, &rset);
    nfds = msock;

    gotactive = 0;

    for (cn = cnlist; cn; cn = cn->next)
    {
      csock = cn->sock;

      switch (cn->state)
      {
      case CS_WRITING:
#if 0
      case (CS_WRITING | CS_BLOCKED):
        logit("WRITE", "xxx");
#endif
        FD_SET(csock, &wset);
        break;

      case CS_READING:
      case (CS_READING | CS_BLOCKED):
        FD_SET(csock, &rset);
        break;

#if 0
      case CS_FREE:
      case CS_CLOSING:
#endif

      default:
        continue;
      }
      gotactive = 1;
      if (nfds < csock)
        nfds = csock;
    }

    tv.tv_sec = SOCK_TIMEOUT;
    tv.tv_usec = 0;

    nfds = select(nfds + 1, &rset, &wset, NULL, gotactive ? &tv : NULL);

#ifdef  DEBUG
    ++iteration;
    if ((iteration & 63) == 0)
    {
      char buf[80];

      sprintf(buf, "I:%d, F:%d, C:%d, S:%d, N:%d",
        iteration, i_free, i_client, cn_serial, nfds);
      logit("WATCH", buf);

      if (nfds == 1)
      {
        for (csock = 0; csock < 64 ; csock++)
        {
          if (FD_ISSET(csock, &rset))
          {
            sprintf(buf, "%d", csock);
            logit("RSET ", buf);
          }
        }

        for (csock = 0; csock < 64 ; csock++)
        {
          if (FD_ISSET(csock, &wset))
          {
            sprintf(buf, "%d", csock);
            logit("WSET ", buf);
          }
        }
      }
    }
#endif  DEBUG

    if (nfds < 0)
    {
#ifndef BSD44
      extern char *sys_errlist[];
#endif

      csock = errno;
      if (csock != EINTR)
      {
        logit("select", sys_errlist[csock]);
      }
      continue;
    }
    else if (nfds == 0)
    {
      /* resource and garbage collection */

      client_free();
      i_free++;
      continue;
    }

    /* serve new connection first */

    if (FD_ISSET(msock, &rset))
    {
      client_new(msock);
      if (--nfds <= 0)
        continue;
    }

    /* serve active clients */

      for (cn = cnlist; cn; cn = cn->next)
      {
        if (cn->state > CS_CLOSING)
        {
          csock = cn->sock;

          if (FD_ISSET(csock, &rset))
          {
            client_read(cn);
            nfds--;
          }
          else if (FD_ISSET(csock, &wset))
          {
            client_write(cn);
            nfds--;
          }
          if (nfds <= 0)
            break;
        }
    }

    /* tail of main loop */
  }
}


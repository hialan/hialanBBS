/*-------------------------------------------------------*/
/* util/bpop3d.c        ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : Simple POP3 server for BBS user              */
/* create : 96/05/10                                     */
/* update : 96/11/15                                     */
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


#define POP3_PORT       1110
#define POP3_HOME       (BBSHOME "/home")
#define POP3_TIMEOUT    (60 * 20)

#define POP3_LOGFILE    BBSHOME "/log/bpop3d.log"
#define POP3_PIDFILE    BBSHOME "/log/bpop3d.pid"

#define POP3_FQDN       ".bbs@" MYHOSTNAME

#define POP3_FREEDOM    2

#define SOCK_QLEN       5
#define SOCK_TIMEOUT    (30 * 60)


#define SND_TIMEOUT     120
#define RCV_TIMEOUT     120


#define SNDBUFSIZ       4096
#define RCVBUFSIZ       1024
#define MAXBUFSIZ       (SNDBUFSIZ > RCVBUFSIZ ? SNDBUFSIZ : RCVBUFSIZ)


/* ----------------------------------------------------- */
/* MapleBBS pop3d message strings                        */
/* ----------------------------------------------------- */


#define POP3_HELLO_MSG  "+OK WDBBS-POP3 server ready\r\n"
#define POP3_HELLO_LEN  (sizeof(POP3_HELLO_MSG) - 1)


#define POP3_BYE_MSG    "+OK WDBBS POP3 server terminated\r\n"
#define POP3_BYE_LEN    (sizeof(POP3_BYE_MSG) - 1)


#define POP3_ERRCMD_MSG "-ERR invalid command"
#define POP3_ERRARG_MSG "-ERR invalid argument"
#define POP3_DELETE_MSG "-ERR message has been deleted"


/* ----------------------------------------------------- */
/* pop3header : extended from fileheader                 */
/* ----------------------------------------------------- */


typedef struct
{
  char filename[FNLEN];     /* M.9876543210.A */
  char *author;
  int mailsize;
  time_t chrono;                /* timestamp */
  char dummy;
  char savemode;                /* file save mode */
  char owner[IDLEN + 2];        /* uid[.] */
  char date[6];                 /* [02/02] or space(5) */
  char title[TTLEN + 1];
  uschar filemode;              /* must be last field @ boards.c */
}      pop3header;


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
  pop3header *cache;
  char state;
  char mode;
  char userid[IDLEN + 1];
  char passwd[PASSLEN];
  char home[IDLEN + 3];

  FILE *fin;                    /* input file */
  char pool[MAXBUFSIZ];         /* output pool */
  int locus;

  struct Client *next;
};
typedef struct Client Client;


Client *cnlist;

char *fn_passwd = BBSHOME "/.PASSWDS";
FILE* fp_passwd;
int usernumber;
time_t start_time;


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
  flog = open(POP3_LOGFILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
  logit("START", "pop3 daemon");
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
/* server side stuff                                     */
/* ----------------------------------------------------- */


int
checkpasswd(passwd, test)
  char *passwd, *test;
{
  char *crypt();
  char *pw;
  static char pwbuf[PASSLEN];

  strncpy(pwbuf, test, PASSLEN);
  pw = crypt(pwbuf, passwd);
  return (!strcmp(pw, passwd));
}


void
mailbox_setup(cn)
  Client *cn;
{
  int fd, fsize, totalbyte, pad;
  struct stat st;
  char fpath[80], *fname;
  pop3header *phdr;
  fileheader *fhdr, *fhdr0;

  cn->totalnum = cn->totalbyte = 0;
  sprintf(fpath, "%s.DIR", cn->home);
  fd = open(fpath, O_RDONLY, 0);

  if (fd < 0)
    return;

  fstat(fd, &st);
  fsize = st.st_size;
  if (fsize < sizeof(fileheader))
  {
    close(fd);
    return;
  }

  cn->totalnum = fsize /= sizeof(fileheader);
  fhdr0 = fhdr = malloc(fsize * sizeof(*fhdr));
  cn->cache = phdr = (pop3header *) malloc(fsize * sizeof(*phdr));
  read(fd, fhdr, fsize * sizeof(*fhdr));
  close(fd);

  totalbyte = 0;
  fname = strchr(fpath, '.');

  pad = 10 + strlen(cn->userid) + strlen(POP3_FQDN) + 9 + 44;
  for (;;)
  {
    char *str, *author;
    int author_len, bytes;
/*definite Patch*/
    char strtmp[IDLEN+1];

    strcpy(phdr->filename, fhdr->filename);
    phdr->savemode = fhdr->savemode;
    strcpy(phdr->owner, fhdr->owner);
    strcpy(phdr->date, fhdr->date);
    strcpy(phdr->title, fhdr->title);
    phdr->filemode = fhdr->filemode;

// wildcat : 原本加 3 ??
    phdr->chrono = atoi(fhdr->filename + 2);

    strcpy(fname, phdr->filename);
    bytes = 0;
    if (!stat(fpath, &st))
      bytes = st.st_size;

/*definite Patch*/
/*    if (!strcmp(phdr->owner,"[備.忘.錄]")){
       strcpy(strtmp,cn->userid);
       author = strtmp;
    }
    else*/
      author = phdr->owner;

    if ((str = strchr(author, '.')) && bytes)
    {
      char data[128];

      fd = open(fpath, O_RDONLY, 0);
      read(fd, data, sizeof(data));
      close(fd);

      /* 作者: xfiles.bbs@alab01.ee.nctu.edu.tw (xfiles) */

      str = data + 6;
      author = strchr(str, ' ');
      *author = '\0';
      author_len = author - str;
      phdr->author = author = (char *) malloc(author_len);
      memcpy(author, str, author_len);
    }
    else
    {
      phdr->author = NULL;
      author_len = strlen(author) + strlen(POP3_FQDN);
    }

    phdr->savemode = ' ';
    bytes += strlen(phdr->title) + author_len + 10 + 40 + 50;
    phdr->mailsize = bytes;
    totalbyte += bytes;

    if (--fsize == 0)
      break;

    phdr++;
    fhdr++;
  }
  free(fhdr0);
  cn->totalbyte = totalbyte;
}


void
mailbox_close(cn)
  Client *cn;
{
  int n, max, saved;
  int fdr, fdw;
  char fpath[80], fold[80], fnew[80], *home;
  fileheader mhdr;
  pop3header *phdr;

  home = cn->home;
  sprintf(fold, "%s.DIR", home);
  if ((fdr = open(fold, O_RDONLY, 0)) < 0)
    return;

  fdw = fexcl_creat(fold, fnew);
  if (fdw < 0)
  {
    close(fdr);
    return;
  }

  n = saved = 0;
  max = cn->totalnum;
  phdr = cn->cache;

  while (read(fdr, &mhdr, sizeof(mhdr)))
  {
    if (n >= max || phdr->savemode == ' ')
    {
      write(fdw, &mhdr, sizeof(mhdr));
      saved = 1;
    }
    else
    {
      sprintf(fpath, "%s%s", home, mhdr.filename);
      unlink(fpath);
    }
    n++;
    phdr++;
  }
  close(fdr);
  close(fdw);

  if (saved)
    f_mv(fnew, fold);
  else
  {
    unlink(fnew);
    unlink(fold);
  }
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
  client_flush(cn, POP3_ERRARG_MSG);
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
/* main commands                                         */
/* ----------------------------------------------------- */


void
cmd_xxxx(cn)
  Client *cn;
{
  client_flush(cn, POP3_ERRCMD_MSG);
}


void
cmd_noop(cn)
  Client *cn;
{
  client_flush(cn, "+OK");
}

typedef struct userec ACCT;


void
cmd_user(cn)
  Client *cn;
{
  int userecno;
  ACCT acct;
  char *userid, *ptr, buf[128], msg[128], data[256];

  if (cn->mode >= CM_LOGIN)
  {
    cmd_xxxx(cn);
    return;
  }

  userid = parse_token(NULL, LOWER);

  if (!userid || !*userid)
  {
    do_argument(cn);
    return;
  }

  buf[0] = msg[0] = '\0';

  getremotename(&cn->cin, buf, msg);
  ptr = data;
  sprintf(ptr, "%s [%s@%s]", userid, msg, buf);
  logit("CONCT", ptr);

  sprintf(msg, "-ERR %s has no mail here", userid);

  /* userid is [folk.bbs] or [folk] */

  if (ptr = strchr(userid, '.'))
  {
    if (strcmp(ptr, ".bbs"))
    {
      client_flush(cn, msg);
      return;
    }
    *ptr = '\0';
  }

  if (userecno = searchuser(userid))
  {
    fseek(fp_passwd, --userecno * sizeof(acct), 0);
    fread(&acct, sizeof(acct), 1, fp_passwd);
    strcpy(cn->userid, acct.userid);
    memcpy(cn->passwd, acct.passwd, PASSLEN);
    sprintf(msg, "+OK Password required for %s%s", acct.userid, POP3_FQDN);
  }
  sprintf(cn->home, "%s/", userid);

  client_flush(cn, msg);
}


void
cmd_password(cn)
  Client *cn;
{
  char *cmd, *passwd;

  if (cn->mode >= CM_LOGIN)
  {
    cmd_xxxx(cn);
    return;
  }

  passwd = cn->passwd;
  if (!*passwd)
  {
    client_flush(cn, "-ERR need USER command");
    return;
  }

  cmd = trail_token;            /* to support password with [space] */
  /* cmd = parse_token(NULL, 0); */

  if (!cmd || !*cmd)
  {
    do_argument(cn);
    return;
  }

  if (!checkpasswd(passwd, cmd))
  {
    logit("ERROR", "password");
    *passwd = '\0';
    client_flush(cn, "-ERR password incorrect");
  }
  else
  {
    char buf[80];

    mailbox_setup(cn);

    sprintf(buf, "+OK %s has %d message(s) (%d octets)",
      cn->userid, cn->totalnum, cn->totalbyte);
    client_flush(cn, buf);
    logit("ENTER", buf);
    cn->mode = CM_LOGIN;
  }
}


void
cmd_stat(cn)
  Client *cn;
{
  if (cn->mode < CM_LOGIN)
  {
    cmd_xxxx(cn);
  }
  else
  {
    char buf[80];

    sprintf(buf, "+OK %d %d", cn->totalnum, cn->totalbyte);
    client_flush(cn, buf);
  }
}


void
cmd_last(cn)
  Client *cn;
{
  if (cn->mode < CM_LOGIN)
  {
    cmd_xxxx(cn);
  }
  else
  {
    char buf[256];

    sprintf(buf, "+OK %d", cn->totalnum);
    client_flush(cn, buf);
  }
}


void
cmd_reset(cn)
  Client *cn;
{
  int n, max;
  char buf[256];
  pop3header *phdr;

  if (cn->mode < CM_LOGIN)
  {
    cmd_xxxx(cn);
    return;
  }

  max = cn->totalnum;
  for (n = 0, phdr = cn->cache; n < max; phdr++, n++)
  {
    phdr->savemode = ' ';
  }

  cn->mode = CM_LOGIN;
  sprintf(buf, "+OK mail reset %d messages %d octets", max, cn->totalbyte);
  client_flush(cn, buf);
}


void
cmd_retrive(cn)
  Client *cn;
{
  int n;

  if (n = do_number(cn, 0))
  {
    pop3header *phdr;

    phdr = &cn->cache[n - 1];
    if (phdr->savemode == 'X')
      client_flush(cn, POP3_DELETE_MSG);
    else
    {
      char buf[256], data[128], *str;
      struct tm *mytm;

      sprintf(buf, "+OK %d octets", phdr->mailsize);
      client_puts(cn, buf);

      str = phdr->author;
      if (str == NULL)
      {
        str = data;
        sprintf(str, "%s%s", phdr->owner, POP3_FQDN);
      }
      sprintf(buf, "From: %s", str);
      client_puts(cn, buf);

      sprintf(buf, "To: %s%s", cn->userid, POP3_FQDN);
      client_puts(cn, buf);

      sprintf(buf, "Subject: %s", phdr->title);
      client_puts(cn, buf);

      mytm = localtime(&phdr->chrono);
      strftime(buf, 46, "Date: %a, %e %h %Y, %T +800(%Z)\r\n", mytm);
      client_puts(cn, buf);

      sprintf(buf, "%s%s", cn->home, phdr->filename);
      cn->fin = fopen(buf, "r");
      client_write(cn);
    }
  }
}


void
cmd_delete(cn)
  Client *cn;
{
  int n;

  if (n = do_number(cn, 0))
  {
    pop3header *phdr;
    phdr = &cn->cache[n - 1];
    if (phdr->savemode == 'X')
      client_flush(cn, POP3_DELETE_MSG);
    else
    {
      phdr->savemode = 'X';
      cn->mode = CM_DIRTY;
      client_flush(cn, "+OK message deleted");
    }
  }
}


void
cmd_list(cn)
  Client *cn;
{
  int n;
  char buf[256];
  pop3header *phdr;

  n = do_number(cn, -1);

  if (n < 0)
  {
    int max;

    max = cn->totalnum;

    client_puts(cn, "+OK");
    for (n = 0, phdr = cn->cache; n < max; phdr++)
    {
      n++;
      if (phdr->savemode == ' ')
      {
        sprintf(buf, "%d %d", n, phdr->mailsize);
        client_puts(cn, buf);
      }
    }
    client_flush(cn, ".");
  }
  else if (n > 0)
  {
    phdr = &cn->cache[n - 1];
    if (phdr->savemode == 'X')
      client_flush(cn, POP3_DELETE_MSG);
    else
    {
      sprintf(buf, "+OK %d %d", n, phdr->mailsize);
      client_flush(cn, buf);
    }
  }
}


void
cmd_uidl(cn)
  Client *cn;
{
  int n;
  char buf[256];
  pop3header *phdr;

  n = do_number(cn, -1);

  if (n < 0)
  {
    int max;

    max = cn->totalnum;

    client_puts(cn, "+OK");
    for (n = 0, phdr = cn->cache; n < max; phdr++)
    {
      n++;
      if (phdr->savemode == ' ')
      {
        sprintf(buf, "%d %s", n, phdr->filename + 2);
        client_puts(cn, buf);
      }
    }
    client_flush(cn, ".");
  }
  else if (n > 0)
  {
    phdr = &cn->cache[n - 1];
    if (phdr->savemode == 'X')
      client_flush(cn, POP3_DELETE_MSG);
    else
    {
      sprintf(buf, "+OK %d %s", n, phdr->filename + 2);
      client_flush(cn, buf);
    }
  }
}


void
cmd_quit(cn)
  Client *cn;
{
  int n;

  n = cn->mode;

  if (n >= CM_LOGIN)
  {
    pop3header *phdr;
    char *str;

    if (n >= CM_DIRTY)
    {
      mailbox_close(cn);
    }

    if (phdr = cn->cache)
    {
      n = cn->totalnum;

      for (;;)
      {
        if (str = phdr->author)
          (void) free(str);

        if (--n == 0)
          break;

        phdr++;
      }
      (void) free(cn->cache);
      cn->cache = NULL;
    }
  }

  n = cn->sock;
  write(n, POP3_BYE_MSG, POP3_BYE_LEN);
  close(n);

  cn->sock = -1;
  cn->state = CS_FREE;
  logit("BYE", cn->userid);
}


struct
{
  char *cmd;
  void (*fun) ();
}      cmdlist[] =
{
  "list", cmd_list,
  "stat", cmd_stat,
  "retr", cmd_retrive,
  "dele", cmd_delete,
  "uidl", cmd_uidl,
  "last", cmd_last,
  "user", cmd_user,
  "pass", cmd_password,
  "rset", cmd_reset,
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
  logit(msg, cn->userid);
  if (cn->mode > CM_LOGIN)
    cn->mode = CM_LOGIN;        /* timeout/closing 的 client 不刪除信件 */
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
        timeout = POP3_TIMEOUT;
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
      if (++freedom >= POP3_FREEDOM)
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

  n = write(csock, POP3_HELLO_MSG, POP3_HELLO_LEN);
  if (n != POP3_HELLO_LEN)
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
  cn->passwd[0] = '\0';
  cn->cache = NULL;
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

  fd = open(POP3_PIDFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
  fsin.sin_port = htons(POP3_PORT);

  if (bind(fd, (struct sockaddr *) & fsin, sizeof(fsin)) < 0) {
    logit("EXIT", "bind error");
    syslog(LOG_NOTICE, "bind error\n");
    exit(1);
  }

  openlog("bpop3d", LOG_PID, LOG_AUTH);
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
  fclose(fp_passwd);
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
  (void) chdir(POP3_HOME);

  log_init();
  if (!(fp_passwd = fopen(fn_passwd, "r"))) {
     logit("EXIT", ".PASSWDS opened error");
     syslog(LOG_NOTICE, ".PASSWDS opened error, abort server\n");
     abort_server();
  }

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


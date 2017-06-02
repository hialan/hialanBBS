/*-------------------------------------------------------*/
/* mbbsd.c       ( NTHU CS MapleBBS Ver 2.36 )           */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : BBS main/			                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#define _MAIN_C_

#include <varargs.h>
#include "bbs.h"
#include <sys/resource.h> 
#include <sys/wait.h> 
#include <sys/socket.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <arpa/telnet.h>
#include <syslog.h>


#define SOCKET_QLEN 4 
#define TH_LOW 100 
#define TH_HIGH 120
#define PID_FILE BBSHOME"/run/mbbsd.pid" 

#ifdef MBBSD_LAST
  #define MBBSD_LASTLOG BBSHOME"/src/.last_mbbsd"
#endif

#define BANNER "【"BOARDNAME"】◎ 電子布告欄系統 ◎("MYHOSTNAME")\r\n\
 %s IP ("MYIP") \r\nPowered by FreeBSD\r\n[mbbsd is running...]\n"

jmp_buf byebye;

char fromhost[STRLEN] = "\0";
char remoteusername[40];
int mbbsd = 1; 

/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol       */
/* ----------------------------------------------------- */

static void
telnet_init()
{
  static char svr[] = {
    IAC, DO, TELOPT_TTYPE,
    IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
    IAC, WILL, TELOPT_ECHO,
    IAC, WILL, TELOPT_SGA
  };

  register int n, len;
  register char *cmd, *data;
  int rset, oset;
  struct timeval to;
  char buf[256];

  data = buf;


  to.tv_sec = 1;
  rset = to.tv_usec = 0;
  FD_SET(0, (fd_set *) & rset);
  oset = rset;
  for (n = 0, cmd = svr; n < 4; n++)
  {
    len = (n == 1 ? 6 : 3);
    write(0, cmd, len);
    cmd += len;

    if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
      recv(0, buf, sizeof(buf), 0);
//      read(0, data, sizeof(buf));
    rset = oset;
  }
}


/* ----------------------------------------------- */
/* 取得 remote user name 以判定身份                */
/* ----------------------------------------------- */


/*
 * rfc931() speaks a common subset of the RFC 931, AUTH, TAP, IDENT and RFC
 * 1413 protocols. It queries an RFC 931 etc. compatible daemon on a remote
 * host to look up the owner of a connection. The information should not be
 * used for authentication purposes. This routine intercepts alarm signals.
 *
 * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
 */

#include <setjmp.h>

#define STRN_CPY(d,s,l) { strncpy((d),(s),(l)); (d)[(l)-1] = 0; }
#define RFC931_TIMEOUT   10
#define RFC931_PORT     113     /* Semi-well-known port */
#define ANY_PORT        0       /* Any old port will do */


/* ------------------------- */
/* timeout - handle timeouts */
/* ------------------------- */


static void
timeout(sig)
  int sig;
{
  (void) longjmp(byebye, sig);
}


static void
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
  if (setjmp(byebye) == 0)
  {
  /* CHANGE by hialan.020729 for hostname*/
    (void)  signal(SIGALRM, timeout);
    (void)  alarm(3);
    hp = gethostbyaddr((char *) &from->sin_addr, sizeof(struct in_addr),
      from->sin_family);
    (void)  alarm(0);
  }

  /* ADD by hialan.020729 for hostname*/
  (void) strcpy(rhost, hp ? hp->h_name : (char *) inet_ntoa(from->sin_addr));

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
    (void) close(s);
    perror("bbsd:fdopen");
    return;
  }

  /*
   * Set up a timer so we won't get stuck while waiting for the server.
   */

  if (setjmp(byebye) == 0)
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

        if (cp = (char *) strchr(user, '\r'))
          *cp = 0;
        strcpy(rname, user);
      }
    }
    alarm(0);
  }
  fclose(fp);
}


/* ----------------------------------------------------- */
/* stand-alone daemon                                    */
/* ----------------------------------------------------- */


static int mainset;             /* read file descriptor set */
static struct sockaddr_in xsin;


static void
reapchild()
{
  int state, pid;

  while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0);
}


static void
start_daemon(genbuf)
  char *genbuf;
{
  int n;
  char buf[80];

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time_t dummy = time(NULL);
  struct tm *dummy_time = localtime(&dummy);
  (void) strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", dummy_time);
/*
  (void) gethostname(myhostname, sizeof(myhostname));
*/

  if (n=fork())
   {
//    printf("pid[%d]\n",n);
    printf("%d [The_main_pid]\n",n);    
    exit(0);
   }
  n = getdtablesize();
  sprintf(genbuf, "%d\t%s", getpid(), buf);

  while (n)
    (void) close(--n);

  n = open("/dev/tty", O_RDWR);
  if (n > 0)
  {
    (void) ioctl(n, TIOCNOTTY, (char *) 0);
    (void) close(n);
  }

  for (n = 1; n < NSIG; n++)
    (void) signal(n, SIG_IGN);
}


static void
close_daemon()
{
  exit(0);
}


static int
bind_port(port)
  int port;
{
  int sock, on;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  on = 1;
  (void) setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
  (void) setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof(on));

  on = 0;
  (void) setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &on, sizeof(on));

  xsin.sin_port = htons(port);
  if (bind(sock, (struct sockaddr *)&xsin, sizeof xsin) < 0) {
    syslog(LOG_INFO, "bbsd bind_port can't bind to %d",port);
    exit(1);
  }

  if (listen(sock, SOCKET_QLEN) < 0) {
    syslog(LOG_INFO, "bbsd bind_port can't listen to %d",port);
    exit(1);
  }

  /*(void)*/ FD_SET(sock, (fd_set *) & mainset);
  return sock;
}

bad_host(char* name)
{
   FILE* list;
   char buf[40];

  if (list = fopen(BBSHOME"/etc/bad_host", "r")) {
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


main(argc, argv,envp)
  int argc;
  char *argv[];
  char *envp[];
{
  extern int errno;
  register int msock, csock;    /* socket for Master and Child */
  register int nfds;            /* number of sockets */
  register pid_t pid;
  int readset;
  int value;
  struct timeval tv;
  char genbuf[1024];  /*hialan*/

  genbuf[0] = 0;
  
  /* --------------------------------------------------- */
  /* setup standalone                                    */
  /* --------------------------------------------------- */
  start_daemon(genbuf);

  (void) signal(SIGCHLD, reapchild);
  (void) signal(SIGTERM, close_daemon);


  /* --------------------------------------------------- */
  /* port binding                                        */
  /* --------------------------------------------------- */

  xsin.sin_family = AF_INET;

  sprintf(margs,"%s ",argv[0]);
  if (argc > 1)
  {
    msock = -1;
    for (nfds = 1; nfds < argc; nfds++)
    {
      csock = atoi(argv[nfds]);
      if (csock > 0)
        {
         msock = bind_port(csock);
         strcat(margs,argv[nfds]);
	 strcat(margs," ");
	}
      else
         break;
    }
    if (msock < 0)
      exit(1);
  }
  else
  {
    static int ports[] = {23,3333,3006};

    for (nfds = 0; nfds < sizeof(ports) / sizeof(int); nfds++)
    {
      csock = ports[nfds];
      msock = bind_port(csock);

      sprintf(genbuf + 512, "\t%d", csock);
      strcat(genbuf, genbuf + 512);
    }
  }
  nfds = msock + 1;

  /* --------------------------------------------------- */
  /* Give up root privileges: no way back from here      */
  /* --------------------------------------------------- */

  (void) setgid(BBSGID);
  (void) setuid(BBSUID);
  (void) chdir(BBSHOME);

  f_cat(PID_FILE, genbuf); 


  initsetproctitle(argc, argv, envp);
  printpt("%s: listening ",margs);

#ifdef MBBSD_LAST
  if(system("test " MBBSD_LASTLOG) == 0)
    f_rm(MBBSD_LASTLOG);
  f_cat(MBBSD_LASTLOG, genbuf);
#endif

  /* --------------------------------------------------- */
  /* main loop                                           */
  /* --------------------------------------------------- */

/*
  th_low = (argc > 1) ? atoi(argv[1]) : TH_LOW;
  th_high = (argc > 2) ? atoi(argv[2]) : TH_HIGH;
*/

  tv.tv_sec = 60 * 30;
  tv.tv_usec = 0;


  for (;;)
  {
again:

    readset = mainset;
    msock = select(nfds, (fd_set *) & readset, NULL, NULL, &tv);

    if (msock < 0)
    {
      goto again;
    }
    else if (msock == 0)        /* No network traffic */
      continue;

    msock = 0;
    csock = 1;
    for (;;)
    {
      if (csock & readset)
        break;
      if (++msock >= nfds)
        goto again;
      csock <<= 1;
    }

    value = sizeof xsin;
    do
    {
      csock = accept(msock, (struct sockaddr *)&xsin, &value);
    } while (csock < 0 && errno == EINTR);

    if (csock < 0)
    {
      goto again;
    }

/*
    pid = *totaluser;
    if (pid >= MAXACTIVE)
    {
      char buf[128];

      (void) sprintf(buf, "目前線上人數 [%d] 人，客滿了，請稍後再來", pid);
      (void) write(csock, buf, strlen(buf));
      (void) close(csock);
      goto again;
    }
*/
    pid = fork();

    if (!pid)
    {
     char info[256];
     time_t now;

     initsetproctitle(argc, argv, envp);
     nice(2);      /*  Ptt lower priority */
//     sprintf(info, BANNER, genbuf);   //marked by hialan
     sprintf(info, BANNER, "");
     info[255]=0;
     write(csock,info , strlen(info));
      while (--nfds >= 0)
        (void) close(nfds);
      (void) dup2(csock, 0);
      (void) close(csock);
      time(&now);
      login_start_time = now;

      getremotename(&xsin, fromhost, remoteusername);   /* FC931 */

      /* ban 掉 bad host / bad user  */
      if (bad_host(fromhost))
         exit(1);
      setenv("REMOTEHOST", fromhost, 1);
      setenv("REMOTEUSERNAME", remoteusername, 1);
      {
        char RFC931[80];
        sprintf(RFC931, "%s@%s", remoteusername, fromhost);
        setenv("RFC931", RFC931, 1);
      }
      telnet_init();
      printpt("mbbsd : %s",fromhost);
      start_client(0);
    }
    
    (void) close(csock);
  }
}

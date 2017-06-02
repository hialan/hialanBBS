/*-------------------------------------------------------*/
/* bbsrf.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : BBS front-end shell                          */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"
#undef MAXPATHLEN
#include <sys/param.h>


#define USE_UTMP

#if defined(SOLARIS) || defined(SunOS)
#define USE_UTMPX
#endif

#ifdef SYSV
#include <sys/utsname.h>
#endif

/*-------------------------------------------------------*/

#ifdef USE_UTMP
#include <utmp.h>
#endif

#ifdef USE_UTMPX
#include <utmpx.h>
#endif

#include <pwd.h>

char *bbs_prog_path = "bin/bbs";


char *ttyname();

#ifdef USE_UTMPX
struct utmpx *
invis()
{
  static struct utmpx data;
  FILE *fp;
  char *tp;
  struct passwd *getpwuid(), *pp;

  tp = ttyname(0);
  if (!tp)
    return NULL;
  tp = index(tp, '/') + 1;
  tp = index(tp, '/') + 1;
  pp = getpwuid(getuid());
  if (!pp)
  {
    fprintf(stderr, "You Don't exist!\n");
    exit(0);
  }

  if ((fp = fopen(UTMPX_FILE, "r")) == NULL)
  {
    printf("Cannot open %s \n", UTMPX_FILE);
    exit(0);
  }

  while (read(fileno(fp), &data, sizeof(struct utmpx)) > 0)
  {
    if (data.ut_type != DEAD_PROCESS && !strcmp(tp, data.ut_line))
    {
      struct utmpx nildata;

      memcpy(&nildata, &data, sizeof(nildata));

#ifdef INVISIBLE
      memset(nildata.ut_name, 0, 8);
      fseek(fp, (off_t) (ftell(fp) - sizeof(struct utmpx)), 0);
      write(fileno(fp), &nildata, sizeof(struct utmpx));
#endif

      fclose(fp);
      return &data;
    }
  }
  fclose(fp);
  return NULL;
}
#endif


#ifdef USE_UTMP
struct utmp *
invis()
{
  static struct utmp data;
  FILE *fp;
  char *tp;
  struct passwd *getpwuid(), *pp;
  char line[20];

  if ((tp = ttyname(0)) == 0)
    return NULL;
  strcpy(line, tp);
  tp = strrchr(line, '/') + 1;
  if (strlen(tp) != 5)
    return NULL;
  pp = getpwuid(getuid());
  if (!pp)
  {
    fprintf(stderr, "You Don't exist!\n");
    exit(0);
  }

  if ((fp = fopen(_PATH_UTMP, "r")) == NULL)
  {
    printf("bbsrf: cannot open %s\n", _PATH_UTMP);
    exit(0);
  }

  while (fread(&data, sizeof(data), 1, fp) == 1)
  {
    if (!strcmp(tp, data.ut_line))
    {
#ifdef INVISIBLE
      struct utmp nildata;

      memcpy(&nildata, &data, sizeof(nildata));

      memset(nildata.ut_name, 0, 8);
      fseek(fp, -(off_t)sizeof(struct utmp), SEEK_CUR);
      write(fileno(fp), &nildata, sizeof(struct utmp));
#endif
      fclose(fp);
      return &data;
    }
  }
  fclose(fp);
  return NULL;
}
#endif

main()
{
  int uid;
  FILE *fp;
  char buff[255];
  float c1,c2,c3;

  if(fp=fopen("/proc/loadavg","r"))
   { 
     if(fscanf(fp,"%f %f %f",&c1,&c2,&c3) == 3)
	{
	  printf( "\r\nу金金龟穨》 瑈︽呼 》"
		  "(ptt.m8.ntu.edu.tw)\r\n(╰参璽更 %.2f %.2f %.2f)"
	          "秸繵[140.112.251.10]\r\n\n",c1,c2,c3);
	}
     fclose(fp);
   }
 
  if(fp=fopen("BAN","r"))
   {
    while(fgets(buff,255,fp)) printf(buff);
    printf("\n==========================================================\n");
    fclose(fp);
    sleep(1);
    exit(1);
   }
  uid = getuid();

//  printf("%d",uid); /*hialan test*/
  /*セ琌9999*/
  if (uid == 10009)
  {
#ifdef USE_UTMP
    struct utmp *whee;
#endif

#ifdef USE_UTMPX
    struct utmpx *whee;
#endif

    char hid[MAXHOSTNAMELEN + 1];
    whee = invis();
#ifndef WITHOUT_CHROOT
    if (chroot(BBSHOME))
    {
      printf("Cannot chroot, exit!\n");
      exit(-1);
    }
    chdir("/");
#else
    chdir(BBSHOME);
#endif

    setuid(uid);

    if (whee)
    {
      char ttybuf[16];
      char *tp;

      strcpy(ttybuf, (tp = ttyname(0)) ? tp : "/dev/ttyp0");

      if (whee->ut_host[0])
        strncpy(hid, whee->ut_host, MAXHOSTNAMELEN);
      else
        gethostname(hid, MAXHOSTNAMELEN);
      hid[MAXHOSTNAMELEN] = '\0';

      execl(bbs_prog_path, "bbs", hid, ttybuf, NULL);
    }
    else
    {
      execl(bbs_prog_path, "bbs", "unknown", "notty", NULL);
    }

    printf("execl failed\n");
    exit(-1);
  }

  setuid(uid);
  printf("UID DOES NOT MATCH\n");
  exit(-1);
}

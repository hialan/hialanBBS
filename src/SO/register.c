/*-------------------------------------------------------*/
/* register.c   ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : user register routines                       */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"
#include "stdarg.h"

/* -------------------------------- */
/* New policy for allocate new user */
/* (a) is the worst user currently  */
/* (b) is the object to be compared */
/* -------------------------------- */

#undef VACATION     // ¬O§_¬°´H´»°²«O¯d±b¸¹´Á¶¡

static int
compute_user_value(urec, clock)
  userec *urec;
  time_t clock;
{
  int value;

  /* if (urec) has XEMPT permission, don't kick it */
  if ((urec->userid[0] == '\0') || (urec->userlevel & PERM_XEMPT))
    return 9999;

  value = (clock - urec->lastlogin) / 60;       /* minutes */

  /* new user should register in 60 mins */
  if (strcmp(urec->userid, str_new) == 0)
    return (60 - value);

#ifdef  VACATION
  return 180 * 24 * 60 - value; /* ´H´»°²«O¦s±b¸¹ 180 ¤Ñ */
#else
  if (!urec->numlogins)         /* ¥¼ login ¦¨¥\ªÌ¡A¤£«O¯d */
    return -1;
  else if (urec->numlogins <= 3)     /* #login ¤Ö©ó¤TªÌ¡A«O¯d 30 ¤Ñ */
    return 30 * 24 * 60 - value;

  /* ¥¼§¹¦¨µù¥UªÌ¡A«O¯d 30 ¤Ñ */
  /* ¤@¯ë±¡ªp¡A«O¯d 180 ¤Ñ */
  else
    return (urec->userlevel & PERM_LOGINOK ? 180 : 30) * 24 * 60 - value;
#endif
}


static int getnewuserid()
{
  static char *fn_fresh = ".fresh";
  extern struct UCACHE *uidshm;
  userec utmp, zerorec;
  time_t clock;
  struct stat st;
  int fd, val, i;
  char genbuf[200];
  char genbuf2[200];

  memset(&zerorec, 0, sizeof(zerorec));
  clock = time(NULL);

  /* -------------------------------------- */
  /* Lazy method : ¥ý§ä´M¤w¸g²M°£ªº¹L´Á±b¸¹ */
  /* -------------------------------------- */

  if ((i = searchnewuser(0)) == 0)
  {

    /* ------------------------------- */
    /* ¨C 1 ­Ó¤p®É¡A²M²z user ±b¸¹¤@¦¸ */
    /* ------------------------------- */

    if ((stat(fn_fresh, &st) == -1) || (st.st_mtime < clock - 3600))
    {
      if ((fd = open(fn_fresh, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      write(fd, ctime(&clock), 25);
      close(fd);
      log_usies("CLEAN", "dated users");

      printf("´M§ä·s±b¸¹¤¤, ½Ðµy«Ý¤ù¨è...\n\r");
      if ((fd = open(fn_passwd, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      i = 0;  /* Ptt¸Ñ¨M²Ä¤@­Ó±b¸¹¦Ñ¬O³Q¬å°ÝÃD */
      while (i < MAXUSERS)
      {
        i++;
        if (read(fd, &utmp, sizeof(userec)) != sizeof(userec))
          break;
	if(i==1) continue;

        if ((val = compute_user_value(&utmp, clock)) < 0) 
        {
           sprintf(genbuf, "#%d %-12s %15.15s %d %d %d",
             i, utmp.userid, ctime(&(utmp.lastlogin)) + 4,
             utmp.numlogins, utmp.numposts, val);
           if (val > -1 * 60 * 24 * 365)
           {
             log_usies("CLEAN", genbuf);
             sprintf(genbuf, "home/%s", utmp.userid);
             sprintf(genbuf2, "tmp/%s", utmp.userid);
// wildcat : ª½±µ mv , ¤£¥Î¶] rm home/userid
             if (dashd(genbuf))
               f_mv(genbuf, genbuf2);
             lseek(fd, (off_t)((i - 1) * sizeof(userec)), SEEK_SET);
             write(fd, &zerorec, sizeof(utmp));
           }
           else
              log_usies("DATED", genbuf);
        }
      }
      close(fd);
      time(&(uidshm->touchtime));
    }
  }
  if ((fd = open(fn_passwd, O_RDWR | O_CREAT, 0600)) == -1)
    return -1;
  flock(fd, LOCK_EX);

  i = searchnewuser(1);
  if ((i <= 0) || (i > MAXUSERS))
  {
    flock(fd, LOCK_UN);
    close(fd);
    if (more("etc/user_full", NA) == -1)
      printf("©êºp¡A¨Ï¥ÎªÌ±b¸¹¤w¸gº¡¤F¡AµLªkµù¥U·sªº±b¸¹\n\r");
    val = (st.st_mtime - clock + 3660) / 60;
    printf("½Ðµ¥«Ý %d ¤ÀÄÁ«á¦A¸Õ¤@¦¸¡A¯¬§A¦n¹B\n\r", val);
    sleep(2);
    exit(1);
  }

  sprintf(genbuf, "uid %d", i);
  log_usies("APPLY", genbuf);

  strcpy(zerorec.userid, str_new);
  zerorec.lastlogin = clock;
  if (lseek(fd, (off_t)(sizeof(zerorec) * (i - 1)), SEEK_SET) == -1)
  {
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }
  write(fd, &zerorec, sizeof(zerorec));
  setuserid(i, zerorec.userid);
  flock(fd, LOCK_UN);
  close(fd);
  return i;
}

#ifdef REG_FORM
/* --------------------------------------------- */
/* ¨Ï¥ÎªÌ¶ñ¼gµù¥Uªí®æ                            */
/* --------------------------------------------- */

static void
getfield(line, info, desc, buf, len)
  int line, len;
  char *info, *desc, *buf;
{
  char prompt[STRLEN];
  char genbuf[200];

  sprintf(genbuf, "­ì¥ý³]©w¡G%-30.30s (%s)", buf, info);
  move(line, 2);
  outs(genbuf);
  sprintf(prompt, "%s¡G", desc);
  if (getdata(line + 1, 2, prompt, genbuf, len, DOECHO,0))
    strcpy(buf, genbuf);
  move(line, 2);
  prints("%s¡G%s", desc, buf);
  clrtoeol();
}


int u_register()
{
  char rname[20], howto[50]="½Ð½T¹ê¶ñ¼g";
  char phone[20], career[40], email[50],birthday[9],sex_is[2],year,mon,day;
  char ans[3], *ptr;
  FILE *fn;
  time_t now;
  char genbuf[200];
  
  if (cuser.userlevel & PERM_LOGINOK)
  {
    pressanykey("±zªº¨­¥÷½T»{¤w¸g§¹¦¨¡A¤£»Ý¶ñ¼g¥Ó½Ðªí");
    return XEASY;
  }
  if (fn = fopen(fn_register, "r"))
  {
    while (fgets(genbuf, STRLEN, fn))
    {
      if (ptr = strchr(genbuf, '\n'))
        *ptr = '\0';
      if (strncmp(genbuf, "uid: ", 5) == 0 &&
        strcmp(genbuf + 5, cuser.userid) == 0)
      {
        fclose(fn);
        pressanykey("±zªºµù¥U¥Ó½Ð³æ©|¦b³B²z¤¤¡A½Ð­@¤ßµ¥­Ô");
        return XEASY;
      }
    }
    fclose(fn);
  }

  move(2, 0);
  clrtobot();
  strcpy(rname, cuser.realname);
  strcpy(email, cuser.email);
  sprintf(birthday, "%02i/%02i/%02i",
        cuser.year, cuser.month, cuser.day);
  sex_is[0]=(cuser.sex >= '0' && cuser.sex <= '7') ? cuser.sex+'1': '1';sex_is[1]=0;
  career[0] = phone[0] = '\0';
  while (1)
  {
    clear();
    move(3, 0);
    prints("%s[1;32m¡i[m%s[1;32m¡j[m ±z¦n¡A½Ð¾Ú¹ê¶ñ¼g¥H¤Uªº¸ê®Æ:(µLÅÜ§ó½Ð«öenter¸õ¹L)",
      cuser.userid, cuser.username);
    getfield(6, "½Ð½T¹ê¶ñ¼g¤¤¤å©m¦W", "¯u¹ê©m¦W", rname, 20);
    getfield(8, "¾Ç®Õ¨t¯Å©Î³æ¦ìÂ¾ºÙ", "ªA°È³æ¦ì", career, 40);
    getfield(10, "¥]¬Aªø³~¼·¸¹°Ï°ì½X", "³sµ¸¹q¸Ü", phone, 20);
    while (1)
    {
    int len;
    getfield(12, " 19xx/¤ë/¤é ¦p: 77/12/01","¥Í¤é",birthday,9);
    len = strlen(birthday);
    if(!len)
       {
         sprintf(birthday, "%02i/%02i/%02i",
         cuser.year, cuser.month, cuser.day);
         year=cuser.year;
         mon=cuser.month;
         day=cuser.day;
       }
    else if (len==8)
       {
        year  = (birthday[0] - '0') * 10 + (birthday[1] - '0');
        mon = (birthday[3] - '0') * 10 + (birthday[4] - '0');
        day   = (birthday[6] - '0') * 10 + (birthday[7] - '0');
       }
    else
        continue;
    if (mon > 12 || mon < 1 || day > 31 || day < 1 || year > 90 || year < 40)
        continue;
    break;
    }
    getfield(14,"1.¸¯®æ 2.©j±µ 3.©³­} 4.¬ü¬Ü","©Ê§O",sex_is,2);
    getfield(16, "¨­¤À»{ÃÒ¥Î", "E-Mail Address", email, 50);
    getfield(18, "±q­þÃäª¾¹D³o­Ó¯¸ªº", "±q¦ó±oª¾", howto, 50);

    ans[0] = getans2(b_lines - 1, 0, "¥H¤W¸ê®Æ¬O§_¥¿½T¡H ", 0, 3, 'n');
    if (ans[0] == 'q')
      return 0;
    if (ans[0] == 'y')
      break;
  }
  cuser.rtimes++;
  strcpy(cuser.realname, rname);
  strcpy(cuser.email, email);  
  cuser.sex= sex_is[0]-'1';
  cuser.month=mon;cuser.day=day;cuser.year=year;
#ifdef  REG_MAGICKEY
  mail_justify(cuser); //»{ÃÒ½X
#endif      
  if (fn = fopen(fn_register, "a"))
  {
    now = time(NULL);
    str_trim(career);
    str_trim(phone);
    fprintf(fn, "num: %d, %s", usernum, ctime(&now));
    fprintf(fn, "uid: %s\n", cuser.userid);
    fprintf(fn, "name: %s\n", rname);
    fprintf(fn, "howto: %s\n", howto);
    fprintf(fn, "career: %s\n", career);
    fprintf(fn, "phone: %s\n", phone);
    fprintf(fn, "email: %s\n", email);
    fprintf(fn, "----\n");
    fclose(fn);
  }
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* °O¿ý */
  return 0;
}
#endif

/*--------------------------*/ 
/* mode==0 ¬°µn¤Jµù¥U(¹w³]) */ 
/* mode==1 ¬°ºÞ²z·s¼W¨Ï¥ÎªÌ */
/*--------------------------*/

void
new_register(mode)
  int mode;
{
  userec newuser;
  char passbuf[STRLEN];
  char origname[IDLEN + 1];	/*hialan*/
  int allocid, try;

  strcpy(origname, cuser.userid);

  memset(&newuser, 0, sizeof(newuser));

  more("etc/register", NA);
  try = 0;
  while (1)
  {
    if (++try >= 6)
    {
      refresh();

      pressanykey("±z¹Á¸Õ¿ù»~ªº¿é¤J¤Ó¦h¡A½Ð¤U¦¸¦A¨Ó§a");
      oflush();
      if(mode==1)
        return;
      exit(1);
    }
    getdata(16, 0, msg_uid, newuser.userid, IDLEN + 1, DOECHO,0);

    if (bad_user_id(newuser.userid))
      outs("µLªk±µ¨ü³o­Ó¥N¸¹¡A½Ð¨Ï¥Î­^¤å¦r¥À¡A¨Ã¥B¤£­n¥]§tªÅ®æ\n");
    else if (searchuser(newuser.userid))
      outs("¦¹¥N¸¹¤w¸g¦³¤H¨Ï¥Î\n");
    else
      break;
  }

  try = 0;
  while (1)
  {
    if (++try >= 6)
    {
      pressanykey("±z¹Á¸Õ¿ù»~ªº¿é¤J¤Ó¦h¡A½Ð¤U¦¸¦A¨Ó§a");
      oflush();
      if(mode==1)
        return;
      exit(1);
    }
    if ((getdata(17, 0, "½Ð³]©w±K½X¡G", passbuf, PASSLEN, PASS,0) < 4) ||
      !strcmp(passbuf, newuser.userid))
    {
      pressanykey("±K½X¤ÓÂ²³æ¡A©ö¾D¤J«I¡A¦Ü¤Ö­n 4 ­Ó¦r¡A½Ð­«·s¿é¤J");
      continue;
    }
    strncpy(newuser.passwd, passbuf, PASSLEN);
    getdata(18, 0, "½ÐÀË¬d±K½X¡G", passbuf, PASSLEN, PASS,0);
    if (strncmp(passbuf, newuser.passwd, PASSLEN))
    {
      outs("±K½X¿é¤J¿ù»~, ½Ð­«·s¿é¤J±K½X.\n");
      continue;
    }
    passbuf[8] = '\0';
    strncpy(newuser.passwd, genpasswd(passbuf), PASSLEN);
    break;
  }
  newuser.userlevel = PERM_DEFAULT;
  newuser.pager = 1;
  newuser.uflag = COLOR_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
  newuser.firstlogin = newuser.lastlogin = time(NULL);
  srandom(time(0));
  newuser.silvermoney = 10000;
  newuser.habit = HABIT_NEWUSER;	/* user.habit */

  newuser.lightbar[0] = 4;   /* bg */       /* lightbar */
  newuser.lightbar[1] = 7;   /* wd */
  newuser.lightbar[2] = 1;   /* light */
  newuser.lightbar[3] = 0;   /* blite*/
  newuser.lightbar[4] = 0;   /* underline */

  strcpy(newuser.cursor, STR_CURSOR);		    /* cursor */
  allocid = getnewuserid();
  if (allocid > MAXUSERS || allocid <= 0)
  {
    fprintf(stderr, "¥»¯¸¤H¤f¤w¹F¹¡©M¡I\n");
    if(mode==1)
    	return;
    exit(1);
  }
  

  if (substitute_record(fn_passwd, &newuser, sizeof(userec), allocid) == -1)
  {
    fprintf(stderr, "«Èº¡¤F¡A¦A¨£¡I\n");
    if(mode==1)
    	return;
    exit(1);
  }

  setuserid(allocid, newuser.userid);
  if (!dosearchuser(newuser.userid))
  {
    fprintf(stderr, "µLªk«Ø¥ß±b¸¹\n");
    if(mode==1)
    	return;
    exit(1);
  }
  
  //³o¸Ì¨S¦³¿ìªk¨Ï¥Îreturn;·|·í±¼
  if(mode==1)
  {
    dosearchuser(origname);
    return;
  }
}



int m_newuser()
{
  clear();
  new_register(1);
  clear();
  return 0;
}

void va_new_register(va_list pvar)
{
  int mode;  
  mode = va_arg(pvar, int);
  new_register(mode);
}
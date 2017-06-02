/*-------------------------------------------------------*/
/* register.c   ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : user register routines                       */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

/* ------------------- */
/* password encryption */
/* ------------------- */

char *crypt();
static char pwbuf[14];

int
chkpasswd(passwd, test)
  char *passwd, *test;
{
  char *pw;
  strncpy(pwbuf, test, 14);
  pw = crypt(pwbuf, passwd);
  return (!strncmp(pw, passwd, 14));
}

/* ------------------ */
/* ÀË¬d user µù¥U±¡ªp */
/* ------------------ */


int
bad_user_id(userid)
  char *userid;
{
  register char ch;

  if (belong("etc/baduser",userid))
    return 1;

  if (strlen(userid) < 2)
    return 1;

  if (not_alpha(*userid))
    return 1;

  if (!ci_strcmp(userid, str_new))
    return 1;

  while (ch = *(++userid))
  {
    if (not_alnum(ch))
      return 1;
  }
  return 0;
}

/* origin: SOB & Ptt              */
/* modify: wildcat/980909         */
/* ½T»{user¬O§_³q¹Lµù¥U¡B¸ê®Æ¥¿½T */
check_register()
{
  char *ptr;
  char genbuf[200],buf[100];

  if(!HAS_PERM(PERM_POST) && (cuser.lastlogin - cuser.firstlogin >= 86400))
    cuser.userlevel |= PERM_POST;  

  stand_title("½Ð¸Ô²Ó¶ñ¼g­Ó¤H¸ê®Æ");

  while (strlen(cuser.username) < 2)
    getdata(2, 0, "ºï¸¹¼ÊºÙ¡G", cuser.username, 24, DOECHO,0);
  strcpy(currutmp->username, cuser.username);

  for (ptr = cuser.username; *ptr; ptr++)
    if (*ptr == 9)              /* TAB convert */
      strcpy(ptr, " ");

  while (strlen(cuser.feeling) < 2)
    getdata(3, 0, "¤ß±¡ª¬ºA¡G", cuser.feeling, 5, DOECHO,0);
  cuser.feeling[4] = '\0';
  strcpy(currutmp->feeling, cuser.feeling);

  while (strlen(cuser.realname) < 4)
    getdata(4, 0, "¯u¹ê©m¦W¡G", cuser.realname, 20, DOECHO,0);

  while (!cuser.month || !cuser.day || !cuser.year)
  {
    sprintf(genbuf, "%02i/%02i/%02i", cuser.year,cuser.month, cuser.day);
    getdata(6, 0, "¥X¥Í¦~¥÷ ¦è¤¸ 19", buf, 3, DOECHO,0);
    cuser.year = (buf[0] - '0') * 10 + (buf[1] - '0');
    getdata(7, 0, "¥X¥Í¤ë¥÷", buf, 3, DOECHO,0);
    cuser.month = (buf[0] - '0') * 10 + (buf[1] - '0');
    getdata(8, 0, "¥X¥Í¤é´Á", buf, 3, DOECHO,0);
    cuser.day = (buf[0] - '0') * 10 + (buf[1] - '0');
    if (cuser.month > 12 || cuser.month < 1 ||
      cuser.day > 31 || cuser.day < 1 || cuser.year > 90 || cuser.year < 40)
      continue;
    break;
  }

  while (cuser.sex > 7)
  {
    char buf;
    char *choose_sex[8]={"11.¸¯®æ","22.©j±µ","33.©³­}","44.¬ü¬Ü","55.Á¦¨û","66.ªü«¼","77.´Óª«","88.Äqª«"};

    buf = getans2(10, 0, "©Ê§O ", choose_sex, 8, cuser.sex + '1');    
//    if (buf >= '1' && buf <= '8')
    cuser.sex = buf - '1';
  }

  if (belong_spam(BBSHOME"/etc/spam-list",cuser.email))
  {
    strcpy(cuser.email,"NULL");
    pressanykey("©êºp,¥»¯¸¤£±µ¨ü§Aªº E-Mail «H½c¦ì¸m");
  }
  
  if (!strchr(cuser.email, '@'))
  {
    bell();
    move(t_lines - 4, 0);
    prints("\
¡° ¬°¤F±zªºÅv¯q¡A½Ð¶ñ¼g¯u¹êªº E-mail address¡A ¥H¸ê½T»{»Õ¤U¨­¥÷¡A\n\
   ®æ¦¡¬° [44muser@domain_name[0m ©Î [44muser@\\[ip_number\\][0m¡C\n\n\
¡° ¦pªG±z¯uªº¨S¦³ E-mail¡A½Ðª½±µ«ö [return] §Y¥i¡C");

    do
    {
      getdata(12, 0, "¹q¤l«H½c¡G", cuser.email, 50, DOECHO,0);
      if (!cuser.email[0])
        sprintf(cuser.email, "%s%s", cuser.userid, str_mail_address);
      if(belong_spam(BBSHOME"/etc/spam-list",cuser.email))
      {
        strcpy(cuser.email, "NULL");
        pressanykey("©êºp,¥»¯¸¤£±µ¨ü§Aªº E-Mail «H½c¦ì¸m");
      }
    } while (!strchr(cuser.email, '@'));

#ifdef  REG_MAGICKEY   
    mail_justify(cuser);
#endif

  }

  cuser.userlevel |= PERM_DEFAULT;
  if (!HAS_PERM(PERM_SYSOP) && !(cuser.userlevel & PERM_LOGINOK))
  {
    /* ¦^ÂÐ¹L¨­¥÷»{ÃÒ«H¨ç¡A©Î´¿¸g E-mail post ¹L */

    setuserfile(genbuf, "email");
    if (dashf(genbuf))
      cuser.userlevel |= ( PERM_POST );

#ifdef  STRICT
    else
    {
      cuser.userlevel &= ~PERM_POST;
      more("etc/justify", YEA);
    }
#endif

  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
/* wildcat 981218 */
    clear();
    update_data(); 

#ifdef REG_FORM
    if (HAS_PERM(PERM_POST) && !HAS_PERM(PERM_LOGINOK)
      && answer("¬O§_­n¶ñ¼gµù¥U³æ (y/N)") == 'y')
    //u_register();
    DL_func("SO/register.so:u_register");
#endif

  }
  if (HAS_PERM(PERM_DENYPOST) && !HAS_PERM(PERM_SYSOP))
    cuser.userlevel &= ~PERM_POST;
}

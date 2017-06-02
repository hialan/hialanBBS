/* ¨Ï¥Î¿úªº¨ç¼Æ */

#include "bbs.h"

/* ªá¿ú¿ï³æ */
unsigned 
setuperm(pbits, nb, money)  /* ¶RÅv­­¥Î */
  unsigned pbits;
  char nb;
  int money;
{
  register int i;
  
  i = nb - 'a';
  if (!((pbits >> i) & 1))
  {
    pbits ^= (1 << i);
    degold(money);
  }
  return(pbits);
}


/*¶R½æ°O¿ı*/
void 
tradelog(userid, i)
  char *userid;  
  int i;
{
  time_t now = time(0);
  char genbuf[200];
  char *item[4] = {"«H½c¤W­­","¥Ã¤[Áô¨­","¥Ã¤[­×§ï¬G¶m","«H¥óµL¤W­­"};

  now = time(NULL) - 6 * 60;
  sprintf(genbuf, "¦b %s ¨Ï¥ÎªÌ [1;32m%s[m ÁÊ¶R¤F[1;36m%s[mªºÅv­­",
    Cdate(&now),cuser.userid,item[i]);
  f_cat("log/trade.log",genbuf);
}

void
p_cloak()
{
  if (getans(currutmp->invisible ? "½T©w­n²{¨­?[y/N]" : "½T©w­nÁô¨­?[y/N]") != 'y')
    return;

  if(!currutmp->invisible)
  {
    if (check_money(2,GOLD)) 
      return;
    degold(2);
  }
  currutmp->invisible %= 2;
  pressanykey((currutmp->invisible ^= 1) ? MSG_CLOAKED : MSG_UNCLOAK);
  return;
}

void
p_fcloak()
{
  register int i;

  if (check_money(500,GOLD) || HAS_PERM(PERM_CLOAK))
  {
    if (HAS_PERM(PERM_CLOAK))
      pressanykey("§A¤w¸g¥i¥HÁô§Î¤FÁÙ¨Ó¶R¡A¶û¿ú¤Ó¦h°Ú¡H");
    return;
  }
  if (getans("½T©w­nªá $500 ¾Ç²ß²×·¥Áô¨­¤jªk¡H[y/N]") != 'y')
    return;
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  i = setuperm(cuser.userlevel,'g',500);
  update_data();
  cuser.userlevel = i;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  tradelog(cuser.userid,1);
  pressanykey("®¥³ß±z¤w¸g¾Ç·|¤F²×·¥Áô¨­¤jªk!!");
  return;
}

void
p_from()
{
  
  if (check_money(5, GOLD)) 
    return;

  if (getans("½T©w­n§ï¬G¶m?[y/N]") != 'y')
    return;

  if (getdata(b_lines, 0, "½Ğ¿é¤J·s¬G¶m:", currutmp->from, 17, DOECHO,0))
  {
    degold(5);
    currutmp->from_alias=0;
  }
  return;
}

void
p_ffrom()
{
  register int i;

  if(check_money(1000,GOLD) || HAS_PERM(PERM_FROM) || HAS_PERM(PERM_SYSOP))
  {
    if(HAS_PERM(PERM_FROM) || HAS_PERM(PERM_SYSOP))
      pressanykey("§A¤w¸g¥i¥H­×§ï¬G¶m¤FÁÙ¨Ó¶R¡A¶û¿ú¤Ó¦h°Ú¡H");
    return;
  }
  
  if (getans("½T©w­nªá $1000 ÁÊ¶R­×§ï¬G¶mÄ_¨å¡H[y/N]") != 'y')
     return;
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  i=setuperm(cuser.userlevel,'t',1000);
  update_data();
  cuser.userlevel=i;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  tradelog(cuser.userid,2);
  pressanykey("®¥³ß§A±o¨ì¤F­×§ï¬G¶mÄ_¨å");
  return;
}


void
p_exmail()
{
  char ans[4],buf[100];
  int  n;
  if(cuser.exmailbox >= MAXEXKEEPMAIL )
  {
    pressanykey("®e¶q³Ì¦h¼W¥[ %d «Ê¡A¤£¯à¦A¶R¤F¡C", MAXEXKEEPMAIL);
    return;
  }
  sprintf(buf,"±z´¿¼WÁÊ %d «Ê®e¶q¡AÁÙ­n¦A¶R¦h¤Ö?",cuser.exmailbox);
  getdata(b_lines, 0, buf,ans,3,LCECHO,"10");
  n = atoi(ans);
  if(!ans[0] || !n )
    return;
  if(n+cuser.exmailbox > MAXEXKEEPMAIL ) 
     n = MAXEXKEEPMAIL - cuser.exmailbox;
  if(check_money(n*100,GOLD))
    return;
  degold(n*100);
  inmailbox(n);
  return;
}

void
p_spmail()
{
  char ans[4],buf[128];
  int n;
  if (HAVE_PERM(PERM_SYSOP) || HAVE_PERM(PERM_MAILLIMIT)) 
  {
    if(HAVE_PERM(PERM_MAILLIMIT))
      pressanykey("¤w¸g«H½cµL¤W­­¤F, ¤£¯à½æ°Õ!!");
    return;
  }
  if(HAS_PERM(PERM_LOGINOK))  // ¬O§_³q¹L¨­¤À»{ÃÒ 
  {
    if(cuser.exmailbox <=0)
    {
      pressanykey("Sorry,±z¨S¦³¶R¹L«H½c©Ò¥H¤£¯à½æ !!");
      return;
    }
    else  // ¦pªG³£ check ¹L¤F¡A¥Dµ{¦¡  
    {
      sprintf(buf,"§A¥Ø«e¦³ %d «Ê«H½c®e¶q¥i¥H½æ¥X¡A§A­n½æ¥X´X«Ê ? ",cuser.exmailbox);
      getdata(b_lines, 0, buf, ans, 3, LCECHO, "10");
      n = atoi(ans);
      if(!ans[0] || !n )
        return;
      if (n > cuser.exmailbox)  
      {
        n = cuser.exmailbox;
        pressanykey("¶W¹L¥i¥H½æ¥Xªº¼Æ¶q!! ½æ¥X¼Æ¶q§ï¬° %d", cuser.exmailbox);
      }

      ingold(n*80);
      demailbox(n);
    }
  }
  sprintf(buf,"±z¥Ø«e½æ¥X¤F %d «Ê«H¡AÁÙ¦³³Ñ¤U %d «Ê¼W¥[«H½c®e¶q",n, cuser.exmailbox);  
  pressanykey(buf);
  chkmailbox();
}


void
p_ulmail()
{
  register int i;
  if(check_money(100000,GOLD) || HAS_PERM(PERM_MAILLIMIT))
  {
    if(HAS_PERM(PERM_MAILLIMIT))
      pressanykey("§Aªº«H½c¤w¸g¨S¦³­­¨î¤FÁÙ¨Ó¶R¡A¶û¿ú¤Ó¦h°Ú¡H");
    return;
  }
  if (getans("½T©w­nªá $100000 ÁÊ¶RµL¤W­­«H½c?[y/N]") != 'y')
    return;
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  i=setuperm(cuser.userlevel,'f',100000);
  update_data();
  cuser.userlevel=i;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  tradelog(cuser.userid,3);
  pressanykey("®¥³ß±z¤w¸g±o¨ì¤FµL¤W­­ªº«H½c!!");
  return;
}

void
p_give()
{
   int money;
   char id[IDLEN+1],buf[256],reason[60];
   FILE *fp=fopen("tmp/givemoney","w");
   fileheader mymail;
   time_t now;
   time(&now);
   move(12,0);
   update_data();
   usercomplete("¿é¤J¹ï¤èªºID¡G", id);
   if (!id[0] || !getdata(14, 0, "­nÂà¦h¤Ö¿ú¹L¥h¡H", buf, 9, LCECHO,0)) return;
   money = atoi(buf);
   if(check_money(money,SILVER)) return;
   if(money > 0)
   {
     demoney(money);
     money *= 0.9;
     inumoney(id, money);
     sprintf(buf,"§@ªÌ: %s \n"
                 "¼ĞÃD:[Âà±b³qª¾] °e§A %d ¤¸­ò¡I\n"
                 "®É¶¡: %s\n",cuser.userid,money,ctime(&now));
     fputs(buf,fp);
     while(!getdata(15,0,"½Ğ¿é¤J²z¥Ñ¡G",reason,60,DOECHO ,"¿ú¤Ó¦h"));
     sprintf(buf,"[1;32m%s[37m °e§A [33m%d [37m¤¸¡C\n"
                 "¥Lªº²z¥Ñ¬O¡G[33m %s [m",cuser.userid,money,reason);
     fputs(buf,fp);
     fclose(fp);
     sprintf(buf,"home/%s", id);
     stampfile(buf, &mymail);
     strcpy(mymail.owner, cuser.userid);
     f_mv ("tmp/givemoney",buf);
     sprintf(mymail.title,"[Âà±b³qª¾] °e§A %d ¤¸­ò¡I",money);
     sprintf(buf,"home/%s/.DIR",id);
     rec_add(buf, &mymail, sizeof(mymail));
     sprintf(buf,"[1;33m%s %s [37m§â²{ª÷ [33m%d ¤¸ [37mÂà±bµ¹[33m %s[37m",
     Cdate(&now),cuser.userid,money,id);
     f_cat("log/bank.log",buf);
   }
   return;
}


void
exchange()
{
  char buf[100], ans[10];
  int i, Money = 0;
  time_t now = time(0);
  
  move(12, 0);
  clrtobot();
  prints("§A¨­¤W¦³ª÷¹ô %d ¤¸,»È¹ô %d ¤¸\n", cuser.goldmoney, cuser.silvermoney);
  outs("\nª÷¹ô ¡G »È¹ô  =  1 ¡G 10000\n");
  if (!getdata(17, 0, "(1)»È¹ô´«ª÷¹ô  (2)ª÷¹ô´«»È¹ô ", ans, 3, LCECHO, 0)) 
    return;

  if (ans[0] < '1' || ans[0] > '2') 
    return;

  i = atoi(ans);
  while (Money <= 0 || 
    (i == 1 ? (Money > cuser.silvermoney) : (Money > cuser.goldmoney)))
  {
    if (i == 1)
      getdata(18,0,"­n®³¦h¤Ö»È¹ô¨Ó´«¡H ",ans,10,LCECHO,0);
    else
      getdata(18,0,"­n®³¦h¤Öª÷¹ô¨Ó´«¡H ",ans,10,LCECHO,0);
    if(!ans[0]) return;
    Money = atol(ans);
  }
  if(i == 1)
    sprintf(buf,"¬O§_­nÂà´«»È¹ô %d ¤¸ ¬°ª÷¹ô %d ? [y/N]",Money,Money/10000);
  else
    sprintf(buf,"¬O§_­nÂà´«ª÷¹ô %d ¤¸ ¬°»È¹ô %d ? [y/N]",Money,Money*10000);
  getdata(19,0,buf,ans,3,LCECHO,0);
  if(ans[0] == 'y')
  {
    if(i == 1)
    {       
      Money *= 1.05;
      demoney(Money);
      ingold(Money/10500);
      sprintf(buf,"[1;36m%s %s [37m§â»È¹ô [33m%d ¤¸ [37mÂà´«¬°ª÷¹ô %d ¤¸",
        Cdate(&now),cuser.userid,Money, Money/10500);
    }
    else
    {
      degold(Money);
      inmoney(Money*9500);
      sprintf(buf,"[1;32m%s %s [37m§âª÷¹ô [33m%d ¤¸ [37mÂà´«¬°»È¹ô %d ¤¸",
        Cdate(&now),cuser.userid,Money, Money*9500);
    }
    f_cat("log/bank.log",buf);
    pressanykey("§A¨­¤W¦³ª÷¹ô %d ¤¸,»È¹ô %d ¤¸",cuser.goldmoney,cuser.silvermoney);
  }
  else
    pressanykey("¨ú®ø.....");
}


/* ª÷®w */
void
bank()
{
  char buf[10];
 
  if (lockutmpmode(BANK)) 
    return;

  setutmpmode(BANK);
  stand_title("¯«·µ»È¦æ");
  
  if (count_multi() > 1)
  {
    pressanykey("±z¤£¯à¬£»º¤À¨­¶i¤J»È¦æËç !");
    unlockutmpmode();    
    return;
  }
  
  counter(BBSHOME"/log/counter/»È¦æ","¨Ï¥Î»È¦æ",0);
  move(2, 0);
  update_data();
  prints("\033[1;36m%12s\033[0;1m ±z¦n§r¡IÅwªï¥úÁ{¥»»È¦æ¡C
[1;36mùúùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùû
ùø[32m±z²{¦b¦³»È¹ô[33m %12d [32m¤¸¡Aª÷¹ô [33m%12d[32m ¤¸[36m        ùø
ùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùâ
ùø ¥Ø«e»È¦æ´£¨Ñ¤U¦C¤T¶µªA°È¡G                               ùø",
    cuser.userid, cuser.silvermoney, cuser.goldmoney);
    move(6, 0);
      outs("\
ùø[33m1.[37m Âà±b -- ¥²¶·¦©±¼Á`ÃBªº 10% §@¬°¤âÄò¶O (­­»È¹ô)[36m         ùø
ùø[33m2.[37m ¶×§I -- »È¹ô/ª÷¹ô §I´« (©â¨ú 5% ¤âÄò¶O) [36m               ùø
ùø[33mQ.[37m Â÷¶}»È¦æ[36m                                               ùø
ùüùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùı[m");
  getdata(12, 0, "  ½Ğ¿é¤J±z»İ­nªºªA°È¡G", buf, 3, DOECHO, 0);
  if (buf[0] == '1')
    p_give();
  else if (buf[0] == '2')
    exchange();

  update_data();  
  pressanykey("ÁÂÁÂ¥úÁ{¡A¤U¦¸¦A¨Ó¡I");
  unlockutmpmode();
}


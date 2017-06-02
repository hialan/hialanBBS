
/* 	這是 舊WD(9806以後)==>>WD990617 轉 .PASSWDS 的程式 
	執行後新的 PASSWDS 會出現在 ~bbs/PASSWDS.NEW
 	注意 :  使用前請務必備份現有 .PASSWDS
		覆蓋檔案前請清光 SHM 並確定站上無任何 USER 	
						紫筠軒 Ziyun   */
#include "bbs.h"
#include "userec.old"
#include "userec.new"

main()
{
  int fdr,fdw,i=0;
  new new;
  old tuser;
  
  fdr=open(BBSHOME"/.PASSWDS",O_RDONLY);
  fdw=open(BBSHOME"/PASSWDS.NEW",O_WRONLY | O_CREAT | O_TRUNC, 0644);
  printf("new struct size :%d\n",sizeof(new));

  while(read(fdr,&tuser,sizeof(old))==sizeof(old))
  {
    if (strlen(tuser.userid) < 2) continue;
    if (not_alpha(*tuser.userid)) continue;
    printf("tran user: %s\n", tuser.userid);

  /*基本資料 136 bytes*/
        memcpy(new.userid,tuser.userid,IDLEN+1);         
        memcpy(new.realname,tuser.realname,20);          
        memcpy(new.realname,tuser.realname,20); 
        memcpy(new.username,tuser.username,24); 
        memcpy(new.passwd,tuser.passwd,PASSLEN);  
      	memcpy(new.email,tuser.email,50);      
      	memcpy(new.countryid, "", 11);   	//身分證字號
  	new.month=tuser.month;              
  	new.day=tuser.day;                  
  	new.year=tuser.year;                
  	new.sex=tuser.sex;                  

  /*系統權限 32 bytes*/
        new.uflag=tuser.uflag;                  
        new.userlevel=tuser.userlevel; 
  	new.invisible=tuser.invisible; 
  	new.state=tuser.state;              
  	new.pager=tuser.pager;  
  	new.habit=tuser.habit;         
  	new.exmailbox=tuser.exmailbox;
  	new.exmailboxk=tuser.exmailboxk;  	
  	new.dtime=tuser.dtime;  	
  	new.update_songtime=tuser.update_songtime;	/*點歌次數更新*/
  	new.scoretimes=tuser.scoretimes;		/*評分次數*/

  /*註冊資料 44 bytes*/
    	new.firstlogin=tuser.firstlogin;    
	memcpy(new.justify,tuser.justify,REGLEN + 1);      	
	new.rtimes=tuser.rtimes;

  /*喜好設定 61 bytes*/
        memcpy(new.feeling,tuser.feeling,5);
        new.lightbar[0] = 4;
        new.lightbar[1] = 7;
        new.lightbar[2] = 1;
        new.lightbar[3] = 0;
        new.lightbar[4] = 0;
        memcpy(new.cursor, ">>", 51);


  /*系統資料 130 bytes*/
        new.numlogins=tuser.numlogins;          
        new.numposts=tuser.numposts;            
  	new.lastlogin=tuser.lastlogin;      
  	memcpy(new.lasthost,tuser.lasthost,24);
  	memcpy(new.vhost,tuser.lasthost,24);   
  	memcpy(new.toqid,tuser.userid,IDLEN+1);     
  	memcpy(new.beqid,tuser.userid,IDLEN+1);     
  	new.toquery=tuser.toquery;      
  	new.bequery=tuser.bequery;      
  	new.totaltime=tuser.totaltime;  
  	new.sendmsg=tuser.sendmsg;      
  	new.receivemsg=tuser.receivemsg;
	new.silvermoney=tuser.silvermoney;
        new.goldmoney = tuser.goldmoney;
        new.songtimes=tuser.songtimes;	

        write(fdw,&new,sizeof(new));

        ++i;
   }
   close(fdr);
   close(fdw);
}     

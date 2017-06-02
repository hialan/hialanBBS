#include "bbs.h"

boardheader bh;

struct new
{
  char brdname[IDLEN + 1];      /* 看板英文名稱    13 bytes */
  char title[BTLEN + 1];        /* 看板中文名稱    49 bytes */
  char BM[IDLEN * 3 + 3];       /* 板主ID和"/"     39 bytes */
  usint brdattr;                /* 看板的屬性       4 bytes */
  time_t bupdate;               /* note update time 4 bytes */
  uschar bvote;                 /* Vote flags       1 bytes */
  time_t vtime;                 /* Vote close time  4 bytes */
  usint level;                  /* 可以看此板的權限 4 bytes */
  unsigned long int totalvisit; /* 總拜訪人數       8 bytes */
  unsigned long int totaltime;  /* 總停留時間       8 bytes */
  char lastvisit[IDLEN + 1];    /* 最後看該板的人  13 bytes */
  time_t opentime;              /* 開板時間         4 bytes */
  time_t lastime;               /* 最後拜訪時間     4 bytes */
  char passwd[PASSLEN];         /* 密碼            14 bytes */
  unsigned long int postotal;   /* 總水量 :p        8 bytes */
  usint maxpost;
  usint maxtime;
  char desc[3][80];
  char pad[87];
};

typedef struct new new;

int
invalid_brdname (brd)		/* 定義錯誤看板名稱 */
     char *brd;
{
  register char ch;

  ch = *brd++;
  if (not_alnum (ch))
    return 1;
  while (ch = *brd++)
    {
      if (not_alnum (ch) && ch != '_' && ch != '-' && ch != '.')
	return 1;
    }
  return 0;
}

main()
{
  int fdr,fdw, i = 0;
  new new;
  
  fdr=open(BBSHOME"/.BOARDS",O_RDONLY);
  fdw=open(BBSHOME"/BOARDS.NEW",O_WRONLY | O_CREAT | O_TRUNC, 0644);

  printf("size of new struct is %d\n",sizeof(new));
  while(read(fdr,&bh,sizeof(boardheader))==sizeof(boardheader))
  {     
  	i++;
	if(!bh.brdname[0]) continue;
	if(invalid_brdname(bh.brdname)) continue;
	printf("
=====================================================
brd num   : %d
boardname : %s
title     : %s
totalvisit: %d
=====================================================\n"
,i,bh.brdname,bh.title,bh.totalvisit);
        memcpy(new.brdname,bh.brdname,IDLEN+1);
        memcpy(new.title,bh.title,BTLEN + 1);
        memcpy(new.BM,bh.BM,24);
        new.brdattr = bh.brdattr;  
        new.bupdate=bh.bupdate;
        new.bvote = bh.bvote;
        new.vtime=bh.vtime;
	new.level=bh.level;  
        new.totalvisit=bh.totalvisit;
        new.totaltime=bh.totaltime;
        memcpy(new.lastvisit,bh.lastvisit,IDLEN+1);
        new.opentime=bh.opentime;
        new.lastime=bh.lastime;
        memcpy(new.passwd,bh.passwd,PASSLEN); 
        memcpy(new.desc[0],"尚未編輯",80);
        memcpy(new.desc[1],"尚未編輯",80);
        memcpy(new.desc[2],"尚未編輯",80);
  	new.postotal=bh.postotal;
  	if(bh.maxtime=365)new.maxtime=1000;
  	else new.maxtime=bh.maxtime;
  	new.maxpost=5000;
        write(fdw,&new,sizeof(new));
   }
   close(fdr);
   close(fdw);
}     

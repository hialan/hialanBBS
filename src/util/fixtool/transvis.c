#include "bbs.h"

struct visable
{
  char filename[FNLEN];         /* list name/userid */
  char savemode;                
  char owner[IDLEN + 2];        /* /bbcall */
  char date[6];                 /* /birthday */
  char title[TTLEN + 1];	/* list/user desc */
  uschar filemode;              /* mode: PAL, BAD */
};

typedef struct visable vis;

main()
{
  int fdr,fdw, i = 0;
  new new;
  char buf[IDLEN+1];
  
  FILE fp = fopen(BBSHOME"/boards/CLAMP/visable");
  fdw=open(BBSHOME"/boards/CLAMP/vis",O_WRONLY | O_CREAT | O_TRUNC, 0644);

  while(fget(buf, IDLEN, fp))
  {     
  	i++;
	if(strlen(buf) == 0) continue;
	printf("
=====================================================
%-5d - %s\n"
,i,buf);
        memcpy(vis.brdname,bh.brdname,IDLEN+1);
        write(fdw,&new,sizeof(vis));
   }
   close(fdr);
   close(fdw);
}     

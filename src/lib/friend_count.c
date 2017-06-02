#include "dao.h"
#include <fcntl.h>

int
friend_count(char *fname)
{
 FILE *fp;
 int count = 0;
 char buf[200];
 if(fp=fopen(fname,"r"))
  {
   while(fgets(buf,200,fp)) count++;
  }
 return count;
}

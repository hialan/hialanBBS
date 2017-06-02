#include <stdio.h>
#include <fcntl.h>

int
file_list_count(char *fname)
{
   FILE *fp;
   int count = 0;
   char buf[200];
   if(fp=fopen(fname,"r"))
   {
      while(fgets(buf,200,fp)) 
        count++;
      
      fclose(fp);
   }
   return count;
}

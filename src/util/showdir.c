/*
Show .DIR contents
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bbs.h"
#include "type.h"

#define field_count 3


#define WHITE   "#ffffff"
#define GRAY    "#c0c0c0"
#define BLACK   "#000000"

#define RED     "#a00000"
#define LRED    "#ff0000"

#define GREEN   "#009000"
#define LGREEN  "#00ff00"

#define BLUE    "#0000a0"
#define LBLUE   "#0000ff"

#define YELLOW  "#a0a000"
#define LYELLOW "#ffff00"

#define PURPLE  "#a000a0"
#define LPURPLE "#ff00ff"

#define CRAY    "#00a0a0"
#define LCRAY   "#00ffff"

char field_str[ field_count ][ 128 ];
int  field_lst_no [ field_count ];
int  post_num = 0; 
int  field_lst_size [ field_count ] = {
   6 ,13 ,50
};

char *field_name[] = {
    "時間",
    "作者",
    "標題",
    NULL
};

print_head()
{
    int i, size;

    printf("<center><TD bgcolor=%s>No.\n", LBLUE);
    for (i = 0; i < field_count; i++) {
        size  = field_lst_size[ i ];
        printf("<TD bgcolor=%s>%-*.*s ", BLUE, size, size, field_name[i] );
    }
    printf("<TR>\n");
}

print_record()
{
    int i, size;

    for (i = 0 ; i < field_count; i++) {
        size  = field_lst_size[ i ];
        if(i==0)
        printf("<TD bgcolor=%s align=center>%-d<td bgcolor=%s>"
        "<font size=4><A HREF=\"http:/~bbs/user/%s.html\">%-*.*s</A></font>\n",
        CRAY, post_num+1, GREEN,
        field_str[i], size, size, field_str[i]);

/*nekobe*/
       else if(i==1)
        printf("<TD><A HREF=mailto:%s.bbs@wd.pchome.com.tw>"
               "%s</A>",field_str[i-1],field_str[i]);

       else
        printf("<TD bgcolor=%s>%-*.*s ", BLACK, size, size,
        field_str[i][0] ? field_str[i] : "<pre>");

    }
    printf("<TR>\n");
}

main(argc, argv)
  int argc;
  char **argv;
{
   FILE* fp;
   char buf[200]; 

   if (argc < 2)
   {
     printf("Usage: %s <directory> \n", argv[0]);
     exit(1);
   }
   sprintf(buf,BBSHOME"/%s/.DIR",argv[1]); 
   print_head(); 
   if (fp = fopen(buf, "r")) {
      fileheader fhdr;
      int n = 0;
      char type;

      while (fread(&fhdr, sizeof(fhdr), 1, fp) == 1) {
         fhdr.title[50] = 0;
         type = "+ Mm"[fhdr.filemode];
         if (fhdr.filemode & FILE_TAGED)
            type = 'D';
         printf("%4d %c %13s %-5s %-12s %-41.40s<br>",
          ++n, type, fhdr.filename, fhdr.date, fhdr.owner, fhdr.title);
      }
      fclose(fp);
   }
   else
      fprintf(stderr, "`.DIR` opened error (for read)\n");
}

#include <stdio.h>
#include "bbs.h"
                                                                                
#define BACKUP	   BBSHOME"/BOARDS.backup"
#define DOTPASSWDS BBSHOME"/.BOARDS"
#define PASSWDSBAK BBSHOME"/BOARDS"
#define TMPFILE    BBSHOME"/tmpfile"
                                                                                
unsigned short rev(unsigned short i);

system("cd /home/nation/");
system("cp .BOARDS BOARDS.backup");                                                                                

boardheader board;
                                                                                
main(argc, argv)
int argc;
char **argv;
{
    FILE *foo1, *foo2;    int cnum, match=0;
                                                                                
    if( ((foo1=fopen(DOTPASSWDS, "r")) == NULL)
                || ((foo2=fopen(TMPFILE,"w"))== NULL) ){
        puts("file opening error");     exit(1);
    }
                                                                                
    while( (cnum=fread( &board, sizeof(board), 1, foo1))>0 ) 
    {
       fwrite( &board, sizeof(board), 1, foo2);
       match++;
    }
    fclose(foo1);    fclose(foo2);
                                                                                
    if(rename(DOTPASSWDS, PASSWDSBAK)==-1){
        puts("replace fails");          exit(1);
    }
                                                                                
    printf("fix board count:%d\n", match);
    unlink(DOTPASSWDS);
    rename(TMPFILE,DOTPASSWDS);
    unlink("tmpfile");
    exit(0);
}
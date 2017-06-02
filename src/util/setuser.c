#include <stdio.h>
#include "bbs.h"

#define DOTPASSWDS BBSHOME"/.PASSWDS"
#define PASSWDSBAK BBSHOME"/PASSWDS"
#define TMPFILE    BBSHOME"/tmp/tmpfile"

unsigned short rev(unsigned short i);

struct userec cuser;


main(argc, argv)
int argc;
char **argv;
{
    FILE *foo1, *foo2;
    int cnum,i,match;

    if( argc < 3 || strspn(argv[1], "01") != 32) {
        puts("Usage: setuser perm_32bit user1 [user2...]");
        exit(1);
    }
    if( ((foo1=fopen(DOTPASSWDS, "r")) == NULL)
                || ((foo2=fopen(TMPFILE,"w"))== NULL) ){
        puts("file opening error");
        exit(1);
    }

    while( (cnum=fread( &cuser, sizeof(cuser), 1, foo1))>0 ) {
       for (match =0, i = 2; i < argc; i++)
           if(!strcmp(cuser.userid, argv[i])) {
              match = 1;
              break;
           }
       if (match)
          cuser.userlevel = rev(strtoul(argv[1], 0, 2));
       fwrite( &cuser, sizeof(cuser), 1, foo2);
    }
    fclose(foo1);
    fclose(foo2);

    if(f_mv(DOTPASSWDS,PASSWDSBAK)==-1){
        puts("replace fails");
        exit(1);
    }
    unlink(DOTPASSWDS);
    f_mv(TMPFILE,DOTPASSWDS);
    unlink("tmpfile");

    return 0;
}



unsigned short rev(unsigned short i)
{
   unsigned j, k;

   for (k = i & 1, j = 1; j < 8 * sizeof(i); j++) {
      i >>= 1;
      k <<= 1;
      k += i & 1;
   }

   return k;
}


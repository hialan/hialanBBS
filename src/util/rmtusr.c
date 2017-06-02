/*
Show remote user & # logins in normal text format (remoteuser)
*/

#include <stdio.h>


main()
{
   FILE* fp;

   if (fp = fopen("remoteuser", "r")) {
      char buf[100];
      int n;

      while (fgets(buf, 100, fp)) {
         sscanf(buf + strlen(buf) + 2, "%d", &n);
         printf("%40s%10d\n", buf, n);
      }
      fclose(fp);
   }
   else
      fprintf(stderr, "`remoteuser` opened error (for read)\n");
}

/*---------------------------------------------------------
Remove(strip) left and right English and Chinese blanks in a line
----------------------------------------------------------*/
#include <string.h>

char* trim2(char* buffer)
{
   int j;

   for (j = 0; buffer[j] == ' ' || strncmp(buffer + j, "¡@", 2) == 0; j++)
      if(strncmp(buffer + j, "¡@", 2) == 0)
         j++;
   memmove(buffer, buffer + j, strlen(buffer + j) + 1);
   for (j = strlen(buffer);
        buffer[j - 1] == ' ' || strncmp(buffer + j - 1, "¡@", 2) == 0; j--)
      if(strncmp(buffer + j - 1, "¡@", 2) == 0)
         j--;
   buffer[j] = 0;
   return buffer;
}

#include "bbs.h"

main()
{
userec test;
boardheader test2;
user_info test3;

printf("the size of userec is : %d bytes \n",sizeof(test));
printf("the size of boardhead : %d bytes \n",sizeof(test2));
printf("the size of user_info : %d bytes \n",sizeof(test3));
printf("the size of int    is : %d bytes \n",sizeof(int));
printf("the size of llong  is : %d bytes \n",sizeof(long long));
printf("the size of float  is : %d bytes \n",sizeof(float));
printf("the size of double is : %d bytes \n",sizeof(double));
printf("the size of char   is : %d bytes \n",sizeof(char));
printf("the size of long   is : %d bytes \n",sizeof(long));
printf("the size of usint  is : %d bytes \n",sizeof(usint));
printf("the size of ushort is : %d bytes \n",sizeof(ushort));
printf("the size of uschar is : %d bytes \n",sizeof(uschar));
printf("the size of time_t is : %d bytes \n",sizeof(time_t));

printf("the size of boardh is : %d bytes \n",sizeof(test2));

printf("the size of unsigned long int is : %d bytes \n",sizeof(unsigned long int));
printf("%d\n",  sizeof(TAG_VALID));
}

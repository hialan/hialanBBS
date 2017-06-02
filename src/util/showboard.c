/*-------------------------------------------------------*/
/* util/showboard.c	( NTHU CS MapleBBS Ver 2.36 )	 */
/*-------------------------------------------------------*/
/* target : 看板一覽表(sorted)				 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/
/* Usage:	showboard .BOARDS			 */
/*-------------------------------------------------------*/


#include "bbs.h"

boardheader allbrd[MAXBOARD];

int
board_cmp(a, b)
  boardheader *a, *b;
{
  return (strcasecmp(a->brdname, b->brdname));
}


main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fd;
  int inf, i, count;

  if (argc < 2)
  {
    printf("Usage:\t%s .BOARDS [MAXUSERS]\n", argv[0]);
    exit(1);
  }


  inf = open(argv[1], O_RDONLY);
  if (inf == -1)
  {
    printf("error open file\n");
    exit(1);
  }

  /* read in all boards */

  i = 0;
  memset(allbrd, 0, MAXBOARD * sizeof(boardheader));
  while (read(inf, &allbrd[i], sizeof(boardheader)) == sizeof(boardheader))
  {
    if (allbrd[i].brdname[0])
    {
      i++;
    }
  }
  close(inf);

  /* sort them by name */
  count = i;
  qsort(allbrd, count, sizeof(boardheader), board_cmp);

  /* write out the target file */

  printf(
    "看板名稱     板主                     類別   中文敘述\n"
    "----------------------------------------------------------------------\n");
  for (i = 0; i < count; i++)
  {
    printf("%-13s%-25.25s%s\n", allbrd[i].brdname, allbrd[i].BM, allbrd[i].title);
  }
  exit(0);
}

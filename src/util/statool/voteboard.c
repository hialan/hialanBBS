/*-------------------------------------------------------*/
/* util/voteboard.c     ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : ¯¸¤º§ë²¼¬ÝªO                                 */
/* create : 97/09/05                                     */
/* update : 97/09/05                                     */
/*-------------------------------------------------------*/

/* by wsyfish */

#include "bbs.h"

#define DOTBOARDS BBSHOME"/.BOARDS"

boardheader allbrd[MAXBOARD];
static char STR_bv_topic[] = "topic";
char *str_board_file = BBSHOME"/boards/%s/%s";

struct vote_board
{
  char brdname[IDLEN + 1];
  char topic[50];
};
typedef struct vote_board vb;

void
setbfile(buf, boardname, fname)
  char *buf, *boardname, *fname;
{
  sprintf(buf, str_board_file, boardname, fname);
}

main()
{
  FILE *fp, *ftopic;
  int i;
/*  int inf;*/
  int count;
  vb voteboard[MAXBOARD];  

/*  inf = open(DOTBOARDS, O_RDONLY);
  if (inf == -1)
  {
    printf("error open file\n");
    exit(1);
  }*/

  if ((fp = fopen(DOTBOARDS, "r")) == NULL)
  {
    printf("cann't read");
    return 1;
  }

  i = 0;
  count = 0;
  memset(allbrd, 0, MAXBOARD * sizeof(boardheader));
  memset(voteboard, 0, MAXBOARD * sizeof(vb));

  while (fread(&allbrd[i], sizeof(boardheader), 1, fp))
  {
    if (allbrd[i].brdname[0] && allbrd[i].bvote == 1)
    {
      strcpy(voteboard[i].brdname, allbrd[i].brdname);
      i++;
      count++;
    }
  }
  fclose(fp);

  if ((fp = fopen("etc/votetopic", "w")) != NULL)
  {
    char buf[STRLEN];

    for(i = 0; i < count; i++){
       setbfile(buf, voteboard[i].brdname, STR_bv_topic);
       if((ftopic = fopen(buf, "r")) != NULL){
/*         fscanf(ftopic, "%s", &voteboard[i].topic);*/
         fgets(voteboard[i].topic, 50, ftopic);
         fclose(ftopic);
       }      
       fwrite(&voteboard[i], sizeof(vb), 1, fp);
    }
    fclose(fp);
  }

  if ((fp = fopen(BBSHOME"/etc/voteboard", "w")) == NULL)
  {
    printf("cann't open etc/voteboard\n");
    return 1;
  }
  fprintf(fp, " [41;37;1m¢r¢s¢r¢s¢r¢s[46mùÝùùùùùùùùùùùùùùùùùùùßùÝùùùùùùùùùùùùùùùùùùùùùùùùùùùùùß[41;1m¢s¢r¢s¢r¢s¢r[0m\n");
  fprintf(fp, " [41;37;1m¢s¢r¢s¢r¢s¢r[46mùø[33;1m¡¸ [32m ¿ïÁ|¤½§iÄæ  [33;1m¡¸[m[1;46;37mùøùø[32m½Ð¦Ü¤U¦C§ë²¼©Ò§ë²¼          [1;46;37mùø[0m[41;1m¢r¢s¢r¢s¢r¢s[0m\n");
  fprintf(fp, " [41;37;1m¢r¢s¢r¢s¢r¢s[46mùò¢w¢w¢w¢w¢w¢w¢w¢w¢wùôùø                            [1;46;37mùø[0m[41;1m¢s¢r¢s¢r¢s¢r[0m\n");
  fprintf(fp, " [41;37;1m¢s¢r¢s¢r¢s¢r[46mùø  ¢b¢c¢i¢i¢i¢i¢d  ùøùø [33m%-12.12s  %-12.12s [1;46;37mùø[0m[41;1m¢r¢s¢r¢s¢r¢s[0m\n", voteboard[0].brdname, voteboard[1].brdname);
  fprintf(fp, " [41;37;1m¢r¢s¢r¢s¢r¢s[46mùø      ¡´¡´  ÇqÇÛ  ùøùø [33m%-12.12s  %-12.12s [1;46;37mùø[0m[41;1m¢s¢r¢s¢r¢s¢r[0m\n", voteboard[2].brdname, voteboard[3].brdname);
  fprintf(fp, " [41;37;1m¢s¢r¢s¢r¢s¢r[46mùø       __  / ¢«   ùøùø [33m%-12.12s  %-12.12s [1;46;37mùø[0m[41;1m¢r¢s¢r¢s¢r¢s[0m\n", voteboard[4].brdname, voteboard[5].brdname);
  fprintf(fp, " [41;37;1m¢r¢s¢r¢s¢r¢s[46mùàùùùùùùùùùùùùùùùùùùùâùø [33m%-12.12s  %-12.12s [1;46;37mùø[0m[41;1m¢s¢r¢s¢r¢s¢r[0m\n", voteboard[6].brdname, voteboard[7].brdname);
  fprintf(fp, " [41;37;1m¢s¢r¢s¢r¢s¢r[46mùø[35;1m½Ð§ëµ¹¤@¸¹­Ô¿ï¤H  [1;46;37mùøùø                            [1;46;37mùø[0m[41;1m¢r¢s¢r¢s¢r¢s[0m\n");
  fprintf(fp, " [41;37;1m¢r¢s¢r¢s¢r¢s[46mùø  §ë¤U±z¯«¸tªº¤@²¼ùøùø            ªá¶é¥«¬F©²¤½§i  [1;46;37mùø[0m[1;41m¢s¢r¢s¢r¢s¢r[0m\n");
  fprintf(fp, " [41;37;1m¢s¢r¢s¢r¢s¢r[46mùãùùùùùùùùùùùùùùùùùùùåùãùùùùùùùùùùùùùùùùùùùùùùùùùùùùùå[41;1m¢r¢s¢r¢s¢r¢s[0m");

  fclose(fp);
}

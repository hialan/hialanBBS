/*-------------------------------------------------------*/
/* util/showb.c ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : 看版資料縱覽                                 */
/* create : 95/03/29                                     */
/* update : 96/06/17                                     */
/*-------------------------------------------------------*/
/*      $ showb   .BOARDS ...   : 看 all boards          */
/*      $ ln showuser showsysop : 看 sysop               */
/*      $ ln showuser showmgr   : 看 manager/sysop       */
/*-------------------------------------------------------*/


#include "bbs.h"

boardheader aman;
char *MYPASSFILE, field_str[80];
int field_count = 0;
int field_lst_no[16];
int field_lst_size[16];
int field_default_size[16] = {
  IDLEN + 2, BTLEN + 2, IDLEN * 3 + 3, 13, 25,
  5, 3, 25, 18, 0,
  0, 0, 0, 0, 0, 0
};

char field_idx[] = "ntmpuPvVl";
char *field_name[] = {
  "Name",
  "Title",
  "Manager",
  "Attribute",
  "NoteUpdateTime",
  "VF",
  "VoteCloseTime",
  "Level",
  "pad",
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};


void
set_opt(argc, argv)
  int argc;
  char *argv[];
{
  int i, flag, field, size, *p;
  char *ptr, *field_ptr;

  field_count = 0;

  for (i = 2; i < argc; i++)
  {
    field_ptr = (char *) strchr(field_idx, argv[i][0]);
    if (field_ptr == NULL)
      continue;
    else
      field = field_ptr - field_idx;

    size = atoi(argv[i] + 1);

    field_lst_no[field_count] = field;
    field_lst_size[field_count] = size ? size : field_default_size[field];
    field_count++;
  }
}


void
print_head()
{
  int i, field, size;

  for (i = 0; i < field_count; i++)
  {
    field = field_lst_no[i];
    size = field_lst_size[i];
    printf("%-*.*s ", size, size, field_name[field]);
  }
  printf("\n");
  for (i = 0; i < field_count; i++)
  {
    size = field_lst_size[i];
    while (size--)
      putchar('=');
    putchar(' ');
  }
  printf("\n");
}


char*
my_ctime(t)
  time_t *t;
{
  strcpy(field_str, (char *) ctime(t));
  field_str[strlen(field_str) - 1] = '\0';
}


void
print_record(serial_no, p)
  int serial_no;
  struct boardheader *p;
{
  int i, j, field, size, pat;

  for (i = 0; i < field_count; i++)
  {
    field = field_lst_no[i];
    size = field_lst_size[i];

    switch (field)
    {
    case 0:
      strcpy(field_str, p->brdname);
      break;

    case 1:
      strcpy(field_str, p->title);
      break;

    case 2:
      strcpy(field_str, p->BM);
      break;

    case 3:
      strcpy(field_str,p->brdattr);
      break;

    case 4:
      my_ctime(&p->bupdate);
      break;

    case 5:
      field_str[0] = " VR"[p->bvote];
      field_str[1] = 0;
      break;

    case 6:
      my_ctime(&p->vtime);
      break;

    case 7:
      pat = p->level;
      for (j = 0; j < 31; j++, pat >>= 1)
      {
        field_str[j] = (pat & 1) ? '1' : '0';
      }
      field_str[j] = '\0';
      break;

    case 8:
      strncpy(field_str, p->pad,10);
      field_str[10] = 0;
      break;
    }

    printf("%-*.*s ", size, size, field_str);
  }
  printf("\n");
}


main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *inf;
  int i, level;
  char *p;

  if (argc < 3)
  {
    printf("Usage: %s %s\n", argv[0], "board_file [XN] ....");
    printf("Example: %s %s\n", argv[0], "n10 t10 m20");
    printf("N is field width, X is one of the following char :\n");

    for (i = 0; field_name[i]; i++)
    {
      printf("\t%c[%2d] - %s\n",
        field_idx[i], field_default_size[i], field_name[i]);
    }
    exit(0);
  }
  else
  {
    set_opt(argc, argv);
    MYPASSFILE = argv[1];
  }

  if (strstr(argv[0], "showsysop"))
    level = PERM_SYSOP;
  else if (strstr(argv[0], "showmgr"))
    level = PERM_BM;
  else
    level = 0;

  inf = fopen(MYPASSFILE, "rb");
  if (inf == NULL)
  {
    printf("Error open %s\n", MYPASSFILE);
    exit(0);
  }

  print_head();
  i = 0;

  while (fread(&aman, sizeof(aman), 1, inf))
  {
    i++;
    if (aman.level >= level)
      print_record(i, &aman);
  }
  fclose(inf);
}

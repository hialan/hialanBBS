
/* ----------------------------------------------------- */
/* 檔案檢查函數：檔案、目錄、屬於			 */
/* ----------------------------------------------------- */


/* ----------------------------------------------------- */
/* subroutines for link list				 */
/* ----------------------------------------------------- */


LinkList *list_head;		/* head of link list */
static LinkList *list_tail;	/* tail of link list */


void
CreateLinkList()
{
  LinkList *list, *next;

  list = list_head;

  while (list)
  {
    next = list->next;
    free(list);
    list = next;
  }

  list_head = list_tail = NULL;
}


void
AddLinkList(name)
  char *name;
{
  LinkList *node;
  int len;

  len = strlen(name) + 1;
  node = (LinkList *) malloc(sizeof(LinkList) + len);
  node->next = NULL;
  memcpy(node->data, name, len);

  if (list_head)
    list_tail->next = node;
  else
    list_head = node;
  list_tail = node;
}


int
RemoveLinkList(name)
  char *name;
{
  LinkList *list, *prev, *next;

  prev = NULL;
  for (list = list_head; list; list = next)
  {
    next = list->next;
    if (!strcmp(list->data, name))
    {
      if (prev == NULL)
	list_head = next;
      else
	prev->next = next;

      if (list == list_tail)
	list_tail = prev;

      free(list);
      return 1;
    }
    prev = list;
  }
  return 0;
}


int
InLinkList(name)
  char *name;
{
  LinkList *list;

  for (list = list_head; list; list = list->next)
  {
    if (!strcmp(list->data, name))
      return 1;
  }
  return 0;
}


void
ShowLinkList(row, column, msg)
  int row, column;
  char *msg;
{
  LinkList *list;

  move(row, column);
  clrtobot();
  outs(msg);

  column = 80;
  for (list = list_head; list; list = list->next)
  {
    msg = list->data;
    row = strlen(msg) + 1;
    if (column + row > 78)
    {
      column = row;
      outc('\n');
    }
    else
    {
      column += row;
      outc(' ');
    }
    outs(msg);
  }
}

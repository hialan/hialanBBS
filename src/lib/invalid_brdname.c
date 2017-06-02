int
invalid_brdname (brd)           /* 定義錯誤看板名稱 */
     char *brd;
{
  register char ch;
                                                                                
  ch = *brd++;
  if (not_alnum (ch))
    return 1;
  while (ch = *brd++)
    {
      if (not_alnum (ch) && ch != '_' && ch != '-' && ch != '.')
        return 1;
    }
  return 0;
}

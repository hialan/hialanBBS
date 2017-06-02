/* 濾控制碼的程式 */
int
strip_ansi(buf, str ,mode)
  char *buf, *str;
  int mode;
{
  register int ansi,count=0;
  for (ansi = 0;*str /*&& *str != '\n' */; str++)
  {
    if (*str == 27)
    {
      if(mode)
        {
              if(buf) *buf++ = *str;
              count++;
        }
      ansi = 1;
    }
    else if (ansi && strchr("[;1234567890mfHABCDnsuJKc=n",*str))
    {
      if ((mode == 2 && !strchr("c=n",*str))
	|| (mode == 1 && strchr("[;1234567890m",*str)))
        {
         if(buf) *buf++ = *str;
         count++;
        }
      if(strchr("mHn ",*str))
	ansi = 0;
    }
    else
    {
      ansi =0;
      if(buf) *buf++ = *str;
      count++;
    }
  }
  if(buf) *buf = '\0';
  return count;
}

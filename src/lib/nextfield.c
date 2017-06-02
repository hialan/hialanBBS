char *
nextfield(data, field)
  register char *data, *field;
{
  register int ch;

  while (ch = *data)
  {
    data++;
    if ((ch == '\t') || (ch == '\r' && *data == '\n'))
      break;
    *field++ = ch;
  }
  *field = '\0';
  return data;
}

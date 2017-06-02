int
strstr_lower(str, tag)
  char *str, *tag;              /* tag : lower-case string */
{
  char buf[80];

  str_lower(buf, str);
  return (int) strstr(buf, tag);
}

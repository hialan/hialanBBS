char *
str_dup(src, pad)
  char *src;
  int pad;
{
  char *dst;

  dst = (char *) malloc(strlen(src) + pad);
  strcpy(dst, src);
  return dst;
}

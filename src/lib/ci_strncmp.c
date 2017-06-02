/* Case Independent strncmp */

int
ci_strncmp(s1, s2, n)
  register char *s1, *s2;
  register int n;
{
  register char c1, c2;
  register int diff;

  do
  {
    c1 = *s1++;
    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 32;

    c2 = *s2++;
    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 32;

    if (diff = c1 - c2)
      return (diff);
  } while (--n && c1);

  return 0;
}

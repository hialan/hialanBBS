#ifndef	PASSLEN
#define	PASSLEN 14
#endif


/* ----------------------------------------------------- */
/* password encryption					 */
/* ----------------------------------------------------- */


char *crypt();
static char pwbuf[PASSLEN];


char *
genpasswd(pw)
  char *pw;
{
  char saltc[2];
  int i, c;

  if (!*pw)
    return pw;

  i = 9 * getpid();
  saltc[0] = i & 077;
  saltc[1] = (i >> 6) & 077;

  for (i = 0; i < 2; i++)
  {
    c = saltc[i] + '.';
    if (c > '9')
      c += 7;
    if (c > 'Z')
      c += 6;
    saltc[i] = c;
  }
  strcpy(pwbuf, pw);
  return crypt(pwbuf, saltc);
}


int
checkpasswd(passwd, test)
  char *passwd, *test;
{
  char *pw;

  str_ncpy(pwbuf, test, PASSLEN);
  pw = crypt(pwbuf, passwd);
  return (strncmp(pw, passwd, PASSLEN));
}

/*-------------------------------------------------------*/
/* lib/acl_has.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : Access Control List				 */
/* create : 98/03/20					 */
/* update : 98/03/29					 */
/*-------------------------------------------------------*/


#include <stdio.h>


/* ----------------------------------------------------- */
/* ACL config file format				 */
/* ----------------------------------------------------- */
/* user:	majordomo@* bad@cs.nthu.edu.tw		 */
/* host:	cs.nthu.edu.tw	140.114.77.1		 */
/* subnet:	.nthu.edu.tw	140.114.77.		 */
/* ----------------------------------------------------- */


/* return -1 : ACL file 不存在 */
/* return 0 : ACL 不包含該 pattern */
/* return 1 : ACL 符合該 pattern */


int
acl_has(acl, user, host)
  char *acl;			/* file name of access control list */
  char *user;			/* lower-case string */
  char *host;			/* lower-case string */
{
  int i, cc, luser, lhost;
  FILE *fp;
  char filter[256], *addr, *str;

  if (!(fp = fopen(acl, "r")))
    return -1;

  i = 0;
  luser = strlen(user);		/* length of user name */
  lhost = strlen(host);		/* length of host name */

  str_lower(user, user);
  str_lower(host, host);

  while (fgets(filter, sizeof(filter), fp))
  {
    addr = NULL;

    for (str = filter; (cc = *str) > ' '; str++)
    {
      if (cc == '@')
	addr = str;
    }

    if (str == filter)		/* empty line */
      continue;

    str_lower(filter, filter);
    
    *str = '\0';

    if (addr)			/* match user name */
    {
      if ((luser != addr - filter) || memcmp(user, filter, luser))
	continue;

      if (!*++addr)
      {
	i = 1;
	break;
      }
    }
    else
    {
      addr = filter;
    }

    /* match host name */

    cc = str - addr;

    if (cc > lhost)
      continue;

    if (cc == lhost)
    {
      if (memcmp(addr, host, lhost))
	continue;
    }
    else
    {
      if (((*addr != '.') || memcmp(addr, host + lhost - cc, cc)) &&
	((addr[cc - 1] != '.') || memcmp(addr, host, cc)))
	continue;
    }

    i = 1;
    break;
  }

  fclose(fp);
  return i;
}

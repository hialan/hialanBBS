/* ----------------------------------------------------- */
/* hdr_stamp - create unique HDR based on timestamp	 */
/* ----------------------------------------------------- */
/* fpath - directory					 */
/* token - A / F / 0					 */
/* ----------------------------------------------------- */
/* return : open() fd (not close yet) or link() result	 */
/* ----------------------------------------------------- */


#include "dao.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>


int
hdr_stamp(folder, token, hdr, fpath)
  char *folder;
  int token;
  HDR *hdr;
  char *fpath;
{
  char *fname, *family;
  int rc;
  char *flink, buf[128];

  flink = NULL;
  if (token & HDR_LINK)
  {
    flink = fpath;
    fpath = buf;
  }

  fname = fpath;
  while (rc = *folder++)
  {
    *fname++ = rc;
    if (rc == '/')
      family = fname;
  }
  if (*family != '.')
  {
    fname = family;
    family -= 2;
  }
  else
  {
    fname = family + 1;
    *fname++ = '/';
  }

  if (token &= 0xdf)		/* ÅÜ¤j¼g */
  {
    *fname++ = token;
  }
  else
  {
    *fname = *family = '@';
    family = ++fname;
  }

  token = time(0);

  for (;;)
  {
    *family = radix32[token & 31];
    archiv32(token, fname);

    if (flink)
      rc = f_ln(flink, fpath);
    else
      rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);

    if (rc >= 0)
    {
      memset(hdr, 0, sizeof(HDR));
      hdr->chrono = token;
      str_stamp(hdr->date, &hdr->chrono);
      strcpy(hdr->xname, --fname);
      break;
    }

    if (errno != EEXIST)
      break;

    token++;
  }

  return rc;
}

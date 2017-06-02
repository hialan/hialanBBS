#include "dao.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>


int
rec_del(data, size, pos, fchk, fdel)
  char *data;
  int size;
  int pos;
  int (*fchk) ();
  int (*fdel) ();
{
  int fd;
  off_t off, len;
  char pool[REC_SIZ];
  struct stat st;

  if ((fd = open(data, O_RDWR)) < 0)
    return -1;

#ifdef	SOLARIS				/* lkchu: solaris 用 fcntl 取代 */
  fcntl(fd, F_WRLCK, 0);
#else
  flock(fd, LOCK_EX);
#endif

  fstat(fd, &st);
  len = st.st_size;

  data = pool;
  off = size * pos;

  /* 驗證 pos 位置資料的正確性 */

  if (len > off)
  {
    lseek(fd, off, SEEK_SET);
    read(fd, data, size);

    pos = fchk ? (*fchk) (data) : 1;
  }
  else
  {
    pos = 0;
  }

  /* 不對的話，重頭找起 */

  if (!pos)
  {
    off = 0;
    lseek(fd, off, SEEK_SET);
    while (read(fd, data, size) == size)
    {
      if (pos = (*fchk) (data))
	break;

      off += size;
    }
  }

  /* 找到之後，刪除資料 */

  if (pos)
  {
    if (fdel)
      pos = (*fdel) (data);

    if (pos)
    {
      /* really delete it */

      len -= (off + size);
      data = (char *) malloc(len);
      read(fd, data, len);
    }
    else
    {
      /* just update it */

      len = size;
    }

    lseek(fd, off, SEEK_SET);
    write(fd, data, len);

    if (pos)
    {
      ftruncate(fd, off + len);
      free(data);
    }
  }

#ifdef	SOLARIS				/* lkchu: solaris 用 fcntl 取代 */
  fcntl(fd, F_UNLCK, 0);
#else  
  flock(fd, LOCK_UN);
#endif
  close(fd);

  return 0;
}

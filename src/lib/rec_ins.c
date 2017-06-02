#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>


int
rec_ins(fpath, data, size, pos, num)
  char *fpath;
  void *data;
  int size;
  int pos;
  int num;
{
  int fd;
  off_t off, len;
  struct stat st;

  if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
    return -1;

#ifdef	SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_WRLCK, 0);
#else  
  flock(fd, LOCK_EX);
#endif

  fstat(fd, &st);
  len = st.st_size;

  off = size * pos;
  lseek(fd, off, SEEK_SET);

  size *= num;
  len -= off;
  if (len > 0)
  {
    fpath = (char *) malloc(pos = len + size);
    memcpy(fpath, data, size);
    read(fd, fpath + size, len);
    lseek(fd, off, SEEK_SET);
    data = fpath;
    size = pos;
  }

  write(fd, data, size);

#ifdef	SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_UNLCK, 0);
#else
  flock(fd, LOCK_UN);
#endif
  close(fd);

  if (len > 0)
    free(data);

  return 0;
}

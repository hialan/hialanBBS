#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>


int
rec_mov(data, size, from, to)
  char *data;
  int size;
  int from;
  int to;
{
  int fd, backward;
  off_t off, len;
  char *pool;
  struct stat st;

  if ((fd = open(data, O_RDWR)) < 0)
    return -1;

#ifdef  SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_WRLCK, 0);
#else
  flock(fd, LOCK_EX);
#endif

  fstat(fd, &st);
  len = st.st_size / size - 1;

  if (from > to)
  {
    backward = from;
    from = to;
    to = backward;
    backward = 1;
  }
  else
  {
    backward = 0;
  }

  if (to >= len)
    to = len;

  off = size * from;
  lseek(fd, off, SEEK_SET);

  len = (to - from + 1) * size;
  pool = data = (char *) malloc(len + size);

  if (backward)
    data += size;
  read(fd, data, len);

  data = pool + len;
  if (backward)
    memcpy(pool, data, size);
  else
    memcpy(data, pool, size);

  data = pool;
  if (!backward)
    data += size;

  lseek(fd, off, SEEK_SET);
  write(fd, data, len);

#ifdef  SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_UNLCK, 0);
#else
  flock(fd, LOCK_UN);
#endif
    
  close(fd);
  free(pool);

  return 0;
}

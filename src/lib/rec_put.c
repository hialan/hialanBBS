#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>


int
rec_put(fpath, data, size, pos)
  char *fpath;
  void *data;
  int size, pos;
{
  int fd;

  if ((fd = open(fpath, O_WRONLY | O_CREAT, 0600)) < 0)
    return -1;

#ifdef	SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_WRLCK, 0);
#else  
  flock(fd, LOCK_EX);
#endif

  lseek(fd, (off_t) (size * pos), SEEK_SET);
  write(fd, data, size);

#ifdef	SOLARIS             /* lkchu: 用 fcntl 取代 */
  fcntl(fd, F_UNLCK, 0);
#else
  flock(fd, LOCK_UN);
#endif

  close(fd);

  return 0;
}

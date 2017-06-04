#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

int
rec_add(fpath, data, size)
  char *fpath;
  void *data;
  int size;
{
  int fd;

  if ((fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600)) < 0)
    return -1;

// wildcat : 還是這樣比較保險
  flock(fd, LOCK_EX);
  lseek(fd, 0, SEEK_END);
  
  write(fd, data, size);

  flock(fd, LOCK_UN);
  close(fd);

  return 0;
}

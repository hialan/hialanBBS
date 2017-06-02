#include <fcntl.h>
#include <unistd.h>

int
rec_search(fpath, rptr, size, fptr, farg)
  char *fpath;
  char *rptr;
  int size;
  int (*fptr) ();
  int farg;
{
  int fd;
  int id = 1;

  if ((fd = open(fpath, O_RDONLY)) == -1)
    return 0;
  while (read(fd, rptr, size) == size)
  {
    if ((*fptr) (farg, rptr))
    {
      close(fd);
      return id;
    }
    id++;
  }
  close(fd);
  return 0;
}

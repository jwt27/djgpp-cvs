#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <process.h>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    /* Invoked as the 'parent'.
       Open files and pass their descriptors to the child.  */
    int fd;
    int fd_2;
    char buf[16];
    char buf_2[16];

    fd = open(argv[0], O_RDONLY);
    fd_2 = open(argv[0], O_RDONLY);
    itoa(fd, buf, 10);
    itoa(fd_2, buf_2, 10);

    /* Set the second handle to 'close on exec'.  */
    fcntl(fd_2, F_SETFD, FD_CLOEXEC);

    spawnl(P_WAIT, argv[0], argv[0], buf, buf_2, NULL);
    return 0;
  }
  else
  {
    /* Invoked as a child.
       Check if the descriptors passed by the parent exists.  */
    int fd, index;

    index = 1;
    while (index < argc)
    {
      fd = strtol(argv[index], NULL, 10);

      if (fcntl(fd, F_GETFD) == -1)
        printf("Handle %s was NOT inherited.\n", argv[index]);
      else
        printf("Handle %s was inherited.\n", argv[index]);
      ++index;
    }
    printf("\n");
  }

  return 0;
}


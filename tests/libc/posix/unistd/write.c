#include <unistd.h>
#include <fcntl.h>

char buf[] = "line1\nline2\ncr\rcr\n";

int
main(void)
{
  int i = open("write.out", O_WRONLY|O_CREAT|O_TRUNC);
  write(i, buf, strlen(buf));
  close(i);
  return 0;
}

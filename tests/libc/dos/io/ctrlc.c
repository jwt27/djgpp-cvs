#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <pc.h>
#include <dos.h>

int
main(void)
{
  int key;
  setmode(fileno(stdin), O_BINARY);
  while ((key=getkey()) != 'q')
    printf("key is %d\n", key);
  return 0;
}

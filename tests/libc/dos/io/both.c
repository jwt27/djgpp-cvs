#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <pc.h>
#include <dos.h>
#include <go32.h>

int
main(void)
{
  int key;
  _go32_want_ctrl_break(1);
  setmode(fileno(stdin), O_TEXT);
  setmode(fileno(stdin), O_BINARY);
  while ((key=getkey()) != 'q')
    printf("key is %d\n", key);
  return 0;
}

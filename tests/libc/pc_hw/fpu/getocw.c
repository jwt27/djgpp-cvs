#include <stdio.h>
#include <float.h>

int
main(void)
{
  unsigned int cw;

  cw = _control87(0,0);
  printf("cw = %x\n", cw);

  cw = _control87(0, 0x3f); /* enable all exceptions */
  printf("cw = %x\n", cw);

  cw = _control87(0,0);
  printf("cw = %x\n", cw);

#if 0
  cw = _control87(0xffff, 0x3f); /* disable all exceptions */
  printf("cw = %x\n", cw);
#endif
  _fpreset();

  cw = _control87(0,0);
  printf("cw = %x\n", cw);

  return 0;
}

/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dos.h>
#include <io.h>
#include <libc/atexit.h>

struct __atexit *__atexit_ptr = 0;

void (*__stdio_cleanup_hook)(void);

typedef void (*FUNC)(void);
extern FUNC djgpp_first_dtor[] __asm__("djgpp_first_dtor");
extern FUNC djgpp_last_dtor[] __asm__("djgpp_last_dtor");

void
exit(int status)
{
  int i;
  struct __atexit *a = __atexit_ptr;
  while (a)
  {
    (a->__function)();
    a = a->__next;
  }
  if (__stdio_cleanup_hook)
    __stdio_cleanup_hook();
  for (i=0; i<djgpp_last_dtor-djgpp_first_dtor; i++)
    djgpp_first_dtor[i]();

  /* in case the program set it this way */
  setmode(0, O_TEXT);
  _exit(status);
}

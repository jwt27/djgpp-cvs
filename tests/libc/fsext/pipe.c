#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/fsext.h>

#ifndef PIPE_MAX
#define PIPE_MAX 16384
#endif

static char pbuf[PIPE_MAX];
static int putp, getp, count;

static int
pipe_handler(__FSEXT_Fnumber func_number, int *rv, va_list args);

static int
pipe_open(int *rv, va_list args)
{
  char *fn = va_arg(args, char *);
  printf("pipe_open: checking `%s'\n", fn);
  if (strcmp(fn, "/dev/pipe"))
    return 0;
  *rv = __FSEXT_alloc_fd(pipe_handler);
  printf("pipe_open: it's me, at fd %d\n", *rv);
  putp = getp = count = 0;
  return 1;
}

static int
pipe_read(int *rv, va_list args)
{
  int fd = va_arg(args, int);
  char *buf = va_arg(args, char *);
  int len = va_arg(args, int);
  int r = 0;

  printf("pipe_read: reading %d bytes from %d: ", len, fd);
  while (count && len)
  {
    *buf++ = pbuf[getp++];
    printf(" %02x", buf[-1] & 0xff);
    if (getp > PIPE_MAX)
      getp = 0;
    count--;
    len--;
    r++;
  }
  printf(" returning %d\n", r);
  *rv = r;
  return 1;
}

static int
pipe_write(int *rv, va_list args)
{
  int fd = va_arg(args, int);
  char *buf = va_arg(args, char *);
  int len = va_arg(args, int);
  int r = 0;

  printf("pipe_write: writing %d bytes from %d: ", len,  fd);
  while (PIPE_MAX-count && len)
  {
    pbuf[putp++] = *buf++;
    printf(" %02x", buf[-1] & 0xff);
    if (putp > PIPE_MAX)
      putp = 0;
    count++;
    len--;
    r++;
  }
  printf(" returning %d\n", r);
  *rv = r;
  return 1;
}

/* ARGSUSED */
static int
pipe_close(int *rv, va_list args)
{
  *rv = 0;
  printf("pipe_close\n");
  return 1;
}

static int
pipe_handler(__FSEXT_Fnumber func_number, int *rv, va_list args)
{
  switch (func_number)
  {
  case __FSEXT_open:
  case __FSEXT_creat:
    return pipe_open(rv, args);
  case __FSEXT_read:
    return pipe_read(rv, args);
  case __FSEXT_write:
    return pipe_write(rv, args);
  case __FSEXT_close:
    return pipe_close(rv, args);
  default:
    return 0;
  }
}

int
main(void)
{
  FILE *fd;
  int c;

  __FSEXT_add_open_handler(pipe_handler);

  fd = fopen("/dev/pipe", "r+");
  if (fd == 0)
  {
    perror("/dev/pipe");
    return 0;
  }
  printf("fopen: fd=%d\n", fileno(fd));

  printf("begin write test\n");
  fprintf(fd, "hello ");
  fprintf(fd, "there\n");

  printf("begin read test\n");
  fflush(fd);

  while ((c = fgetc(fd)) != EOF)
  {
    putchar(c);
  }

  printf("begin close\n");
  fclose(fd);
  return 0;
}

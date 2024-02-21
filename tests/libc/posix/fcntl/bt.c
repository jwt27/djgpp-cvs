#include <stdio.h>
#include <fcntl.h>
#ifdef __GO32__
#include <unistd.h>
#endif
#include <io.h>

#define BS 10
#define TMP "bt.dat"

int
mk(const char *s, int m)
{
  int fd = open(TMP, O_WRONLY | O_CREAT | O_TRUNC | m, 0600);
  int i;
  char buf[4000];
  int b=0;
  for (i=0; s[i]; i++)
    switch (s[i])
    {
    case 'r':
      buf[b++] = '\r';
      break;
    case 'n':
      buf[b++] = '\n';
      break;
    case 'z':
      buf[b++] = 26;
      break;
    case '0':
      buf[b++] = 0;
      break;
    default:
      buf[b++] = s[i];
      break;
    }
  b = write(fd, buf, b);
  printf("%d bytes written\n", b);
  close(fd);
  return 0;
}

void
p1(int c)
{
  switch (c)
  {
  case 0:
    printf(" \\0");
    break;
  case '\r':
    printf(" \\r");
    break;
  case 26:
    printf(" \\Z");
    break;
  case '\n':
    printf(" \\n");
    break;
  default:
    if (c < ' ')
      printf(" %02x", c & 0xff);
    else
      printf(" %c", c);
    break;
  }
}

void
binary_test(void)
{
  char buf[20];
  int i, r;
  int fd = open(TMP, O_RDONLY|O_BINARY);
  r = read(fd, buf, 20);
  printf("binary[%03d]:", r);
  for (i=0; i<r; i++)
    p1(buf[i]);
  printf("\n");
  close(fd);
}

void
read_test(void)
{
  char buf[BS];
  int i, r;
  int fd = open(TMP, O_RDONLY|O_TEXT);
  r = read(fd, buf, BS);
  printf("  read[%03d]:", r);
  for (i=0; i<r; i++)
    p1(buf[i]);
  printf("\n");
  close(fd);
}

void
fread_test(void)
{
  char buf[BS];
  int i, r;
  FILE *f = fopen(TMP, "rt");
  r = fread(buf, 1, BS, f);
  printf(" fread[%03d]:", r);
  for (i=0; i<r; i++)
    p1(buf[i]);
  printf("\n");
  fclose(f);
}

void
fgetc_test(void)
{
  int i;
  FILE *f = fopen(TMP, "rt");
  printf("      fgetc:");
  while ((i=fgetc(f)) != EOF)
    p1(i);
  printf("\n");
  fclose(f);
}

int
main(int argc, char **argv)
{
  const char *ptr = "xxxrnrnnrxnxnrnxrnrrrnxnnxrnrnrnxrnrxnxnr";
  if (argc > 1)
    ptr = argv[1];

  printf("Request Size = %d\n", BS);

  printf("Binary write...\n");
  mk(ptr, O_BINARY);
  binary_test();
  read_test();
  fread_test();
  fgetc_test();

  printf("Text write...\n");
  mk(ptr, O_TEXT);
  binary_test();
  read_test();
  fread_test();
  fgetc_test();

  remove(TMP);
  return 0;
}

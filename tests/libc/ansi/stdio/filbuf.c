#include <stdio.h>

int
main(void)
{
  int last_count=-1000000, ch, so_far=0;
  FILE *f;
  char buf[65536];

  f = fopen("filbuf.exe", "rb");
  while ((ch = getc(f)) != EOF)
  {
    if (f->_cnt > last_count)
    {
      printf("Count: %5d  Fill: %5d  Bufsiz: %5d  SoFar: %5d\n",
	     f->_cnt, f->_fillsize, f->_bufsiz, so_far);
    }
    last_count = f->_cnt;
    so_far++;
  }
  fclose(f);

  f = fopen("filbuf.exe", "rb");
  ch = fread(buf, 1, 16384, f);
  printf("Wanted 16384, got %d\n", ch);
  getc(f);
  ch = fread(buf, 1, 16384, f);
  printf("Wanted 16384, got %d\n", ch);
  fclose(f);

  return 0;
}

#include <stdio.h>
#ifdef __DJGPP__
#include <libc/file.h>
#endif
#include <io.h>

const char *rfname = "file.c";
const char *wfname = "filetemp.dat";

int enable_read = 0;
int enable_write = 0;
int enable_text = 0;
int enable_binary = 0;

void
do_headers(void)
{
  printf(" ftell   tell  flags  cnt     ptr     base    bufsiz   remarks  \n");
  printf("------- ------ ----- ----- -------- -------- -------- --------- \n");
   
}

void
debug_file(FILE *f, const char *remarks)
{
  printf("%7ld %6d ", ftell(f), tell(fileno(f)));
#ifdef __DJGPP__
  putchar(ferror(f) ? 'E' : ' ');
  putchar(f->_flag & 001000  ? 'M' : ' ');
  putchar(f->_flag & 000020  ? 'W' : ' ');
  putchar(f->_flag & 000010  ? 'R' : ' ');
  printf("  %5d %7p %8p %8d", f->_cnt, f->_ptr, f->_base, f->_bufsiz);
#endif
  printf("  %s\n", remarks);
}

void
must(int a, int b)
{
  if (a != b)
    printf("*** warning - read %d, not %d\n", a, b);
}

void
do_mode(const char *rmode, const char *wmode, const char *description)
{
  char line[2048], c;
  FILE *f;
  int j;

  if (enable_read)
  {
    f = fopen(rfname, rmode);
    setvbuf(f, 0, _IOFBF , 512);

#ifdef __DJGPP__
    printf("\n---- %s File, Read, bufsiz=%d ----\n", description, f->_bufsiz);
    do_headers();
#endif

    debug_file(f, "After fopen");

    fgetc(f);
    debug_file(f, "After fgetc");

    fgets(line, 2048, f);
    debug_file(f, "After fgets");

    fseek(f, 5, SEEK_CUR);
    debug_file(f, "After fseek cur+5");

    c = fgetc(f);
    sprintf(line, "After fgetc -> %d", c);
    debug_file(f, line);

    fseek(f, 0x6b3, SEEK_SET);
    debug_file(f, "After fseek 0x6b3");

    fread(line, 1, 10, f);
    for (j=0; j<10; j++)
      printf(" %02x", line[j]);
    printf("\n");

#ifdef __DJGPP__
    fseek(f, 0, SEEK_SET);
    fgetc(f);
    while (f->_cnt)
      fgetc(f);
    debug_file(f, "after reading a full buffer");
#endif

    fseek(f, 0, SEEK_SET);
    must(fread(line, 1, 600, f), 600);
    must(fread(line, 1, 600, f), 600);
    debug_file(f, "after reading 1200 from beginning");
    fread(line, 10, 1, f);
    debug_file(f, "then read 10");
    for (j=0; j<10; j++)
      printf(" %02x", line[j]);
    printf("\n");

    fseek(f, 0, SEEK_SET);
    must(fread(line, 1, 600, f), 600);
    must(fread(line, 1, 600, f), 600);
    debug_file(f, "re-start");
    j = ftell(f);
    debug_file(f, "ftell");
    fseek(f, j, SEEK_SET);
    debug_file(f, "fseek there");
    fread(line, 1, 10, f);
    debug_file(f, "re-read");
    for (j=0; j<10; j++)
      printf(" %02x", line[j]);
    printf("\n");

    fclose(f);
  }

  if (enable_write)
  {
    f = fopen(wfname, wmode);
    setvbuf(f, 0, _IOFBF , 512);

#ifdef __DJGPP__
    printf("\n---- %s File, Write, bufsiz=%d ----\n", description, f->_bufsiz);
    do_headers();
#endif

    debug_file(f, "After fopen");

    fputc('x', f);
    debug_file(f, "After fputc x");

    fputs("hello", f);
    debug_file(f, "After fputs hello");

    fputc('\n', f);
    debug_file(f, "After fputc NL");

    fputs("jfkldsjakl;fdsaj;\nfdjksla;jfkldsja\nfjkdsl;ajfkdls;ajkfds\nfjdks;jafkld;sa\n", f);
    debug_file(f, "After fputs long");

    fflush(f);
    debug_file(f, "After fflush");

    fseek(f, 15, SEEK_SET);
    debug_file(f, "After fseek 15");

    fputc('x', f);
    debug_file(f, "After fputc x");

    fseek(f, 1, SEEK_CUR);
    debug_file(f, "After fseek cur+1");

    fflush(f);
    debug_file(f, "After fflush");

    fclose(f);
  }

}

int
main(int argc, char **argv)
{
  while (argc > 1 && argv[1][0] == '-')
  {
    switch (argv[1][1]) {
    case 'r':
      enable_read = 1;
      break;
    case 'w':
      enable_write = 1;
      break;
    case 'b':
      enable_binary = 1;
      break;
    case 't':
      enable_text = 1;
      break;
    }
    argc--;
    argv++;
  }
  if (argc > 1)
    rfname = argv[1];

  if (!enable_binary && !enable_text)
    enable_binary = enable_text = 1;
  if (!enable_read && !enable_write)
    enable_read = enable_write = 1;

  if (enable_binary)
    do_mode("rb", "wb", "Binary");
  if (enable_text)
    do_mode("rt", "wt", "Text");
  remove(wfname);
  return 0;
}

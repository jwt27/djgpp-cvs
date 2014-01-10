#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef __DJGPP__
#include <unistd.h>
#include <crt0.h>

int _crt0_startup_flags = _CRT0_FLAG_DROP_EXE_SUFFIX;
#endif

int errcode = 0;

void report_test_results (int as_expected)
{
  printf ("done\n====== the test %s.\n",
          as_expected ? "SUCCEEDED :-)" : "FAILED :-(");
}

void do_or_die (int failed, const char *msg)
{
  ++errcode;
  if (failed)
    {
      perror (msg);
      exit (errcode);
    }
}

/* Like fread(), but fills BUF with exactly ELSIZE*NELEMS bytes of text.  */
size_t text_read (void *buf, size_t elsize, size_t nelems, FILE *fp)
{
  char   *bp = buf;
  size_t to_read = elsize * nelems;
  int    this_read = 0;

  errno = 0;

  while (to_read && !feof (fp) && !ferror (fp) && this_read != -1)
    {
      this_read = fread (bp, 1, to_read, fp);
      if (this_read > -1)
        {
          bp += this_read;
          to_read -= this_read;
        }
    }
  return elsize * nelems - to_read;
}

void
diag(char *buf1, char *buf3, int buflen)
{
  int i;
  for (i=0; i<buflen; i++)
    printf(" %02x", buf1[i]);
  printf("\n");
  for (i=0; i<buflen; i++)
    printf(" %02x", buf3[i]);
  printf("\n");
}

int main (int argc, char *argv[])
{
  char *source = (char *)malloc (strlen (*argv) + 3);

  if (source)
    {
      FILE *fp;
      long flen;
      size_t buflen;
      char *buf1 = 0;
      char *buf2 = 0;
      char *buf3 = 0;
      long fpos1 = 0;
      int i;

      strcat (strcpy (source, *argv), ".c");

      printf ("**** Testing fseek/ftell in BINARY mode...\n");
      do_or_die ((fp = fopen (source, "rb")) == 0, source);
      flen = lseek (fileno (fp), 0, SEEK_END);
      buflen = flen / 100;      /* assume this source is a few KB-long */
      if (buflen < 20)
        buflen = 20;
      if (buflen > 60)
        buflen = 60;
      errno = ENOMEM;
      do_or_die ((buf1 = (char *)malloc (buflen)) == 0
                 || (buf2 = (char *)malloc (buflen)) == 0
                 || (buf3 = (char *)malloc (buflen)) == 0,
                 "allocate buffers");

      printf ("------ testing fseek() to beginning of file...");
      lseek (fileno (fp), 0, SEEK_SET);
      setvbuf (fp, (char *)0, _IOFBF, buflen * 3);
      do_or_die (fread (buf1, 1, buflen, fp) < buflen,
                 "fread buffer-1 at 0 offset");
      for (i = 0; i < 5; i++)
        {
          do_or_die (fread (buf3, 1, buflen, fp) < buflen,
                     "move 5 buffers' worth from the beginning");
          if (i == 3)
            do_or_die ((fpos1 = ftell (fp)) < 0, "get pos1");
        }
      do_or_die (fseek (fp, 0, SEEK_SET), "seek to beginning");
      do_or_die (fread (buf2, 1, buflen, fp) < buflen,
                 "fread buffer-2 at (presumably) 0 offset");
      report_test_results (memcmp (buf1, buf2, buflen) == 0);

      printf ("------ testing fseek() to end of file...");
      do_or_die (fseek (fp, 0, SEEK_END), "seek to EOF");
      report_test_results (fgetc (fp) == EOF);

      printf ("------ testing fseek() to a position returned by ftell()...");
      do_or_die (fseek (fp, fpos1, SEEK_SET), "seek into the file");
      do_or_die (fread (buf1, 1, buflen, fp) < buflen,
                 "fread buffer-1 at position reported by ftell");
      report_test_results (memcmp (buf1, buf3, buflen) == 0);

      printf ("------ testing fseek() to current position...");
      do_or_die (fseek (fp, 0, SEEK_SET), "seek to beginning");
      do_or_die (fread (buf1, 1, buflen, fp) < buflen,
                 "fread buffer-1 at (presumably) 0 offset");
      do_or_die (fread (buf2, 1, buflen, fp) < buflen,
                 "fread buffer-2 after buffer-1");
      do_or_die (fseek (fp, 0, SEEK_SET), "seek to beginning");
      do_or_die (fread (buf1, 1, buflen, fp) < buflen,
                 "fread buffer-1 at (presumably) 0 offset");
      do_or_die (fseek (fp, 0, SEEK_CUR), "seek to current pos");
      do_or_die (fread (buf3, 1, buflen, fp) < buflen,
                 "fread buffer-3 after buffer-1");
      report_test_results (memcmp (buf2, buf3, buflen) == 0);

      do_or_die (fclose (fp), source);


      printf ("\n**** Testing fseek/ftell in TEXT mode...\n");
      do_or_die ((fp = fopen (source, "rt")) == 0, source);
      setvbuf (fp, (char *)0, _IOFBF, buflen * 3);

      printf ("------ testing fseek() to beginning of file...");
      do_or_die (text_read (buf1, 1, buflen, fp) < buflen,
                 "text_read buffer-1 at 0 offset");
      for (i = 0; i < 5; i++)
        {
          do_or_die (text_read (buf3, 1, buflen, fp) < buflen,
                     "move 5 buffers' worth from the beginning");
          if (i == 3)
	  {
            do_or_die ((fpos1 = ftell (fp)) < 0, "get pos1");
	  }
        }
      do_or_die (fseek (fp, 0, SEEK_SET), "seek to beginning");
      do_or_die (text_read (buf2, 1, buflen, fp) < buflen,
                 "text_read buffer-2 at (presumably) 0 offset");
      report_test_results (memcmp (buf1, buf2, buflen) == 0);

      printf ("------ testing fseek() to end of file...");
      do_or_die (fseek (fp, 0, SEEK_END), "seek to EOF");
      report_test_results (fgetc (fp) == EOF);

      /* Do this twice: for file positions inside and outside of the
         current buffered portion of the file.  */
      printf ("------ testing fseek() to a place returned by ftell()=0x%lx [1]...", fpos1);
      do_or_die (fseek (fp, fpos1, SEEK_SET), "seek into the file");
      do_or_die (text_read (buf1, 1, buflen, fp) < buflen,
                 "text_read buffer-1 at position reported by ftell");
      report_test_results (memcmp (buf1, buf3, buflen) == 0);
      if (memcmp(buf1, buf3, buflen))
	diag(buf1, buf3, buflen);

      printf ("------ testing fseek() to a place returned by ftell()=0x%lx [2]...", fpos1);
      do_or_die (fseek (fp, 0, SEEK_SET), "seek to beginning");
      for (i = 0; i < 5; i++)
        do_or_die (text_read (buf1, 1, buflen, fp) < buflen,
                   "move 5 buffers' worth from the beginning");
      do_or_die (fseek (fp, fpos1, SEEK_SET), "seek into the file");
      do_or_die (text_read (buf1, 1, buflen, fp) < buflen,
                 "text_read buffer-1 at position reported by ftell");
      report_test_results (memcmp (buf1, buf3, buflen) == 0);
      if (memcmp(buf1, buf3, buflen))
	diag(buf1, buf3, buflen);

      printf ("------ testing fseek() to current position...");
      do_or_die (fseek (fp, 0, SEEK_SET), "seek to beginning");
      do_or_die (text_read (buf1, 1, buflen, fp) < buflen,
                 "text_read buffer-1 at (presumably) 0 offset");
      do_or_die (text_read (buf2, 1, buflen, fp) < buflen,
                 "text_read buffer-2 after buffer-1");
      do_or_die (fseek (fp, 0, SEEK_SET), "seek to beginning");
      do_or_die (text_read (buf1, 1, buflen, fp) < buflen,
                 "text_read buffer-1 at (presumably) 0 offset");
      do_or_die (fseek (fp, 0, SEEK_CUR), "seek to current pos");
      do_or_die (text_read (buf3, 1, buflen, fp) < buflen,
                 "text_read buffer-3 after buffer-1");
      report_test_results (memcmp (buf2, buf3, buflen) == 0);

      do_or_die (fclose (fp), source);
      return 0;
    }

  errno = ENOMEM;
  perror ("buffer for source filename");
  return ++errcode;
}

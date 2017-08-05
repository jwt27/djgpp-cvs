#include <stdio.h>
#include <string.h>

int convert_and_print (const char *fmt, const char *buf)
{
  size_t fmt_len;
  int cnv_spec;
  int cnv_qual;
  int converted;

  fputs ("Format: ", stdout);
  fputs (fmt ? (*fmt ? fmt : "(empty)") : "(null)", stdout);
  fputs ("\nString to convert: ", stdout);
  fputs (buf ? (*buf ? buf : "(empty)") : "(null)", stdout);
  puts ("");
  if (!fmt || !buf)
    return -1;

  fmt_len = strlen (fmt);
  cnv_spec = fmt_len ? fmt[fmt_len - 1] : '\0';
  cnv_qual = fmt_len > 1 ? fmt[fmt_len - 2] : '\0';
  if (cnv_qual == 'l' && fmt_len > 2 && fmt[fmt_len - 3] == 'l')
    cnv_qual = 'L';

  switch (cnv_spec)
  {
    case 'c':
    case 's':
    case ']':
    {
      char cbuf[200];  /* big enough? */

      memset (cbuf, 0, sizeof cbuf);
      converted = sscanf (buf, fmt, cbuf);
      printf ("`%s' converted %lu characters, result: %s\n",
      fmt, strlen (cbuf), cbuf);
      return converted;
    }
    case 'd':
    case 'i':
    case 'o':
    case 'u':
    case 'x':
      if (cnv_qual == 'h')
      {
        short hnum[5];

        memset (hnum, 0, sizeof hnum);
        converted = sscanf (buf, fmt, hnum);
        if (hnum[1] != 0 || hnum[2] != 0 || hnum[3] != 0 || hnum[4] != 0)
        {
          printf ("ERROR: more than a single short converted!\n");
          printf ("Result: %hd (0x%hx)\n", hnum[0], hnum[0]);
          converted = 0;
        }
        else
          printf ("Result: short %hd (0x%hx)\n", hnum[0], hnum[0]);

        return converted;
      }
      else if (cnv_qual == 'l')
        cnv_qual = '\0';  /* and fall through */
      else if (cnv_qual == 'L')
        cnv_qual = 'l';  /* and fall through */
      else
      {
        int inum[3];

        memset (inum, 0, sizeof inum);
        converted = sscanf (buf, fmt, inum);
        if (inum[1] != 0 || inum[2] != 0)
        {
          printf ("ERROR: more than a single int converted!\n");
          printf ("Result: %d (0x%x)\n", inum[0], inum[0]);
          converted = 0;
        }
        else
          printf ("Result: int %d (0x%x)\n", inum[0], inum[0]);

        return converted;
      }
      /* FALLTHROUGH */
    case 'D':
    case 'I':
    case 'O':
    case 'U':
    case 'X':
      if (cnv_qual == 'l' && cnv_spec != 'X')  /* %lX is long! */
      {
        long long llnum[2];

        memset (llnum, 0, sizeof llnum);
        converted = sscanf (buf, fmt, llnum);
        if (llnum[1] != 0)  /* aha! they converted more than 8 bytes! */
        {
          printf ("ERROR: more than a single long long converted!\n");
          printf ("Result: %lld (0x%llx)\n", llnum[0], llnum[0]);
          converted = 0;
        }
        else
          printf ("Result: long long %lld (0x%llx)\n", llnum[0], llnum[0]);

        return converted;
      }
      else
      {
        long lnum[3];

        memset (lnum, 0, sizeof lnum);
        converted = sscanf (buf, fmt, lnum);
        if (lnum[1] != 0 || lnum[2] != 0)
        {
          printf ("ERROR: more than a single long converted!\n");
          printf ("Result: long %ld (0x%lx)\n", lnum[0], lnum[0]);
          converted = 0;
        }
        else
          printf ("Result: long %ld (0x%lx)\n", lnum[0], lnum[0]);

        return converted;
      }
    case 'a': case 'A':
    case 'e': case 'E':
    case 'f': case 'F':
    case 'g': case 'G':
      if (cnv_qual == 'L')
      {
        long double ldnum[2];

        memset (ldnum, 0, sizeof ldnum);
        converted = sscanf (buf, fmt, ldnum);
        if (ldnum[1] != 0)  /* aha! they converted more than 10 bytes! */
        {
          printf ("ERROR: more than a single long double converted!\n");
          printf ("Result: %.21Lg\n", ldnum[0]);
          converted = 0;
        }
        else
        {
          if (cnv_spec == 'a' || cnv_spec == 'A')
            printf ("Result: long double hex %.15La  long double dec %.21Lg\n", ldnum[0], ldnum[0]);
          else
            printf ("Result: long double %.21Lg\n", ldnum[0]);
        }

        return converted;
      }
      else if (cnv_qual == 'l')
      {
        double dnum[2];

        memset (dnum, 0, sizeof dnum);
        converted = sscanf (buf, fmt, dnum);
        if (dnum[1] != 0)  /* aha! they converted more than 8 bytes! */
        {
          printf ("ERROR: more than a single double converted!\n");
          printf ("Result: %.17g\n", dnum[0]);
          converted = 0;
        }
        else
        {
          if (cnv_spec == 'a' || cnv_spec == 'A')
            printf ("Result: double hex %.13a  double dec %.17g\n", dnum[0], dnum[0]);
          else
            printf ("Result: long double %.17g\n", dnum[0]);
        }

        return converted;
      }
      else
      {
        float fnum[3];

        memset (fnum, 0, sizeof fnum);
        converted = sscanf (buf, fmt, fnum);
        if (fnum[1] != 0 || fnum[2] != 0)
        {
          printf ("ERROR: more than a single float converted!\n");
          printf ("Result: %.7g\n", fnum[0]);
          converted = 0;
        }
        else
        {
          if (cnv_spec == 'a' || cnv_spec == 'A')
            printf ("Result: float hex %.6a  float dec %.7g\n", fnum[0], fnum[0]);
          else
            printf ("Result: float %.7g\n", fnum[0]);
        }

        return converted;
      }
    default:
      printf ("I don't know what to do with format `%s'...\n", fmt);
      return -1;
  }

  return -1;
}

int main (void)
{
  char fmt[200], buf[200];
  int go_on;

  printf (\
"This is a test for *scanf functions.\n\n\
You enter a string or a number, and a format to read that number, and\n\
I will try to read the number using the format, and print the result.\n\
Make sure the last character of the format is the conversion specifier,\n\
and that the format is for converting a single number/string.\n\n\
If you want to test whether a certain conversion overwrites the stack,\n\
type an input that is longer than what the conversion can handle.\n\n\
To exit the program, press Ctrl-Z and hit [Enter]\n\n\
Let's play!\n");

  do {
    printf ("Type your input ==> "); fflush (stdout);
    go_on = fgets (buf, sizeof buf, stdin) != NULL;
    if (go_on)
    {
      printf ("Type the format for the above input ==> "); fflush (stdout);
      go_on = fgets (fmt, sizeof fmt, stdin) != NULL;
      if (go_on)
      {
        int len = strlen (fmt);

        if (fmt[len - 1] == '\n')
          fmt[len - 1] = '\0';
        len = strlen (buf);
        if (buf[len - 1] == '\n')
          buf[len - 1] = '\0';
        go_on = convert_and_print (fmt, buf) != -1;
      }
    }
  } while (go_on);

  printf ("\n\nThank you so much for playing with me!\n");

  return 0;
}

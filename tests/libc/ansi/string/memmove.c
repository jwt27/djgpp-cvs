/* Test program for memmove function.          -*- coding: raw-text -*-  */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* This might look funny on some terminals or with some editors.
   Whatever it looks like, DON'T EDIT IT!  */
static const unsigned char srcbuf[] =
"`1234567890-=\\qwertyuiop[]asdfghjkl;'zxcvbnm,./"
"€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ ¡¢£¤¥¦§¨©ª«¬­®¯"
"~!@#$%^&*()_+|QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?"
"°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞß"
"/.,mnbvcxz';lkjhgfdsa][poiuytrewq\\=-0987654321`"
"àáâãäåæçèéêëìíîïğñòóôõö÷øùúûüışŒŠˆ‡†…„ƒ‚€‘"
"?><MNBVCXZ\":LKJHGFDSA}{POIUYTREWQ|+_)(*&^%$#@!~";

static int
test (unsigned char *buf, int buflen, int offset)
{
  unsigned before = *(unsigned *)(buf + offset - 4);
  unsigned after  = *(unsigned *)(buf + buflen + offset);
  int failed = 0;

  memmove (buf + offset, buf, buflen);
  if (memcmp (buf + offset, srcbuf, buflen))
    {
      printf ("Test %d, %d failed: corrupted copy.\n", buflen, offset);
      failed++;
    }
  if (before != *(unsigned *)(buf + offset - 4))
    {
      printf ("Test %d, %d failed: corrupted BEFORE.\n", buflen, offset);
      failed++;
    }
  if (after  != *(unsigned *)(buf + buflen + offset))
    {
      printf ("Test %d, %d failed: corrupted AFTER.\n", buflen, offset);
      failed++;
    }
  return failed;
}

static int
empty (void *d, void *s, size_t len)
{
  return len;
}

int main (void)
{
  const int nmax = sizeof (srcbuf);
  unsigned char *testbuf = (unsigned char *) malloc (3 * nmax + 8);
  int off, len, result = 0;
  uclock_t t1, t2, t3, t4;

  if (!testbuf)
    {
      printf ("Get more memory and then come back again.\n");
      return 1;
    }

  memset (testbuf, 0, 3 * nmax + 8);
  memcpy (testbuf + nmax + 4, srcbuf, nmax);

  t1 = uclock ();
  for (off = -nmax; off <= nmax; off++)
    for (len = 1; len <= nmax; len++)
      {
	memmove (testbuf + nmax + 4 + off, testbuf + nmax + 4, len);
      }

  t2 = uclock ();

  t3 = uclock ();
  for (off = -nmax; off <= nmax; off++)
    for (len = 1; len <= nmax; len++)
      {
	empty (testbuf + nmax + 4 + off, testbuf + nmax + 4, len);
      }

  t4 = uclock ();

  printf ("Speed test took %Ld uclocks.\n", t2 - t1 - (t4 - t3));

  for (off = -nmax; off <= nmax; off++)
    for (len = 1; len <= nmax; len++)
      {
	memset (testbuf, 0, 3 * nmax + 8);
	memcpy (testbuf + nmax + 4, srcbuf, nmax);
	result += test (testbuf + nmax + 4, len, off);
      }

  if (result)
    printf ("%d tests failed.\n", result);
  else
    printf ("All tests passed.\n");

  return result;
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILL_CHAR 'X'

static void
check_too_small (const char *somestring,
		 char *buf2, const size_t buf2size,
		 const size_t fakesize)
{
  const size_t len_somestring = strlen(somestring);
  int i;

  assert(fakesize < buf2size);

  memset(buf2, FILL_CHAR, buf2size);
  strcpy(buf2, somestring);

  assert(strlcat(buf2, somestring, fakesize) >= fakesize);
  assert(strncmp(buf2, somestring, len_somestring) == 0);
  assert(strncmp(buf2 + len_somestring, somestring,
		 fakesize - len_somestring - 1) == 0);
  assert(buf2[fakesize - 1] == '\0');
  for (i = fakesize; i < buf2size; i++) {
    assert(buf2[i] == FILL_CHAR);
  }
}

int
main (void)
{
  const char somestring[] = "somestring";
  const size_t len_somestring = strlen(somestring);
  char buf[22];  /* More than big enough to contain two somestrings. */
  char buf2[20]; /* Just too small to contain two somestrings. */
  int i;

  /* Try a zero size. Check that the buffer is untouched. */
  memset(buf, FILL_CHAR, sizeof(buf));
  buf[0] = '\0'; /* Make buf a valid string. */

  assert(strlcat(buf, somestring, 0) == strlen(somestring));
  for (i = 1; i < sizeof(buf); i++) {
    assert(buf[i] == FILL_CHAR);
  }

  /* Check that two strings fit into buf. Check that it hasn't overrun. */
  memset(buf, FILL_CHAR, sizeof(buf));
  strcpy(buf, somestring);

  assert(strlcat(buf, somestring, sizeof(buf)) < sizeof(buf));
  assert(strncmp(buf, somestring, len_somestring) == 0);
  assert(strncmp(buf + len_somestring, somestring, len_somestring) == 0);
  assert(buf[sizeof(buf) - 1] == FILL_CHAR);

  /* Check that two strings just fail to fit into buf2. Check
   * its nul-termination. */
  memset(buf2, FILL_CHAR, sizeof(buf2));
  strcpy(buf2, somestring);

  assert(strlcat(buf2, somestring, sizeof(buf2)) >= sizeof(buf2));
  assert(strncmp(buf2, somestring, len_somestring) == 0);
  assert(strncmp(buf2 + len_somestring, somestring,
		 sizeof(buf2) - len_somestring - 1) == 0);
  assert(buf2[sizeof(buf2) - 1] == '\0');

  /* Copy somestring into a way-too-small buffer. Lie about the size
   * of the buffer and check for buffer overrun. */

  /* Case 1: None of the source string will fit. */
  check_too_small(somestring, buf2, sizeof(buf2), sizeof(somestring));

  /* Case 2: Some of the source string will fit. */
  check_too_small(somestring, buf2, sizeof(buf2), sizeof(somestring) + 1);
  check_too_small(somestring, buf2, sizeof(buf2), sizeof(somestring) + 2);
  check_too_small(somestring, buf2, sizeof(buf2), sizeof(somestring) + 4);

  /* Check that copying into a buffer that is not nul-terminated
   * returns size of destination buffer plus the length of string
   * we're concatenating. */
  memset(buf, FILL_CHAR, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  assert(   strlcat(buf, somestring, sizeof(buf) - 1)
	 == (sizeof(buf) - 1 + strlen(somestring)));

  puts("PASS");
  return(EXIT_SUCCESS);
}

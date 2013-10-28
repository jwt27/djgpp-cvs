#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILL_CHAR 'X'

int
main (void)
{
  const char somestring[] = "somestring";
  char buf[12];  /* More than big enough to contain somestring */
  char buf2[10]; /* Just too small to contain somestring. */
  int i;

  /* Try a zero size. Check that the buffer is untouched. */
  memset(buf, FILL_CHAR, sizeof(buf));

  assert(strlcpy(buf, somestring, 0) == strlen(somestring));
  for (i = 0; i < (int)sizeof(buf); i++) {
    assert(buf[i] == FILL_CHAR);
  }

  /* Copy somestring into a large-enough buffer. Check that it hasn't
   * overrun. */
  memset(buf, FILL_CHAR, sizeof(buf));

  assert(strlcpy(buf, somestring, sizeof(buf)) < sizeof(buf));
  assert(strcmp(buf, somestring) == 0);
  assert(buf[sizeof(buf) - 1] == FILL_CHAR);

  /* Copy somestring into a just-too-small buffer. Check
   * its nul-termination. */
  memset(buf2, FILL_CHAR, sizeof(buf2));

  assert(strlcpy(buf2, somestring, sizeof(buf2)) == sizeof(buf2));
  assert(strncmp(buf2, somestring, sizeof(buf2) - 1) == 0);
  assert(buf2[sizeof(buf2) - 1] == '\0');

  /* Copy somestring into a way-too-small buffer. Lie about the size
   * of the buffer and check for buffer overrun. */
#define MADE_UP_SIZE 3
  assert(MADE_UP_SIZE < sizeof(buf2));

  memset(buf2, FILL_CHAR, sizeof(buf2));

  assert(strlcpy(buf2, somestring, MADE_UP_SIZE) >= MADE_UP_SIZE);
  assert(strncmp(buf2, somestring, MADE_UP_SIZE - 1) == 0);
  assert(buf2[MADE_UP_SIZE - 1] == '\0');
  for (i = MADE_UP_SIZE; i < (int)sizeof(buf2); i++) {
    assert(buf2[i] == FILL_CHAR);
  }
#undef MADE_UP_SIZE

  puts("PASS");
  return(EXIT_SUCCESS);
}

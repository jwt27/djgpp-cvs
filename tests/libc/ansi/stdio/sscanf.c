#include <stdio.h>

int
main(void)
{
   char name[256];
   int ch_read, q;

   ch_read = -12345;

   printf("sscanf=%d\n", sscanf("abc123", "%255[a-zA-Z0-9]%n", name, &ch_read));
   printf("ch_read=%d\n",ch_read);
   printf("name=%s\n",name);

   if (ch_read != 6) {
      printf("Test failed!\n");
      return 1;
    }

   ch_read = -12345;

   printf("sscanf=%d\n", sscanf("abc123 ", "%255[a-zA-Z0-9]%n", name, &ch_read));
   printf("ch_read=%d\n",ch_read);
   printf("name=%s\n",name);

   if (ch_read != 6) {
      printf("Test failed!\n");
      return 1;
    }

   ch_read = -12345;

   printf("sscanf=%d\n", sscanf("abc123 ", "%c%n", name, &ch_read));
   printf("ch_read=%d\n",ch_read);
   printf("name=%s\n",name);

   if (ch_read != 1) {
      printf("Test failed!\n");
      return 1;
    }

   ch_read = -12345;

   printf("sscanf=%d\n", sscanf("abc123 ", "%s%n", name, &ch_read));
   printf("ch_read=%d\n",ch_read);
   printf("name=%s\n",name);

   if (ch_read != 6) {
      printf("Test failed!\n");
      return 1;
    }

   ch_read = -12345;

   printf("sscanf=%d\n", sscanf("123abc", "%d%s%n", &q, name, &ch_read));
   printf("ch_read=%d\n",ch_read);
   printf("name=%s\n",name);

   if (ch_read != 6) {
      printf("Test failed!\n");
      return 1;
    }

   return 0;
}


/*
   --------------------------------------------------------------------
   The following comment is no longer true, at least as of v2.02 (and I
   suspect in v2.01 as well).  I leave it here for historical purposes,
   but the test program worked for me without any glitches.

   In general, doscan.c is much more stable now than it was in v2.00
   and before.  So if anybody tells you there's a bug there, suspect a
   cockpit error first ;-).

                     Eli Zaretskii, 19-Apr-1999
   --------------------------------------------------------------------

---8<---

It outputs:
---8<---
sscanf=1
ch_read=-1058078715
name=abc123
---8<---

That is ch_read is uninitialized and sscanf succseeded! :-(

I think it is because a bug in doscan.c. It checks 'fileended' variable
and immediately exits. But in the format string next token is '%n' which does
not require more input.

This test works correctly on Solaris2.4.
---
Alexander Lukyanov
lav@video.yars.free.net

*/

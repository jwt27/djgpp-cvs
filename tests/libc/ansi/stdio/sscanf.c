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

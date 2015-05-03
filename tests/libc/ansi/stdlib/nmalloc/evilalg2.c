#include <stdio.h>
#include <stdlib.h>

int main(void)
{
   char    *p;
   unsigned int i;

   puts("Testing calloc(i, i).  Be patient");
   for (i = 65535; i > 0; i--) {
      if ((p = calloc(i,i))) {
         printf("\n%d appears to be the largest succesful integer\n",
                   i);
         break;
      }
      if (0 == (i % 500)) {
         if (0 == (i % 5000)) printf(" %8d\n", i);
         else printf("\r%8d", i);
         fflush(stdout);
      }
   }
   return 0;
}

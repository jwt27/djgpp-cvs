/* From djgpp mail list - an evil algorithm */
/* This shows up the difference between DJGPP2.03 malloc and
   nmalloc, both in speed and in efficiency of memory use.
   Snaffled/cleaned up for testing use by C.B. Falconer.

   To use the new malloc, simply link it with the object module.
*/
#include <stdio.h>
#include <stdlib.h>

typedef struct {
   char af[10];
   char name[10];
} record;

record **dt, *d;

int main(int argc, char ** argv)
{
   unsigned long n = 1000;  /* was 200000L */
   unsigned int  count;
   void         *v;

   if (argc > 1) n = strtoul(argv[1], NULL, 10);

   dt = NULL;
   for (count = 0; count < n; count++){
      if ((v = realloc(dt, (count + 1) * sizeof *dt))) dt = v;
      else break;
      if (!(d = dt[count] = calloc(10, sizeof *d))) break;
      if (!(count & 0xfff)) putc('*', stderr);
   }
   printf("\ncount=%d\n", count);
   return(0);
} /* evilalgo */

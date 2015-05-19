/* ------- tstmalloc -------- */

/* Copyright (c) 2003 by Charles B. Falconer
   Licensed under the terms of the GNU LIBRARY GENERAL PUBLIC
   LICENSE and/or the terms of COPYING.DJ, all available at
   <http://www.delorie.com>.

   Bug reports to <mailto:cbfalconer@worldnet.att.net>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "fakesbrk.h"
#include "cokusmt.h"
#include <libc/sysquery.h>
#include <unistd.h>     /* write */

#  define nmalloc   malloc
#  define nfree     free
#  define nrealloc  realloc
#  define nmemalign memalign
#  define INTERVAL  1000 /* for emitting free list dumps */

#include "nmalloc.h"


/* Magic 1500 below must be > MINSBRK in nmalloc.c */
enum {FAKESIZE = 1234567,
      HIFAKESZ = 1500,
      HIFAKE   = FAKESIZE - HIFAKESZ,
      NFLISTS  = (int)(CHAR_BIT * sizeof(size_t))
     };

char fakearea[FAKESIZE];

int notquiet;   /* 0 suppresses most output in this module */

/* 1------------------1 */

/* we can fool with this to generate test cases      */
/* If I could only figure a way to peg the addresses */
/* the test runs would be a good regression test.    */
void *fakesbrk(int delta)
{
static char *unused = fakearea;
static char *hiarea = fakearea + HIFAKE;
char        *next, *p;

   if (hiarea && (delta >= 32) && (delta < HIFAKESZ)) {
      /* return something above normal use to test
         action under decreasing sbrk values */
      p = hiarea;
      hiarea = 0;
      return (void *)p;
   }
   /* otherwise just act normally */
   next = unused + delta;
   if ((unsigned) next > HIFAKE) return (void *)-1;
   else {
      p = unused;
      unused = next;
      return (void *)p;
   }
} /* fakesbrk */

/* =====================================================
   This portion is an example of using the _sysmalloc()
   call to access the inner workings of the malloc pkg.
   Note the exact parallel between these routines and
   the corresponding ones in nmalloc.c.
   =====================================================
*/

/* 1------------------1 */

static struct _sysquery sysinfo;
void                 *(*freehdrsp)[NFLISTS];


#define NONE          sysinfo.nilp
#define lastsbrk      freehdrs[0]
#define memblockp     void*
typedef unsigned int  ulong;
typedef unsigned char byte;

/* conversion and access macros */
#define DATAOFFSET sysinfo.data

#define MEMBLKp(p) (memblockp)((byte*)(p) - DATAOFFSET)
#define PTR(m)     (void*)((byte*)(m) + DATAOFFSET)

/* field access macros (AFTER sysinfo loaded)    */
/* Examples - replace "m->prv" by "fld(m, prv)"  */
/*            replace "m->sz"  by "szof(m)"      */
/* where field is prvf, nxtf, prv, nxt           */
#define fld(m, field)   *((void**)((char*)m + sysinfo.field))
#define szof(m)         *(ulong*)((char*)m + sysinfo.sz)
#define freehdrs        (*freehdrsp)

/* 1------------------1 */

/* Fouls if sysinfo has not been initialized    */
/* See main for sysinfo initialization sequence */
void showsysquery(void)
{
   printf("sysinfo is: nil = %p\n"
          "    DATAOFFSET  = %d\n"
          "    gdlo offset = %d\n"
          "    sz   offset = %d\n"
          "    prvf offset = %d\n"
          "    nxtf offset = %d\n"
          "    nxt  offset = %d\n"
          "    prv  offset = %d\n"
          "    ohead       = %d\n"
          "    &freehdrs   = %p\n"
          "    &anchors    = %p\n"
          "    &hookset()  = %p\n",
          sysinfo.nilp, sysinfo.data, sysinfo.gdlo,
          sysinfo.sz,  sysinfo.prvf, sysinfo.nxtf,
          sysinfo.nxt, sysinfo.prv,  sysinfo.ohead,
          freehdrs, sysinfo.anchors, sysinfo.hookset);

} /* showsysquery */

/* 1------------------1 */

/* m is the allocated ptr treated by MEMBLKp    */
/* Fouls if sysinfo has not been initialized    */
/* See main for sysinfo initialization sequence */
static void xshowblock(void *m, const char *id)
{
   if (m) {
      printf(" %s %p", id, m);
      printf(" sz=%u nxt=%p prv=%p nxtf=",
             szof(m), fld(m, nxt), fld(m, prv));
      if (fld(m, nxtf)) {
         if (NONE == fld(m, nxtf))
            printf("NONE prvf=");
         else
            printf("%p prvf=", fld(m, nxtf));
         if (NONE == fld(m, prvf))
            printf("NONE");
         else
            printf("%p", fld(m, prvf));
      }
      else printf("0");
   }
   else
      printf(" %s NULL", id);
   fflush(stdout);  /* to coexist with internal debuggery */
} /* xshowblock */

/* 1------------------1 */

/* dump the entire free chain group             */
/* Fouls if sysinfo has not been initialized    */
/* See main for sysinfo initialization sequence */
static void xdumpfree(void)
{
   int       i;
   memblockp m;
   ulong     totfree;

   totfree = 0;
   for (i = 0; i < NFLISTS; i++) {
      if ((m = freehdrs[i])) {
         printf("\n%2d: ", i);
         do {
            printf("%p(%u)->", m, szof(m));
            totfree += szof(m);
            m = fld(m, nxtf);
         } while (m && (NONE != m));
         printf("0");
         m = freehdrs[i];
         while (m && (NONE !=m )) {
            xshowblock(m, "\n     ");
            m = fld(m, nxtf);
         }
      }
   }
   printf("\nTotal Free = %u\n", totfree);
   fflush(stdout);  /* to coexist with internal debuggery */
} /* xdumpfree */

/* ========== End of debuggery examples ============= */
/* ============== Start of tests ==================== */

/* Just allocate increasing sizes */
void test01(int n)
{
   int   i;
   void *m;

   for (i = 0; i < n; i++) m = nmalloc(65 * i);
   (void)m;
} /* test01 */

/* 1------------------1 */

/* allocate increasing sizes, freeing what was allocated
   10 items earlier.  Monitor free list every 10th.  Free
   everything at end.  Perturb sbrk return values.
*/
void test02(int n)
{
   int   i;
   void *m[10] = {0};

   for (i = 0; i < n; i++) {
      nfree(m[i % 10]);
      m[i % 10] = nmalloc(15 * i);
      if (0 == (i % 30)) (void)fakesbrk(3);
      if ( (INTERVAL - 1) == (i % INTERVAL )
          && notquiet)
         xdumpfree();
   }
   for (i = 0; i < 10; i++) nfree(m[i]);
   xdumpfree();
} /* test02 */

/* 1------------------1 */

enum {STDIN = 0, STDOUT, STDERR, STDAUX, STDPRN}; /* handles */

/* made to be compatible with nmalloc internal debugger */
void inject(int i)
{
   char buf[20];

#ifndef NDEBUG
   sprintf(buf, "%03d: ", i);
   write(STDOUT, buf, strlen(buf));
#endif
} /* inject */

/* 1------------------1 */

/* allocate random sizes, free, finally abort. Perturb sbrk */
void test03(int n)
{
   int   i;
   void *m[10] = {0};

   if (0 == n) n = 10;

   for (i = 0; i < n; i++) {
      if (m[i % 10]) inject(i);
      nfree(m[i % 10]);
      if (notquiet) inject(i);
      m[i % 10] = nmalloc(randomMT() % 10000);

      if (0 == i) xshowblock(MEMBLKp(m[0]), "\n*testing*");

      if (0 == (i % 30)) (void)fakesbrk(3);
      if ( (INTERVAL - 1) == (i % INTERVAL )
          && notquiet)
         xdumpfree();
   }
   for (i = 0; i < 10; i++) {
      if (m[i] && notquiet) inject(i);
      nfree(m[i]);
   }
   xdumpfree();
   if (n & 1) {
      printf("\nDeliberately refreeing pointer, should abort\n\n");
      fflush(stdout);
      nfree(m[0]);
   }
} /* test03 */

/* 1------------------1 */

/* allocate random sizes, realloc, free. Perturb sbrk */
void test04(int n)
{
   int   i;
   void *m[10] = {0};
   void *temp;

   for (i = 0; i < n; i++) {
      if (m[i % 10]) inject(i);
      nfree(m[i % 10]);
      if (notquiet) inject(i);
      m[i % 10] = nmalloc(randomMT() % 10000);
      if (notquiet) inject(i);
      temp = nrealloc(m[i % 10], randomMT() % 10000);
      if (temp) m[i % 10] = temp;
      if (0 == (i % 30)) (void)fakesbrk(3);
      if ( (INTERVAL - 1) == (i % INTERVAL)
           && notquiet)
         xdumpfree();
   }
   for (i = 0; i < 10; i++) {
      if (m[i] && notquiet) inject(i);
      nfree(m[i]);
   }
   xdumpfree();
} /* test04 */

/* 1------------------1 */

/* made to be compatible with nmalloc internal debugger */
void showijk(int i, int j, int k)
{
   char buf[20];

#ifndef NDEBUG
   sprintf(buf, "%03d:%d:%d ", i, j, k);
   write(STDOUT, buf, strlen(buf));
#endif
} /* showijk */

/* 1------------------1 */

/* free 10 items, allocate 10 random sizes, realloc 10 random  */
/* Much like test 4, but a different sequence. No sbrk perturb */
void test05(int n)
{
   int   i, j, k /*, ix */;
   void *m[10] = {0};
   void *temp;

   for (i = 0; i < n; i++) {
      j = (i / 10) % 3;   /* 0, 1, or 2 */
      k = (i % 10);
      /* ix = k + 10 * (i / 30); */
      if (0 == j) {
         if (notquiet) showijk(i, j, k);
         temp = nrealloc(m[k], randomMT() % 10000);
         if (temp) m[k] = temp;
         if ( (INTERVAL - 1) == (i % INTERVAL)
             && notquiet)
            xdumpfree();
      }
      if (1 == j) {
         if (m[k]) {
            if (notquiet) showijk(i, j, k);
            nfree(m[k]);
            m[k] = NULL;
         }
         if ( (INTERVAL - 1) == (i % INTERVAL)
              && notquiet)
            xdumpfree();
      }
      if (2 == j) {
         if (notquiet) showijk(i, j, k);
         m[k] = nmalloc(randomMT() % 10000);
         if ( (INTERVAL - 1) == (i % INTERVAL)
              && notquiet)
            xdumpfree();
      }
      if (0 == i) (void)fakesbrk(3); /* make 2 blocks */
   }
   for (i = 0; i < 10; i++) {
      if (m[i]) {
         if (notquiet) inject(i);
         nfree(m[i]);
      }
   }
   xdumpfree();
} /* test05 */

/* 1------------------1 */

/* reallocate random sizes continuously */
void test06(int n)
{
   int   i;
   void *m[10] = {0};
   void *temp;

   for (i = 0; i < n; i++) {
      if (notquiet) inject(i);
      temp = nrealloc(m[i % 10], randomMT() % 10000);
      if (temp) m[i % 10] = temp;
      if ( (INTERVAL - 1) == (i % INTERVAL)
           && notquiet)
         xdumpfree();
   }
   for (i = 0; i < 10; i++) {
      if (m[i] && notquiet) inject(i);
      nfree(m[i]);
   }
   xdumpfree();
} /* test06 */

/* 1------------------1 */

/* reallocate random sizes continuously, check data unharmed */
/* This writes over all allocated memory, so it is a fairly  */
/* good test that nothing is out of place.                   */
void test07(int n)
{
   int             i, j, k;
   void           *m[10] = {0};
   int             sz[10] = {0};
   int             newsz, minsz;
   unsigned char  *p;

   void *temp;

   for (i = 0; i < n; i++) {
      if (notquiet) inject(i);
      k = i % 10;
      minsz = newsz = randomMT() % 10000;
      if (sz[k] < minsz) minsz = sz[k];
      if (m[k]) {
         p = m[k];
         for (j = 0; j < sz[k]; j++) {
            /* null loop for initial access */
            p[j] = j & 0xff;
         }
      }
      temp = nrealloc(m[k], newsz);
      if (temp) {
         p = m[k] = temp;
         sz[k] = newsz;
         for (j = 0; j < minsz; j++) {
            /* null loop for initial access */
            if (p[j] != (j & 0xff)) {
               printf("Data failure at index %d!!\n", j);
               printf("is %d should be %d\n",
                       p[j], j & 0xff);
               fflush(stdout);
               exit(EXIT_FAILURE);
            }
         }
      }
      if ( (INTERVAL - 1) == (i % INTERVAL)
           && notquiet)
         xdumpfree();
   }
   for (i = 0; i < 10; i++) {
      if (m[i] && notquiet) inject(i);
      nfree(m[i]);
   }
   xdumpfree();
} /* test07 */

/* 1------------------1 */

/* From the C-FAQ, slightly modified
 * Most likely value is 0, + or - 5 are rare
 */
double gaussrand(void)
{
   static double V2, X;
   static int    phase = 0;
   double        Y, U1, U2, V1, S;

   if (phase) Y = V2 * X;
   else {
      do {
         U1 = (double)randomMT() / ranMTMAX;
         U2 = (double)randomMT() / ranMTMAX;

         V1 = 2 * U1 - 1;
         V2 = 2 * U2 - 1;
         S = V1 * V1 + V2 * V2;
      } while (S >= 1 || S == 0);

      Y = V1 * (X = sqrt(-2 * log(S) / S));
   }
   phase = 1 - phase;
   return Y;
} /* gaussrand */

/* 1------------------1 */

/* maps gaussrand -inf .. 0 into 0..1 and
 *                0 .. +inf into 1..inf.
 * Most likely value is slightly less than 1
 * 5.0 is fairly rare, 120.0 extremely rare
 */
double gausspos(void)
{
#define GAUSSLIMIT 10.0
   double r;

   if (GAUSSLIMIT < (r = gaussrand()))  /* limit */
      r = GAUSSLIMIT;
   return exp(r);
} /* gausspos */

/* 1------------------1 */

typedef struct node {
   struct node *next;
   char        *wastage;
} node, *nodeptr;

/* Under development for long term thrashing */
void test08(int n, int reps)
{
   int     i, j;
   nodeptr root, temp;
   size_t  sz, totalsz;

   printf("Under development\n"); fflush(stdout);
   j = 0;
   do {
      if (reps) {
         printf("Repetion %d\n", j + 1); fflush(stdout);
      }
      root = NULL; totalsz = 0;
      for (i = 0; i < n; i++) {
         /* form singly linked list of various sizes */
         if (!(temp = nmalloc(sizeof *temp))) {
            fprintf(stderr, "malloc node failed\n");
            exit(EXIT_FAILURE);
         }
         else {
            sz = (1 + gausspos()) * 32.0;
            if (!(temp->wastage = nmalloc(sz))) {
               fprintf(stderr, "malloc wastage failed\n");
               exit(EXIT_FAILURE);
            }
            else {
               temp->next = root;
               root = temp;
               totalsz += sz;
            }
         }
      } /* for, formed base list */

      xdumpfree();

      /* liberate it all */
      while (root) {
         nfree(root->wastage);
         temp = root->next;
         nfree(root);
         root = temp;
      } /* while */

      xdumpfree();
   } while (++j < reps);
} /* test08 */

/* 1------------------1 */

/* Under development for memalign exercise */
void test09(int n, int reps)
{
   int   i;
   void *p;

   printf("Under development\n"); fflush(stdout);
   if (!reps) reps = 512;
   for (i = 0; i < n; i++) {
     p = nmemalign(reps, 65 * i);
   }
   (void)p; /* Silence warning */
} /* test09 */

/* 1------------------1 */

int main(int argc, char *argv[])
{
   unsigned long t = 0, n = 0, reps = 0;

   if (argc > 1) t = strtoul(argv[1], NULL, 10);
   if (argc > 2) n = strtoul(argv[2], NULL, 10);
   if (argc < 4) notquiet = 1;
   else {
      reps = strtoul(argv[3], NULL, 10);
      notquiet = !(reps & 1);
   }

   if (0 == n) n = 10;

   printf("test%02lu-%lu (%lu)\n", t, n, reps);
   fflush(stdout);  /* Needed to coexist with debug pkg */

   sysinfo   = _sysmalloc();
   freehdrsp = (void*)((byte*)(sysinfo.nilp)-sizeof(void*));

   (void) fakesbrk(1);  /* start it off on odds */
   switch (t) {
case 1: test01(n); break;
case 2: test02(n); break;
case 3: test03(n); break;
case 4: test04(n); break;
case 5: test05(n); break;
case 6: test06(n); break;
case 7: test07(n); break;
case 8: test08(n, reps); break;
case 9: test09(n, reps); break;
default:
        printf("Usage: tnmalloc [testnumber [quantity [reps]]]\n");
        printf("fakearea at %p through %p (was f594)\n",
               &fakearea, &fakearea[FAKESIZE-1]);
        printf("CHAR_BIT * sizeof(size_t) = %lu\n",
                (unsigned long)(CHAR_BIT * sizeof(size_t)));
        showsysquery();
        printf(
           "Test Purpose\n"
           "  1  malloc only\n"
           "  2  malloc and free\n"
           "  3  malloc(random), free, aborts for odd quantity\n"
           "  4  malloc(random), realloc(random), and free\n"
           "  5  malloc(10 random), realloc(10 random) and free 10\n"
           "  6  realloc(random), monitor free lists\n"
           "  7  realloc(random), check data unharmed\n"
           "  8  run a long faked sequence, not complete\n"
           "  9  test memalign operation\n"
           "Any odd entry for reps suppresses free list dumps\n"
           );
        break;
   } /* switch (t) */
   return 0;
} /* main */

/* ------- tstmalloc -------- */

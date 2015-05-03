/* ------- tmalldbg -------- */

/* Copyright (c) 2003 by Charles B. Falconer
   Licensed under the terms of the GNU LIBRARY GENERAL PUBLIC
   LICENSE and/or the terms of COPYING.DJ, all available at
   <http://www.delorie.com>.

   Bug reports to <mailto:cbfalconer@worldnet.att.net>
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <libc/malldbg.h>
#include <libc/sysquery.h>

/*#include <string.h>
#include <math.h>
#include "cokusMT.h"
*/

/* Number of free lists in system */
#define NFLISTS ((int)(CHAR_BIT * sizeof(size_t)))

typedef struct testnode {
   struct testnode *next;
   char             string[30];
} testnode;

testnode *root;

int notquiet;

/* 1------------------1 */

/* Build something to display the structure of */
/* a linked list headed by the global var 'root' */
static testnode *buildlist(int items, testnode *root_)
{
   testnode *this;

   while (items) {
      this = malloc(items + sizeof *this);
      this->next = root_;
      root_ = this;
      sprintf(this->string, "item #%d", items);
      items--;
   }
   return root_;
} /* buildlist */

/* 1------------------1 */

/* retains 1 in three of original list,
   Ex: a -> b -> c -> d ::= a -> d, b & c freed
   This allows exercizing the free list compaction */
static void prunelist(testnode *root_)
{
   testnode *this, *keep;

   while (root_) {
      keep = root_;
      this = root_->next;
      if ((root_ = this)) {
         this = root_->next;
         free(root_);         
         if ((root_ = this)) {
            this = root_->next;
            free(root_);
            root_ = this;
         }
      }
      keep->next = root_;
   }
} /* prunelist */

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
   printf("sysinfo is: NONE = %p\n"
          "     DATAOFFSET  = %d\n"
          "     gdlo offset = %d\n"
          "     sz   offset = %d\n"
          "     prvf offset = %d\n"
          "     nxtf offset = %d\n"
          "     nxt  offset = %d\n"
          "     prv  offset = %d\n"
          "     ohead       = %d\n"
          "     &freehdrs   = %p\n"
          "     &anchors    = %p\n"
          "     &hookset()  = %p\n",
          sysinfo.nilp, sysinfo.data, sysinfo.gdlo,
          sysinfo.sz,  sysinfo.prvf, sysinfo.nxtf,
          sysinfo.nxt, sysinfo.prv,  sysinfo.ohead,
          freehdrs, sysinfo.anchors, sysinfo.hookset);


} /* showsysquery */

/* 1------------------1 */

/* m is the allocated ptr treated by MEMBLKp    */
/* Fouls if sysinfo has not been initialized    */
/* See main for sysinfo initialization sequence */
static void xshowblock(const void *m, const char *id)
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

/* ========== End of debuggery examples ============= */
/* ============== Start of tests ==================== */

struct blkspace {
   unsigned long totspace;
   unsigned long blkcount;
};

/* 1------------------1 */

struct blkspace freeblocks(void)
{
   struct blkspace blksp;
   int             i;
   memblockp       m;

   blksp.totspace = blksp.blkcount = 0;
   for (i = 0; i < NFLISTS; i++) {
      if ((m = freehdrs[i])) {
         while (m && (NONE !=m )) {
            blksp.totspace += szof(m);
            blksp.blkcount++;
            m = fld(m, nxtf);
         }
      }
   }
   return blksp;
} /* freeblocks */

/* 1------------------1 */

void printinfo(struct mallinfo *info)
{
   printf("   arena = %d\n", info->arena);
   printf(" ordblks = %d\n", info->ordblks);
   printf("  smblks = %d\n", info->smblks);
   printf("   hblks = %d\n", info->hblks);
   printf("  hblkhd = %d\n", info->hblkhd);
   printf(" usmblks = %d\n", info->usmblks);
   printf(" fsmblks = %d\n", info->fsmblks);
   printf("uordblks = %d\n", info->uordblks);
   printf("fordblks = %d\n", info->fordblks); /* free space */
   printf("keepcost = %d\n", info->keepcost);
} /* printinfo */

/* 1------------------1 */

void foul2ndlast(testnode *root_)
{
   testnode *this, *prev;
   void     *m;

   this = prev = NULL;
   while (root_) {
      prev = this; this = root_; root_ = root_->next;
   }
   /* Now prev=>this=>NULL */
   m = MEMBLKp(prev);

   xshowblock(m, "Fouling block: ");
   puts("");

   /* acts like a write past the previous block */
   (*(char*)m)--;

   xshowblock(m, " which became: ");
   puts("\n");

} /* foul2ndlast */

/* 1------------------1 */

void test01(unsigned long n)
{
   struct mallinfo info;
   struct blkspace blkspace;
   void  *m;

   root = buildlist(n, root);
   m = MEMBLKp(root);
   xshowblock(m, "\nLAST allocated:");
   puts("");

   blkspace = freeblocks();
   info = mallinfo();
   printinfo(&info);

   printf("\nnfreeblk = %lu\n", blkspace.blkcount);
   printf(" holding ( %lu )\n", blkspace.totspace);
} /* test01 */

/* 1------------------1 */

void test02(unsigned long n)
{
   root = buildlist(n, root);
   mallocmap();
} /* test02 */

/* 1------------------1 */

void test03(unsigned long n)
{
   struct mallinfo info;

   root = buildlist(n, root);
   info = mallinfo();
   puts("\nBefore pruning:");
   printinfo(&info);
   prunelist(root);
   info = mallinfo();
   puts("\nAfter pruning:");
   printinfo(&info);
   prunelist(root);
   info = mallinfo();
   puts("\nAfter repruning:");
   printinfo(&info);
   puts("\nComplete map:");
   mallocmap();
} /* test03 */

/* 1------------------1 */

void test04(unsigned long n)
{
   struct mallinfo info;
   testnode       *this;

   info = mallinfo();
   puts("\nAt startup:");
   printinfo(&info);
   root = buildlist(n, root);
   info = mallinfo();
   puts("\nBefore pruning:");
   printinfo(&info);
   prunelist(root);
   info = mallinfo();
   puts("\nAfter pruning:");
   printinfo(&info);
   prunelist(root);
   info = mallinfo();
   puts("\nAfter repruning:");
   printinfo(&info);
   free(NULL);         /* to check the alert */
   while ((this = root)) {
      root = root->next;
      free(this);
   }
   info = mallinfo();
   puts("\nAfter freeing all:");
   printinfo(&info);
   puts("\nComplete map:");
   mallocmap();
} /* test04 */

/* 1------------------1 */

void test06(unsigned long n)
{
   root = buildlist(n, root);
   printf("malloc_verify() returns %d\n", malloc_verify());
   mallocmap();
} /* test06 */

/* 1------------------1 */

void test07(unsigned long n)
{
   root = buildlist(n, root);
   foul2ndlast(root);
   printf("malloc_verify() returns %d\n", malloc_verify());
   mallocmap();
} /* test07 */

/* 1------------------1 */

void test08(unsigned long n)
{
   struct mallinfo info;
   void  *p, *p1;

   info = mallinfo();
   puts("\nAt startup:");
   printinfo(&info);
   root = buildlist(n, root);
   info = mallinfo();
   puts("\nBefore pruning:");
   printinfo(&info);
   prunelist(root);
   info = mallinfo();
   puts("\nAfter pruning:");
   printinfo(&info);
   prunelist(root);
   info = mallinfo();
   puts("\nAfter repruning:");
   printinfo(&info);
   puts("\nAfter attempting malloc/realloc(INT_MAX):");
   p = malloc(INT_MAX);
   p = malloc(2);
   if (p && (p1 = realloc(p, INT_MAX))) {
      p = p1; /* shouldn't happen */
      puts("\nSomething is wrong");
   }
   info = mallinfo();
   printinfo(&info);
   puts("\nComplete map:");
   mallocmap();
} /* test08 */

/* 1------------------1 */

int main(int argc, char *argv[])
{
   unsigned long t = 0, n = 0, dbglvl = 0;

   if (argc > 1) t = strtoul(argv[1], NULL, 10);
   if (argc > 2) n = strtoul(argv[2], NULL, 10);
   if (argc > 3) dbglvl = strtoul(argv[3], NULL, 10);

   if (0 == n) n = 10;

   printf("test%02lu-%lu (%lu)\n", t, n, dbglvl);

   sysinfo   = _sysmalloc();
   freehdrsp = (void*)((byte*)(sysinfo.nilp)-sizeof(void*));
   malloc_debug(dbglvl);
   if (t != 5) malldbgdumpfile(stdout);

   switch (t) {
case 1: test01(n); break;
case 2: test02(n); break;
case 3: test03(n); break;
case 4: test04(n); break;
case 5: test04(n); break; /* see malldbgdumfile call above */
case 6: test06(n); break;
case 7: test07(n); break;
case 8: test08(n); break;
default:
        printf("Usage: tmalldbg [testnumber [quantity [level]]]\n");
        printf("CHAR_BIT * sizeof(size_t) = %lu\n",
                (unsigned long)(CHAR_BIT * sizeof(size_t)));
        showsysquery();
        printf(
           "\n"
           "Test Purpose (usually to stdout)\n"
           "  1  Allocate items, execute/show mallinfo\n"
           "  2  Allocate items, execute mallocmap\n"
           "  3  Allocate/prune items, do mallocmap & info\n"
           "  4  As test 3, but all freed and free(NULL)\n"
           "  5  As test 4, but dumps to stderr\n"
           "  6  Allocate items, call malloc_verify / map\n"
           "  7  As 6, but deliberately foul 2nd last malloc\n"
           "  8  As 3, but attempt to malloc INT_MAX items\n"
           );
        break;
   } /* switch (t) */
   return 0;
} /* main */

/* ------- tmalldbg -------- */

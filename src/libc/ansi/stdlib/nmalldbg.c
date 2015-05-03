/* -------- malldbg.c --------- */

/* Copyright (c) 2003 by Charles B. Falconer
   Licensed under the terms of the GNU LIBRARY GENERAL PUBLIC
   LICENSE and/or the terms of COPYING.DJ, all available at
   <http://www.delorie.com>.

   Bug reports to <mailto:djgpp@delorie.com>

   Revised 2015 for use in DJGPP libs by Andris Pavenis <andris.pavenis@iki.fi>
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>   /* raise, SIGABRT */
#include <libc/malldbg.h>  /* and sysquery.h */

/* This is to be used in conjunction with a version of
   nmalloc.c compiled with:

     gcc -DNDEBUG -o malloc.o -c nmalloc.c
*/

static int   dbglevel;
static FILE *dumpfile;
static int   initialized;

/* array of hook function pointers, for cleaner interface */
/* This is purely a record of values set by sethook()     */
static M_HOOKFN hookptr[HKCOUNT];

/* Number of free lists in system */
#define NFLISTS ((int)(CHAR_BIT * sizeof(size_t)))

/* Loaded by initsysinfo() to access nmalloc guts */
static struct _sysquery sysinfo;

/* freehdrsp is pointer to array[NFLISTS] of void* */
/* These are the headers of the actual free lists  */
/* also loaded by initsysinfo() call               */
static void          *(*freehdrsp)[NFLISTS];

#define NONE          sysinfo.nilp
#define lastsbrk      freehdrs[0]
#define memblockp     void*
typedef unsigned int  ulong;
typedef unsigned char byte;

/* conversion and access macros */
#define DATAOFFSET sysinfo.data

#define MEMBLKp(p) (memblockp)((byte*)(p) - DATAOFFSET)
#define PTR(m)     (void*)((byte*)(m) + DATAOFFSET)

/* This accesses the list of discrete memory chains     */
/* which are created when sbrk returns unexpected value */
#define SBRKBGN    ((void **)(sysinfo.anchors))

/* field access macros (AFTER sysinfo loaded)    */
/* Examples - replace "m->prv" by "fld(m, prv)"  */
/*            replace "m->sz"  by "szof(m)"      */
/* where field is prvf, nxtf, prv, nxt           */
#define fld(m, field)   *((void**)((char*)m + sysinfo.field))
#define szof(m)         *(ulong*)((char*)m + sysinfo.sz)
#define freehdrs        (*freehdrsp)

/* ----------------- */

/* Set up the access to the nmalloc module */
static void initsysinfo(void)
{
   sysinfo   = _sysmalloc();
   freehdrsp = (void*)((byte*)(sysinfo.nilp)-sizeof(void*));
   if (!dumpfile) dumpfile = stderr;
   initialized = 1;
} /* initsysinfo */

/* 1------------------1 */

/* m is the allocated ptr treated by MEMBLKp    */
/* Fouls if sysinfo has not been initialized    */
/* Display info about a particular memory block */
static void xshowblock(FILE *f, void *m, const char *id)
{
   if (NULL == f) return;
   if (m) {
      fprintf(f, " %s %p", id, m);
      fprintf(f, " sz=%u nxt=%p prv=%p nxtf=",
             szof(m), fld(m, nxt), fld(m, prv));
      if (fld(m, nxtf)) {
         if (NONE == fld(m, nxtf))
            fprintf(f, "NONE prvf=");
         else
            fprintf(f, "%p prvf=", fld(m, nxtf));
         if (NONE == fld(m, prvf))
            fprintf(f, "NONE");
         else
            fprintf(f, "%p", fld(m, prvf));
      }
      else fprintf(f, "0");
   }
   else
      fprintf(f, " %s NULL", id);
   fflush(f);  /* to coexist with internal debuggery */
} /* xshowblock */

/* 1------------------1 */

/* dump the entire free chain group             */
/* Fouls if sysinfo has not been initialized    */
/* See main for sysinfo initialization sequence */
static void xdumpfree(FILE *f)
{
   int       i;
   memblockp m;
   ulong     totfree;

   if (NULL == f) return;
   totfree = 0;
   for (i = 0; i < NFLISTS; i++) {
      if ((m = freehdrs[i])) {
         fprintf(f, "\n%2d: ", i);
         do {
            fprintf(f, "%p(%u)->", m, szof(m));
            totfree += szof(m);
            m = fld(m, nxtf);
         } while (m && (NONE != m));
         fprintf(f, "0");
         m = freehdrs[i];
         while (m && (NONE !=m )) {
            xshowblock(f, m, "\n     ");
            m = fld(m, nxtf);
         }
      }
   }
   fprintf(f, "\nTotal Free = %u\n", totfree);
   fflush(f);  /* to coexist with internal debuggery */
} /* xdumpfree */

/* ----------------- */

/* show the content of a block, flag it as BAD */
static void showbad(FILE *f, void * m)
{
   void *n;

   if ((dbglevel >= 3) && (NULL == f)) f = dumpfile;
   if (f) {
      n = fld(m, nxt);
      xshowblock(f, m, "\n BAD?:");
      xshowblock(f, n, "\n BAD?:");
      putc('\n', f);
      fflush(f);
   }
} /* showbad */

/* ----------------- */

/* scans the complete malloc structures to collect
   info.  If f is non-NULL outputs a detailed listing
   returns NULL unless a bad block is found.
   Any bad blocks are displayed on dumpfile */
static void * mallocscan(FILE *f, struct mallinfo *mi)
{
   unsigned long totalmem, totalfree, blks, freeblks;
   void *m, *n, *badblk;
   int   i;

   badblk = NULL;
   if (!initialized) initsysinfo();
   mi->smblks = mi->hblks = mi->hblkhd = mi->usmblks = 0;
   mi->fsmblks = mi->keepcost = 0;

   /* this initialization accounts for the fact that
      the lastsbrk field will be counted as used */
   blks = 0; totalmem = 0;
   mi->hblkhd = totalfree = szof(freehdrs[0]);
   freeblks = 1;

   for (i = 0; (m = SBRKBGN[i]); i++) {
      if (f) fprintf(f, "\n\nGroup %d:", i);
      do {
         n = fld(m, nxt);
         if (f) xshowblock(f, m, "\n ");
         totalmem += szof(m);
         blks++;
         if (dbglevel && n)
            if (m != fld(n, prv)) {
               badblk = m;
               showbad(dumpfile, m);
               if (dbglevel >= 3) {
                  fflush(dumpfile);
                  raise(SIGABRT);
               }
            }
         if (fld(m, nxtf)) { /* a free block */
            freeblks++;
            totalfree += szof(m);
         }
      } while ((m = n));
   }
   if (f) fprintf(f, "\n");

   /* return the collected info in struct mi */
   mi->arena    = totalmem;
   mi->ordblks  = blks;
   mi->hblks    = freeblks;
   mi->uordblks = totalmem - totalfree
                       - DATAOFFSET * (blks - freeblks);
   mi->fordblks = totalfree;
   mi->keepcost = DATAOFFSET * blks;

   return badblk;
} /* mallocscan */

/* ----------------- */

/* Return summary details about the arena */
struct mallinfo mallinfo(void)
{
   struct mallinfo mi;
   int    valid;

   if (!initialized) initsysinfo();
   valid = (NULL == mallocscan(NULL, &mi));
   (void)valid;
   return mi;
} /* mallinfo */

/* ----------------- */

/* Verify the integrity of the arena */
int malloc_verify(void)
{
   struct mallinfo mi;
   void  *badblk;

   if (!initialized) initsysinfo();
   badblk = mallocscan(NULL, &mi);
   if (badblk) showbad(dumpfile, badblk);
   return (NULL == badblk);
} /* malloc_verify */

/* ----------------- */

/* dump a complete map of the arena */
void mallocmap(void)
{
   struct mallinfo mi;
   void  *badblk;

   if (!initialized) initsysinfo();
   fprintf(dumpfile, "\nmallocmap at level %d\n", dbglevel);
   xdumpfree(dumpfile);
   badblk = mallocscan(dumpfile, &mi);
   (void)badblk;
} /* mallocmap */

/* ----------------- */

/* Set the file on which to display results */
FILE *malldbgdumpfile(FILE *fp)
{
   FILE *tmp;

   if (!initialized) initsysinfo();
   tmp = dumpfile;
   if (fp) dumpfile = fp;
   else dumpfile = stderr;
   return tmp;
} /* malldbgdumpfile */

/* ----------------- */

/* The following three functions are called by hooks */

/* Do malloc_verify function via hook ptr */
/* No output unless a bad block found     */
/* This is suitable for setting hooks.    */
static void checkarena(size_t sz, void *bk)
{
   struct mallinfo mi;
   void  *badblk;

   if (bk) sz = sz; /* anti warn */
   if (dbglevel > 1) {  /* else ignore, safety */
      if ((badblk = mallocscan(NULL, &mi)))
         showbad(dumpfile, badblk);
   }
} /* checkarena */

/* ----------------- */

static void freenullalert(size_t sz, void *bk)
{
   if (bk) sz = sz; /* anti warn */
   fputs("\n***Freeing NULL\n", dumpfile);
} /* freenullalert */

/* ----------------- */

static void mallocfailalert(size_t sz, void *bk)
{
   if (bk)
      fprintf(dumpfile,
              "\n***realloc failed expanding %p to %lu bytes\n",
              bk, (unsigned long)sz);
   else
      fprintf(dumpfile,
              "\n***malloc failed allocating %lu bytes\n",
              (unsigned long)sz);
} /* mallocfailalert */

/* ----------------- */

/* Check that no hooks are presently in use */
/* uses the locally stored copy of hooks so */
/* mistakes are possible.  Maybe sysinfo    */
/* should contain a pointer to the real tbl */
/* Our own hooks are allowable              */
static int somehookinuse(void)
{
   enum m_hook_kind hk;

   for (hk = malloc_HK; hk < HKCOUNT; hk++) {
      /* structured for ease of modification */
      if (NULL            == hookptr[hk]) continue;
      if (checkarena      == hookptr[hk]) continue;
      if (freenullalert   == hookptr[hk]) continue;
      if (mallocfailalert == hookptr[hk]) continue;
      return 1;
   }
   return 0;
} /* somehookinuse */

/* ----------------- */

/* sethook, bypassing validity checks */
static M_HOOKFN sethook(enum m_hook_kind which,
                        M_HOOKFN newhook)
{
   M_HOOKFN tmp;

   hookptr[which] = newhook; /* keep local record */
   tmp = (*sysinfo.hookset)(which, newhook);
   return tmp;
} /* sethook */

/* ----------------- */

M_HOOKFN mallsethook(enum m_hook_kind which,
                     M_HOOKFN newhook)
{
   if (!initialized) initsysinfo();
   if (which >= HKCOUNT) return NULL; /* validity check */
   if (dbglevel != 1) return NULL;    /* in use, refuse */

   return sethook(which, newhook);
} /* mallsethook */

/* ----------------- */

static void releaseallhooks(void)
{
   enum m_hook_kind hk;

   for (hk = malloc_HK; hk < HKCOUNT; hk++)
      sethook(hk, NULL);
} /* freeallhooks */

/* ----------------- */

static inline void setfreenullhook(void)
{
   sethook(free_null_HK, freenullalert);
} /* setfreenullhook */

/* ----------------- */

static inline void setmallocfailhook(void)
{
   sethook(malloc_fail_HK, mallocfailalert);
} /* setmallocfailhook */

/* ----------------- */

static inline void setverifyhooks(void)
{
   sethook(malloc_HK,  checkarena);
   sethook(free_HK,    checkarena);
   sethook(realloc_HK, checkarena);
} /* setverifyhooks */

/* ----------------- */

/* level  action
      0   Only passive checks
      1   Passive checks, hook setting enabled
      2   Checks on each alloc/realloc, no hooks allowed
      3   Same, but aborts if fault found, signals malloc_fail
      4   Same, but signals on free(NULL)

   A level value outside 0..4 is rejected.
   Returns current debug_level (before any change).
*/
int malloc_debug(int level)
{
   int oldlevel;

   if (!initialized) initsysinfo();
   oldlevel = dbglevel;
   if ((level >= 0) && (level <= 4) && (level != oldlevel)) {
      if ((oldlevel < 2) && (level >= 2)) {
         if (somehookinuse()) { /* refuse */
            fprintf(dumpfile, "\n***malldbglvl refused\n");
            return oldlevel;
         }
      }
      /* Either all hooks free or our own, or level < 2 */
      /* The change is feasible, level is changed and valid */
      dbglevel = level;
      releaseallhooks();
      switch (level) { /* falling through */
case 4:  setfreenullhook();
case 3:  setmallocfailhook();
case 2:  setverifyhooks();
default: break;
      } /* switch (level) */
   } /* valid level change */
   return oldlevel;
} /* malloc_debug */

/* -------- malldbg.c ----------- */

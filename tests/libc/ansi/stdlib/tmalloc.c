/* Testbed for the malloc-debug facilities.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
print_malloc_info (void)
{
  struct mallinfo info = mallinfo ();

  printf (">>> mallinfo reports:\n"
	  "    Arena: %d\n"
	  "    Ordinary blocks: %d\n"
	  "    Small blocks: %d\n"
	  "    Space in small blocks in use: %d\n"
	  "    Space in free small blocks: %d\n"
	  "    Space in ordinary blocks in use: %d\n"
	  "    Space in free ordinary blocks: %d\n",
	  info.arena, info.ordblks, info.smblks,
	  info.usmblks, info.fsmblks, info.uordblks, info.fordblks);
}

#define MAXTBL 1024
struct alloc  {
  char *ptr;
  size_t size;
};

int main (int argc, char *argv[])
{
  struct alloc table[MAXTBL];
  size_t total = 0;
  volatile int i, j;
  char buf[80];
  int corrupted_blocks;

  printf ("\n *** Test of malloc debugging facilities ***\n\n");
  sprintf (buf, "MALLOC_DEBUG=%d", MAXTBL * 2);
  putenv (buf);
  print_malloc_info ();
  printf ("\n>>> mallocmap reports (expect to see only free blocks):\n");
  mallocmap ();

  /* First, allocate and then free some memory, to prime the malloc
     internal data structures.  */
  malloc_debug (1);
  for (i = 0; i < MAXTBL; i++)
    {
      size_t chunk_size = (rand() >> 18) + 10;

      table[i].ptr = malloc (chunk_size);
      table[i].size = chunk_size;
      if (table[i].ptr == NULL)
	{
	  printf ("!!!malloc failed after %d requests for chunk size %lu%s\n",
		  i, chunk_size < 1024 ? chunk_size : chunk_size / 1024,
		  chunk_size < 1024 ? "" : "KB");
	  break;
	}
      total += chunk_size;
    }

  printf ("\n=== Application requested %lu bytes in %d allocations\n",
	  total, i);
  print_malloc_info ();
  printf ("\n>>> mallocmap reports (expect to see only used blocks):\n");
  mallocmap ();
  printf ("\n=== Heap corruption test:\n");
  if (malloc_verify () == 0)
    printf ("!!! FAILED (??? better look for bugs...)\n");
  else
    printf (">>> PASSED (as expected)\n");
  for (j = 0; j < i; j++)
    free (table[j].ptr);
  printf ("\n=== Freed everything.\n");
  print_malloc_info ();

  /* Now allocate memory again, and fill each chunk with random
     characters, overrunning some of the buffers in the process.  */
  printf ("\n=== Corrupt memory test (expect early exit and corrupted block reports).\n");
  malloc_debug (1);
  for (i = 0, corrupted_blocks = 0; i < MAXTBL; i++)
    {
      size_t chunk_size = (rand() >> 18) + 10;
      int fill_value = (rand () & 0x7f) + ' ';
      size_t n_to_fill = chunk_size +
	/* about 1% of corrupted blocks */
	((rand () > RAND_MAX - RAND_MAX/100) ? 50 : 0);

      fflush (stdout);
      table[i].ptr = malloc (chunk_size);
      table[i].size = chunk_size;
      if (table[i].ptr == NULL)
	{
	  printf ("!!!malloc failed after %d requests for chunk size %lu%s\n",
		  i, chunk_size < 1024 ? chunk_size : chunk_size / 1024,
		  chunk_size < 1024 ? "" : "KB");
	  break;
	}
      if (n_to_fill > chunk_size)
	corrupted_blocks++;
      memset (table[i].ptr, fill_value, n_to_fill);
      if (malloc_verify () == 0)
	{
	  printf ("!!!Heap corruption detected after %d allocations\n"
		  "   and %d corrupted blocks; exiting.\n",
		  i, corrupted_blocks);
	  return 1;
	}
    }

  return 0;
}

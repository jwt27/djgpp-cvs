/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <unistd.h>
#include "t-main.h"


int main(void)
{
  int failed = 0, functions = 0;

  printf("Testing llrint...\n");
  failed += llrint_test(); functions++;
  printf("Testing llrintl...\n");
  failed += llrintl_test(); functions++;
  printf("Testing llround...\n");
  failed += llround_test(); functions++;
  printf("Testing llroundf...\n");
  failed += llroundf_test(); functions++;
  printf("Testing llroundl...\n");
  failed += llroundl_test(); functions++;
  printf("Testing lrint...\n");
  failed += lrint_test(); functions++;
  printf("Testing lrintf...\n");
  failed += lrintf_test(); functions++;
  printf("Testing lrintl...\n");
  failed += lrintl_test(); functions++;
  printf("Testing lround...\n");
  failed += lround_test(); functions++;
  printf("Testing lroundf...\n");
  failed += lroundf_test(); functions++;
  printf("Testing lroundl...\n");
  failed += lroundl_test(); functions++;
  printf("Testing round...\n");
  failed += round_test(); functions++;
  printf("Testing roundf...\n");
  failed += roundf_test(); functions++;
  printf("Testing roundl...\n");
  failed += roundl_test(); functions++;
  printf("Testing trunc...\n");
  failed += trunc_test(); functions++;
  printf("Testing truncf...\n");
  failed += truncf_test(); functions++;
  printf("Testing truncl...\n");
  failed += truncl_test(); functions++;

  printf("Tested %d functions, %d errors detected\n", functions, failed);
  if (!isatty(fileno(stdout)))
    fprintf(stderr, "Tested %d functions, %d errors detected\n", functions, failed);

  return failed;
}

/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */

#include <unistd.h>
#include "main-t.h"


int main(void)
{
  int failed = 0, number_of_functions = 0;

  printf("Testing llrint...\n");
  failed += llrint_test(); number_of_functions++;
  printf("Testing llrintl...\n");
  failed += llrintl_test(); number_of_functions++;
  printf("Testing llround...\n");
  failed += llround_test(); number_of_functions++;
  printf("Testing llroundf...\n");
  failed += llroundf_test(); number_of_functions++;
  printf("Testing llroundl...\n");
  failed += llroundl_test(); number_of_functions++;
  printf("Testing lrint...\n");
  failed += lrint_test(); number_of_functions++;
  printf("Testing lrintf...\n");
  failed += lrintf_test(); number_of_functions++;
  printf("Testing lrintl...\n");
  failed += lrintl_test(); number_of_functions++;
  printf("Testing lround...\n");
  failed += lround_test(); number_of_functions++;
  printf("Testing lroundf...\n");
  failed += lroundf_test(); number_of_functions++;
  printf("Testing lroundl...\n");
  failed += lroundl_test(); number_of_functions++;
  printf("Testing round...\n");
  failed += round_test(); number_of_functions++;
  printf("Testing roundf...\n");
  failed += roundf_test(); number_of_functions++;
  printf("Testing roundl...\n");
  failed += roundl_test(); number_of_functions++;
  printf("Testing trunc...\n");
  failed += trunc_test(); number_of_functions++;
  printf("Testing truncf...\n");
  failed += truncf_test(); number_of_functions++;
  printf("Testing truncl...\n");
  failed += truncl_test(); number_of_functions++;

  printf("Tested %d functions, %d errors detected\n", number_of_functions, failed);
  if (!isatty(fileno(stdout)))
    fprintf(stderr, "Tested %d functions, %d errors detected\n", number_of_functions, failed);

  return failed;
}

#include <stdio.h>
#include <string.h>

extern unsigned char __dj_collate_table[256];

int main(int ac, char *av[])
{
  int c;

  __dj_collate_table['a'] = 'A';
  __dj_collate_table['A'] = 'a';

  c = strcoll("a", "A");
  printf("strcoll: %s [%s] %s\n", "a", c ? (c > 0) ? ">" : "<" : "==", "A");
  c = strcmp("a", "A");
  printf("strcmp: %s [%s] %s\n", "a", c ? (c > 0) ? ">" : "<" : "==", "A");
  return 0;
}

#define __dj_ENFORCE_FUNCTION_CALLS
#include <stdio.h>
#include <netinet/in.h>

int
main(void)
{
  int x = 0x12345678;

  printf("x = %x\n", x);

  printf("htonl = %lx\n", htonl(x));
  printf("htons = %x\n", htons(x));
  printf("ntohl = %lx\n", ntohl(x));
  printf("ntohs = %x\n", ntohs(x));

  return 0;
}

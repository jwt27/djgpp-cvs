#include <sys/dxe.h>

static int (*add)(int a, int b);

int main(void)
{
  printf("loading add.dxe...\n");
  add = _dxe_load("add.dxe");
  if (add == 0) {
    printf("Cannot load add.dxe\n");
    return 1;
  }
  printf("Okay, 3 + 4 = %d\n", add(3,4));
  return 0;
}

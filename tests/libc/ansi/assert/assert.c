#include <assert.h>
#include <assert.h>
#include <assert.h>
#include <assert.h>

void
debug(void)
{
  assert(1==1);
  assert(1==0);
}

#define NDEBUG 1
#include <assert.h>
#include <assert.h>
#include <assert.h>
#include <assert.h>

int
main(void)
{
  assert(1==1);
  assert(1==0);
  debug();
  return 0;
}

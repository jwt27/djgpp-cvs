#include <io.h>
#include <unistd.h>

int
chsize(int handle, long size)
{
  return ftruncate(handle, size);
}

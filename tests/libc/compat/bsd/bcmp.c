#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char s1[] = "abcdefghijklmnopqrstuvwxyz";
char s2[] = "abcdefghijklmnopqrstuvwxyz";
char s3[] = "abcdefghijklMNOPQRSTUVWXYZ";
char s4[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int ec = 0;

void
expect(const char *p1, const char *p2, int len, int ex)
{
  int rv = bcmp(p1, p2, len);
  if (rv != ex)
  {
    printf("failure: bcmp(%s,%s,%d) = %d, and not %d\n",
	   p1, p2, len, rv, ex);
    ec ++;
  }
}

int
main(void)
{
  expect(s1, s4, 0, 0);
  expect(s1, s2, 26, 0);
  expect(s1, s3, 26, 14);
  expect(s1, s3, 14, 2);
  expect(s1, s3, 13, 1);
  expect(s1, s3, 12, 0);
  expect(0, s3, 1, -1);
  expect(0, 0, 1, 0);
  expect(s3, 0, 1, -1);
  if (!ec)
    printf("pass\n");
  return ec;
}

#include <stdio.h>
#include <conio.h>

int
main(void)
{
  int clx, cly;
  int mvx, mvy;
  clrscr();
  clx = wherex();
  cly = wherey();
  gotoxy(5,5);
  mvx = wherex();
  mvy = wherey();
  gotoxy(2,2);
  printf("X <- (2,2)\n");
  printf("clear - x,y is %d,%d\n", clx, cly);
  printf("(5,5) - x,y is %d,%d\n", mvx, mvy);
  return 0;
}

#include <stdio.h>
#include <conio.h>

int
main(void)
{
  int y;
  struct text_info ti;

  gettextinfo(&ti);
  clrscr();
  for (y = 1; y <= ti.winbottom; y++)
  {
    gotoxy(1,y);
    cprintf("line %3d",y);
  }
  while (getch() != 27)
  {
    gotoxy(1,1);
    delline();
  }
  return 0;
}

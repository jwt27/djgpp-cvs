/*
 * printf4.c
 * Test cases for numeric conversion specifiers.
 */

#include <stdio.h>
#include <string.h>
#include <libc/unconst.h>

int main(void)
{
  int i, width, precision;
  double darg[] = {1.1, 2.2, 3.3};
  const char *title[] = {"En castellano:", "Auf deutsch:", "In english:"};
  const char *format[] = {
   "%5$s, %2$d de %1$s, %3$*6$.*7$d:%4$*6$.*7$d\n",
   "%5$s, %2$d. %1$s, %3$*6$.*7$d:%4$*6$.*7$d\n",
   "%5$s, %1$s %2$d, %3$*6$.*7$d:%4$*6$.*7$d\n",
   NULL
  };
  const char *weekday[] = {"Sabado", "Samstag", "Saturday"};
  const char *month[] = {"febrero", "Februar", "february"};
  int day = 2;
  int hour = 12;
  int min = 34;


  printf("Testing numeric conversion specifiers.\n"
         "======================================\n");

  width = 10;
  precision = 1;
  printf("Printing a sequence of numbers using a given field width and precision\n"
         "accessing the variables a multiple times in different order.\n"
         "The sequence of arguments after the format string is:\n"
         "  width, precision, darg[0], darg[1], darg[2]\n"
         "with the values:\n"
         "  width:     %d\n"
         "  precision: %d\n"
         "  darg[0]:   %f\n"
         "  darg[1]:   %f\n"
         "  darg[2]:   %f\n",
         width, precision, darg[0], darg[1], darg[2]);
  printf("Format string: \"%%3$-*1$.*2$f###%%4$*1$.*2$f###%%5$*1$.*2$f\"     <%3$-*1$.*2$f###%4$*1$.*2$f###%5$*1$.*2$f>\n"
         "Printing again but accessing the arguments in inverse order:\n"
         "Format string: \"%%5$-*1$.*2$f###%%4$*1$.*2$f###%%3$*1$.*2$f\"     <%5$-*1$.*2$f###%4$*1$.*2$f###%3$*1$.*2$f>\n\n\n",
         width, precision, darg[0], darg[1], darg[2]);



  width = 2;
  precision = 2;
  printf("Printing Language-Independent Date and Time.\n\n");
  for (i = 0; format[i]; i++)
  {
    char *fmt;
    int len = strlen(format[i]);
    printf("%s  ", title[i]);
    printf(format[i], month[i], day, hour, min, weekday[i], width, precision);
    fmt = unconst((&format[i][--len]), char *);
    *fmt = '\0';
    printf("Produced with:\n"
           "  printf(\"%1$s\\n\", month[%2$i], day, hour, min, weekday[%2$i], width, precision);\n\n",
           format[i], i);
  }
  return 0;
}

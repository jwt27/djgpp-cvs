/* Test the n$ numeric conversion specifier.  */

#include <stdio.h>
#include <string.h>

int main(void)
{
  const char *buffer[3] = {
    "Fecha y hora es: 21 de diciembre de 2012, 01:02:03",
    "Datum und Uhrzeit ist: 21. Dezember 2012, 01:02:03",
    "Date and hour is: december 21, 2012, 01:02:03"
  };
  const char *format[3] = {
    "%*[A-Za-z :] %3$d %*[a-z] %2$10s %*[a-z] %1$d %*[,] %4$d%*[:] %5$d%*[:] %6$d%*",
    "%*[A-Za-z :] %3$d %*[.] %2$9s %1$d %*[,] %4$d%*[:] %5$d%*[:] %6$d%*",
    "%*[A-Za-z ]%*[:] %2$9s %3$d %*[,] %1$d %*[,] %4$d%*[:] %5$d%*[:] %6$d%*"
  };
  const char *language[3] = {
    "spanish",
    "german",
    "english"
  };
  typedef struct {
    const int year;
    const char *month;
    const int day;
    const int hour;
    const int min;
    const int sec;
  } result;
  result testcases[3] = {
    { 2012, "diciembre", 21, 1, 2, 3},  /* spanish */
    { 2012, "Dezember", 21, 1, 2, 3},   /* german */
    { 2012, "december", 21, 1, 2, 3}    /* english */
  };
  char month[10];
  int year, day, hour, min, sec, i, status;


  for (status = i = 0; i < (sizeof buffer / sizeof buffer[0]);)
  {
    printf("This string will be scanned:\n%s\n\n", buffer[i]);

    sscanf(buffer[i], format[i], &year, month, &day, &hour, &min, &sec);

    if (year == testcases[i].year)
    {
      status++;
      if (!strcmp(month, testcases[i].month))
      {
        status++;
        if (day == testcases[i].day)
        {
          status++;
          if (hour == testcases[i].hour)
          {
            status++;
            if (min == testcases[i].min)
            {
              status++;
              if (sec == testcases[i].sec)
                status++;
              else
                printf("At least one conversion failed.\nConversion should have been: %d but is %d\n", testcases[i].sec, sec);
            }
            else
              printf("At least one conversion failed.\nConversion should have been: %d but is %d\n", testcases[i].min, min);
          }
          else
            printf("At least one conversion failed.\nConversion should have been: %d but is %d\n", testcases[i].hour, hour);
        }
        else
          printf("At least one conversion failed.\nConversion should have been: %d but is %d\n", testcases[i].day, day);
      }
      else
        printf("At least one conversion failed.\nConversion should have been: %s but is %s\n", testcases[i].month, month);
    }
    else
      printf("At least one conversion failed.\nConversion should have been: %d but is %d\n", testcases[i].year, year);

    printf("Result of scanf using the %s format string\n\"%s\":\n"
           "  year:  %d\n"
           "  month: %s\n"
           "  day:   %d\n"
           "  hour:  %d\n"
           "  min:   %d\n"
           "  sec:   %d\n\n", language[i], format[i], year, month, day, hour, min, sec);

    if (status != 6 * ++i)
      return status;
  }

  printf("Test passed successfully.");

  return 0;
}

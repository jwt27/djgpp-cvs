/* Test the GNU C library specific m modifier.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compare_arg1(const int arg, const int result)
{
  if (arg != result)
  {
    printf("At least one conversion failed.\nConversion should have been: %d but is %d\n", arg, result);
    return 0;
  }
  return 1;
}

int compare_arg2(const double arg, const double result)
{
  if (arg != result)
  {
    printf("At least one conversion failed.\nConversion should have been: %g but is %g\n", arg, result);
    return 0;
  }
  return 1;
}

int compare_arg3(const int length, char *arg, const char *result)
{
  int i;
  arg[length] = '\0';
  for (i = 0; i < length; i++)
    if (result[i] != arg[i])
    {
      printf("At least one conversion failed.\nConversion should have been: %s but is %s\n", arg, result);
      return 0;
    }
  return 1;
}

int compare_arg4(const char *arg, const char *result)
{
  int i;
  for (i = 0; result[i] && arg[i]; i++)
    if (result[i] != arg[i])
    {
      printf("At least one conversion failed.\nConversion should have been: %s but is %s\n", arg, result);
      return 0;
    }
  return 1;
}

int main(void)
{
  const char printf_format[] = "The buffer contains: %d   %.6g   %s   %s\n";
  const char scanf_format1[] = "%*[a-zA-Z: ] %1d   %7lg   %12mc   %ms";
  const char scanf_format2[] = "%*[a-zA-Z: ] %1d   %7lg   %12c   %s";
  const char scanf_format3[] = "%*[a-zA-Z: ] %1d   %7lg   %m[a-z_]   %ms";
  char buffer[128];
  int arg1 = 1, iv;
  double arg2 = 2.34567, dv;
  char arg3[] = "first_string";
  char arg4[] = "second_string";
  char cv[sizeof("first_string") - 1], sv[sizeof("second_string")];
  char *cvp, *svp;
  char *svp1, *svp2;


  sprintf(buffer, printf_format, arg1, arg2, arg3, arg4);
  printf("%s\n\n", buffer);

  sscanf(buffer, scanf_format1, &iv, &dv, &cvp, &svp);
  printf("Result of scanf using \"%s\":\n"
         "  arg1:  %d\n"
         "  arg2:  %g\n"
         "  arg3(length = %zd):  %s\n"
         "  arg4(length = %zd):  %s\n", scanf_format1, iv, dv, strlen(cvp) + 1, cvp, strlen(svp) + 1, svp);
  if (!compare_arg1(arg1, iv))
    return 1;
  else if (!compare_arg2(arg2, dv))
    return 2;
  else if (!compare_arg3(12, arg3, cvp))
    return 3;
  else if (!compare_arg4(arg4, svp))
    return 4;
  /*  The caller must free the allocated buffers.  */
  free(cvp);
  free(svp);


  printf("\n");
  sscanf(buffer, scanf_format2, &iv, &dv, cv, sv);
  printf("Result of scanf using \"%s\":\n"
         "  arg1:  %d\n"
         "  arg2:  %g\n"
         "  arg3(length = %zd):  %s\n"
         "  arg4(length = %zd):  %s\n", scanf_format2, iv, dv, sizeof("first_string") - 1, cv, sizeof("second_string"), sv);
  if (!compare_arg1(arg1, iv))
    return 5;
  else if (!compare_arg2(arg2, dv))
    return 6;
  else if (!compare_arg3(12, arg3, cv))
    return 7;
  else if (!compare_arg4(arg4, sv))
    return 8;


  printf("\n");
  sscanf(buffer, scanf_format3, &iv, &dv, &svp1, &svp2);
  printf("Result of scanf using \"%s\":\n"
         "  arg1:  %d\n"
         "  arg2:  %g\n"
         "  arg3(length = %zd):  %s\n"
         "  arg4(length = %zd):  %s\n", scanf_format3, iv, dv, strlen(svp1) + 1, svp1, strlen(svp2) + 1, svp2);
  if (!compare_arg1(arg1, iv))
    return 9;
  else if (!compare_arg2(arg2, dv))
    return 10;
  else if (!compare_arg4(arg3, svp1))
    return 11;
  else if (!compare_arg4(arg4, svp2))
    return 12;
  /*  The caller must free the allocated buffers.  */
  free(svp1);
  free(svp2);

  printf("\nTest passed successfully.");

  return 0;
}

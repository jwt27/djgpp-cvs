#include <stdio.h>

int
main(void)
{
  FILE *f;
  char text_written[] = "A_test_text.";
  int failure = 0;


  f = fopen("file.txt", "w");
  if (f)
  {
    int fd = fileno(f);
    int written = dprintf(fd, "%s", text_written);

    if (written + 1== sizeof text_written)
    {
      fclose(f);
      f = fopen("file.txt", "r");

      if (f)
      {
        char text_read[sizeof text_written + 1];
        int read = fscanf(f, "%s", text_read);

        if (read)
        {
          unsigned int i;

          for (i = 0; i < sizeof text_written; i++)
            if (text_written[i] != text_read[i])
            {
              failure++;
              break;
            }
        }
        else
          failure++;
      }
      else
        failure++;
    }
    else
      failure++;
  }
  else
    failure++;

  printf("dprintf check %s\n", failure ? "failed." : "was successful.");

  return failure;
}

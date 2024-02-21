/*
 * File strerr_r.c.
 *
 * Copyright (C) 2005 Martin Str@"omberg <ams@ludd.ltu.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define FILE_NAME "Doesn't ::: exist!"

int main(void)
{
  FILE *f;

  f = fopen(FILE_NAME, "r");
  if( f == NULL )
  {
    char buf[1];
    int error_again;
	
    error_again = strerror_r(errno, buf, 1);
    if( error_again )
    {
      char buf2[1024];
      int error_again2;

      error_again2 = strerror_r(error_again, buf2, 1024 );
      if( error_again2 )
      {
	printf("Error while trying to decode error from calling strerror_r(). I give up!\n"
	       "Calling strerror: '%s'\n", strerror(error_again2));
      }
      printf("Error while trying to decode error from calling fopen(): '%s'\n",
	     buf2);
    }
    else
    {
      printf("Error from fopen(): %s\n", buf);
    }
  }
  else
  {
    printf("File name '%s' could be opened! This should not be possible in DOS.\n", FILE_NAME);

    return 1;
  }

  return 0;
}

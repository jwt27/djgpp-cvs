/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
#include <string.h>
#include <stdlib.h>

char *
strndup(const char *_s, size_t _n)
{
  if (_s == NULL)
    return NULL;
  else
  {
    register const size_t length = strlen(_s);
    register const size_t bytes = length > _n ? _n : length;
    register char *new_string = malloc(bytes + 1);

    if (new_string)
    {
      memcpy(new_string, _s, bytes);
      new_string[bytes] = '\0';
    }

    return new_string;
  }
}

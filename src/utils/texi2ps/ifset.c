/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* texi2ps -- convert texinfo format files into Postscript files.

   Author: Eli Zaretskii (eliz@is.elta.co.il)

   Copyright (C) 1995 DJ Delorie (dj@delorie.com)

   texi2ps is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.  No author or distributor accepts
   responsibility to anyone for the consequences of using it or for
   whether it serves any particular purpose or works at all, unless he
   says so in writing.  Refer to the GNU General Public License
   for full details.

   Everyone is granted permission to copy, modify and redistribute
   texi2ps, but only under the conditions described in the GNU
   General Public License.   A copy of this license is supposed to
   have been given to you along with texi2ps so you can know your
   rights and responsibilities.  It should be in a file named COPYING.
   Among other things, the copyright notice and this notice must be
   preserved on all copies.  */

/* Functions to manipulate flags: @set, @clear, @value, @ifset, @ifclear.

   The flags are placed into a char array in this form:

     "\0name=value\0name=value\0...\0\0"

   (yes, like DOS environment variables), where the value may be empty.
   This array grows as needed, if the space isn't large enough after the
   garbage was collected.  To clear a flag, we just overwrite the place
   it was stored with `?' characters.  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fileio.h"
#include "screenio.h"

/* The table to hold flags which are currently set.  */
static char *flag_table;

/* The size of currently allocated table.  */
static size_t flag_table_size;

/* The initial size we allocate for the table.  */
#define INITIAL_SIZE    1024

/* How much is actually used by the table entries.  */
static size_t used_size;

/* Return a pointer to NAME in the table if it is followed by `=',
   or NULL otherwise.  */
static char *
find_flag(char *name)
{
  char *p = flag_table;
  size_t namelen = strlen(name);

  if (p == (char *)0)
    return p;

  /* Look for "\0name=".  */
  while (*p == '\0' && p[1] != '\0')
    {
      if (strncmp(p + 1, name, namelen) == 0 && p[namelen + 1] == '=')
        return p + namelen + 2;
      p += strlen(p + 1) + 1;
    }

  return (char *)0;
}

/* Ensure we have enough space to add a new variable, return a pointer to
   first free slot.  */
static char *
alloc_flag(size_t need_bytes)
{
  size_t min_req_size = used_size + need_bytes;

  if (flag_table_size == 0)
    {
      /* Initial allocation.  */
      flag_table = (char *)malloc(INITIAL_SIZE);
      flag_table_size = INITIAL_SIZE;
      flag_table[0] = flag_table[1] = '\0';
      used_size = 2;
    }

  if (min_req_size > flag_table_size)
    {
      int need_realloc = 0;
      
      /* Not enough free space up front.  Before we reallocate, find
         unused slots and reuse them by unfragmenting the table.
         Unused slots look like this: "\0?????????\0" (the number of
         `?'s depends on the length of the name=value string which
         once occupied that slot).  */
      char *d = flag_table;
      char *s = d + 1;

      while (s - flag_table < (long signed int)used_size - 1)
        {
          size_t i = strspn(s, "?");    /* find first char which ISN'T `?' */

          if (i && s[i] == '\0')        /* the entire string is `?'s */
            {
              /* This slot is unused.  Compact it.  */
              s += i;
              memmove(d, s, used_size - (s - flag_table));
              used_size -= s - d;
              min_req_size -= s - d;
            }
          else
            /* This slot is used.  Go to next slot.  */
            d = s + i + strlen(s + i);

          s = d + 1;
        }

      while (flag_table_size < min_req_size)
        {
          need_realloc++;
          flag_table_size *= 2;
        }

      if (need_realloc)
        /* What?  1KB isn't enough for them??  Oh, well...  */
        flag_table = (char *)realloc(flag_table, flag_table_size);
    }

  if (flag_table == (char *)0)
    {
      fprintf(stderr,
              "Error: too many flags set: memory exhausted at %s\n",
              fileio_where());
      exit (1);
    }

  return flag_table + used_size - 1;
}

/* --------------------------------------------------------------------- */
    

/* Insert a flag and its value into the table.  */
void
set_flag(char *name, char *new_value)
{
  size_t namlen = strlen(name), vallen = strlen(new_value);
  char *current_value = find_flag(name);
  char *new;

  if (current_value)
    {
      if (strcmp(current_value, new_value) == 0)
        return;     /* it's already there with the right value */

      /* It's there, but with another value.  Strike-out this slot.  */
      memset(current_value - namlen - 1, '?',
             namlen + strlen(current_value) + 1);
    }
  new = alloc_flag(namlen + vallen + 2); /* name, value, `=' and `\0' */
  memcpy(new, name, namlen);
  used_size += namlen - 1;      /* we've overwritten the last `\0' */
  flag_table[used_size] = '=';
  used_size++;
  memcpy(flag_table + used_size, new_value, vallen + 1);
  used_size += vallen + 1;
  flag_table[used_size++] = '\0';
}

/* Clear a flag.  */
void
clear_flag(char *name)
{
  size_t namlen = strlen(name);
  char *current = find_flag(name);

  if (current)
    memset(current - namlen - 1, '?', namlen + strlen(current) + 1);

  /* It's OK to clear a flag which wasn't set. */
  screenio_print("Warning: flag `%s\' wasn\'t set, but is cleared at %s",
                 name, fileio_where());
}

/* Return a value of a flag, or an error string if it isn't set.  */

static char no_value_fmt[] = "{No value for \"%s\"}";

char *
flag_value(char *name)
{
  char *p = find_flag(name);

  if (p)
    return strdup(p);

  p = (char *)malloc(strlen(name) + sizeof(no_value_fmt) - 2);
  if (p != (char *)0)
    {
      screenio_print("Warning: flag `%s\' wasn\'t set, but its value used at %s",
                     name, fileio_where());
      sprintf(p, no_value_fmt, name);
    }
  return p;
}

/* Take a name of a flag out of ARG, move pointer past the name.
   The name is the first word, which may include embedded quoted
   whitespace.  */
char *
take_name(char *arg)
{
  char *p = arg;
  int quote = 0;

  while (quote || (*p && *p != ' ' && *p != '\t'))
    {
      if (quote == 0 && (*p == '"' || *p == '\''))
        quote = *p;
      else if (quote && *p == quote)
        quote = 0;

      p++;
    }

  return p;
}

/* Return non-zero if the flag, which is the first word of
   LINE, is set, and push the rest of line back onto input.  */
int
ifset(char *line)
{
  /* We got an entire line (instead of a single word) becase we
     want to support flag names with embedded quoted whitespace.
     We will push the rest of the line back onto input after we
     get the first word.  */
  char *s = line + strlen(line) - 1;  /* don't push back the '\0' char */
  char *p = take_name(line);
  char *name = (char *)alloca(p - line + 1);
  int   delim;

  delim = *p;       /* remember the delimiter */
  *p = '\0';
  strcpy(name, line);

  /* Push back the rest of the line.  */
  while (s > p)
    fileio_unget(*s--);

  /* Push back the delimiter.  */
  fileio_unget(delim);

  if (find_flag(name))
    return 1;

  return 0;
}

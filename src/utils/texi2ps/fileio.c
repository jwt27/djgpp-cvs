/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* texi2ps -- convert texinfo format files into Postscript files.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileio.h"
#include "screenio.h"

typedef struct IncludeItem {
  char *path;
  struct IncludeItem *next;
} IncludeItem;

static IncludeItem dot_path =
{ 0, 0 };

IncludeItem *include_path=&dot_path, *last_include_path=&dot_path;

typedef struct FileStack {
  FILE *file;
  char *name;
  int line_no;
  struct FileStack *prev;
} FileStack;

static FileStack *file_stack = 0;

typedef struct FileQueue {
  char *string;
  char *ptr;
  struct FileQueue *next;
} FileQueue;

static FileQueue *file_queue = 0;

static char unget_buffer[100];
static int unget_ptr = 0;

int fileio_get(void)
{
  if (unget_ptr)
    {
      int rv = unget_buffer[--unget_ptr];
      if (rv == '\n')
        file_stack->line_no++;
      return rv;
    }

  if (file_queue)
  {
    while (file_queue && *(file_queue->ptr) == 0)
    {
      FileQueue *fq = file_queue;
      file_queue = file_queue->next;
      free(fq->string);
      free(fq);
    }
    if (file_queue)
    {
      int rv = *(file_queue->ptr)++;
      if (rv == '\n')
        file_stack->line_no++;
      return rv;
    }
  }

  while (file_stack)
  {
    int rv = fgetc(file_stack->file);

    if (rv == '\n')
      file_stack->line_no++;

    if (rv == EOF && file_stack)
    {
      FileStack *tmp = file_stack;
      fclose(file_stack->file);
      screenio_print("Done:    %s", file_stack->name);
      free(file_stack->name);
      file_stack = file_stack->prev;
      free(tmp);
    }
    else
      return rv;
  }
  return EOF;
}

void fileio_unget(int c)
{
  unget_buffer[unget_ptr++] = c;
  if (c == '\n')
    file_stack->line_no--;
}

void fileio_include(char *n)
{
  FILE *f=0;
  FileStack *fs;
  IncludeItem *ip;
  char tmp[1000];
  char *found=0;

  for (ip=include_path; ip; ip=ip->next)
  {
    if (ip->path)
    {
      sprintf(tmp, "%s/%s", ip->path, n);
      found = tmp;
      f = fopen(tmp, "r");
    }
    else
    {
      f = fopen(n, "r");
      found = n;
    }
    if (f)
      break;
  }
  if (f == 0)
  {
    screenio_enabled++;
    if (file_stack)
      screenio_print("Error: unable to include file %s from %s", n, fileio_where());
    else
      screenio_print("Error: unable to open file %s", n);
    perror("The error was");
    screenio_enabled--;
    return;
  }
  screenio_print("Loading: %s", found);

  fs = (FileStack *)malloc(sizeof(FileStack));
  fs->name = strdup(found);
  fs->file = f;
  fs->line_no = 1;
  fs->prev = file_stack;
  file_stack = fs;
}

char *fileio_where(void)
{
  static char buf[1000];
  sprintf(buf, "file %s, line %d", file_stack->name, file_stack->line_no);
  return buf;
}

void fileio_queue(char *s)
{
  FileQueue *fq = (FileQueue *)malloc(sizeof(FileQueue));
  fq->string = strdup(s);
  fq->ptr = fq->string;
  fq->next = file_queue;
  file_queue = fq;
}

void fileio_add_path(char *p)
{
  IncludeItem *ip = (IncludeItem *)malloc(sizeof(IncludeItem));
  ip->next = 0;
  ip->path = (char *)malloc(strlen(p));
  strcpy(ip->path, p);
  last_include_path->next = ip;
  last_include_path = ip;
}

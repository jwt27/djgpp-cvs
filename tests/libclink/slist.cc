#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "slist.h"

StringList::StringList()
{
  count = 0;
  max = 10;
  maxc = 0;
  data = (char **)malloc(10*sizeof(char *));
}

StringList::~StringList()
{
  int i;
  for (i=0; i<maxc; i++)
    if (data[i])
      free(data[i]);
  free(data);
}

char *StringList::operator[](int i)
{
  if (maxc > count)
  {
    int cc, mc;
    for (cc=0, mc=0; mc<maxc; mc++)
      if (data[mc])
	data[cc++] = data[mc];
    if (cc != count)
      fprintf(stderr, "Programmer error: cc != count (%d != %d)\n", cc, count);
    maxc = count;
  }
  if (i >= count || i < 0)
    return 0;
  return data[i];
}

void StringList::add(char *s)
{
  if (has(s))
    return;
  if (maxc >= max)
  {
    max += 10;
    data = (char **)realloc(data, max * sizeof(char *));
  }
  data[maxc] = (char *)malloc(strlen(s)+1);
  strcpy(data[maxc], s);
  count++;
  maxc++;
}

void StringList::del(char *s)
{
  int i;
  for (i=0; i<maxc; i++)
    if (data[i] && strcmp(data[i], s) == 0)
    {
      data[i] = 0;
      count--;
    }
}

int StringList::has(char *s)
{
  int i;
  for (i=0; i<maxc; i++)
    if (data[i] && (strcmp(data[i], s) == 0))
      return 1;
  return 0;
}

void StringList::flush(void)
{
  for (count=0; count<maxc; count++)
    if (data[count])
      free(data[count]);
  count = maxc = 0;
}

static int cmp(const void *va, const void *vb)
{
  char *a = *(char **)va;
  char *b = *(char **)vb;
  return strcmp(a, b);
}

void StringList::sort(void)
{
  this[0];
  qsort(data, count, sizeof(char *), cmp);
}

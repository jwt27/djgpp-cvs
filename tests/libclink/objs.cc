#include <stdlib.h>
#include <string.h>

#include "slist.h"
#include "objs.h"

Object *Object::first = 0;

Object::Object(char *Pname)
{
  name = new char[strlen(Pname)+1];
  strcpy(name, Pname);
  df = rf = lf = busy = 0;
  next = first;
  first = this;
}

Object::~Object()
{
  delete name;
}

ObjList::ObjList()
{
  objs = (Object **)malloc(10*sizeof(Object *));
  max = 10;
  count = 0;
}

ObjList::~ObjList()
{
  free(objs);
}

void ObjList::add(Object *o)
{
  int i;
  for (i=0; i<count; i++)
    if (objs[i] == o)
	return;
  if (count >= max)
  {
    max += 10;
    objs = (Object **)realloc(objs, max * sizeof(Object *));
  }
  objs[count++] = o;
}

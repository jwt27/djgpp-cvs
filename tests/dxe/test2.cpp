/*
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 *
 * Usage of this library is not restricted in any way.  No warranty.
 *
 * Sample DXE module -- with unresolved external symbols.
 * This DXE module uses several external symbols.
 * These symbols are left unresolved and are resolved at the load time.
 *
 * Also here we have a example of constructors and destructors, which
 * are called automatically at library load/unload time.
 *
 * Define TEST_CPP_EXC to see how C++ exceptions are handled.
 */


#include <stdio.h>

#ifdef TEST_CPP_EXC
class exception {
public:
      void handle (void) {
           printf("%s%c", __PRETTY_FUNCTION__, '\n');
      }
};
#endif

extern "C" void extern_func ();

extern "C" int test_func ()
{
  extern_func ();
#ifdef TEST_CPP_EXC
  try {
       throw exception();
  } catch (exception e) {
       e.handle();
  }
#endif
  printf ("hello world! (&extern_func = %p)\n", (void *)(&extern_func));
  return 0;
}

int x_counter = 0;

extern "C" int doit ()
{
  return ++x_counter;
}

class TestClass
{
public:
  TestClass ()
  {
    printf ("%s%c", __PRETTY_FUNCTION__, '\n');
  }
  ~TestClass ()
  {
    printf ("%s%c", __PRETTY_FUNCTION__, '\n');
  }
} test_object;

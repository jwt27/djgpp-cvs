class Object;

class ObjList {
 public:
  ObjList();
  ~ObjList();
  Object **objs;
  int count, max;
  void add(Object *o);
};

class Object {
 public:
  Object(char *name);
  ~Object();
  StringList defs, refs;
  int df, rf, lf, busy;
  ObjList deps;
  Object *next;
  static Object *first;
  char *name;
};

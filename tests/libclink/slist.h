class StringList {
 public:
  StringList();
  ~StringList();
  int count;
  char *operator[](int);
  void add(char *);
  void del(char *);
  int  has(char *);
  void flush(void);
  void sort(void);
 private:
  int maxc;
  int max;
  char **data;
};

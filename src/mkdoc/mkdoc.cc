/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

char *dj_strlwr(char *s)
{
  char *p = s;
  while (*s)
  {
    if ((*s >= 'A') && (*s <= 'Z'))
      *s += 'a'-'A';
    s++;
  }
  return p;
}

struct TreeNode;
struct Node;
int count_nodes = 0;

typedef void (*TFunc)(Node *);

struct Tree {
  TreeNode *nodes;
  Tree();
  void add(Node *n);
  static void Traverse(TreeNode *tn, TFunc tf);
  void Traverse(TFunc tf);
  Node *find(char *name);
};

struct Line {
  struct Line *next;
  char *line;
};

struct Node {
  Node *prev, *next;
  char *name;
  char *sname;
  char *cat;
  Line *lines;
  Line *lastline;
  Tree subnodes;
  char *filename;
  Node(char *name, char *cat);
  void add(char *line);
};

struct TreeNode {
  TreeNode *before, *after;
  Node *node;
  TreeNode(Node *n);
};

Tree categories;
Tree nodes;

//-----------------------------------------------------------------------------

Node::Node(char *Pname, char *Pcat)
{
  char *cp;
  name = strdup(Pname);
  for (cp=name; *cp == '_'; cp++);
  sname = strdup(cp);
  dj_strlwr(sname);
  cat = strdup(Pcat);
  lines = 0;
  lastline = 0;
}

void
Node::add(char *l)
{
  Line *lp = new Line;
  lp->next = 0;
  lp->line = strdup(l);
  if (lastline)
    lastline->next = lp;
  if (!lines)
    lines = lp;
  lastline = lp;
}

//-----------------------------------------------------------------------------

TreeNode::TreeNode(Node *n)
{
  before = after = 0;
  node = n;
}

Tree::Tree()
{
  nodes = 0;
}

void
Tree::add(Node *n)
{
  TreeNode *tp = new TreeNode(n);
  TreeNode **np = &nodes;
  while (*np)
  {
    if (strcmp((*np)->node->sname, n->sname) < 0)
      np = &((*np)->after);
    else
      np = &((*np)->before);
  }
  *np = tp;
}

Node *set_np_prev = 0;
void
set_np(Node *n)
{
  n->prev = set_np_prev;
  n->next = 0;
  if (set_np_prev)
    set_np_prev->next = n;
  set_np_prev = n;
}

void
Tree::Traverse(TreeNode *tn, TFunc tf)
{
  if (!tn)
    return;
  Traverse(tn->before, tf);
  tf(tn->node);
  Traverse(tn->after, tf);
}

void
Tree::Traverse(TFunc tf)
{
  set_np_prev = 0;
  Traverse(nodes, set_np);
  Traverse(nodes, tf);
}

Node *
Tree::find(char *name)
{
  char *sname;
  for (sname = name; *sname == '_'; sname++);
  sname = strdup(sname);
  dj_strlwr(sname);

  TreeNode *tn = nodes;
  while (tn)
  {
    if (strcmp(tn->node->sname, sname) == 0)
    {
      return tn->node;
    }
    if (strcmp(sname, tn->node->sname) < 0)
      tn = tn->before;
    else
      tn = tn->after;
  }
  Node *n = new Node(name, "");
  add(n);
  return n;
}

//-----------------------------------------------------------------------------

FILE *co;
int print_filenames = 0;

void
pnode(Node *n, char *up)
{
  fprintf(co, "@c -----------------------------------------------------------------------------\n");
  fprintf(co, "@node %s, %s, %s, %s\n", n->name,
	  n->next ? n->next->name : "", n->prev ? n->prev->name : "", up);
  fprintf(co, "@unnumberedsec %s\n", n->name);
  if (print_filenames)
    fprintf(co, "@c From file %s\n", n->filename);
}

void
cprint1(Node *n)
{
  fprintf(co, "* %s::\n", n->name);
}

void
cprint2b(Node *n)
{
  fprintf(co, "* %s::\n", n->name);
}

void
cprint2(Node *n)
{
  pnode(n, "Functional Categories");
  fprintf(co, "@menu\n");
  n->subnodes.Traverse(cprint2b);
  fprintf(co, "@end menu\n");
}

void
nprint1(Node *n)
{
  fprintf(co, "* %s::\n", n->name);
}

void
nprint2(Node *n)
{
  pnode(n, "Alphabetical List");
  Line *l;
  for (l=n->lines; l; l=l->next)
  {
    fputs(l->line, co);
    if (strncmp(l->line, "@heading ", 9) == 0)
      fprintf(co, "@iftex\n@donoderef()\n@end iftex\n");
  }
}

int is_directory(char *name)
{
      struct stat statbuf;
      int result;

      result = stat(name, &statbuf);
      if(result < 0)
              return 0;
      if(S_ISDIR(statbuf.st_mode))
              return 1;
      else    return 0;
}

//-----------------------------------------------------------------------------

scan_directory(char *which)
{
  Node *curnode;
  DIR *d = opendir(which);
  struct dirent *de;
  while (de = readdir(d))
  {
    if (de->d_name[0] == '.')
      continue;
    char buf[4000];
    sprintf(buf, "%s/%s", which, de->d_name);

#ifdef D_OK
    if (access(buf, D_OK) == 0)
    {
      scan_directory(buf);
    }
#else
   /* determine if a directory */
   if(is_directory(buf))
   {
      scan_directory(buf);
   }
#endif
    else if (strstr(buf, ".txh"))
    {
      char *filename = new char[strlen(buf)+1];
      strcpy(filename, buf);
      FILE *ci = fopen(buf, "r");
      if (!ci)
      {
	perror(buf);
	continue;
      }
      curnode = 0;
      while (fgets(buf, 4000, ci))
      {
	if (strncmp(buf, "@c ---", 6) == 0)
	{
	}
	else if (strncmp(buf, "@node ", 6) == 0)
	{
	  char name[1000];
	  char cat[1000];
	  name[0] = 0;
	  cat[0] = 0;
	  sscanf(buf, "%*s %[^,\n], %[^\n]", name, cat);
	  strcat(cat, " functions");
	  curnode = new Node(name, cat);
	  count_nodes ++;
	  curnode->filename = filename;

	  nodes.add(curnode);
	  Node *catn = categories.find(cat);
	  catn->filename = filename;
	  catn->subnodes.add(curnode);
	}
	else
	{
	  if (curnode)
	    curnode->add(buf);
	}
      }
      fclose(ci);
    }
  }
}
//-----------------------------------------------------------------------------

main(int argc, char **argv)
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: mkdoc <directory> <output file>\n");
    return 1;
  }

  scan_directory(argv[1]);

  co = fopen(argv[2], "w");

  // Functional Categories
  fprintf(co, "@c -----------------------------------------------------------------------------\n");
  fprintf(co, "@node Functional Categories, Alphabetical List, Introduction, Top\n");
  fprintf(co, "@unnumbered Functional Categories\n");
  fprintf(co, "\n");
  fprintf(co, "@menu\n");
  categories.Traverse(cprint1);
  fprintf(co, "@end menu\n");

  categories.Traverse(cprint2);

  // Alphabetical List
  fprintf(co, "@c -----------------------------------------------------------------------------\n");
  fprintf(co, "@node Alphabetical List, , Functional Categories, Top\n");
  fprintf(co, "@unnumbered Alphabetical List\n");
  fprintf(co, "\n");
  fprintf(co, "@menu\n");
  nodes.Traverse(nprint1);
  fprintf(co, "@end menu\n");

  print_filenames = 1;

  nodes.Traverse(nprint2);
  printf("%d nodes processed\n", count_nodes);
  fclose(co);
  return 0;
}

/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

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


#define NUM_PORT_TARGETS 2

#define PORT_UNKNOWN 0
#define PORT_NO      1
#define PORT_PARTIAL 2
#define PORT_YES     3

/* Tokens for use in .txh files */
char *port_target[NUM_PORT_TARGETS] = { "ansi", "posix" };
/* Strings to output in .txi files */
char *port_target_string[NUM_PORT_TARGETS] = { "ANSI", "POSIX" };


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

struct PortNote {
  struct PortNote *next;
  int target;
  int number;
  char *note;
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
  int port_info[NUM_PORT_TARGETS];
  PortNote *port_notes;
  PortNote *last_port_note;
  Node(char *name, char *cat);
  void add(char *line);
  void process(char *line);
  void error(char *str, ...);
  void extend_portability_note(char *str);
  void read_portability_note(char *str);
  void read_portability(char *str);
  void write_portability();
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
  for (int i = 0; i < NUM_PORT_TARGETS; i++) port_info[i] = PORT_UNKNOWN;
  port_notes = NULL;
  last_port_note = NULL;
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

void
Node::error (char *str, ...)
{
  va_list arg;
  char s[1024];

  va_start (arg, str);
  vsprintf (s, str, arg);
  va_end (arg);

  fprintf (stderr, "Error (file %s, node %s): %s\n", filename, name, s);
}

void
Node::extend_portability_note(char *str)
{
  int newsize = strlen (last_port_note->note) + strlen (str) + 1;
  char *newstring = (char *) realloc (last_port_note->note, newsize);
  strcat (newstring, str);
  last_port_note->note = newstring;
}

void
Node::read_portability_note(char *str)
{
  char *work_str = strdup (str);
  char *s = work_str;
  char *target;
  int targ_num;

  while (isspace(*s)) s++;
  target = s;
  while (*s && !isspace(*s)) s++;
  if (*s) *s++ = 0;
  while (isspace(*s)) s++;
  dj_strlwr (target);

  for (targ_num = 0; targ_num < NUM_PORT_TARGETS; targ_num++)
    if (!strcmp (target, port_target[targ_num])) break;

  if (targ_num == NUM_PORT_TARGETS)
  {
    error ("unrecognised portability note target `%s' ignored.\n", target);
  }
  else
  {
    PortNote *p = new PortNote;
    p->next = NULL;
    p->number = 0;
    p->target = targ_num;
    p->note = strdup ("");

    if (port_notes)
    {
      last_port_note->next = p;
    }
    else
    {
      port_notes = p;
    }
    last_port_note = p;

    if (*s) extend_portability_note (s);
  }
  free (work_str);
}

void
Node::read_portability(char *str)
{
  char *targets = dj_strlwr (strdup (str));
  char *x, *target = targets;
  int type,i;

  while (isspace (*target)) target++;
  while (*target) {

    type = PORT_YES;
    if (*target == '~')
    {
      type = PORT_PARTIAL;
      target++;
    } else if (*target == '!')
    {
      type = PORT_NO;
      target++;
    }

    for (x = target; *x && !isspace(*x) && (*x != ','); x++);
    if (*x) *x++ = 0;

    for (i = 0; i < NUM_PORT_TARGETS; i++)
      if (!strcmp (target, port_target[i])) break;

    if (i < NUM_PORT_TARGETS)
      port_info[i] = type;
    else
      error ("unrecognised portability target `%s' ignored.\n", target);

    target = x;
    while (isspace (*target)) target++;
  }

  free (targets);
}

void
Node::write_portability()
{
  char buffer[1024] = { 0 };
  int note_number = 1;

  for (int i = 0; i < NUM_PORT_TARGETS; i++)
  {
    switch (port_info[i])
    {
      case PORT_NO:
	strcat (buffer, "not ");
	strcat (buffer, port_target_string[i]);
	break;
      case PORT_YES:
	strcat (buffer, port_target_string[i]);
	break;
      case PORT_PARTIAL:
	strcat (buffer, "partially ");
	strcat (buffer, port_target_string[i]);
	break;
    }
    if (port_info[i] != PORT_UNKNOWN)
    {
      for (PortNote *p = port_notes; p; p = p->next)
      {
	if (p->target == i)
	{
	  char smallbuffer[20];
	  p->number = note_number++;
	  sprintf (smallbuffer, " (see note %d)", p->number);
	  strcat (buffer, smallbuffer);
	  break;
	}
      }
      strcat (buffer, ", ");
    }
  }

  {
    char *ch = strchr (buffer, 0) - 2;
    if (*ch == ',')
      *ch = 0;
    else
      strcpy (buffer, "Unknown.");
  }

  strcat (buffer, "\n\n");
  add(buffer);

  if (note_number > 1)
  {
    add("@noindent\n");
    add("Notes:\n");
    add("\n");
    add("@enumerate\n");

    for (int i = 1; i < note_number; i++)
    {
      add("@item\n");
      for (PortNote *p = port_notes; p; p = p->next)
	if (p->number == i)
	  add(p->note);
    }

    add("@end enumerate\n");
  }

  /* Now free the portability notes */
  while (port_notes) {
    PortNote *p = port_notes;
    port_notes = p->next;
    free (p->note);
    delete p;
  }
  last_port_note = NULL;
}

void
Node::process(char *line)
{
  if (line[0] == '@') {
    if ((strncmp (line, "@portability", 12) == 0) && isspace (line[12]))
    {
      read_portability(line+13);
      write_portability();
      return;
    }
    else if ((strncmp (line, "@port-note", 10) == 0) && isspace (line[10]))
    {
      read_portability_note(line+11);
      return;
    }
  }
  
  /* If `last_port_note' is not NULL, we're in the middle of a note */
  if (last_port_note)
  {
    extend_portability_note(line);
  }
  else
  {
    add(line);
  }
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

void scan_directory(char *which)
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
    int buflen = strlen(buf);

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
    else if (strcmp(buf+buflen-4, ".txh") == 0
	     && !strchr(buf, '~')
	     && !strchr(buf, '#'))
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
	    curnode->process(buf);
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
  fprintf(co, "@node Alphabetical List, Unimplemented, Functional Categories, Top\n");
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

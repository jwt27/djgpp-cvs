/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
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

#define PORT_TARGET_NONE              0x00
/* ANSI/ISO C */
#define PORT_TARGET_ANSI_C89          0x10
#define PORT_TARGET_ANSI_C99          0x11
/* POSIX */
#define PORT_TARGET_POSIX_1003_2_1992 0x20
#define PORT_TARGET_POSIX_1003_1_2001 0x21
/* Single Unix Specification(s) (SUS) */
#define PORT_TARGET_UNIX_98           0x30

#define PORT_UNKNOWN 0
#define PORT_NO      1
#define PORT_PARTIAL 2
#define PORT_YES     3

/* This structure is used to store both the default information about a
 * particular porting target and the information parsed from the texinfo.
 * For the default case, complete specifies the default value for this
 * qualifier if the prefix is matched, but the suffix is not. */
typedef struct {
  char *suffix_token;  /* Suffix token used in texinfo, e.g. 'c89' */
  char *suffix_name;   /* Portability qualifier name, e.g. C89 */
  int target;          /* One of PORT_TARGET_* */
  int complete;        /* One of PORT_UNKNOWN, etc. */
} PortQualifier;

#define MAX_PORT_QUALIFIERS 2

typedef struct {
  char *prefix_token; /* Token used in texinfo, e.g. 'ansi' */
  char *prefix_name;  /* Actual textual name for token, e.g. ANSI/ISO C */
  PortQualifier pq[MAX_PORT_QUALIFIERS];
} PortInfo;

#define NUM_PORT_TARGETS    3

PortInfo port_target[] = {
  /* ANSI/ISO C */
  { "ansi",  "ANSI/ISO C",
    {
      { "c89", "C89", PORT_TARGET_ANSI_C89, PORT_YES },
      { "c99", "C99", PORT_TARGET_ANSI_C99, PORT_YES }
    }
  },
  /* POSIX */
  { "posix", "POSIX",
    {
      { "1003.2-1992", "1003.2-1992",
	PORT_TARGET_POSIX_1003_2_1992, PORT_YES },
      { "1003.1-2001", "1003.1-2001",
	PORT_TARGET_POSIX_1003_1_2001, PORT_YES }
    }
  },
  /* SUSv2 */
  { "unix",  "Unix",
    {
      { "98", "Unix98", PORT_TARGET_UNIX_98, PORT_UNKNOWN },
      { 0 }
    }
  }
};

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
  PortInfo *pi;
  PortQualifier *pq;
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
  PortInfo port_info[NUM_PORT_TARGETS];
  PortNote *port_notes;
  PortNote *last_port_note;
  int written_portability;
  Node(char *name, char *cat);
  void add(char *line);
  void process(char *line);
  void error(char *str, ...);
  void warning(char *str, ...);
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
  for (int i = 0; i < NUM_PORT_TARGETS; i++)
    memset(&port_info[i], 0, sizeof(port_info[i]));
  port_notes = NULL;
  last_port_note = NULL;
  written_portability = 0;
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
Node::warning (char *str, ...)
{
  va_list arg;
  char s[1024];

  va_start (arg, str);
  vsprintf (s, str, arg);
  va_end (arg);

  fprintf (stderr, "Warning (file %s, node %s): %s\n", filename, name, s);
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
  char *s = work_str, *x = NULL;
  char *target;
  int i, j;

  if (written_portability) {
    warning("%s", "@port-note must come before @portability.");
    warning("%s", "Ignoring all @port-note for this node.");
  }

  while (isspace(*s)) s++;
  target = s;
  while (*s && !isspace(*s)) s++;
  if (*s) *s++ = 0;
  while (isspace(*s)) s++;
  dj_strlwr (target);

  for (i = 0; i < NUM_PORT_TARGETS; i++) {
    if (   (strlen (target) >= strlen (port_target[i].prefix_token))
	&& !strncmp (target, port_target[i].prefix_token,
		     strlen (port_target[i].prefix_token))) {
      /* If matched, check that the next character is either: null, a dash
       * (to indicate a qualifier) or null. */
      x = target + strlen (port_target[i].prefix_token);
      if ((*x == '\0') || (*x == '-') || isspace((int) *x))
	break;
    }
  }  

  if (i == NUM_PORT_TARGETS) {
    error ("unrecognised portability note target `%s' ignored.\n", target);
  } else {
    PortNote *p = new PortNote;
    p->next = NULL;
    p->number = 0;
    p->pi = &port_target[i];
    p->pq = NULL;
    p->note = strdup ("");
    
    /* Try to match the portability note to a portability qualifier. */
    x = target + strlen (p->pi->prefix_token);
    if (*x == '-')
      x++;

    for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
      if (p->pi->pq[j].suffix_token == NULL) continue;
      if (!strcmp (x, p->pi->pq[j].suffix_token)) break;
    }

    if (j < MAX_PORT_QUALIFIERS)
      p->pq = &p->pi->pq[j];

    /* Attach portability note to note chain. */
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
  char *p = NULL, *x = NULL, *target = targets;
  int type, i, j;

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

    for (i = 0; i < NUM_PORT_TARGETS; i++) {
      if (   (strlen (target) >= strlen (port_target[i].prefix_token))
	  && !strncmp (target, port_target[i].prefix_token,
		       strlen (port_target[i].prefix_token))) {
	/* If matched, check that the next character is either: null, a dash
	 * (to indicate a qualifier) or null. */
	p = target + strlen (port_target[i].prefix_token);
	if ((*p == '\0') || (*p == '-') || isspace((int) *p))
	  break;
      }
    }

    if (i < NUM_PORT_TARGETS) {
      /* Now match the portability qualifier, if present. */
      p = target + strlen (port_target[i].prefix_token);

      if (port_info[i].prefix_name == NULL) {
	/* Copy default portability information to uninitialised port
	 * info, qualifier list. */
	memcpy(&port_info[i], &port_target[i], sizeof(port_target[i]));
      }


      if (*p == '-') {
	/* A qualifier is present, so set the portability type for just
	 * this qualifier. */
	p++;

	for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
	  if (port_target[i].pq[j].suffix_token == NULL)
	    continue;

	  if (!strcmp (p, port_target[i].pq[j].suffix_token))
	    break;
	}

	if (j < NUM_PORT_TARGETS)
	  port_info[i].pq[j].complete = type;
      } else {
	/* A qualifier is not present, so set the type for all qualifiers. */
	/* TODO: If the bare prefix appears after the prefix has appeared
	 * with qualifiers in the line, then this will reset all qualifiers.
	 * This is a bug. The solution is to be careful with @portability. */
	for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
	  port_info[i].pq[j].complete = type;
	}
      }
    } else {
      error ("unrecognised portability target `%s' ignored.\n", target);
    }

    target = x;
    while (isspace (*target)) target++;
  }

  free (targets);
}

void
Node::write_portability()
{  
  /* Column-width calculation variables */
  size_t maxsize = 0;
  ssize_t size = 0;
  static int largest_target = -1;
  static char rightpad[80] = { 0 };

  char buffer[1024] = { 0 };
  int qualifier_number = 0;
  PortNote *p = NULL;
  int note_number = 1;

  /* If all qualifiers are set to a particular value, store it here
   * (one of PORT_*). Otherwise it should be set to -1. */
  int all_port_qualifiers = -1;

  int i, j;

  /* Deduce the largest target name length, for table's left-hand column. */
  if (largest_target == -1)
  {    
    for (i = 0; i < NUM_PORT_TARGETS; i++)
    {
      if (strlen(port_target[i].prefix_name) > maxsize)
      {
	maxsize = strlen(port_target[i].prefix_name);
	largest_target = i;
      }
    }
  }

  /* Make the right-hand column 80 columns less the left-hand column width,
   * less some more for safety. */
  if (rightpad[0] == '\0') {
    size = sizeof(rightpad) - maxsize - 10;
    if (size > 0) memset(rightpad, (int) 'x', size);
  }

  strcat (buffer, "@multitable {");
  strcat (buffer, port_target[largest_target].prefix_name);
  strcat (buffer, "} {");  
  strcat (buffer, rightpad);
  strcat (buffer, "}\n");

  for (i = 0; i < NUM_PORT_TARGETS; i++)
  {
    /* No information given => unknown => skip it. */
    if (port_info[i].prefix_name == NULL)
      continue;

    /* Are all qualifiers set to the same value of one of PORT_*? */
    for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
      /* Skip unnamed suffixes */
      if (port_info[i].pq[j].suffix_name == NULL)
	continue;

      if (all_port_qualifiers == -1) {
	all_port_qualifiers = port_info[i].pq[j].complete;
      } else {
	if (all_port_qualifiers != port_info[i].pq[j].complete) {
	  /* Not all port qualifiers have the same completion status. */
	  all_port_qualifiers = -1;
	  break;
	}
      }
    }

    /* If all qualifiers are all set to unknown, skip this target. */
    if (all_port_qualifiers == PORT_UNKNOWN)
      continue;

    /* Add an entry to the table. */
    strcat (buffer, "@item ");
    strcat (buffer, port_target[i].prefix_name);
    strcat (buffer, " @tab ");    

    qualifier_number = 0;

    /* Add positive or partial qualifiers to the list. */
    for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
      /* Skip unnamed suffixes */
      if (port_info[i].pq[j].suffix_name == NULL)
	continue;

      if (   (port_info[i].pq[j].complete != PORT_YES)
	  && (port_info[i].pq[j].complete != PORT_PARTIAL))
	continue;

      /* Add separator, if this isn't the first entry. */
      qualifier_number++;
      if (qualifier_number > 1)
	strcat (buffer, "; ");
      
      if (port_info[i].pq[j].complete == PORT_YES) {
	strcat (buffer, port_info[i].pq[j].suffix_name);
      } else if (port_info[i].pq[j].complete == PORT_PARTIAL) {
	strcat (buffer, port_info[i].pq[j].suffix_name);
	strcat (buffer, " (partial)");
      }

      /* Attach any qualifier-specific portability notes. */
      for (p = port_notes; p; p = p->next)
      {
	if (   !strcmp (p->pi->prefix_token, port_info[i].prefix_token)
	    && (p->pq != NULL)
	    && !strcmp (p->pq->suffix_token, port_info[i].pq[j].suffix_token))
	{
	  char smallbuffer[20];
	  p->number = note_number++;
	  sprintf (smallbuffer, " (see note %d)", p->number);
	  strcat (buffer, smallbuffer);
	  break;
	}
      }
    }

    /* Add negative qualifiers to the list. */
    if (all_port_qualifiers == PORT_NO)
      strcat (buffer, "No");

    for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
      /* Skip unnamed suffixes */
      if (port_info[i].pq[j].suffix_name == NULL)
	continue;

      if (port_info[i].pq[j].complete != PORT_NO)
	continue;

      /* If all port qualifiers == PORT_NO, then we've already output. */
      if (all_port_qualifiers != PORT_NO) {
	/* Add separator, if this isn't the first entry. */
	qualifier_number++;
	if (qualifier_number > 1)
	  strcat (buffer, "; ");
	
	strcat (buffer, "not ");
	strcat (buffer, port_info[i].pq[j].suffix_name);
      }

      /* Attach any qualifier-specific portability notes. */
      for (p = port_notes; p; p = p->next)
      {
	if (   !strcmp (p->pi->prefix_token, port_info[i].prefix_token)
	    && (p->pq != NULL)
	    && !strcmp (p->pq->suffix_token, port_info[i].pq[j].suffix_token))
	{
	  char smallbuffer[20];
	  p->number = note_number++;
	  sprintf (smallbuffer, " (see note %d)", p->number);
	  strcat (buffer, smallbuffer);
	  break;
	}
      }
    }

    /* Attach any target-specific portability notes. */
    for (p = port_notes; p; p = p->next)
    {
      if (   (port_info[i].prefix_token != NULL)
	  && !strcmp (p->pi->prefix_token, port_info[i].prefix_token)
	  && (p->pq == NULL))
      {
	char smallbuffer[20];
	p->number = note_number++;
	sprintf (smallbuffer, " (see note %d)", p->number);
	strcat (buffer, smallbuffer);
	break;
      }
    }

    strcat (buffer, "\n");
  }

  strcat (buffer, "@end multitable\n\n");

  add(buffer);

  if (note_number > 1)
  {
    add("@noindent\n");
    add("Notes:\n");
    add("\n");
    add("@enumerate\n");

    for (i = 1; i < note_number; i++)
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

  written_portability++;
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
  while ((de = readdir(d)) != NULL)
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

void list_portability (void)
{
  int i, j;
  char buffer[40];

  printf("Built-in portability targets:\n");

  for (i = 0; i < NUM_PORT_TARGETS; i++) {    
    if (port_target[i].pq[0].suffix_token  == NULL) {
      printf("%-32s = %-32s\n",
	     port_target[i].prefix_token,
	     port_target[i].prefix_name);
    } else {
      for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
	if (port_target[i].pq[j].suffix_token == NULL) break;

	strcpy(buffer, port_target[i].prefix_token);
	strcat(buffer, "-");
	strcat(buffer, port_target[i].pq[j].suffix_token);

	printf("%-32s = ", buffer);

	strcpy(buffer, port_target[i].prefix_name);
	strcat(buffer, ": ");
	strcat(buffer, port_target[i].pq[j].suffix_name);

	printf("%-32s\n", buffer);
      }
    }
  }
}

//-----------------------------------------------------------------------------

void usage (void)
{
  fprintf(stderr,
	  "Usage: mkdoc [<switches>] <directory> <output file>\n"
	  "\n"
	  "Switches:\n"
	  "       -h, -?, --help      -  Display this help\n"
	  "       -l, --list-targets  -  List built-in portability targets\n"
	  "\n");
}

//-----------------------------------------------------------------------------

int main (int argc, char **argv)
{
  int i;  

  // Scan for help options
  for (i = 1; i < argc; i++) {
    if (   !strcmp (argv[i], "-h")
	|| !strcmp (argv[i], "--help")
	|| !strcmp (argv[i], "-?") ) {
      usage();
      return 1;
    }

    if (   !strcmp (argv[i], "-l")
	|| !strcmp (argv[i], "--list-targets") ) {
      list_portability();
      return 1;
    }
  }

  if (argc < 3) {
    usage();
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

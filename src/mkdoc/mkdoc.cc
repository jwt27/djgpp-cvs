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

#include <string>

static char *dj_strlwr(char *s)
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

static char *make_sname(const char *name)
{
  char *sname;
  for (; *name == '_'; name++);
  sname = strdup(name);
  dj_strlwr(sname);
  return sname;
}

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
struct PortQualifier {
  const char *suffix_token;  /* Suffix token used in texinfo, e.g. 'c89' */
  const char *suffix_name;   /* Portability qualifier name, e.g. C89 */
  int target;          /* One of PORT_TARGET_* */
  int complete;        /* One of PORT_UNKNOWN, etc. */
};

#define MAX_PORT_QUALIFIERS 2

struct PortInfo {
  const char *prefix_token; /* Token used in texinfo, e.g. 'ansi' */
  const char *prefix_name;  /* Actual textual name for token, e.g. ANSI/ISO C */
  PortQualifier pq[MAX_PORT_QUALIFIERS];
};

#define NUM_PORT_TARGETS    3

static const PortInfo port_target[] = {
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
      { NULL, NULL, 0, 0 }
    }
  }
};

template <typename N>
struct TreeNode {
  TreeNode *before, *after;
  TreeNode *prev, *next;
  std::string name;
  char *sname;
  N *node;
  TreeNode(const char *name, N *n);
  ~TreeNode();
  void Traverse(void (&tf)(const TreeNode &)) const;
  int Compare(const char *sn) const;
  void Pnode(void) const;
  const char *up(void) const;
};

template <typename N>
class Tree {
  TreeNode<N> *nodes;
public:
  Tree() : nodes(NULL) {}
  ~Tree() { delete nodes; }
  void Add(TreeNode<N> &);
  TreeNode<N> &Find(const char *);
  void Traverse(void (&tf)(const TreeNode<N> &)) const { nodes->Traverse(tf); }
  void Print1(void) const;
};

class Lines {
  std::string lines;
public:
  void Add(const std::string &l) { lines.append(l); }
  void Add(const char *l) { lines.append(l); }
  void Print1(void) const;
};

class NodeSource {
  const std::string name;
  const std::string filename;
  void Message(const char *, const char *, va_list) const;
public:
  NodeSource(const char *n, const std::string &fn) : name(n), filename(fn) {}
  void Error(const char *str, ...) const;
  void Warning(const char *str, ...) const;
};

struct PortNote {
  PortNote *next;
  int number;
  std::string note;
  PortNote(void) : next(NULL), number(0), note() {}
};

struct Node {
  NodeSource source;
  Lines &lines;
  PortInfo port_info[NUM_PORT_TARGETS];
  PortNote *port_notes[NUM_PORT_TARGETS];
  PortNote **port_note_tail[NUM_PORT_TARGETS];
  PortNote *q_port_notes[NUM_PORT_TARGETS][MAX_PORT_QUALIFIERS];
  PortNote **q_port_note_tail[NUM_PORT_TARGETS][MAX_PORT_QUALIFIERS];
  PortNote *last_port_note;
  int written_portability;
  static int count_nodes;
  Node(Lines &, const char *name, const char *fn);
  void process(const char *line);
  void read_portability_note(const char *str);
  void read_portability(const char *str);
  void write_portability(void);
};

static Tree<Tree<void> > categories;
static Tree<Lines> nodes;
int Node::count_nodes(0);

//-----------------------------------------------------------------------------

Node::Node(Lines &l, const char *n, const char *fn)
  : source(n, fn), lines(l)
{
  for (int i = 0; i < NUM_PORT_TARGETS; i++)
  {
    PortInfo &pii = port_info[i];
    memset(&pii, 0, sizeof(pii));
    port_notes[i] = NULL;
    port_note_tail[i] = &port_notes[i];
    for (int j = 0; j < MAX_PORT_QUALIFIERS; j++)
    {
      q_port_notes[i][j] = NULL;
      q_port_note_tail[i][j] = &q_port_notes[i][j];
    }
  }
  last_port_note = NULL;
  written_portability = 0;
  count_nodes++;
}

void
Node::read_portability_note(const char *str)
{
  char *work_str = strdup (str);
  char *s = work_str, *x = NULL;
  char *target;
  int i, j;

  if (written_portability) {
    source.Warning("%s", "@port-note must come before @portability.");
    source.Warning("%s", "Ignoring all @port-note for this node.");
  }

  while (isspace(*s)) s++;
  target = s;
  while (*s && !isspace(*s)) s++;
  if (*s) *s++ = 0;
  while (isspace(*s)) s++;
  dj_strlwr (target);

  for (i = 0; i < NUM_PORT_TARGETS; i++) {
    const char *pt = port_target[i].prefix_token;
    if (   (strlen (target) >= strlen (pt))
	&& !strncmp (target, pt,
		     strlen (pt))) {
      /* If matched, check that the next character is either: null, a dash
       * (to indicate a qualifier) or null. */
      x = target + strlen (pt);
      if ((*x == '\0') || (*x == '-') || isspace((int) *x))
	break;
    }
  }  

  if (i == NUM_PORT_TARGETS) {
    source.Error ("unrecognised portability note target `%s' ignored.\n", target);
  } else {
    const PortInfo &pti = port_target[i];
    PortNote *p = new PortNote;
    
    /* Try to match the portability note to a portability qualifier. */
    x = target + strlen (pti.prefix_token);
    if (*x == '-')
      x++;

    for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
      const char *st = pti.pq[j].suffix_token;
      if (st == NULL) continue;
      if (!strcmp (x, st)) break;
    }

    /* Attach portability note to note chain. */
    if (j < MAX_PORT_QUALIFIERS) {
      *q_port_note_tail[i][j] = p;
      q_port_note_tail[i][j] = &p->next;
    } else {
      *port_note_tail[i] = p;
      port_note_tail[i] = &p->next;
    }
    last_port_note = p;

    if (*s)
      last_port_note->note.append(s);
  }
  free (work_str);
}

void
Node::read_portability(const char *str)
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
      const char *pt = port_target[i].prefix_token;
      if (   (strlen (target) >= strlen (pt))
	  && !strncmp (target, pt,
		       strlen (pt))) {
	/* If matched, check that the next character is either: null, a dash
	 * (to indicate a qualifier) or null. */
	p = target + strlen (pt);
	if ((*p == '\0') || (*p == '-') || isspace((int) *p))
	  break;
      }
    }

    if (i < NUM_PORT_TARGETS) {
      const PortInfo &pti = port_target[i];
      PortInfo &pii = port_info[i];

      /* Now match the portability qualifier, if present. */
      p = target + strlen (pti.prefix_token);

      if (pii.prefix_name == NULL) {
	/* Copy default portability information to uninitialised port
	 * info, qualifier list. */
	memcpy(&pii, &pti, sizeof(pti));
      }


      if (*p == '-') {
	/* A qualifier is present, so set the portability type for just
	 * this qualifier. */
	p++;

	for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
	  const char *st = pti.pq[j].suffix_token;

	  if (st == NULL)
	    continue;

	  if (!strcmp (p, st))
	    break;
	}

	if (j < MAX_PORT_QUALIFIERS)
	  pii.pq[j].complete = type;
      } else {
	/* A qualifier is not present, so set the type for all qualifiers. */
	/* TODO: If the bare prefix appears after the prefix has appeared
	 * with qualifiers in the line, then this will reset all qualifiers.
	 * This is a bug. The solution is to be careful with @portability. */
	for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
	  pii.pq[j].complete = type;
	}
      }
    } else {
      source.Error ("unrecognised portability target `%s' ignored.\n", target);
    }

    target = x;
    while (isspace (*target)) target++;
  }

  free (targets);
}

void
Node::write_portability(void)
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
      const char *pn = port_target[i].prefix_name;
      if (strlen(pn) > maxsize)
      {
	maxsize = strlen(pn);
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
    PortInfo &pii = port_info[i];

    /* No information given => unknown => skip it. */
    if (pii.prefix_name == NULL)
      continue;

    /* Are all qualifiers set to the same value of one of PORT_*? */
    for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
      PortQualifier &pii_pqj = pii.pq[j];

      /* Skip unnamed suffixes */
      if (pii_pqj.suffix_name == NULL)
	continue;

      if (all_port_qualifiers == -1) {
	all_port_qualifiers = pii_pqj.complete;
      } else {
	if (all_port_qualifiers != pii_pqj.complete) {
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
    strcat (buffer, "\n@tab ");    

    qualifier_number = 0;

    /* Add positive or partial qualifiers to the list. */
    for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
      PortQualifier &pii_pqj = pii.pq[j];

      /* Skip unnamed suffixes */
      if (pii_pqj.suffix_name == NULL)
	continue;

      if (   (pii_pqj.complete != PORT_YES)
	  && (pii_pqj.complete != PORT_PARTIAL))
	continue;

      /* Add separator, if this isn't the first entry. */
      qualifier_number++;
      if (qualifier_number > 1)
	strcat (buffer, "; ");
      
      if (pii_pqj.complete == PORT_YES) {
	strcat (buffer, pii_pqj.suffix_name);
      } else if (pii_pqj.complete == PORT_PARTIAL) {
	strcat (buffer, pii_pqj.suffix_name);
	strcat (buffer, " (partial)");
      }

      /* Attach any qualifier-specific portability notes. */
      for (p = q_port_notes[i][j]; p; p = p->next)
      {
	  char smallbuffer[20];
	  p->number = note_number++;
	  sprintf (smallbuffer, " (see note %d)", p->number);
	  strcat (buffer, smallbuffer);
      }
    }

    /* Add negative qualifiers to the list. */
    if (all_port_qualifiers == PORT_NO)
      strcat (buffer, "No");

    for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
      PortQualifier &pii_pqj = pii.pq[j];

      /* Skip unnamed suffixes */
      if (pii_pqj.suffix_name == NULL)
	continue;

      if (pii_pqj.complete != PORT_NO)
	continue;

      /* If all port qualifiers == PORT_NO, then we've already output. */
      if (all_port_qualifiers != PORT_NO) {
	/* Add separator, if this isn't the first entry. */
	qualifier_number++;
	if (qualifier_number > 1)
	  strcat (buffer, "; ");
	
	strcat (buffer, "not ");
	strcat (buffer, pii_pqj.suffix_name);
      }

      /* Attach any qualifier-specific portability notes. */
      for (p = q_port_notes[i][j]; p; p = p->next)
      {
	  char smallbuffer[20];
	  p->number = note_number++;
	  sprintf (smallbuffer, " (see note %d)", p->number);
	  strcat (buffer, smallbuffer);
      }
    }

    /* Add separator, if there are qualifiers. */
    if (port_notes[i] && qualifier_number > 0)
      strcat (buffer, ";");

    /* Attach any target-specific portability notes. */
    for (p = port_notes[i]; p; p = p->next)
    {
	char smallbuffer[20];
	p->number = note_number++;
	sprintf (smallbuffer, " (see note %d)", p->number);
	strcat (buffer, smallbuffer);
    }

    strcat (buffer, "\n");
  }

  strcat (buffer, "@end multitable\n\n");

  lines.Add(buffer);

  if (note_number > 1)
  {
    lines.Add("@noindent\n");
    lines.Add("Notes:\n");
    lines.Add("\n");
    lines.Add("@enumerate\n");

    for (int n = 1; n < note_number; n++)
    {
      lines.Add("@item\n");
      for (i = 0; i < NUM_PORT_TARGETS; i++) {
	for (PortNote *p = port_notes[i]; p; p = p->next)
	  if (p->number == n)
	    lines.Add(p->note);
	for (j = 0; j <MAX_PORT_QUALIFIERS; j++)
	  for (PortNote *p = q_port_notes[i][j]; p; p = p->next)
	    if (p->number == n)
	      lines.Add(p->note);
      }
    }

    lines.Add("@end enumerate\n");
  }

  /* Now free the portability notes */
  for (i = 0; i < NUM_PORT_TARGETS; i++) {
    while (port_notes[i]) {
      PortNote *p = port_notes[i];
      port_notes[i] = p->next;
      delete p;
    }
    for (j = 0; j <MAX_PORT_QUALIFIERS; j++)
      while (q_port_notes[i][j]) {
	PortNote *p = q_port_notes[i][j];
	q_port_notes[i][j] = p->next;
	delete p;
    }
  }
  last_port_note = NULL;

  written_portability++;
}

void
Node::process(const char *line)
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
    last_port_note->note.append(line);
  }
  else
  {
    lines.Add(line);
    if (strncmp(line, "@heading ", 9) == 0)
      lines.Add("@iftex\n@donoderef()\n@end iftex\n");
  }
}

//-----------------------------------------------------------------------------

void
NodeSource::Message(const char *msg, const char *str, va_list arg) const
{
  char s[1024];

  vsprintf (s, str, arg);
  fprintf (stderr, "%s (file %s, node %s): %s\n", msg, filename.c_str(), name.c_str(), s);
}

void
NodeSource::Error(const char *str, ...) const
{
  va_list arg;

  va_start (arg, str);
  Message("Error", str, arg);
  va_end (arg);
}

void
NodeSource::Warning(const char *str, ...) const
{
  va_list arg;

  va_start (arg, str);
  Message("Warning", str, arg);
  va_end (arg);
}

template <typename N>
TreeNode<N>::TreeNode(const char *Pname, N *n)
{
  before = after = prev = next = NULL;
  name = Pname;
  sname = make_sname(Pname);
  node = n;
}

template <typename N>
TreeNode<N>::~TreeNode()
{
  delete before;
  delete after;
  free(sname);
  delete node;
}

TreeNode<void>::~TreeNode()
{
  delete before;
  delete after;
  free(sname);
}

template <typename N>
void
TreeNode<N>::Traverse(void (&tf)(const TreeNode &)) const
{
  if (before)
    before->Traverse(tf);
  tf(*this);
  if (after)
    after->Traverse(tf);
}

template <typename N>
int
TreeNode<N>::Compare(const char *sn) const
{
  return strcmp(sname, sn);
}

const char *TreeNode<Tree<void> >::up(void) const
{
  return "Functional Categories";
}

const char *TreeNode<Lines>::up(void) const
{
  return "Alphabetical List";
}

template <typename N>
void
Tree<N>::Add(TreeNode<N> &tn)
{
  TreeNode<N> **np = &nodes;
  while (*np)
  {
    if ((*np)->Compare(tn.sname) < 0)
    {
      tn.prev = *np;
      np = &((*np)->after);
    }
    else
    {
      tn.next = *np;
      np = &((*np)->before);
    }
  }
  if (tn.prev)
    tn.prev->next = &tn;
  if (tn.next)
    tn.next->prev = &tn;
  *np = &tn;
}

template <typename N>
TreeNode<N> &
Tree<N>::Find(const char *name)
{
  char *sname = make_sname(name);
  TreeNode<N> *tn = nodes;
  while (tn)
  {
    int c = tn->Compare(sname);
    if (c == 0)
    {
      free(sname);
      return *tn;
    }
    if (c > 0)
      tn = tn->before;
    else
      tn = tn->after;
  }
  free(sname);
  tn = new TreeNode<N>(name, new Tree<void>);
  Add(*tn);
  return *tn;
}

//-----------------------------------------------------------------------------

static FILE *co;

template <typename N>
void
TreeNode<N>::Pnode(void) const
{
  fprintf(co, "@c -----------------------------------------------------------------------------\n");
  fprintf(co, "@node %s, %s, %s, %s\n", name.c_str(),
	  next ? next->name.c_str() : "", prev ? prev->name.c_str() : "", up());
  fprintf(co, "@unnumberedsec %s\n", name.c_str());
}

template <typename N>
static void
print1(const TreeNode<N> &n)
{
  fprintf(co, "* %s::\n", n.name.c_str());
}

template <typename N>
void
Tree<N>::Print1(void) const
{
  fputs("@menu\n", co);
  Traverse(print1);
  fputs("@end menu\n", co);
}

void
Lines::Print1() const
{
  fputs(lines.c_str(), co);
}

template <typename N>
static void
print2(const TreeNode<N> &n)
{
  n.Pnode();
  n.node->Print1();
}

#ifndef D_OK
static int is_directory(const char *name)
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
#endif

//-----------------------------------------------------------------------------

static void scan_directory(const char *which)
{
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
      char *filename = strdup(buf);
      FILE *ci = fopen(buf, "r");
      if (!ci)
      {
	perror(buf);
	continue;
      }
      Node *curnode = NULL;
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
	  Lines &lines = *new Lines();
	  delete curnode;
	  curnode = new Node(lines, name, filename);
	  sprintf(buf, "@c From file %s\n", filename);
	  lines.Add(buf);
	  nodes.Add(*new TreeNode<Lines>(name, &lines));
	  categories.Find(cat).node->Add(*new TreeNode<void>(name, NULL));
	}
	else
	{
	  if (curnode)
	    curnode->process(buf);
	}
      }
      delete curnode;
      fclose(ci);
      free(filename);
    }
  }
  closedir(d);
}

//-----------------------------------------------------------------------------

static void list_portability (void)
{
  int i, j;
  char buffer[40];

  printf("Built-in portability targets:\n");

  for (i = 0; i < NUM_PORT_TARGETS; i++) {    
    const PortInfo &pti = port_target[i];
    if (pti.pq[0].suffix_token  == NULL) {
      printf("%-32s = %-32s\n",
	     pti.prefix_token,
	     pti.prefix_name);
    } else {
      for (j = 0; j < MAX_PORT_QUALIFIERS; j++) {
	const PortQualifier &pti_pqj = pti.pq[j];
	if (pti_pqj.suffix_token == NULL) break;

	strcpy(buffer, pti.prefix_token);
	strcat(buffer, "-");
	strcat(buffer, pti_pqj.suffix_token);

	printf("%-32s = ", buffer);

	strcpy(buffer, pti.prefix_name);
	strcat(buffer, ": ");
	strcat(buffer, pti_pqj.suffix_name);

	printf("%-32s\n", buffer);
      }
    }
  }
}

//-----------------------------------------------------------------------------

static void usage (void)
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
  categories.Print1();

  categories.Traverse(print2);

  // Alphabetical List
  fprintf(co, "@c -----------------------------------------------------------------------------\n");
  fprintf(co, "@node Alphabetical List, Unimplemented, Functional Categories, Top\n");
  fprintf(co, "@unnumbered Alphabetical List\n");
  fprintf(co, "\n");
  nodes.Print1();

  nodes.Traverse(print2);
  printf("%d nodes processed\n", Node::count_nodes);
  fclose(co);
  return 0;
}

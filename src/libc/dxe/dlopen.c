/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 Borca Daniel <dborca@yahoo.com>
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 * Partly based on work by Charles Sandmann and DJ Delorie.
 * Usage of this library is not restricted in any way.  
 * ABSOLUTELY no warranties.  Contributed to the DJGPP project.
 */

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dxe.h>
#include <sys/stat.h>

#ifdef __DJGPP__
#include <io.h>
#define ACCESS(f)  (_chmod(f, 0) != -1)
#define OPEN _open
#define READ _read
#define CLOSE _close
#define OPENFLAGS O_RDONLY
#define REALPATH _truename
#else
#include <unistd.h>
#define ACCESS(f)  (access(f, R_OK) == 0)
#define OPEN open
#define READ read
#define CLOSE close
#define OPENFLAGS O_RDONLY | O_BINARY
#define REALPATH realpath
#endif

#ifndef ELOOP
#define ELOOP EMLINK
#endif

#define DEBUG_DXE3 0	/* Prints struct __dxe_handle members.  Always commited as 0. */

/* private stack */
typedef struct __stk_node {
  const char *name;
  struct __stk_node *next;
} stk_node;

/* Exported symbols table entry */
typedef struct
{
  unsigned long offset;
  char name [1];		/* expanded as needed */
} __attribute__((packed)) exp_table_entry;

/* Unresolved symbols table entry */
typedef struct
{
  unsigned short n_rel_relocs;
  unsigned short n_abs_relocs;
  char name [1];		/* expanded as needed */
} __attribute__((packed)) unres_table_entry;

/* This is a linked list of implicitly loaded dxe modules.  */
typedef struct __llist {
  struct __dxe_handle *handle;	/* last implicitly opened module  */
  struct __llist *next;
} dxe_list;

/* This is the private dxe_h structure */
typedef struct __dxe_handle
{
  struct __dxe_handle *next;		/* Pointer to next module in chain */
  char fname[FILENAME_MAX];		/* Full module pathname */
  int mode;				/* Module open mode */
  int inuse;				/* In-use counter */
  int n_exp_syms;			/* Number of entries in export table */
  exp_table_entry **exp_table;		/* Exported symbols table */
  char *header;				/* The resident portion of header */
  char *section;			/* code+data+bss section */
  long _fini;				/* finalization */
  dxe_list *deps;			/* Linked list of implicitly open module by this module or NULL */
} dxe_handle, *dxe_h;

/* Last-resort symbol resolver */
void *(*_dlsymresolver) (const char *symname) = NULL;
/* Last-error unresolved symbol count */
int _dl_unresolved_count = 0;
/* Last-error unresolved symbol */
char _dl_unresolved_symbol[128];
/* The list of symbol tables */
dxe_symbol_table **_dl_symtabs = NULL;
/* Number of symbol table & max space allocated for symbol tables */
int _dl_nsymtab = 0, _dl_nmaxsymtab = 0;
/* The chained list of linked modules */
static dxe_h dxe_chain = NULL;

static
#ifdef __GNUC__
__attribute__((destructor))
#endif
void _closeall(void)
{
  while (dxe_chain)
    dlclose(dxe_chain);
}

void *dlopen(const char *filename, int mode)
{
  dxe3_header dxehdr;			/* The header of DXE module */
  dxe_h cur;				/* A module handle */
  dxe_handle dxe;			/* The module handle */
  int i, j, fh;				/* Miscelaneous variables */
  int hdrsize;				/* Resident header size */
  char *scan;				/* Work variable */
  char realfn[FILENAME_MAX];		/* Real module filename */
  char tempfn[FILENAME_MAX];		/* Temporary filename */
  int discardsize;
  char *discardable;

  stk_node *node;
  static stk_node *stk_top = NULL;

  dxe_list *deps;

#ifndef __GNUC__
  static int cleanup = 0;
#endif

  _dl_unresolved_count = 0;
  errno = 0;

  if (!filename || !*filename)
  {
    errno = EINVAL;
    return NULL;
  }

  /* Find the dynamic module along the LD_LIBRARY_PATH */
  if (!ACCESS(filename))
  {
    const char *env_names[] = {"LD_LIBRARY_PATH", "SYSTEM_DXE_PATH"};
    const int num_env_names = sizeof(env_names) / sizeof(*env_names);
    char *nextscan;
    size_t fnl = strlen(filename) + 1;
    /* LD_LIBRARY_PATH is scanned only for relative paths */
    if (filename[0] != '/' && filename[0] != '\\' && filename[1] != ':')
    {
      for (i = 0; i < num_env_names; i++)
      {
        for (scan = getenv(env_names[i]); scan && *scan;
             scan = nextscan + strspn(nextscan, "; \t"))
        {
          char *name;
          int name_len;
          nextscan = strchr(scan, ';');
          if (!nextscan) nextscan = strchr(scan, 0);
          name_len = nextscan - scan;
          if (name_len == 0)
            continue;
          if (nextscan[-1] == '/' || nextscan[-1] == '\\') name_len++;
          if (name_len + fnl > FILENAME_MAX)
            continue;
          memcpy(tempfn, scan, nextscan - scan);
          name = tempfn + (nextscan - scan);
          if (name [-1] != '/' && name [-1] != '\\')
            *name++ = '/';
          memcpy(name, filename, fnl);
          if (ACCESS(tempfn))
          {
            filename = tempfn;
            goto found;
          }
        }
      }
    }
    errno = ENOENT;
    return NULL;
  }
found:
  if (REALPATH(filename, realfn) == NULL)
    return NULL;

  /* First of all, look through the semi-loaded list */
  for (node = stk_top; node; node=node->next)
    if (!strcmp(node->name, realfn))
    {
      errno = ELOOP;
      return NULL;
    }
  
  /* Look through the loaded modules list */
  for (cur = dxe_chain; cur; cur = cur->next)
    if (!strcmp(realfn, cur->fname))
    {
      cur->inuse++;
      return cur;
    }

  fh = OPEN(filename, OPENFLAGS);
  if (fh < 0) return NULL;

  if (READ(fh, &dxehdr, sizeof(dxehdr)) != sizeof(dxehdr))
  {
    CLOSE(fh);
    return NULL;
  }

  if (dxehdr.magic != DXE_MAGIC || dxehdr.element_size != -1 || dxehdr.major > 1)
  {
    errno = ENOEXEC;
    CLOSE(fh);
    return NULL;
  }

  /* O.k, fill the module handle structure */
  strcpy(dxe.fname, realfn);
  dxe.inuse = 1;
  dxe.mode = mode;
  dxe.n_exp_syms = dxehdr.n_exp_syms;
  dxe.deps = NULL;

  /* Read DXE tables and the data section */
  hdrsize = dxehdr.symbol_offset - sizeof(dxehdr);
  discardsize = dxehdr.dep_size + dxehdr.unres_size + dxehdr.nrelocs * sizeof(long);
  if ((dxe.header = malloc(hdrsize + dxehdr.sec_size)) == NULL)
  {
    errno = ENOMEM;
    CLOSE(fh);
    return NULL;
  }
  if ((discardable = malloc(discardsize)) == NULL)
  {
    errno = ENOMEM;
    CLOSE(fh);
    goto midwayerror;
  }
  /* [dBorca]
   * the resident header and actual section share the same block
   */
  dxe.section = dxe.header + hdrsize;
  if ((READ(fh, dxe.header, hdrsize + dxehdr.sec_f_size) != (hdrsize + dxehdr.sec_f_size))
      || (READ(fh, discardable, discardsize) != discardsize))
  {
    CLOSE(fh);
    goto unrecoverable;
  }

  /* We don't need the file anymore */
  CLOSE(fh);

  /* Fill the unfilled portion of code+data+bss segment with zeros */
  memset(dxe.section + dxehdr.sec_f_size, 0, dxehdr.sec_size - dxehdr.sec_f_size);

  /* Load the dependencies */
  scan = discardable;
  for (deps = NULL, i = 0; i < dxehdr.n_deps; i++)
  {
    stk_node tmp;
    dxe_h dep_h;

    tmp.name = realfn;
    tmp.next = stk_top;
    stk_top = &tmp;

    if ((dep_h = dlopen(scan, RTLD_GLOBAL)) == NULL)
    {
      stk_top = tmp.next;
      goto unrecoverable;
    }
    else
    {
      dxe_list *next;

      stk_top = tmp.next;

      scan = strchr(scan, 0) + 1;

      /* Register all implicitly open modules by this one.  */
      if ((next = malloc(sizeof(dxe_list))) == NULL)
      {
        dlclose(dep_h);
        errno = ENOMEM;
        goto unrecoverable;
      }
      next->handle = dep_h;
      next->next = NULL;

      if (deps)
      {
        deps->next = next;
        deps = deps->next;
      }
      else
        dxe.deps = deps = next;
    }
  }

  /* Allright, now we're ready to resolve all unresolved symbols */
  _dl_unresolved_count = dxehdr.n_unres_syms;
  _dl_unresolved_symbol[0] = 0;
  for (i = 0; i < dxehdr.n_unres_syms; i++)
  {
    unres_table_entry *ute = (unres_table_entry *)scan;
    long offset = (long)(long *)dlsym(RTLD_DEFAULT, ute->name);

    if (offset)
    {
      /* Resolve all the references to this symbol */
      long *relocs = (long *)(strchr(ute->name, 0) + 1);
      for (j = 0; j < ute->n_rel_relocs; j++, relocs++)
      {
        char *fixaddr = dxe.section + *relocs;
        *(long *)fixaddr = offset - (long)fixaddr - sizeof(long);
      }
      for (j = 0; j < ute->n_abs_relocs; j++, relocs++)
        *(long *)(dxe.section + *relocs) += offset;
      _dl_unresolved_count--;
    }
    else if (_dl_unresolved_symbol[0] == 0)
      strcpy(_dl_unresolved_symbol, ute->name);

    scan = strchr(ute->name, 0) + 1 +
      sizeof(long) * (ute->n_rel_relocs + ute->n_abs_relocs);
  }

  /* Are there any unresolved symbols? */
  if (_dl_unresolved_count)
    goto unrecoverable;

  /* Apply relocations */
  for (i = 0; i < dxehdr.nrelocs; i++)
    *(long *)(dxe.section + ((long *)scan)[i]) += (long)dxe.section;

  /* And now discard the transient portion of header */
  free(discardable);

  /* Parse the header again and fill the exported names table */
  scan = dxe.header;
  if ((dxe.exp_table = malloc(dxehdr.n_exp_syms * sizeof(void *))) == NULL)
  {
    errno = ENOMEM;
    goto midwayerror;
  }
  for (i = 0; i < dxehdr.n_exp_syms; i++)
  {
    dxe.exp_table[i] = (exp_table_entry *)scan;
    scan = strchr(dxe.exp_table[i]->name, 0) + 1;
  }

  /* initialization */
  if (dxehdr._init != -1)
    ((void (*) (void))(dxehdr._init + (long)dxe.section))();
  dxe._fini = dxehdr._fini;

  /* Put the dxe module in loaded modules chain */
  dxe.next = dxe_chain;
  if ((dxe_chain = malloc(sizeof(dxe_handle))) == NULL)
  {
    free(dxe.exp_table);
    errno = ENOMEM;
    goto midwayerror;
  }
  memcpy(dxe_chain, &dxe, sizeof(dxe_handle));

#if (DEBUG_DXE3 -0) == 1
  {
    FILE *f = fopen("c:/tmp/dxe_chain.txt", "a");

    if (f)
    {
      fprintf(f, "dxe_chain                    : 0x%p\n"
                 "  next                       : 0x%p\n"
                 "  fname                      : %s\n"
                 "  mode                       : %s\n"
                 "  inuse                      : %d\n"
                 "  n_exp_syms                 : %d\n"
                 "  exp_table                  : 0x%p\n",
                 dxe_chain, dxe_chain->next, dxe_chain->fname,
                 dxe_chain->mode == RTLD_LAZY ? "RTLD_LAZY" :
                 dxe_chain->mode == RTLD_NOW ? "RTLD_NOW" :
                 dxe_chain->mode == RTLD_LOCAL ? "RTLD_LOCAL" :
                 dxe_chain->mode == RTLD_GLOBAL ? "RTLD_GLOBAL" : "unknown",
                 dxe_chain->inuse, dxe_chain->n_exp_syms, dxe_chain->exp_table);
      for (i = 0; i < dxe_chain->n_exp_syms; i++)
        fprintf(f, "    exp_table[%d]->offset     : %ld\n"
                   "    exp_table[%d]->name       : %s\n",
                   i, dxe_chain->exp_table[i]->offset, i, dxe_chain->exp_table[i]->name);
      fprintf(f, "  header                     : 0x%p\n"
                 "  section                    : 0x%p\n"
                 "  _fini                      : %ld\n",
                 dxe_chain->header, dxe_chain->section, dxe_chain->_fini);
      if ((deps = dxe_chain->deps))
        for (; deps; deps = deps->next)
          fprintf(f, "  deps                       : 0x%p\n"
                     "    handle                   : 0x%p\n"
                     "    handle->fname            : %s\n\n",
                     deps, deps->handle, deps->handle->fname);
      else
        fprintf(f, "  deps                       : 0x00000000\n\n");
      fclose(f);
    }
  }
#endif

#ifndef __GNUC__
  if (!cleanup)
  {
    cleanup = !0;
    atexit(_closeall);
  }
#endif

  return dxe_chain;

unrecoverable:
  free(discardable);
midwayerror:
  free(dxe.header);

  deps = dxe.deps;
  while (deps)
  {
    dxe_list *next = deps->next;

    dlclose(deps->handle);
    free(deps);
    deps = next;
  }

  return NULL;
}

int dlclose(void *dxe)
{
  if (!dxe)
    return -1;

  if (--((dxe_h)dxe)->inuse)
    return 0;

  /* finalization */
  if (((dxe_h)dxe)->_fini != -1)
    ((void (*) (void))(((dxe_h)dxe)->_fini + (long)((dxe_h)dxe)->section))();

  /* Remove the module from the list of loaded DXE modules */
  {
    dxe_h *cur;
    for (cur = &dxe_chain; *cur; cur = &(*cur)->next)
      if (*cur == dxe)
      {
        *cur = ((dxe_h)dxe)->next;
        break;
      }
  }

  /* remove all implicitly loaded modules by this module.  */
  {
    dxe_list *deps = ((dxe_h)dxe)->deps;
    while (deps)
    {
      dxe_list *next = deps->next;

      dlclose(deps->handle);
      free(deps);
      deps = next;
    }
  }

  free(((dxe_h)dxe)->header);
  free(((dxe_h)dxe)->exp_table);
  free(dxe);

  return 0;
}

void *dlsym(void *dxe, const char *symname)
{
  int i, j;

  if (dxe == RTLD_DEFAULT)
  {
    void *sym = 0;
    dxe_h cur;

    for (i = 0; (i < _dl_nsymtab) && !sym; i++)
    {
      dxe_symbol_table *table = _dl_symtabs[i];
      for (j = 0; table[j].name != 0; j++)
        if (!strcmp(symname, table[j].name))
        {
          sym = table[j].offset;
          break;
        }
    }

    for (cur = dxe_chain; cur; cur = cur->next)
      if (cur->mode & RTLD_GLOBAL)
        for (i = 0; i < cur->n_exp_syms; i++)
          if (!strcmp(symname, cur->exp_table[i]->name))
          {
            sym = cur->section + cur->exp_table[i]->offset;
            goto modscan_done;
          }
modscan_done:

    if (!sym && _dlsymresolver)
      sym = _dlsymresolver(symname);

    return sym;
  }
  else
    for (i = 0; i < ((dxe_h)dxe)->n_exp_syms; i++)
      if (!strcmp(((dxe_h)dxe)->exp_table[i]->name, symname))
        return ((dxe_h)dxe)->section + ((dxe_h)dxe)->exp_table[i]->offset;

  return NULL;
}

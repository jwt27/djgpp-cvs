/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 Borca Daniel <dborca@yahoo.com>
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 * Partly based on work by Charles Sandmann and DJ Delorie.
 * Usage of this library is not restricted in any way.  
 * ABSOLUTELY no warranties.  Contributed to the DJGPP project.
 */

/*
    Program exit codes:
       0: o.k.
      -1: wrong command line
      -2: i/o error
      -3: unresolved symbols
      otherwise the exit code of GNU ld is returned
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DXE_LD			/* Cross compile ld name/location */
#define DXE_LD "ld"
#else
/* Linux violates POSIX.1 and defines this, but it shouldn't.  We fix it. */
#undef _POSIX_SOURCE
#endif

/* Always include local copies of sys/dxe.h and coff.h because of
 * NATIVE_TYPES stuff. */
#include "../../include/sys/dxe.h"
#include "../../include/coff.h"

#define VERSION "1.0.1"

#define TEMP_BASE	"dxe_tmp"	/* 7 chars, 1 char suffix */
#define TEMP_O_FILE	TEMP_BASE".o"
#define TEMP_S_FILE	TEMP_BASE".s"

#define VALID_RELOC(r) ((r.r_type != RELOC_REL32) && (r.r_symndx != (ULONG32)-1))

typedef enum {
        FALSE = 0,
        TRUE = !FALSE
} BOOL;



static const char *progname;

static
#include "fini1.h"
static
#include "fini2.h"
static
#include "fini3.h"

static
#include "init1.h"
static
#include "init2.h"
static
#include "init3.h"

static struct {
       void *data;
       int size;
} inits[3] = {
       {init1, sizeof(init1)},
       {init2, sizeof(init2)},
       {init3, sizeof(init3)}
}, finis[3] = {
       {fini1, sizeof(fini1)},
       {fini2, sizeof(fini2)},
       {fini3, sizeof(fini3)}
};

/* Command-line options */
static struct {
  BOOL legacy;			/* legacy DXE1 */
  BOOL unresolved;		/* allow unresolved symbols in output */
  BOOL autoresolve;		/* imbed the resolution table into implib */
  BOOL verbose;			/* verbose output */
  BOOL showdep;			/* show dependencies */
  BOOL showexp;			/* show exported symbols */
  BOOL showunres;		/* show unresolved symbols */
  char *output;			/* output file name */
  int objcount;			/* number of object files given on command line */
  char *implib;			/* name of import library */
  char *dxefile;		/* the name of dxe file on command line */
  char *description;		/* a description of the module */
  unsigned num_prefix;		/* number of exported prefixes */
  unsigned max_prefix;		/* maximal number of exported prefixes */
  char **export_prefix;		/* exported symbol prefixes */
  unsigned num_excl;		/* number of excluded prefixes */
  unsigned max_excl;		/* maximal number of excluded prefixes */
  char **excl_prefix;		/* excluded symbol prefixes */
  unsigned num_deps;		/* number of deps */
  unsigned max_deps;		/* maximal number of deps */
  char **deps;			/* deps */
} opt = {
  FALSE,
  FALSE,
  FALSE,
  FALSE,
  FALSE,
  FALSE,
  FALSE,
  NULL,
  0,
  NULL,
  NULL,
  NULL,
  0,
  0,
  NULL,
  0,
  0,
  NULL,
  0,
  0,
  NULL
};



/* Desc: remove temporary files
 *
 * In  : -
 * Out : -
 *
 * Note: this function is called upon exit
 */
static void exit_cleanup (void)
{
 remove(TEMP_O_FILE);
 remove(TEMP_S_FILE);
}



/* Desc: return the filespec's base name (w/o extension)
 *
 * In  : filespec
 * Out : static string
 *
 * Note: -
 */
static char *base_name (const char *path)
{
 char *scan;
 static char p[FILENAME_MAX];

 scan = strchr(path, 0);
 while ((scan > path) && (scan[-1] != '/') && (scan[-1] != '\\') && (scan[-1] != ':')) {
       scan--;
 }

 scan = strcpy(p, scan);

 while (*scan == '.') {
       scan++;
 }

 if ((scan = strrchr(scan, '.')) != NULL) {
    *scan = 0;
 }

 return p;
}



/* Desc: display program version
 *
 * In  : -
 * Out : -
 *
 * Note: -
 */
static void display_version (void)
{
 printf("dxe3gen version " VERSION "\n");
}



/* Desc: display help
 *
 * In  : -
 * Out : -
 *
 * Note: -
 */
static void display_help (void)
{
 display_version();
 printf("Usage: dxe3gen [-o output.dxe] [options] [object-files] [ld-options]\n");
 printf("   or: dxe3gen -1 output.dxe symbol input.o [input2.o ... -lgcc -lc]\n");
 printf("Create a dynamically-loadable executable module for DJGPP\n\n");
 printf("-o output.dxe\tDefine the name of output DXE file\n");
 printf("-P module.dxe\tSpecify dependency module (cumulative)\n");
 printf("-I import.a\tCreate an import library for given DXE file\n");
 printf("-Y import.a\tCreate an autoresolved import library for given DXE file\n");
 printf("-D description\tSet module description string\n");
 printf("-E prefix\tExport only symbols that start with <prefix> (cumulative)\n");
 printf("-X prefix\tExclude symbols that start with <prefix> (cumulative)\n");
 printf("-U\t\tAllow unresolved symbols in DXE file\n");
 printf("-V\t\tVerbose output (minimal output by default)\n");
 printf("--show-dep\tShow dependencies for specified module\n");
 printf("--show-exp\tShow symbols exported by the DXE module\n");
 printf("--show-unres\tShow unresolved symbols in the DXE module\n");
 printf("[ld-options]\tAny other options are passed unchanged to ld\n\n");
 printf("-1\t\tSwitch into legacy mode (disables all other options)\n\n");
 printf("You should provide appropriate environment at load-time for unresolved modules.\n");
 exit(-1);
}



/* Desc: process command line args
 *
 * In  : no of arguments, argument list, ptr to store linker args
 * Out : -
 *
 * Note: -
 */
static void process_args (int argc, char *argv[], const char *new_argv[])
{
 static char libdir[FILENAME_MAX];
 char *p;
 int i;
 int new_argc = 10;

 p = getenv("DXE_LD_LIBRARY_PATH");
 if (p)
    strcpy(libdir, p);
 else {
    p = getenv("DJDIR");
    if (p) {
       strcpy(libdir, p);
       strcat(libdir, "/lib");
    } else {
       fprintf(stderr, "Error: neither DXE_LD_LIBRARY_PATH nor DJDIR are set in environment\n");
       exit(1);
    }
 }

 new_argv[0] = DXE_LD;
 new_argv[1] = "-X";
 new_argv[2] = "-S";
 new_argv[3] = "-r";
 new_argv[4] = "-o";
 new_argv[5] = TEMP_O_FILE;
 new_argv[6] = "-L";
 new_argv[7] = libdir;
 new_argv[8] = "-T";
 new_argv[9] = "dxe.ld";

 if (!strcmp(base_name(argv[0]), "dxegen")) {
    /* invoked as `dxegen' */
    opt.legacy = TRUE;
 } else if ((argc > 1) && !strcmp(argv[1], "-1")) {
    /* invoked as `dxe3gen -1' */
    opt.legacy = TRUE;
    argc--;
    argv++;
 }
 if (opt.legacy) {
    /* legacy mode */
    progname = "dxegen";
    if (argc < 4) {
       printf("Usage: %s output.dxe symbol input.o [input2.o ... -lgcc -lc]\n", progname);
       exit(-1);
    }
    opt.max_prefix = 16;
    opt.export_prefix = (char **)malloc(opt.max_prefix * sizeof(char *));
    opt.num_prefix = 1;
    opt.output = argv[1];
    opt.export_prefix[0] = argv[2];
    for (i = 3; i < argc; i++) {
        char *dot;
        new_argv[new_argc++] = argv[i];
        if ((dot = strrchr(argv[i], '.'))) {
           if (!strcasecmp(dot, ".o") || !strcasecmp(dot, ".a")) {
              opt.objcount++;
           }
        }
    }
 } else {
    /* standard mode */
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
           display_help();
        } else if (!strcmp(argv[i], "-U")) {
           opt.unresolved = TRUE;
        } else if (!strcmp(argv[i], "-o")) {
           opt.output = argv[++i];
        } else if (!strcmp(argv[i], "-V")) {
           opt.verbose = TRUE;
        } else if (!strcmp(argv[i], "-D")) {
           opt.description = argv[++i];
        } else if (!strcmp(argv[i], "-I")) {
           opt.implib = argv[++i];
        } else if (!strcmp(argv[i], "-Y")) {
           opt.implib = argv[++i];
           opt.autoresolve = TRUE;
        } else if (!strcmp(argv[i], "-E")) {
           if (opt.num_prefix >= opt.max_prefix) {
              opt.max_prefix += 16;
              opt.export_prefix = (char **)realloc(opt.export_prefix, opt.max_prefix * sizeof(char *));
           }
           opt.export_prefix[opt.num_prefix++] = argv[++i];
        } else if (!strcmp(argv[i], "-X")) {
           if (opt.num_excl >= opt.max_excl) {
              opt.max_excl += 16;
              opt.excl_prefix = (char **)realloc(opt.excl_prefix, opt.max_excl * sizeof(char *));
           }
           opt.excl_prefix[opt.num_excl++] = argv[++i];
        } else if (!strcmp(argv[i], "-P")) {
           if (opt.num_deps >= opt.max_deps) {
              opt.max_deps += 16;
              opt.deps = (char **)realloc(opt.deps, opt.max_deps * sizeof(char *));
           }
           opt.deps[opt.num_deps++] = argv[++i];
        } else if (!strcmp(argv[i], "--show-dep")) {
           opt.showdep = TRUE;
        } else if (!strcmp(argv[i], "--show-exp")) {
           opt.showexp = TRUE;
        } else if (!strcmp(argv[i], "--show-unres")) {
           opt.showunres = TRUE;
        } else {
           char *dot;
           new_argv[new_argc++] = argv[i];
           dot = strrchr(argv[i], '.');
           if (dot) {
              if (!strcasecmp(dot, ".o") || !strcasecmp(dot, ".a")) {
                 opt.objcount++;
              } else if (!strcasecmp(dot, ".dxe")) {
                 opt.dxefile = argv[i];
              }
           }
        }
    }
 }
 new_argv[new_argc] = NULL;
}

static int myspawn(const char **argv)
{
  char cmd[65536];
  strcpy(cmd, argv[0]);
  argv++;
  while (argv[0]) {
    strcat(cmd, " ");
    strcat(cmd, argv[0]);
    argv++;
  };
  return system(cmd);
}

/* Desc: run linker to obtain relocatable output
 *
 * In  : linker command line, ptr to store output header
 * Out : file ptr of resulting file
 *
 * Note: -
 */
static FILE *run_ld (const char *argv[], FILHDR *fh)
{
 int rv;
 FILE *f;

 if ((rv = myspawn(argv)) != 0) {
    if (rv == -1) {
       perror(DXE_LD);
    }
    exit(rv);
 }

 if ((f = fopen(TEMP_O_FILE, "rb")) == NULL) {
    fprintf(stderr, "%s: cannot open linker output file `%s'\n", progname, TEMP_O_FILE);
    exit(-2);
 } else {
    atexit(exit_cleanup);
 }

 fread(fh, 1, FILHSZ, f);
 if (fh->f_nscns != 1) {
    fclose(f);
    fprintf(stderr, "%s: linker output file has more than one section\n", TEMP_O_FILE);
    exit(-2);
 }

 if (!opt.legacy) {
    unsigned i;
    ULONG32 stsz;
    SYMENT *sym;
    char *strings;
    long frame_begin = -1, frame_end = -1;
    long first_ctor = -1, last_ctor = -1, first_dtor = -1, last_dtor = -1;

    int init, fini;

    /* Read all symbols */
    sym = (SYMENT *)malloc(fh->f_nsyms * sizeof(SYMENT));
    fseek(f, fh->f_symptr, SEEK_SET);
    fread(sym, fh->f_nsyms, SYMESZ, f);

    /* Read symbol name table */
    fread(&stsz, 1, sizeof(stsz), f);
    strings = (char *)malloc(stsz);
    fread(strings + 4, 1, stsz - 4, f);
    strings[0] = 0;

    for (i = 0; i < fh->f_nsyms; i += 1 + sym[i].e_numaux) {
        char tmp[E_SYMNMLEN + 1], *name;

        if (sym[i].e.e.e_zeroes) {
           memcpy(tmp, sym[i].e.e_name, E_SYMNMLEN);
           tmp[E_SYMNMLEN] = 0;
           name = tmp;
        } else {
           name = strings + sym[i].e.e.e_offset;
        }

        if (!strcmp(name, "___EH_FRAME_BEGIN__")) {
           frame_begin = sym[i].e_value;
        } else if (!strcmp(name, "___EH_FRAME_END__")) {
           frame_end = sym[i].e_value;
        } else if (!strcmp(name, "djgpp_first_ctor")) {
           first_ctor = sym[i].e_value;
        } else if (!strcmp(name, "djgpp_last_ctor")) {
           last_ctor = sym[i].e_value;
        } else if (!strcmp(name, "djgpp_first_dtor")) {
           first_dtor = sym[i].e_value;
        } else if (!strcmp(name, "djgpp_last_dtor")) {
           last_dtor = sym[i].e_value;
        }
    }

    /* 3 = ctor + frame
     * 2 = frame
     * 1 = ctor
     * 0 = <none>
     */
    if (frame_begin != frame_end) {
       init = (first_ctor != last_ctor) ? 3 : 2;
       fini = (first_dtor != last_dtor) ? 3 : 2;
    } else {
       init = (first_ctor != last_ctor) ? 1 : 0;
       fini = (first_dtor != last_dtor) ? 1 : 0;
    }

    if ((init > 0) || (fini > 0)) {
       fclose(f);

       for (i = 0; argv[i] != NULL; i++)
         ;

       if (init > 0) {
          f = fopen(TEMP_BASE"i.o", "wb");
          fwrite(inits[init - 1].data, inits[init - 1].size, 1, f);
          fclose(f);
          argv[i++] = TEMP_BASE"i.o";
       }
       if (fini > 0) {
          f = fopen(TEMP_BASE"f.o", "wb");
          fwrite(finis[fini - 1].data, finis[fini - 1].size, 1, f);
          fclose(f);
          argv[i++] = TEMP_BASE"f.o";
       }
       argv[i] = NULL;

       rv = myspawn(argv);

       if (init > 0) {
          remove(TEMP_BASE"i.o");
       }
       if (fini > 0) {
          remove(TEMP_BASE"f.o");
       }

       if (rv) {
          if (rv == -1) {
             perror(DXE_LD);
          }
          exit(rv);
       }

       f = fopen(TEMP_O_FILE, "rb");
       fread(fh, 1, FILHSZ, f);
    } else {
       fseek(f, FILHSZ, SEEK_SET);
    }
 }

 return f;
}



/* Desc: write DXE
 *
 * In  : input handle, output handle, input file header
 * Out : 0 if success
 *
 * Note: -
 */
static int write_dxe (FILE *inf, FILE *outf, FILHDR *fh)
{
 dxe3_header dh;
 unsigned int startbss;
 char *data;
 SYMENT *sym;
 ULONG32 stsz;
 char *strings;
 RELOC *relocs;
 unsigned i, j, errcount;
 size_t hdrsize;

 /* Exported symbols table */
 char *expsym_table = NULL;
 size_t expsym_size = 0, expsym_maxsize = 0;
 /* Unresolved symbols table */
 char *unres_table = NULL;
 size_t unres_size = 0, unres_maxsize = 0;

 /* Get the section header */
 SCNHDR sc;
 fseek(inf, fh->f_opthdr, SEEK_CUR);
 fread(&sc, 1, SCNHSZ, inf);

 dh.magic = DXE_MAGIC;
 dh.element_size = -1;
 dh.nrelocs = sc.s_nreloc;
 dh.n_exp_syms = 0;
 dh.exp_table = sizeof(dh);
 dh.n_deps = opt.num_deps;
 dh.n_unres_syms = 0;
 dh.sec_size = sc.s_size;
 dh.flags = 0;
 dh.major = 1;
 dh.minor = 0;
 dh.hdr_size = sizeof(dh);
 dh._init = dh._fini = -1;

 /* keep track of the BSS start */
 startbss = dh.sec_size;

 /* Read section data */
 data = (char *)malloc(sc.s_size);
 fseek(inf, sc.s_scnptr, 0);
 fread(data, 1, sc.s_size, inf);

 /* Read all symbols */
 sym = (SYMENT *)malloc(fh->f_nsyms * sizeof(SYMENT));
 fseek(inf, fh->f_symptr, SEEK_SET);
 fread(sym, fh->f_nsyms, SYMESZ, inf);

 /* Read symbol name table */
 fread(&stsz, 1, sizeof(stsz), inf);
 strings = (char *)malloc(stsz);
 fread(strings + 4, 1, stsz - 4, inf);
 strings[0] = 0;

 /* Read the relocation table */
 relocs = (RELOC *)malloc(sc.s_nreloc * sizeof(RELOC));
 fseek(inf, sc.s_relptr, SEEK_SET);
 fread(relocs, sc.s_nreloc, RELSZ, inf);

 /* Close input file */
 fclose(inf);

 errcount = 0;

 for (i = 0; i < fh->f_nsyms; i += 1 + sym[i].e_numaux) {
     char tmp[E_SYMNMLEN + 1], *name;
     int namelen;
     size_t newsize;

     if (sym[i].e.e.e_zeroes) {
        memcpy(tmp, sym[i].e.e_name, E_SYMNMLEN);
        tmp[E_SYMNMLEN] = 0;
        name = tmp;
     } else {
        name = strings + sym[i].e.e.e_offset;
     }
     namelen = strlen(name) + 1;

     /* update BSS start */
     if (!strcmp(name, ".bss")) {
        if (startbss > sym[i].e_value) {
           startbss = sym[i].e_value;
        }
     }

     /* sanity check */
     if (namelen <= 1) {
       continue;
     }

#if 0
     printf("[%3d] 0x%08lx 0x%08x 0x%04x %d %s\n",
            i,
            sym[i].e_value,
            sym[i].e_scnum,
            sym[i].e_sclass,
            sym[i].e_numaux,
            name
           );
#endif

     /* do not process private symbols */
     if (sym[i].e_sclass == C_STAT) {
        /* usually, private symbols are discarded anyway.
         * This is because we end up with neither relative,
         * nor absolute relocs pointing to them.  However,
         * C++ causes some trouble, apparently related to
         * weak symbols.  They will have only one absolute
         * reloc to themselves.
         */
     } else if (sym[i].e_scnum == 0) {
        short *count;
        LONG32 *rel_relocs, *abs_relocs;
        int n_abs_relocs = 0, n_rel_relocs = 0;

        /* count the amount of relocations pointing to this symbol */
        for (j = 0; j < sc.s_nreloc; j++) {
            if (relocs[j].r_symndx == i) {
               if (relocs[j].r_type == RELOC_REL32) {
                  n_rel_relocs++;
               } else {
                  n_abs_relocs++;
               }
            }
        }

        /* If there are no references to this symbol, skip it */
        if (n_rel_relocs == 0 && n_abs_relocs == 0) {
           continue;
        }

        /* unresolved symbol */
        dh.n_unres_syms++;

        if (!opt.unresolved) {
           fprintf(stderr, "%s: unresolved symbol `%s'\n", progname, name);
           errcount++;
           continue;
        }

        if (n_rel_relocs > 0xffff || n_abs_relocs > 0xffff) {
           fprintf(stderr, "%s: FATAL ERROR: too many relocations for unresolved"
                            " symbol `%s'\n", progname, name);
           fclose(outf);
           remove(opt.output);
           exit(-4);
        }

        newsize = unres_size + 2 * sizeof(short) + namelen + (n_rel_relocs + n_abs_relocs) * sizeof(LONG32);
        if (newsize > unres_maxsize) {
           int addlen=newsize - unres_maxsize + 317;
           if (!((addlen+unres_maxsize) & 1)) {
              addlen++;
           }
           unres_table = (char *)realloc(unres_table, unres_maxsize += addlen);
           /* in the original code below djgpp 2.03 jumped the gun if the size
            * is simply incremented by 1024
            * unres_table = (char *)realloc(unres_table, unres_maxsize += 1024);
            */
        }
        memcpy(unres_table + unres_size + 2 * sizeof(short), name, namelen);

        /* Store number of references to this unresolved symbol */
        count = (short *)(unres_table + unres_size);
        count[0] = n_rel_relocs;
        count[1] = n_abs_relocs;

        rel_relocs = (LONG32 *)(unres_table + unres_size + 2 * sizeof(short) + namelen);
        abs_relocs = (LONG32 *)(unres_table + newsize);

        unres_size = newsize;

        for (j = 0; j < sc.s_nreloc; j++) {
            if (relocs[j].r_symndx == i) {
               /* mark the relocation as processed */
               relocs[j].r_symndx = (ULONG32)-1;

               if (relocs[j].r_type == RELOC_REL32) {
                  *rel_relocs++ = relocs[j].r_vaddr;
               } else {
                  *--abs_relocs = relocs[j].r_vaddr;
               }
            }
        }
        if (opt.verbose) {
           printf("unresolved: `%s' (%d references)\n", name, n_rel_relocs + n_abs_relocs);
        }
     } else if (sym[i].e_sclass == C_EXT) {
        if (!strcmp(name, "_init")) {
           dh._init = sym[i].e_value;
           continue;
        }
        if (!strcmp(name, "_fini")) {
           dh._fini = sym[i].e_value;
           continue;
        }

        if (!strncmp(name, "__GLOBAL_$", 10)
            || !strcmp(name, "djgpp_first_ctor")
            || !strcmp(name, "djgpp_last_ctor")
            || !strcmp(name, "djgpp_first_dtor")
            || !strcmp(name, "djgpp_last_dtor")
            || !strcmp(name, "___EH_FRAME_BEGIN__")
            || !strcmp(name, "___EH_FRAME_END__")
           ) {
           continue;
        }

        if (opt.num_excl) {
           BOOL ok = FALSE;
           for (j = 0; j < opt.num_excl; j++) {
               if (memcmp(opt.excl_prefix[j], name, strlen(opt.excl_prefix[j])) == 0) {
                  ok = TRUE;
                  break;
               }
           }
           if (ok) {
              continue;
           }
        }
     
        if (opt.num_prefix) {
           BOOL ok = FALSE;
           for (j = 0; j < opt.num_prefix; j++) {
               if (memcmp(opt.export_prefix[j], name, strlen(opt.export_prefix[j])) == 0) {
                  ok = TRUE;
                  break;
               }
           }
           if (!ok) {
              continue;
           }
        }

        /* exported symbol */
        dh.n_exp_syms++;

        if (opt.legacy && (dh.n_exp_syms != 1)) {
           fprintf(stderr, "%s: multiple symbols with same prefix: `%s'", progname, name);
           errcount++;
        }

        newsize = expsym_size + sizeof(LONG32) + namelen;
        if (newsize > expsym_maxsize) {
           expsym_table = (char *)realloc(expsym_table, expsym_maxsize += 1024);
        }

        *(LONG32 *)(expsym_table + expsym_size) = sym[i].e_value;
        memcpy(expsym_table + expsym_size + sizeof(LONG32), name, namelen);
        expsym_size = newsize;
        if (opt.verbose) {
           printf("export: `%s'\n", name);
        }
    }
 }

 if (errcount) {
    fclose(outf);
    remove(opt.output);
    exit(-3);
 }

 /* Compute the amount of valid relocations */
 for (i = 0; i < sc.s_nreloc; i++) {
#if 0
     printf("[%3d] %08lX %03ld %04X\n",
            i,
            relocs[i].r_vaddr,
            relocs[i].r_symndx,
            relocs[i].r_type);
#endif
     if (!VALID_RELOC(relocs[i])) {
        dh.nrelocs--;
     }
 }

 if (opt.legacy) {
    if (dh.n_exp_syms != 1) {
       fprintf(stderr, "%s: symbol `%s' not found\n", progname, opt.export_prefix[0]);
       fclose(outf);
       remove(opt.output);
       exit(-4);
    }
    dh.element_size = dh.sec_size;
    dh.symbol_offset = *(LONG32 *)expsym_table;
    memset(data + startbss, 0, dh.sec_size - startbss);
    fwrite(&dh, 1, sizeof(dxe_header), outf);
    /* Write the actual code+data+bss section */
    fwrite(data, 1, dh.sec_size, outf);
 } else {
    /* A small array for padding with zeros */
    char fill[16];
    memset(fill, 0, 16);

    /* [dBorca]
     * when the linker injects BSS space into TEXT, it fills that area with
     * garbage. To ensure BSS will get zeroed, set the end of the in-file
     * section as the start of the BSS (according to dxe.ld, nothing beyond
     * this point is initialized). The rest is up to `dlopen()'
     */
    dh.sec_f_size = startbss;

    /* Compute table sizes */
    dh.exp_size = expsym_size;
    dh.unres_size = unres_size;
    dh.dep_size = 0;
    for (i=0; i<opt.num_deps; i++) {
        dh.dep_size += strlen(opt.deps[i]) + 1;
    }
    /* Compute the offset of .text+.data+.bss sections */
    hdrsize = dh.exp_table + expsym_size;
    dh.symbol_offset = (hdrsize + 15) & ~15UL;
    /* Compute the offset of tables */
    dh.dep_table = dh.symbol_offset + dh.sec_f_size;
    dh.unres_table = dh.dep_table + dh.dep_size;
    dh.reloc_table = dh.unres_table + dh.unres_size;

    /* Output the DXE header */
    fwrite(&dh, 1, sizeof(dh), outf);

    /* Output the exported symbols table */
    fwrite(expsym_table, 1, expsym_size, outf);
    free(expsym_table);

    /* Write the actual code+data+bss section */
    fwrite(fill, 1, dh.symbol_offset - hdrsize, outf);
    fwrite(data, 1, dh.sec_f_size, outf);

    /* Output the dependency table */
    for (i = 0; i < opt.num_deps; i++) {
        fwrite(opt.deps[i], 1, strlen(opt.deps[i]) + 1, outf);
    }

    /* Output the unresolved symbols table */
    fwrite(unres_table, 1, unres_size, outf);
    free(unres_table);
 }

 free(data);

 /* Output the relocations */
 for (i = 0; i < sc.s_nreloc; i++) {
     if (VALID_RELOC(relocs[i])) {
        fwrite(&relocs[i].r_vaddr, 1, sizeof(relocs[0].r_vaddr), outf);
     }
 }

 /* If we have a description string, put it here */
 if (opt.description != NULL) {
    fwrite(opt.description, 1, strlen(opt.description) + 1, outf);
 }

 fclose(outf);

 free(strings);
 free(relocs);
 free(sym);

 return 0;
}



/* Desc: open existing DXE3 file
 *
 * In  : filename, ptr to store DXE3 header
 * Out : file handle
 *
 * Note: does not work with legacy DXE
 */
static FILE *open_dxe_file (const char *fname, dxe3_header *dh)
{
 FILE *f;

 if ((f = fopen(fname, "rb")) == NULL) {
    fprintf(stderr, "%s: cannot read DXE module `%s'\n", progname, fname);
    exit(-2);
 }

 fread(dh, 1, sizeof(dxe3_header), f);
 if ((dh->magic != DXE_MAGIC) || (dh->element_size != -1)) {
    fclose(f);
    fprintf(stderr, "%s: the file `%s' is not an extended DXE module\n", progname, fname);
    exit(-2);
 }

 return f;
}



/* Desc: create import library
 *
 * In  : -
 * Out : 0 if success
 *
 * Note: -
 */
static int make_implib (void)
{
 int i;
 int rv;
 char *scan;
 const char *omode;
 char *symtab;
 char basename_fix[FILENAME_MAX]; /* changed size - chan kar heng 20030112 */
 char basename_org[FILENAME_MAX];
 dxe3_header dh;
 char cmdbuf[FILENAME_MAX+100];
 FILE *implib, *f = open_dxe_file(opt.dxefile, &dh);

 fseek(f, dh.exp_table, SEEK_SET);
 symtab = (char *)malloc(dh.exp_size);
 fread(symtab, 1, dh.exp_size, f);
 fclose(f);

 /* Compute the base name of the library */
 scan = strcpy(basename_fix, strcpy(basename_org, base_name(opt.dxefile)));
 while (*scan) {
     char c = *scan;
     /* Convert illegal chars to underscore */
     *scan++ = isalnum(c) ? toupper(c) : '_';
 }

 if (opt.autoresolve) {
    /* Fire the resolver. It should take care of the dependencies (if any) */
    strcpy(cmdbuf, "dxe3res -o "TEMP_BASE".c ");
    strcat(cmdbuf, opt.dxefile);
    if ((rv = system(cmdbuf)) != 0) {
       if (rv == -1) {
          perror("dxe3res");
       }
       exit(rv);
    }

    /* Pre-compile the resolver's output. */
    rv = system("gcc -o "TEMP_S_FILE" -O2 -S "TEMP_BASE".c");
    remove(TEMP_BASE".c");
    if (rv != 0) {
       if (rv == -1) {
          perror("gcc");
       }
       exit(rv);
    }

    /* [dBorca]
     * the reason we are doing it in one single file is to make sure the
     * resolution table resides in the very same source with the wrappers,
     * otherwise the linker might get smart... and get rid of it!
     */
    omode = "a";
 } else {
    omode = "w";
 }

 /* `omode' holds the fopen mode */
 if ((implib = fopen(TEMP_S_FILE, omode)) == NULL) {
    free(symtab);
    fprintf(stderr, "%s: cannot open file `%s' for writing\n", progname, TEMP_S_FILE);
    exit(-2);
 }

 fprintf(implib,
       ".text\n"
       "Lmodh:	.long	0\n"
       "Lmodn:	.ascii	\"%s.DXE\\0\"\n"
       "	.globl	_dlload_%s\n"
       "_dlload_%s:\n"
       "	pushl	$Lsyms\n"		/* symbol names */
       "	pushl	$Lstubs\n"		/* stubs */
       "	pushl	$Lmodh\n"		/* module handle */
       "	pushl	$Lmodn\n"		/* module name */
       "	call	_dlstatbind\n"		/* statically bind */
       "	addl	$16,%%esp\n"
       "	ret\n"
       "	.globl	_dlunload_%s\n"
       "_dlunload_%s:\n"
       "	pushl	$Lload\n"		/* loader address */
       "	pushl	$Lsyms\n"		/* symbol names */
       "	pushl	$Lstubs\n"		/* stubs */
       "	pushl	$Lmodh\n"		/* module handle */
       "	pushl	$Lmodn\n"		/* module name */
       "	call	_dlstatunbind\n"	/* unbind module */
       "	addl	$20,%%esp\n"
       "	ret\n"
       "Lload:	pushal\n"
       "	call	_dlload_%s\n"
       "	popal\n"
       "	subl	$5,(%%esp)\n"
       "	ret\n"
       , basename_org
       , basename_fix, basename_fix, basename_fix, basename_fix, basename_fix);

 /* Emit the names of all imported functions */
 fprintf(implib, "Lsyms:\n");
 for (i = 0, scan = symtab; i < dh.n_exp_syms; i++) {
     scan += sizeof(LONG32);
     fprintf(implib, "\t.ascii\t\"%s\\0\"\n", scan);
     scan = strchr(scan, 0) + 1;
 }
 fprintf(implib, "\t.byte\t0\n");

 /* And now emit the stubs */
 for (i = 0, scan = symtab; i < dh.n_exp_syms; i++) {
     scan += sizeof(LONG32);
     fprintf(implib, "\t.align\t2,0xcc\n");
     if (i == 0) {
        fprintf(implib, "Lstubs:\n");
     }
     fprintf(implib, "\t.globl\t%s\n%s:\n\tcall\tLload\n", scan, scan);
     scan = strchr(scan, 0) + 1;
 }

 fclose(implib);
 free(symtab);

 /* We already have what to clean up */
 atexit(exit_cleanup);

 /* Allright, now run the assembler on the resulting file */
 if ((rv = system("as -o "TEMP_O_FILE" "TEMP_S_FILE)) != 0) {
    if (rv == -1) {
       perror("as");
    }
    exit(rv);
 }

 /* Okey-dokey, let's stuff the object file into the archive */
 sprintf(cmdbuf, "ar crs %s "TEMP_O_FILE, opt.implib);
 if ((rv = system(cmdbuf)) != 0) {
    if (rv == -1) {
       perror("ar");
    }
    exit(rv);
 }

 return 0;
}



/* Desc: show DXE3 info
 *
 * In  : filename
 * Out : 0 if success
 *
 * Note: -
 */
static int show_symbols (const char *fname)
{
 int i;
 char *scan;
 char *symtab;
 dxe3_header dh;
 FILE *f = open_dxe_file(fname, &dh);

 scan = symtab = (char *)malloc(dh.exp_size + dh.unres_size + dh.dep_size);
 fseek(f, dh.exp_table, SEEK_SET);
 fread(symtab, 1, dh.exp_size, f);
 fseek(f, dh.unres_table, SEEK_SET);
 fread(symtab + dh.exp_size, 1, dh.unres_size, f);
 fseek(f, dh.dep_table, SEEK_SET);
 fread(symtab + dh.exp_size + dh.unres_size, 1, dh.dep_size, f);
 fclose(f);

 for (i = 0; i < dh.n_exp_syms; i++) {
     scan += sizeof(LONG32);
     if (opt.showexp) {
        puts(scan);
     }
     scan = strchr(scan, 0) + 1;
 }

 for (i = 0; i < dh.n_unres_syms; i++) {
     unsigned short n1 = ((unsigned short *)scan)[0];
     unsigned short n2 = ((unsigned short *)scan)[1];
     scan += sizeof(short) * 2;
     if (opt.showunres) {
        puts(scan);
     }
     scan = strchr(scan, 0) + 1 + (n1 + n2) * sizeof(LONG32);
 }

 for (i = 0; i < dh.n_deps; i++) {
     if (opt.showdep) {
        puts(scan);
     }
     scan = strchr(scan, 0) + 1;
 }

 free(symtab);
 return 0;
}



/* Desc: main entry point
 *
 * In  : command-line
 * Out : 0 if success
 *
 * Note: -
 */
int main (int argc, char **argv)
{
 int i;
 int rv;
 const char **new_argv;

 progname = argv[0];
 /* Prepare the command line for ld */
 new_argv = (const char **)malloc((argc - 1 + 11 + 2) * sizeof(char *));
 process_args(argc, argv, new_argv);

 if (opt.showdep || opt.showexp || opt.showunres) {
    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
           char *dot = strchr(argv[i], '.');
           if (dot && !strcasecmp(dot, ".dxe")) {
              if ((rv = show_symbols(argv[i])) != 0) {
                 return rv;
              }
           }
        }
    }
    return 0;
 }

 if (!opt.output && (!opt.implib || opt.objcount)) {
    fprintf(stderr, "%s: no output file name given (-h for help)\n", progname);
    exit(-1);
 }

 if (opt.output && !opt.objcount) {
    fprintf(stderr, "%s: no object file(s) given (-h for help)\n", progname);
    exit(-1);
 }

 if (opt.implib) {
    if (!opt.dxefile) {
       opt.dxefile = opt.output;
    }
    if (!opt.dxefile) {
      fprintf(stderr, "%s: no DXE module name given (-h for help)\n", progname);
      exit(-1);
    }
 }

 if (opt.verbose) {
    /* print the command line for ld */
    for (i = 0; new_argv[i]; i++) {
        printf("%s ", new_argv[i]);
    }
    printf("\n");
 }

 if (opt.objcount) {
    /* Run linker */
    FILHDR fh;
    FILE *inf = run_ld(new_argv, &fh);

    /* Now `inf' is an opened single-section COFF module. Create the output file */
    FILE *outf;
    if ((outf = fopen(opt.output, "wb")) == NULL) {
       fclose(inf);
       fprintf(stderr, "%s: cannot open file `%s' for writing\n", progname, opt.output);
       exit(-2);
    }

    /* Allright, now write the DXE file and quit */
    if ((rv = write_dxe(inf, outf, &fh)) != 0) {
       return rv;
    }
 }

 if (opt.implib) {
    if ((rv = make_implib()) != 0) {
       return rv;
    }
 }

 return 0;
}

/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */

#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/cdefs.h>

#define constant_var 0
#define confstr_var  1
#define pathconf_var 2
#define sysconf_var  3

struct variable
{
  const char *name;   /* Name of the variable.  */
  long value;         /* A value or an argument.  */
  unsigned int type;  /* Variable is either a constant, or is retrieved by
                         calling either confstr, pathconf, or sysconf.  */
};

const struct variable var_list[] =
{
  { "ARG_MAX", _SC_ARG_MAX, sysconf_var },
  { "CHILD_MAX", _SC_CHILD_MAX, sysconf_var },
  { "CLK_TCK", _SC_CLK_TCK, sysconf_var },
  { "LINK_MAX", _PC_LINK_MAX, pathconf_var },
  { "MAX_CANON", _PC_MAX_CANON, pathconf_var },
  { "MAX_INPUT", _PC_MAX_INPUT, pathconf_var },
  { "NAME_MAX", _PC_NAME_MAX, pathconf_var },
  { "NGROUPS_MAX", _SC_NGROUPS_MAX, sysconf_var },
  { "OPEN_MAX", _SC_OPEN_MAX, sysconf_var },
  { "PATH", _CS_PATH, confstr_var },
  { "PATH_MAX", _PC_PATH_MAX, pathconf_var },
  { "PIPE_BUF", _PC_PIPE_BUF, pathconf_var },
  { "POSIX_V6_ILP32_OFF32_CFLAGS", _CS_POSIX_V6_ILP32_OFF32_CFLAGS,
    confstr_var },
  { "POSIX_V6_ILP32_OFF32_LDFLAGS", _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,
    confstr_var },
  { "POSIX_V6_ILP32_OFF32_LIBS", _CS_POSIX_V6_ILP32_OFF32_LIBS,
    confstr_var },
  { "STREAM_MAX", _SC_STREAM_MAX, sysconf_var },
#ifdef _PC_SYMLINK_MAX
  { "SYMLINK_MAX", _PC_SYMLINK_MAX, pathconf_var },
#endif
  { "TZNAME_MAX", _SC_TZNAME_MAX, sysconf_var },
  { "_POSIX_ARG_MAX", _POSIX_ARG_MAX, constant_var },
  { "_POSIX_CHILD_MAX", _POSIX_CHILD_MAX, constant_var },
  { "_POSIX_CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED, pathconf_var },
  { "_POSIX_LINK_MAX", _POSIX_LINK_MAX, constant_var },
  { "_POSIX_MAX_CANON",  _POSIX_MAX_CANON, constant_var },
  { "_POSIX_MAX_INPUT", _POSIX_MAX_INPUT, constant_var },
  { "_POSIX_NAME_MAX", _POSIX_NAME_MAX, constant_var },
  { "_POSIX_NGROUPS_MAX", _POSIX_NGROUPS_MAX, constant_var },
  { "_POSIX_NO_TRUNC", _PC_NO_TRUNC, pathconf_var },
  { "_POSIX_JOB_CONTROL", _SC_JOB_CONTROL, sysconf_var },
  { "_POSIX_OPEN_MAX", _POSIX_OPEN_MAX, constant_var },
  { "_POSIX_PATH_MAX", _POSIX_PATH_MAX, constant_var },
  { "_POSIX_PIPE_BUF", _POSIX_PIPE_BUF, constant_var },
  { "_POSIX_SAVED_IDS", _SC_SAVED_IDS, sysconf_var },
  { "_POSIX_SSIZE_MAX", _POSIX_SSIZE_MAX, constant_var },
  { "_POSIX_STREAM_MAX", _POSIX_STREAM_MAX, constant_var },
  { "_POSIX_TZNAME_MAX", _POSIX_TZNAME_MAX, constant_var },
  { "_POSIX_V6_ILP32_OFF32", _POSIX_V6_ILP32_OFF32, sysconf_var },
  { "_POSIX_VERSION", _SC_VERSION, sysconf_var },
  { "_POSIX_VDISABLE", _PC_VDISABLE, pathconf_var },
};

const char *conf_specs[] =
{
  "POSIX_V6_ILP32_OFF32",
};

#define SIZEOF_ARRAY(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

static void print_usage(void);
static void usage(void);
static void help(void);
static void undefined(void);

static int var_compare(const void *key, const void *elem)
{
  return strcmp((const char *)key, ((const struct variable *)elem)->name);
}

int main(int argc, char *argv[])
{
  char c;
  const char *var_name;
  const struct variable *var;

  if (argc == 1)
    usage();

  while ((c = getopt(argc, argv, ":hv:")) != -1)
  {
    switch (c)
    {
      case 'h':
      {
        help();
        break;
      }
      case 'v':
      {
        const char *spec = optarg;
        const int num_specs = SIZEOF_ARRAY(conf_specs);
        int i;
        
        /* Verify the specification is valid.  */
        for (i = 0; i < num_specs; ++i)
        {
          if (strcmp (spec, conf_specs[i]) == 0)
            break;
        }
        
        if (i == num_specs)
        {
          fprintf(stderr, "undefined specification: %s\n", spec);
        }
        
        /* Since only one specification is supported,
           there's nothing more to do.  */
      }
      default:
      {
        usage();
        break;
      }
    }
  }

  var_name = argv[optind];

  var = bsearch(var_name, var_list, sizeof(var_list) / sizeof(var_list[0]),
                sizeof(var_list[0]), var_compare);
  if (var == NULL)
    undefined();

  if ((argc - optind) != ((var->type == pathconf_var) ? 2 : 1))
    usage();

  /* Ensure sysconf and pathconf errors are detected.  */
  errno = 0;

  switch (var->type)
  {
    case constant_var:
    {
      printf("%li\n", var->value);
      break;
    }

    case confstr_var:
    {
      size_t buf_len, ret _ATTRIBUTE(__unused__);
      char *buf;

      buf_len = confstr(var->value, 0, 0);
      if (buf_len)
      {
        buf = alloca(buf_len);
        ret = confstr(var->value, buf, buf_len);
        printf("%s\n", buf);
      }
      else
        undefined(); /* Shouldn't happen, but you never know.  */
      break;
    }

    case sysconf_var:
    {
      long val;

      val = sysconf(var->value);
      if (errno == 0)
        printf("%li\n", val);
      else
        undefined();
      break;
    }

   case pathconf_var:
   {
     long val;

     val = pathconf(argv[optind + 1], var->value);
     if (errno == 0)
       printf("%li\n", val);
     else
       undefined();
     break;
   }

  }
  
  return 0;
}

static void
undefined(void)
{
  fprintf(stderr, "undefined\n");
  exit(1);
}

static void
print_usage(void)
{
  fprintf(stderr, "Usage: getconf [-v specification] [-h] [sysvar] [pathvar path]\n");
}

static void
usage(void)
{
  print_usage();
  exit(1);
}

static void
help(void)
{
  size_t i;

  print_usage();
  fprintf(stderr, "\nValid specifications:\n");
  for (i = 0; i < SIZEOF_ARRAY(conf_specs); ++i)
    fprintf(stderr, "%s\n", conf_specs[i]);

  fprintf(stderr, "\nValid variable names:\n");
  for (i = 0; i < SIZEOF_ARRAY(var_list); ++i)
  {
    if (var_list[i].type != pathconf_var)
      fprintf(stderr, "%s\n", var_list[i].name);
    else
      fprintf(stderr, "%s <path>\n", var_list[i].name);
  } 
  exit(1);
}

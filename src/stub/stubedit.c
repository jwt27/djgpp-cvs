/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

#include "../../include/stubinfo.h"

unsigned long size_of_stubinfo = 0;
char *client_stub_info;

void find_info(char *filename)
{  
  FILE *f;
  unsigned char header[4];
  unsigned char test_magic[16];

  f = fopen(filename, "rb");
  if (f == 0)
  {
    char buf[100];
    sprintf(buf, "Fatal error in stubedit reading %s", filename);
    perror(buf);
    exit(1);
  }

  fseek(f, 512L, 0);
  fread(test_magic, 16, 1, f);
  if (memcmp(test_magic, "go32stub", 8) != 0)
  {
    printf("Error: %s is not a go32 v2.0 or higher stub\n");
    exit(1);
  }

  fread(header, 4, 1, f);
  size_of_stubinfo = (header[0]) | (header[1]<<8)
                   | (header[2])<<16 | (header[3]<<24);

  fseek(f, 512L, 0);
  client_stub_info = (char *)malloc(size_of_stubinfo);
  fread(client_stub_info, size_of_stubinfo, 1, f);

  fclose(f);
  return;
}

void store_info(char *filename)
{
  FILE *f;
  f = fopen(filename, "r+b");
  if (f == 0)
  {
    char buf[100];
    sprintf(buf, "Fatal error in stubedit writing %s", filename);
    perror(buf);
    exit(1);
  }
  fseek(f, 512L, 0);
  fwrite(client_stub_info, 1, size_of_stubinfo, f);
  fclose(f);
}

char *pose_question(char *question, char *default_answer)
{
  static char response[200];
  printf("%s ? [%s] ", question, default_answer);
  fflush(stdout);
  gets(response);
  if (response[0] == '\0')
    return 0;
  return response;
}

typedef void (*PerFunc)(void *address_of_field, char *buffer);

void str_v2s(void *addr, char *buf, int len)
{
  if (*(char *)addr == 0)
    strcpy(buf, "\"\"");
  else
  {
    buf[len] = 0;
    strncpy(buf, (char *)addr, len);
  }
}

void str_s2v(void *addr, char *buf, int len)
{
  if (strcmp(buf, "\"\"") == 0)
    *(char *)addr = 0;
  else
  {
    ((char *)addr)[len-1] = 0;
    strncpy((char *)addr, buf, len);
  }
}

void str_v2s8(void *addr, char *buf)
{
  str_v2s(addr, buf, 8);
}

void str_s2v8(void *addr, char *buf)
{
  str_s2v(addr, buf, 8);
}

void str_v2s16(void *addr, char *buf)
{
  str_v2s(addr, buf, 16);
}

void str_s2v16(void *addr, char *buf)
{
  str_s2v(addr, buf, 16);
}

void num_v2s(void *addr, char *buf)
{
  unsigned long v = *(unsigned long *)addr;
  sprintf(buf, "%#lx (%dk)", v, v / 1024L);
}

void num_s2v(void *addr, char *buf)
{
  unsigned long r = 0;
  char s = 0;
  sscanf(buf, "%i%c", &r, &s);
  switch (s)
  {
    case 'k':
    case 'K':
      r *= 1024L;
      break;
    case 'm':
    case 'M':
      r *= 1048576L;
      break;
  }
  *(unsigned long *)addr = r;
}

struct {
  char *short_name;
  char *long_name;
  int offset_of_field;
  PerFunc val2string;
  PerFunc string2val;
} per_field[] = {
  {
    "minstack",
    "Minimum amount of stack space (bytes/K/M)",
    STUBINFO_MINSTACK,
    num_v2s, num_s2v
  },
  {
    "bufsize",
    "Size of real-memory transfer buffer (bytes/K/M)",
    STUBINFO_MINKEEP,
    num_v2s, num_s2v
  },
  {
    "runfile",
    "Base name of file to actually run (max 8 chars, \"\"=self)",
    STUBINFO_BASENAME,
    str_v2s8, str_s2v8
  },
  {
    "argv0",
    "Value to pass as file component of argv[0] (max 16 chars, \"\"=default)",
    STUBINFO_ARGV0,
    str_v2s16, str_s2v16
  },
  {
    "dpmi",
    "Program to load to provide DPMI services (if needed)",
    STUBINFO_DPMI_SERVER,
    str_v2s16, str_s2v16
  }
};

#define NUM_FIELDS (sizeof(per_field) / sizeof(per_field[0]))

#define HFORMAT "%-16s %s\n"

void give_help(void)
{
  int i;
  fprintf(stderr, "Usage: stubedit [-v] [-h] filename.exe [field=value . . . ]\n");
  fprintf(stderr, "-h = give help   -v = view info  field=value means set w/o prompt\n");
  fprintf(stderr, HFORMAT, "-field-", "-description-");

  for (i=0; i < NUM_FIELDS; i++)
    fprintf(stderr, HFORMAT, per_field[i].short_name, per_field[i].long_name);
  exit(1);
}

main(int argc, char **argv)
{
  int view_only = 0;
  int i;
  int need_to_save;

  if (argc > 1 && strcmp(argv[1], "-h") == 0)
    give_help();

  if (argc > 1 && strcmp(argv[1], "-v") == 0)
  {
    view_only = 1;
    argc--;
    argv++;
  }

  if (argc < 2)
    give_help();

  find_info(argv[1]);

  if (view_only)
  {
    char buf[100];
    fprintf(stderr, HFORMAT, "-value-", "-field description-");
    for (i=0; i<NUM_FIELDS; i++)
    {
      if (per_field[i].offset_of_field < size_of_stubinfo)
      {
        per_field[i].val2string(client_stub_info + per_field[i].offset_of_field, buf);
        fprintf(stderr, HFORMAT, buf, per_field[i].long_name);
      }
    }
    exit(0);
  }

  if (argc > 2)
  {
    int f, got, got_any = 0;
    char fname[100], fval[100];
    for (i=2; i < argc; i++)
    {
      fname[0] = 0;
      fval[0] = 0;
      sscanf(argv[i], "%[^=]=%s", fname, fval);
      got = 0;
      for (f=0; f<NUM_FIELDS; f++)
      {
        if (strcmp(per_field[f].short_name, fname) == 0)
        {
          got = 1;
          got_any = 1;
          if (per_field[i].offset_of_field < size_of_stubinfo)
          {
            per_field[f].string2val(client_stub_info + per_field[f].offset_of_field, fval);
          }
          else
            fprintf(stderr, "Warning: This stub does not support field %s\n", fname);
        }
      }
      if (!got)
      {
        fprintf(stderr, "Error: %s is not a valid field name.\n", fname);
        give_help();
      }
    }
    if (got_any)
      store_info(argv[1]);
    return 0;
  }

  need_to_save = 0;
  for (i=0; i<NUM_FIELDS; i++)
  {
    char buf[100], *resp;
    if (per_field[i].offset_of_field < size_of_stubinfo)
    {
      per_field[i].val2string(client_stub_info + per_field[i].offset_of_field, buf);
      if ((resp = pose_question(per_field[i].long_name, buf)) != 0)
      {
        per_field[i].string2val(client_stub_info + per_field[i].offset_of_field, resp);
        need_to_save = 1;
      }
    }
  }
  if (need_to_save)
    store_info(argv[1]);

  return 0;
}

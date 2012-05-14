/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern int errno;

#include "oread.h"
#include "zread.h"

/*
extern int text_unix;
extern int text_dos;
extern int ignore_csum;
*/

extern int to_stdout;
extern int to_tty;
extern int list_only;
extern FILE *log_out;

#define CRYFLG 1
#define EXTFLG 8

static int epoutfile = 0;
static char *changed_name = NULL;

int epcopy(char *, long);

void
epunzip_read(char *zipfilename)
{
  int dont_understand = 0;
  int at_local_header = 0;

  clear_bufs();
  errno = 0;
  ifd = oread_open(zipfilename);
  if (errno)
  {
    fprintf(log_out, "%s: %s\n", zipfilename, strerror(errno));
    return;
  }

  /* Find the first local header. Skip over spanning markers
   * for a one-segment ZIP, but fail on other headers. */
  while(1)
  {
    char buffer[4];

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();

    if (*(__dj_short_a *)buffer != ('K' << 8) + 'P')
      break;

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();

    if (*(__dj_short_a *)buffer == ('\004' << 8) + '\003')
    {
      /* local header */
      at_local_header = 1;
      break;
    }
    else if(*(__dj_short_a *)buffer == ('0' << 8) + '0')
    {
      /* spanning marker, but only one segment
       * => need to find local header. */
      continue;
    }
    else if(*(__dj_short_a *)buffer == ('\010' << 8) + '\007')
    {
      /* spanning marker, multiple segments. */
      fprintf(log_out, "%s: spanning is not supported\n", zipfilename);
      dont_understand = 1;
      break;
    }
    else
    {
      /* unknown header */
      fprintf(log_out,
              "%s: don't understand header type 0x%02x%02x\n",
              zipfilename, buffer[0], buffer[1]);
      dont_understand = 1;
      break;
    }
  }

  if (!at_local_header && !dont_understand)
    fprintf(log_out, "%s: invalid zip file structure\n", zipfilename);

  if (at_local_header) while(1)
  {
    int ext_header, timedate, crc, size, length, name_length,
    extra_length, should_be_written, count, real_file = 0;
    char buffer[4];
    char filename[2048];

    if (!at_local_header)
    {
      buffer[0] = (char)get_byte();
      buffer[1] = (char)get_byte();

      if(*(__dj_short_a *)buffer != ('K' << 8) + 'P')
      {
        fprintf(log_out, "%s: invalid zip file structure\n", zipfilename);
        break;
      }

      buffer[0] = (char)get_byte();
      buffer[1] = (char)get_byte();

      if(*(__dj_short_a *)buffer != ('\004' << 8) + '\003')
      {
        /* not a local header - all done */
        break;
      }
    }
    else
      at_local_header = 0;

    /* version info - ignore it */
    get_byte();
    get_byte();

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();

    if (*(__dj_short_a *)buffer & CRYFLG)
    {
      fprintf(log_out, "%s has encrypted file(s) - use unzip\n", zipfilename);
      break;
    }
    ext_header = *(__dj_short_a *)buffer & EXTFLG ? 1 : 0;

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();

    method = *(__dj_short_a *)buffer;
    if (method != 8 && method != 0)
    {
      fprintf(log_out, "%s has file(s) compressed with unsupported method - use unzip\n", zipfilename);
      break;
    }

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();
    buffer[2] = (char)get_byte();
    buffer[3] = (char)get_byte();
    timedate = *(__dj_int_a *)buffer;

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();
    buffer[2] = (char)get_byte();
    buffer[3] = (char)get_byte();
    crc = *(__dj_int_a *)buffer;

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();
    buffer[2] = (char)get_byte();
    buffer[3] = (char)get_byte();
    size = *(__dj_int_a *)buffer;

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();
    buffer[2] = (char)get_byte();
    buffer[3] = (char)get_byte();
    length = *(__dj_int_a *)buffer;

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();
    name_length = *(__dj_short_a *)buffer;

    buffer[0] = (char)get_byte();
    buffer[1] = (char)get_byte();
    extra_length = *(__dj_short_a *)buffer;

    for (count = 0; count < name_length; count++)
      filename[count] = (char)get_byte();
    filename[name_length] = 0;

    for (count = 0; count < extra_length; count++)
      get_byte();

    changed_name = get_new_name(filename, &should_be_written);

    fprintf(log_out, "%s%s\n", changed_name,
            !should_be_written && !list_only ? "\t[ skipped ]" : "");

    if (!should_be_written)
      epoutfile = open("/dev/null",
                       O_WRONLY | O_BINARY,
                       S_IWRITE | S_IREAD);
    else if (changed_name[strlen(changed_name) - 1] == '/' && !to_stdout)
    {
      changed_name[strlen(changed_name) - 1] = 0;
      make_directory(changed_name);
      continue;
    }
    else
    {
open_file:
      if (!to_stdout)
      {
        do_directories(changed_name);
        rename_if_dos_device(changed_name);
        epoutfile = open(changed_name,
                         O_WRONLY | O_BINARY | O_CREAT | O_EXCL,
                         S_IWRITE | S_IREAD);
        if (epoutfile < 0)
        {
          if (change(changed_name, "Cannot exclusively open file", 0))
            goto open_file;
          else
            epoutfile = open("/dev/null",
                             O_WRONLY | O_BINARY,
                             S_IWRITE | S_IREAD);
        }
        real_file = 1;
      }
      else
      {
        epoutfile = fileno(stdout);
        if (!to_tty)
          setmode(epoutfile, O_BINARY);
      }
    }

    updcrc(NULL, 0);

    if(method == 0) /* stored */
    {

      if(ext_header)
      {
        fprintf(log_out, "%s has stored file with extended local header\
\n\aKill the program that produced it!!!\n", zipfilename);
        break;
      }

      if(size != length)
      {
        fprintf(log_out, "%s has stored file with different lengths\n",
                zipfilename);
        break;
      }

      for (count = length; count; count--)
      {
        char c = (char)get_byte();
        put_ubyte(c, epcopy);
      }
      flush_window(epcopy);

    }
    else /* deflated */
    {
      if(inflate(epcopy))
      {
        fprintf(log_out, "inflation failed on %s\n", zipfilename);
        break;
      }

      if(ext_header)
      {
        buffer[0] = (char)get_byte();
        buffer[1] = (char)get_byte();
        buffer[2] = (char)get_byte();
        buffer[3] = (char)get_byte();
        if (*(__dj_int_a *)buffer != ('\010' << 24) + ('\007' << 16) + ('K' << 8) + 'P')
        {
          fprintf(log_out, "%s: invalid zip file structure\n", zipfilename);
          break;
        }

        buffer[0] = (char)get_byte();
        buffer[1] = (char)get_byte();
        buffer[2] = (char)get_byte();
        buffer[3] = (char)get_byte();
        crc = *(__dj_int_a *)buffer;

        buffer[0] = (char)get_byte();
        buffer[1] = (char)get_byte();
        buffer[2] = (char)get_byte();
        buffer[3] = (char)get_byte();
        size = *(__dj_int_a *)buffer;

        buffer[0] = (char)get_byte();
        buffer[1] = (char)get_byte();
        buffer[2] = (char)get_byte();
        buffer[3] = (char)get_byte();
        length = *(__dj_int_a *)buffer;
      }
    }

    if ((unsigned)crc != updcrc((uch *)outbuf, 0))
    {
      fprintf(log_out, "invalid crc on %s\n", zipfilename);
      break;
    }

    if (epoutfile != fileno(stdout))
      close(epoutfile);

    if (real_file)
    {
      epoutfile = open(changed_name, O_RDONLY);
      setftime(epoutfile, (struct ftime *)(void *)&timedate);
      close(epoutfile);
      real_file = 0;
    }
  }

  oread_close(ifd);
}

int
epcopy(char *buffer, long size)
{
  errno = 0;
  if (write(epoutfile, buffer, size) < size)
  {
    if (errno == 0)
      errno = ENOSPC;
    fprintf(log_out, "%s: %s\n", changed_name, strerror(errno));
  }
  return 0;
}

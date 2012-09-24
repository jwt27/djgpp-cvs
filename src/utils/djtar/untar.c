/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

extern int errno;

/*------------------------------------------------------------------------*/

#include "oread.h"
#include "zread.h"

extern int text_unix;
extern int text_dos;
extern int to_stdout;
extern int to_tty;
extern int ignore_csum;
extern int list_only;

extern FILE *log_out;

/*------------------------------------------------------------------------*/

typedef struct {
  char name[100];
  char operm[8];
  char ouid[8];
  char ogid[8];
  char osize[12];
  char otime[12];
  char ocsum[8];
  char flags[1];
  char filler[355];
} TARREC;

static TARREC header;
static int error_message_printed;
static int looking_for_header;
static char *changed_name;
static int first_block = 1;
static File_type file_type = DOS_BINARY;
static long perm, uid, gid, size;
static long posn = 0;
static time_t ftime;
static struct ftime ftimes;
static struct tm *tm;
static int r;
static int skipping;

extern char new[];

int
tarread(char *buf, long buf_size)
{
  int should_be_written, batch_file_processing = 0;

  while (buf_size)
  {
    int write_errno = 0;
    int dsize = 512, wsize;

    if (skipping)
    {
      if (skipping <= buf_size)
      {
        bytes_out += skipping;
        buf       += skipping;
        buf_size  -= skipping;
        skipping   = 0;
        looking_for_header = 1;
        if (buf_size < (long)(sizeof header))
          return 0;
      }
      else
      {
        bytes_out += buf_size;
        skipping -= buf_size;
        return 0;
      }
    }

    if (looking_for_header)
    {
      char *extension;
      int head_csum = 0;
      int i;
      size_t nlen;

      memcpy(&header, buf, sizeof header);
      if (header.name[0] == 0)
      {
        bytes_out += buf_size;  /* assume everything left should be counted */
        return EOF;
      }
      buf += sizeof header;
      buf_size -= sizeof header;
      bytes_out += sizeof header;
      first_block = 1;
      file_type = DOS_BINARY;
      looking_for_header = 0;

      /* command.com refuses to run batch files
         that have been stored with UNIX-style EOL,
         so we will extract them with DOS-style EOL. */
      extension = strrchr(basename(header.name), '.');
      if (extension && !stricmp(extension, ".bat"))
        batch_file_processing = 1;  /* LF -> CRLF */

      sscanf(header.operm, " %lo", &perm);
      sscanf(header.ouid, " %lo", &uid);
      sscanf(header.ogid, " %lo", &gid);
      sscanf(header.osize, " %lo", &size);
      sscanf(header.otime, " %o", &ftime);
      sscanf(header.ocsum, " %o", &head_csum);
      for (i = 0; i < (int)(sizeof header); i++)
      {
        /* Checksum on header, but with the checksum field blanked out.  */
        int j = (i > 147 && i < 156) ? ' ' : *((unsigned char *)&header + i);

        head_csum -= j;
      }
      if (head_csum && !ignore_csum)
      {
        /* Probably corrupted archive.  Bail out.  */
        if (!error_message_printed)
        {
          error_message_printed = 1;
          fprintf(log_out, "--- !!Directory checksum error!! ---\n");
        }
        /* We have still not found a valid tar header in this buf[],
           so we MUST continue looking for a header next time that
           tarread() is called with a new buf[]. */
        looking_for_header = 1;
        bytes_out += buf_size;
        return EOF;
      }

      changed_name = get_new_name(header.name, &should_be_written);

      if (v_switch)
        fprintf(log_out, "%08lx %6lo ", posn, perm);
      else
        fprintf(log_out, "%c%c%c%c ",
                S_ISDIR(perm)  ? 'd' : header.flags[0] == '2' ? 'l' : '-',
                perm & S_IRUSR ? 'r' : '-',
                perm & S_IWUSR ? 'w' : '-',
                perm & S_IXUSR ? 'x' : '-');
      fprintf(log_out, "%.20s %9ld %s", ctime(&ftime) + 4, size, changed_name);
#if 0
      fprintf(log_out, "(out: %ld)", bytes_out);
#endif
      if (header.flags[0] == '2')
        fprintf(log_out, " -> %s", header.filler);
      else if (header.flags[0] == '1')
        fprintf(log_out, " link to %s", header.filler);
      fprintf(log_out, "%s\n",
              !should_be_written && !list_only ? "\t[ skipped ]" : "");
      posn += 512 + ((size + 511) & ~511);
#if 0
      fprintf(log_out, "%6lo %02x %12ld %s\n", perm, header.flags[0], size, changed_name);
#endif

      if (header.flags[0] == '1' || header.flags[0] == '2')
      {
        /* Symbolic links always have zero data, but some broken
           tar programs claim otherwise.  */
        size = 0;
      }
      if (should_be_written == 0)
      {
        skipping = (size + 511) & ~511;
        if (!skipping)	/* an empty file or a directory */
        {
          looking_for_header = 1;
          if (buf_size < (long)(sizeof header))
            return 0;
        }
        continue;
      }
      else if ((changed_name[nlen = strlen(changed_name) - 1] == '/'
                || header.flags[0] == '5') /* '5' flags a directory */
               && !to_stdout)
      {
        if (changed_name != new)
        {
          memcpy(new, changed_name, nlen + 2);
          changed_name = new;
        }
        if (changed_name[nlen] == '/')
          changed_name[nlen] = 0;
        make_directory(changed_name);
        looking_for_header = 1;
        continue;
      }
      else
      {
open_file:
        if (!to_stdout)
        {
          if (changed_name != new)
          {
            memcpy(new, changed_name, nlen + 2);
            changed_name = new;
          }
          do_directories(changed_name);
          rename_if_dos_device(changed_name);
          r = open(changed_name,
                   O_WRONLY | O_BINARY | O_CREAT | O_EXCL, S_IWRITE | S_IREAD);
          if (r < 0)
          {
            if (change(changed_name, "Cannot exclusively open file", 0))
              goto open_file;
            else
            {
              skipping = (size + 511) & ~511;
              continue;
            }
          }
        }
        else
        {
          r = fileno(stdout);
          if (!to_tty)
            setmode(r, O_BINARY);
        }
      }

    }

    while (size)
    {
      char tbuf[1024];
      char *wbuf = buf;

      if (buf_size <= 0)    /* this buffer exhausted */
        return 0;
      if (size < 512)
        dsize = size;
      else if (buf_size < 512)
        dsize = buf_size;
      else
        dsize = 512;
      if (batch_file_processing && !to_tty)
      {
        /* LF -> CRLF.
           Note that we don't alter the original uncompressed
           data so as not to screw up the CRC computations.  */
        char *src = buf, *dest = tbuf;
        int buflen = 0;
        while (buflen < dsize)
        {
          if (buflen && *src == '\n' && src[-1] != '\r')
            *dest++ = '\r';
          *dest++ = *src++;
          buflen = src - buf;
        }
        wsize = dest - tbuf;
        wbuf = tbuf;
      }
      else
      {
        if (first_block && (text_dos || text_unix || to_tty))
        {
          file_type = guess_file_type(buf, dsize);
          first_block = 0;
          if (file_type == UNIX_TEXT && text_dos)
            setmode(r, O_TEXT);   /* will add CR chars to each line */
        }
        if ((text_unix || to_tty) && file_type == DOS_TEXT)
        {
          /* If they asked for text files to be written Unix style, or
             we are writing to console, remove the CR and ^Z characters
             from DOS text files.
             Note that we don't alter the original uncompressed data so
             as not to screw up the CRC computations.  */
          char *s=buf, *d=tbuf;
          while (s-buf < dsize)
          {
            if (*s != '\r' && *s != 26)
              *d++ = *s;
            s++;
          }
          wsize = d - tbuf;
          wbuf = tbuf;
        }
        else
        {
          wbuf = buf;
          wsize = dsize;
        }
      }
      errno = 0;
      if (write(r, wbuf, wsize) < wsize)
      {
        if (errno == 0)
          errno = ENOSPC;
        fprintf(log_out, "%s: %s\n", changed_name, strerror(errno));
        write_errno = errno;
        break;
      }
      size      -= dsize;
      buf_size  -= dsize;
      buf       += dsize;
      bytes_out += dsize;
    }

    if (!to_stdout)
    {
      close(r);
      r = open(changed_name, O_RDONLY);
      tm = localtime(&ftime);
      ftimes.ft_tsec = tm->tm_sec / 2;
      ftimes.ft_min = tm->tm_min;
      ftimes.ft_hour = tm->tm_hour;
      ftimes.ft_day = tm->tm_mday;
      ftimes.ft_month = tm->tm_mon + 1;
      ftimes.ft_year = tm->tm_year - 80;
      setftime(r, &ftimes);
      close(r);
      chmod(changed_name, perm);
    }
    batch_file_processing = 0;
    looking_for_header = 1;
    if (write_errno == ENOSPC)  /* target disk full: quit early */
    {
      bytes_out += buf_size;
      return EOF;
    }
    else if (write_errno)       /* other error: skip this file, try next */
      skipping = (size - dsize + 511) & ~511;
    else    /* skip the slack garbage to the next 512-byte boundary */
      skipping = 512 - dsize;
  }
  return 0;
}

/*------------------------------------------------------------------------*/

int part_nb;
static const char *zip_description[] = {
  "uncompressed",
  "COMPRESS'ed",
  "PACK'ed",
  "LZH'ed",
  "compressed by unknown method (4)",
  "compressed by unknown method (5)",
  "compressed by unknown method (6)",
  "compressed by unknown method (7)",
  "deflated by ",
  "BZIP2'ed"
};

void
tar_gz_read(char *fname)
{
  void *f;

  errno = 0;
  f = oread_open(fname);
  if (errno)
  {
    fprintf(log_out, "%s: %s\n", fname, strerror(errno));
    return;
  }
  ifname = fname;
  header_bytes = 0;
  clear_bufs(); /* clear input and output buffers */
  part_nb = 0;  /* FIXME!! handle multi-part gzip's */
  method = get_method(f);
  if (method < 0)
  {
    oread_close(f);
    return;     /* error message already emitted */
  }
  if (v_switch)
    fprintf(log_out, "-- `%s\' is %s%s --\n\n",
            fname,
            method > MAX_METHODS ? "corrupted (?)" : zip_description[method],
            method == DEFLATED ? (pkzip ? "PKZip" : "GZip") : "");

  bytes_out = 0;
  error_message_printed = 0;
  looking_for_header = 1;
  posn = 0;

  if ((*decompressor)(f) != OK)
  {
    fprintf(log_out,
            "\n%s: corrupted file; I might have written garbage\n", fname);
    fflush(log_out);
  }
  oread_close(f);
  method = -1;
}

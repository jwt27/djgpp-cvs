/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
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
/* tar Header Block, from POSIX 1003.1-1990.  */

/* POSIX header.  */

typedef struct posix_header
{                            /* byte offset */
  char name[100];            /*   0 */
  char mode[8];              /* 100 */
  char uid[8];               /* 108 */
  char gid[8];               /* 116 */
  char size[12];             /* 124 */
  char mtime[12];            /* 136 */
  char chksum[8];            /* 148 */
  char typeflag;             /* 156 */
  char linkname[100];        /* 157 */
  char magic[6];             /* 257 */
  char version[2];           /* 263 */
  char uname[32];            /* 265 */
  char gname[32];            /* 297 */
  char devmajor[8];          /* 329 */
  char devminor[8];          /* 337 */
  char prefix[155];          /* 345 */
  char filler[12];           /* 500 */
                             /* 512 */
} TARREC;


#define NAME_FIELD_SIZE     100
#define PREFIX_FIELD_SIZE   155
#define FIRST_CHKSUM_OCTET  148
#define LAST_CHKSUM_OCTET   155


#define IS_USTAR_HEADER(m)  ((m)[0] == 'u' &&  \
                             (m)[1] == 's' &&  \
                             (m)[2] == 't' &&  \
                             (m)[3] == 'a' &&  \
                             (m)[4] == 'r' &&  \
                             (m)[5] == '\0')

#define IS_PAX_HEADER(h)    ((((h).typeflag == XGLTYPE) || ((h).typeflag == XHDTYPE)) &&  \
                             IS_USTAR_HEADER((h).magic))

#define IS_CHKSUM_OCTET(d)  ((d) > (FIRST_CHKSUM_OCTET - 1) &&  \
                             (d) < (LAST_CHKSUM_OCTET + 1))


/* tar files are made in basic blocks of this size.  */
#define BLOCKSIZE 512


/* Values used in typeflag field.  */
#define REGTYPE  '0'    /* regular file */
#define AREGTYPE '\0'   /* regular file */
#define LNKTYPE  '1'    /* link */
#define SYMTYPE  '2'    /* reserved */
#define CHRTYPE  '3'    /* character special */
#define BLKTYPE  '4'    /* block special */
#define DIRTYPE  '5'    /* directory */
#define FIFOTYPE '6'    /* FIFO special */
#define CONTTYPE '7'    /* reserved */

#define XHDTYPE  'x'    /* Extended header referring to the
                           next file in the archive */
#define XGLTYPE  'g'    /* Global extended header */


static TARREC header;
static int error_message_printed;
static int looking_for_header;
static char *changed_name;
static int first_block = 1;
static File_type file_type = DOS_BINARY;
static long mode, uid, gid, size;
static long posn = 0;
static time_t ftime;
static struct ftime ftimes;
static struct tm *tm;
static int r;
static int skipping;
static unsigned int skipped_pax_global_headers = 0;
static unsigned int skipped_pax_extended_headers = 0;

extern char new[];


void
print_info_about_skipped_pax_headers()
{
  if (skipped_pax_global_headers || skipped_pax_extended_headers)
  {
    fprintf(log_out, "\n-- \"%s\" contains ", ifname);
    if (skipped_pax_global_headers && !skipped_pax_extended_headers)
      fprintf(log_out, "%d pax global extended headers.", skipped_pax_global_headers);
    else if (skipped_pax_extended_headers && !skipped_pax_global_headers)
      fprintf(log_out, "%d pax extended headers.", skipped_pax_extended_headers);
    else
      fprintf(log_out, "%d pax global extended headers and %d pax extended headers.",
                       skipped_pax_global_headers, skipped_pax_extended_headers);
    fprintf(log_out, "  All discarded. --\n\n");
  }
}

int
tarread(char *buf, long buf_size)
{
  int should_be_written, batch_file_processing = 0;

  while (buf_size)
  {
    int write_errno = 0;
    int dsize = BLOCKSIZE, wsize;

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
        skipping  -= buf_size;
        return 0;
      }
    }

    if (looking_for_header)
    {
      char name[PREFIX_FIELD_SIZE + 1 + NAME_FIELD_SIZE + 1];
      char *extension;
      int head_csum = 0;
      int i;
      size_t nlen;

      memcpy(&header, buf, sizeof header);

      /* Skip global extended and extended pax headers
         or extract them as regular files depending of s_switch.  */
      if (IS_PAX_HEADER(header) && s_switch)
      {
        /*
         *  The pax header block is identical to a ustar header block
         *  except that two additional typeflag values are defined:
         *    x: represents extended header records for the following
         *       file in the archive (with its one ustar header block).
         *    g: represents global extended header records for the
         *       following files in the archive.
         *
         *  Skip header plus all pax data blocks that follows
         *  until the next header is found.
         */

        sscanf(header.mode, " %lo", &mode);
        sscanf(header.size, " %lo", &size);
        sscanf(header.mtime, " %o", &ftime);
        memcpy(name, header.name, sizeof header.name);
        name[sizeof header.name] = '\0';

        skipping = (size + (BLOCKSIZE - 1)) & ~(BLOCKSIZE - 1);

        if (v_switch)
        {
          fprintf(log_out, "%08lx %6lo %.20s %9ld %s", posn, mode, ctime(&ftime) + 4, size, name);
          if (header.typeflag == XGLTYPE)
            fprintf(log_out, "  [global extended header + ");
          else if (header.typeflag == XHDTYPE)
            fprintf(log_out, "  [extended header + ");
          fprintf(log_out, "%d data block(s) skipped]\n", skipping / BLOCKSIZE);
        }

        switch (header.typeflag)
        {
        case XGLTYPE:
          skipped_pax_global_headers++;
          break;
        case XHDTYPE:
          skipped_pax_extended_headers++;
          break;
        }

        posn += BLOCKSIZE + skipping;
        buf += sizeof header;
        buf_size -= sizeof header;
        bytes_out += sizeof header;

        continue;
      }

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

      sscanf(header.mode, " %lo", &mode);
      sscanf(header.uid, " %lo", &uid);
      sscanf(header.gid, " %lo", &gid);
      sscanf(header.size, " %lo", &size);
      sscanf(header.mtime, " %o", &ftime);
      sscanf(header.chksum, " %o", &head_csum);
      for (i = 0; i < (int)(sizeof header); i++)
      {
        /* Checksum on header, but with the checksum field blanked out.  */
        int j = IS_CHKSUM_OCTET(i) ? ' ' : *((unsigned char *)&header + i);

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

      /* Accept file names as specified by
         POSIX.1-1996 section 10.1.1.  */
      changed_name = name;
      if (header.prefix[0] && IS_USTAR_HEADER(header.magic))
      {
        /*
         *  A new pathname shall be formed by concatenating
         *  prefix (up to the first NUL character), a slash
         *  character, and name; otherwise, name is used alone.
         */
        size_t len = sizeof header.prefix;
        memcpy(changed_name, header.prefix, len);
        changed_name[len] = '/';
        changed_name += ++len;
      }
      memcpy(changed_name, header.name, sizeof header.name);
      changed_name[sizeof header.name] = '\0';

      changed_name = get_new_name(name, &should_be_written);

      if (v_switch)
        fprintf(log_out, "%08lx %6lo ", posn, mode);
      else
        fprintf(log_out, "%c%c%c%c ",
                S_ISDIR(mode)  ? 'd' : header.typeflag == SYMTYPE ? 'l' : '-',
                mode & S_IRUSR ? 'r' : '-',
                mode & S_IWUSR ? 'w' : '-',
                mode & S_IXUSR ? 'x' : '-');
      fprintf(log_out, "%.20s %9ld %s", ctime(&ftime) + 4, size, changed_name);
#if 0
      fprintf(log_out, "(out: %ld)", bytes_out);
#endif
      if (header.typeflag == SYMTYPE)
        fprintf(log_out, " -> %s", header.linkname);
      else if (header.typeflag == LNKTYPE)
        fprintf(log_out, " link to %s", header.linkname);
      fprintf(log_out, "%s\n",
              !should_be_written && !list_only ? "\t[ skipped ]" : "");
      posn += BLOCKSIZE + ((size + (BLOCKSIZE - 1)) & ~(BLOCKSIZE - 1));
#if 0
      fprintf(log_out, "%6lo %02x %12ld %s\n", mode, header.typeflag, size, changed_name);
#endif

      if (header.typeflag == LNKTYPE || header.typeflag == SYMTYPE)
      {
        /* Symbolic links always have zero data, but some broken
           tar programs claim otherwise.  */
        size = 0;
      }
      if (should_be_written == 0)
      {
        skipping = (size + (BLOCKSIZE - 1)) & ~(BLOCKSIZE - 1);
        if (!skipping)    /* an empty file or a directory */
        {
          looking_for_header = 1;
          if (buf_size < (long)(sizeof header))
            return 0;
        }
        continue;
      }
      else if ((changed_name[nlen = strlen(changed_name) - 1] == '/'
                || header.typeflag == DIRTYPE) && !to_stdout)
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
              skipping = (size + (BLOCKSIZE - 1)) & ~(BLOCKSIZE - 1);
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
      if (size < BLOCKSIZE)
        dsize = size;
      else if (buf_size < BLOCKSIZE)
        dsize = buf_size;
      else
        dsize = BLOCKSIZE;
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
          char *s = buf, *d = tbuf;
          while (s - buf < dsize)
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
      chmod(changed_name, mode);
    }
    batch_file_processing = 0;
    looking_for_header = 1;
    if (write_errno == ENOSPC)  /* target disk full: quit early */
    {
      bytes_out += buf_size;
      return EOF;
    }
    else if (write_errno)       /* other error: skip this file, try next */
      skipping = (size - dsize + (BLOCKSIZE - 1)) & ~(BLOCKSIZE - 1);
    else    /* skip the slack garbage to the next BLOCKSIZE-byte boundary */
      skipping = BLOCKSIZE - dsize;
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
  else if (s_switch)
    print_info_about_skipped_pax_headers();
  oread_close(f);
  method = -1;
}

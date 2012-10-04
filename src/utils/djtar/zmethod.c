/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* zmethod -- detect compressed tar files for on-the-fly decompression.
 *
 * Derived from GNU Zip program, Copyright (C) 1992-1993 Jean-loup Gailly
 * The unzip code was written and put in the public domain by Mark Adler.
 * Portions of the lzw code are derived from the public domain 'compress'
 * written by Spencer Thomas, Joe Orost, James Woods, Jim McKie, Steve Davies,
 * Ken Turkowski, Dave Mack and Peter Jannesen.
 *
 * Adaptation to DJTAR program: Eli Zaretskii, November 1995
 */

/* This software is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.  No author or distributor accepts
   responsibility to anyone for the consequences of using it or for
   whether it serves any particular purpose or works at all, unless he
   says so in writing.  Refer to the GNU General Public License
   for full details.

   Everyone is granted permission to copy, modify and redistribute
   this software, but only under the conditions described in the GNU
   General Public License.   A copy of this license is supposed to
   have been given to you along with this software so you can know your
   rights and responsibilities.  It should be in a file named COPYING.
   Among other things, the copyright notice and this notice must be
   preserved on all copies.  */


#include "tailor.h"
#include "zread.h"
#include "lzw.h"

                /* global buffers */

char inbuf[INBUFSIZ + INBUF_EXTRA];
char outbuf[OUTBUFSIZ + OUTBUF_EXTRA];
ush d_buf[DIST_BUFSIZE];
uch window[2L * WSIZE];
ush tab_prefix[1L << BITS];


                /* local variables */

int test = 0;                         /* test .gz file integrity */
char *progname;                       /* program name */
int maxbits = BITS;                   /* max bits per code for LZW */
int method = DEFLATED;                /* compression method */
int level = 6;                        /* compression level */
int exit_code = OK;                   /* program exit code */
char *ifname;                         /* input file name */
void *ifd;                            /* input file descriptor */
unsigned insize;                      /* valid bytes in inbuf */
unsigned inptr;                       /* index of next byte to be processed in inbuf */
unsigned outcnt;                      /* bytes in output buffer */
int (*decompressor)(void *) = unzip;  /* function to call */
long header_bytes, bytes_out;

/* ========================================================================
 * Check the magic number of the input file and update ofname if an
 * original name was given and to_stdout is not set.
 * Return the compression method, -1 for error, -2 for warning.
 * Set inptr to the offset of the next byte to be processed.
 * Updates time_stamp if there is one and --no-time is not used.
 * This function may be called repeatedly for an input file consisting
 * of several contiguous gzip'ed members.
 * IN assertions: there is at least one remaining compressed member.
 *   If the member is a zip file, it must be the only one.
 */

int last_member;

int
get_method(void *in)
{
  uch flags;     /* compression flags */
  char magic[2]; /* magic header */
  ulg stamp;     /* time stamp */

  ifd = in;
  magic[0] = (char)get_byte();
  magic[1] = (char)get_byte();

  method = -1;                 /* unknown yet */
  part_nb++;                   /* number of parts in gzip file */
  header_bytes = 0;
  last_member = RECORD_IO;
  /* assume multiple members in gzip file except for record oriented I/O */

  if (memcmp(magic, GZIP_MAGIC, 2) == 0
      || memcmp(magic, OLD_GZIP_MAGIC, 2) == 0)
  {
    method = (int)get_byte();
    if (method != DEFLATED)
    {
      fprintf(log_out,
              "%s: %s: unknown method %d -- get newer version of zread\n",
              progname, ifname, method);
      exit_code = ERROR;
      return -1;
    }
    decompressor = unzip;
    flags  = (uch)get_byte();

    if ((flags & ENCRYPTED) != 0)
    {
      fprintf(log_out,
              "%s: %s is encrypted -- get newer version of zread\n",
              progname, ifname);
      exit_code = ERROR;
      return -1;
    }
    if ((flags & CONTINUATION) != 0)
    {
      fprintf(log_out,
              "%s: %s is a a multi-part gzip file -- get newer version of zread\n",
              progname, ifname);
      exit_code = ERROR;
    }
    if ((flags & RESERVED) != 0)
    {
      fprintf(log_out,
              "%s: %s has flags 0x%x -- get newer version of zread\n",
              progname, ifname, flags);
      exit_code = ERROR;
    }
    stamp  = (ulg)get_byte();
    stamp |= ((ulg)get_byte()) << 8;
    stamp |= ((ulg)get_byte()) << 16;
    stamp |= ((ulg)get_byte()) << 24;

    (void)get_byte();  /* Ignore extra flags for the moment */
    (void)get_byte();  /* Ignore OS type for the moment */

    if ((flags & CONTINUATION) != 0)
    {
      unsigned part = (unsigned)get_byte();
      part |= ((unsigned)get_byte()) << 8;
      if (v_switch)
        fprintf(log_out,"%s: %s: part number %u\n", progname, ifname, part);
    }
    if ((flags & EXTRA_FIELD) != 0)
    {
      unsigned len = (unsigned)get_byte();
      len |= ((unsigned)get_byte()) << 8;
      if (v_switch)
        fprintf(log_out,"%s: %s: extra field of %u bytes ignored\n", progname, ifname, len);
      while (len--) (void)get_byte();
    }

    /* Get original file name if it was truncated */
    if ((flags & ORIG_NAME) != 0)
    {
      char orig_name[MAX_PATH_LEN], *p = orig_name;
      for (;;)
      {
        *p = (char)get_byte();
        if (*p++ == '\0') break;
        if (p >= orig_name + sizeof(orig_name))
          error("corrupted input -- file name too large");
      }
    } /* ORIG_NAME */

    /* Discard file comment if any */
    if ((flags & COMMENT) != 0)
      while (get_char() != 0) /* null */ ;
    if (part_nb == 1)
      header_bytes = inptr + 2*sizeof(long); /* include crc and size */
  }
  else if (memcmp(magic, PKZIP_MAGIC, 2) == 0 && inptr == 2
           && memcmp((char*)inbuf, PKZIP_MAGIC, 4) == 0)
  {
    /* To simplify the code, we support a zip file when alone only.
     * We are thus guaranteed that the entire local header fits in inbuf.
     */
    inptr = 0;
    decompressor = unzip;
    if (check_zipfile() != OK) return -1;
    /* check_zipfile may get ofname from the local header */
    last_member = 1;
  }
  else if (memcmp(magic, PACK_MAGIC, 2) == 0)
  {
    decompressor = unpack;
    method = PACKED;
  }
  else if (memcmp(magic, LZW_MAGIC, 2) == 0)
  {
    decompressor = unlzw;
    method = COMPRESSED;
    last_member = 1;
  }
  else if (memcmp(magic, LZH_MAGIC, 2) == 0)
  {
    decompressor = unlzh;
    method = LZHED;
    last_member = 1;
   }
   else if (memcmp(magic, BZIP2_MAGIC, 2) == 0 && inptr == 2
            && memcmp((char*)inbuf, BZIP2_MAGIC, 3) == 0)
   {
     decompressor = unbzip2;
     method = BZIP2ED;
     last_member = 1;
  }
  else if (part_nb == 1)
  {
    /* pass input unchanged by default */
    method = STORED;
    decompressor = copy;
    inptr = 0;
    last_member = 1;
  }
  if (method >= 0) return method;

  if (part_nb == 1 && z_switch)
  {
    fprintf(log_out, "\n%s: %s: not in gzip format\n", progname, ifname);
    exit_code = ERROR;
    return -1;
  }
  else
  {
    WARN((log_out, "\n%s: %s: decompression OK, trailing garbage ignored\n",
         progname, ifname));
    return -2;
  }
}

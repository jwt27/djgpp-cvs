/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */

/*
 * This file provides bzip2 support for DJTAR.
 */


#include <stdlib.h>
#include "oread.h"
#include "zread.h"
#include "unbzip2.h"

#define FALSE 0
#define TRUE  1

local int read_stream (void)
{
  int bytes_in, bytes_read;

  /* Read as much as possible */
  bytes_in = bytes_read = 0;
  do
  {
    bytes_read = oread_read(ifd, inbuf + bytes_in);
    if (bytes_read == 0 || bytes_read == EOF)
      break;
    bytes_in += bytes_read;
  }
  while (bytes_in < INBUFSIZ);

  return bytes_in;
}


#define ABORT_IF_ERROR(bzip_status)                               \
  do {                                                            \
    if ((bzip_status) != BZ_OK && (bzip_status) != BZ_STREAM_END) \
      goto error_handler;                                         \
    if ((bzip_status) == BZ_OK && nbytes == EOF &&                \
        data->avail_in == 0 && data->avail_out > 0)               \
      goto error_handler;                                         \
  } while (FALSE)

#define CHECK_IF_BZ_STREAM_END_IS_EOF(buf)                        \
  do {                                                            \
    if ((buf)[0] == 'B' && (buf)[1] == 'Z' && (buf)[2] == 'h')    \
      EOF_reached = FALSE;                                        \
    else                                                          \
      EOF_reached = TRUE;                                         \
  } while (FALSE)

int unbzip2 (void *f)
{
  bz_stream* data = NULL;
  int        bzip_status;
  int        EOF_reached = TRUE;
  int        small = 0;      /* Use fast decompressing algorithm. */
  int        verbosity = 0;  /* No verbose output at all. */
  int        nbytes = 0;



  if (f == NULL)
    return ERROR;
  ifd = f;


  /* Initialise bzip stream structure. */
  data = (bz_stream *) xmalloc (sizeof (bz_stream));
  data->next_in   = inbuf;
  data->avail_in  = INBUFSIZ;
  data->next_out  = outbuf;
  data->avail_out = OUTBUFSIZ;
  data->bzalloc   = NULL;
  data->bzfree    = NULL;
  data->opaque    = NULL;
  bzip_status = BZ2_bzDecompressInit (data, verbosity, small);
  if (bzip_status != BZ_OK)
    goto error_handler;

  /* Decompress every stream (.bz2 file) contained in this file. */
  while (TRUE)
  {
    /* Decompress the actual stream (.bz2 file) contained in this file. */
    while (bzip_status == BZ_OK)
    {
      /* Read the complete compressed block and decompress it. */
      while (bzip_status == BZ_OK)
      {
        bzip_status = BZ2_bzDecompress (data);
        ABORT_IF_ERROR (bzip_status);

        /* Reading compressed block. */
        if (data->avail_in == 0)
        {
          nbytes = read_stream ();
          data->avail_in = nbytes;
          data->next_in  = inbuf;
        }

        /* Writing decompressed block. */
        if (data->avail_out == 0)
        {
          tarread (outbuf, (unsigned long) OUTBUFSIZ);
          data->avail_out = OUTBUFSIZ;
          data->next_out  = outbuf;
        }
        if (bzip_status == BZ_STREAM_END)
        {
          nbytes = OUTBUFSIZ - data->avail_out;
          tarread (outbuf, (unsigned long) nbytes);
          CHECK_IF_BZ_STREAM_END_IS_EOF(data->next_in);
        }
      } /* End of block loop. */

      if (bzip_status != BZ_STREAM_END)
        goto error_handler;
    } /* End of stream (single .bz2 file) loop. */

    if (bzip_status != BZ_STREAM_END)
      goto error_handler;
    else
    {
      /* Release all allocated ressources for the processed stream. */
      bzip_status = BZ2_bzDecompressEnd (data);
      if (bzip_status != BZ_OK)
        goto error_handler;

      if (EOF_reached)
        break;
      else
      {
        /* Reinitialise inbuf[] with the unused but
           still availabe compressed data of the next
           stream and allocate all resources needed
           to processes it. */

        size_t i;
        for (i = 0; i < data->avail_in; i++)
          inbuf[i] = data->next_in[i];

        bzip_status = BZ2_bzDecompressInit (data, verbosity, small);
        if (bzip_status != BZ_OK)
          goto error_handler;
        data->next_in   = inbuf;
        data->next_out  = outbuf;
        data->avail_out = OUTBUFSIZ;
     }
    }
  } /* End of full file loop. */

  free (data);
  return OK;


error_handler:

  BZ2_bzDecompressEnd (data);
  free (data);
  exit_code = ERROR;
  switch (bzip_status)
  {
    case BZ_SEQUENCE_ERROR:
      error ("wrong sequence of bzip2 commands");
      break;
    case BZ_PARAM_ERROR:
      error ("wrong bzip2 parameters");
      break;
    case BZ_MEM_ERROR:
      error ("memory exhausted");
      break;
    case BZ_DATA_ERROR:
      error ("bzip2 data block corrupted");
      break;
    case BZ_DATA_ERROR_MAGIC:
      error ("not compressed with bzip2");
      break;
    case BZ_IO_ERROR:
      error ("I/O error");
      break;
    case BZ_UNEXPECTED_EOF:
      error ("unexpected end of file");
      break;
    case BZ_CONFIG_ERROR:
      error ("program has been mis-compiled");
      break;
  }

  return ERROR;
}

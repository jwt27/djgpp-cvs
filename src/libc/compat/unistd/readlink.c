/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <dir.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "xsymlink.h"

int readlink(const char * filename, char * buffer, size_t size)
{
   ssize_t       bytes_read         = 0;
   char          buf[_SYMLINK_FILE_LEN];
   char        * data_buf;
   int           fd;
   struct ffblk  file_info;

   /* Reject NULLs */
   if (!filename || !buffer)
   {
      errno = EINVAL;
      return -1;
   }

   /* Does symlink file exist at all? */
   if (findfirst(filename, &file_info, 0))
   {
      errno = ENOENT;
      return -1;
   }

   /* Is symlink file size a fixed magic value? */
   if (file_info.ff_fsize != _SYMLINK_FILE_LEN)
   {
      errno = EINVAL; /* Sad but true */
      return -1;
   }

   /* Now we check for special DJGPP symlink format */
   fd = _open(filename, O_RDONLY);
   if (fd < 0)
   {
      /* Retry with DENY-NONE share bit set. It might help in some cases
       * when symlink file is opened by another program. We don't try with
       * DENY-NONE set in the first _open() call, because it might fail under
       * some circumstances. For details, see Ralf Brown's Interrupt List,
       * description of INT 0x21, function 0x3D.
       */
      fd = _open(filename, O_RDONLY | SH_DENYNO);
      if (fd < 0)
         return -1; /* errno from open() call */
   } 

   bytes_read = read(fd, &buf, _SYMLINK_FILE_LEN);
   _close(fd);
   if (bytes_read == -1)
      return -1; /* Return errno set by _read() (_close() in worse case) */
      
   /* Check for symlink file header */
   if (strncmp(buf, _SYMLINK_PREFIX, _SYMLINK_PREFIX_LEN))
   {
      close(fd);
      errno = EINVAL;
      return -1;
   }
   
   data_buf = buf + _SYMLINK_PREFIX_LEN;
   bytes_read = strchr(data_buf, '\n') - data_buf;
   bytes_read = ((unsigned)bytes_read > size) ? size : bytes_read;
   memcpy(buffer, data_buf, bytes_read);
   return bytes_read;
}


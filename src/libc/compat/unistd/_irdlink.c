/* Copyright (C) 2004 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */

#include <libc/stubs.h>
#include <libc/symlink.h>
#include <sys/fsext.h>
#include <libc/fsexthlp.h>
#include <dir.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "xsymlink.h"

int __internal_readlink(const char * __path, int __fhandle, char * __buf, 
                        size_t __max)
{
   ssize_t       bytes_read         = 0;
   char          buf[_SYMLINK_FILE_LEN];
   char        * data_buf;
   int           fd;
   int           ret;
   off_t         old_pos = 0;
   long          file_size = 0;

   /* Reject NULL and impossible arg combinations */
   if (!__buf || (__path && __fhandle) || !(__path || __fhandle))
   {
      errno = EINVAL;
      return -1;
   }

   /* Provide ability to hook symlink support */
   if (__path)
   {
      struct ffblk  file_info;
      if (__FSEXT_call_open_handlers_wrapper(__FSEXT_readlink, &ret,
					     __path, __buf, __max))
         return ret;
      /* We will check if file exists by the way */
      if (findfirst(__path, &file_info, FA_RDONLY|FA_ARCH))
      {
         errno = ENOENT;
         return -1;
      }
      file_size = file_info.ff_fsize;
   }
   else if (__fhandle)
   {
      __FSEXT_Function *func = __FSEXT_get_function(__fhandle);
      if (func)
      {
         int rv;
         if (__FSEXT_func_wrapper(func, __FSEXT_readlink, &rv, __path))
            return rv;
      }
      file_size = filelength(__fhandle);
   }

   /* Is symlink file size a fixed magic value? */
   if (file_size != _SYMLINK_FILE_LEN)
   {
      errno = EINVAL; /* Sad but true */
      return -1;
   }

   /* Now we check for special DJGPP symlink format */

   if (__fhandle)
   {
      /* Remember old file pos */
      old_pos = tell(__fhandle);
      if (old_pos == -1)
         return -1;
      lseek(__fhandle, 0, SEEK_SET);
      fd = __fhandle;
   }
   else
   {
      fd = _open(__path, O_RDONLY);
      if (fd < 0)
      {
         /* Retry with DENY-NONE share bit set. It might help in some cases
          * when symlink file is opened by another program. We don't try with
          * DENY-NONE set in the first _open() call, because it might fail under
          * some circumstances. For details, see Ralf Brown's Interrupt List,
          * description of INT 0x21, function 0x3D.
          */
         fd = _open(__path, O_RDONLY | SH_DENYNO);
         if (fd < 0)
            return -1; /* errno from _open() call */
      } 
   }

   bytes_read = _read(fd, &buf, _SYMLINK_FILE_LEN);

   if (__fhandle)
      lseek(__fhandle, old_pos, SEEK_SET);
   else
      _close(fd);

   if (bytes_read == -1)
      return -1; /* Return errno set by _read() (_close() in worse case) */
      
   /* Check for symlink file header */
   if (strncmp(buf, _SYMLINK_PREFIX, _SYMLINK_PREFIX_LEN))
   {
      if (!__fhandle)
         close(fd);
      errno = EINVAL;
      return -1;
   }
   
   data_buf = buf + _SYMLINK_PREFIX_LEN;
   bytes_read = strpbrk(data_buf, "\r\n") - data_buf;
   if( (unsigned)bytes_read > __max ) 
     bytes_read = __max;
   memcpy(__buf, data_buf, bytes_read);
   return bytes_read;
}


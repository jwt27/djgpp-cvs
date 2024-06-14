/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dpmi.h>
#include <crt0.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

/* Sets the DPMI page attributes for all pages in the given range.
 * See the DPMI 1.0 documentation for function 0x507 (0507H) for a
 * description of what the _attributes parameter means.  Both the
 * address and number of bytes must be page-aligned.  Returns 0 on
 * success, -1 on failure.  On failure, it is possible that some
 * of the pages will have been affected.
 */
int
__djgpp_set_page_attributes (void *_our_addr, ULONG32 _num_bytes,
			     unsigned short _attributes)
{
  unsigned long p, end;
  int i, num_pages;
  short *attr;

  /* Make sure all arguments are page aligned, and attribute is legal. */
  if (((unsigned long) _our_addr & 0xfff )
      || (_num_bytes & 0xfff)
      || (_attributes & 0xff84)
      || ((_attributes & 0x3) == 2))
    {
      errno = EINVAL;
      goto fail;
    }

  /* Set up an array of page attribute information. */
  num_pages = _num_bytes / 0x1000;
  attr = (short int *)alloca (num_pages * sizeof attr[0]);
  for (i = num_pages - 1; i >= 0; i--)
    attr[i] = _attributes;

  /* Loop through the memory range, identify individual handles
   * that intersect the range, and map the appropriate memory
   * within each handle.
   */
  for (p = (uintptr_t) _our_addr, end = p + _num_bytes; p < end; )
    {
      const __djgpp_sbrk_handle *d;
      unsigned long handle_end_addr, num_pages2;
      __dpmi_meminfo meminfo;

      /* Find the memory handle corresponding to the first byte. */
      d = __djgpp_memory_handle (p);
      if (d == NULL)
        {
          errno = EINVAL;
          goto fail;
        }

      /* Base address of the memory handle must be page aligned too. */
      if (d->address & 0xfff)
        {
          errno = EINVAL;
          goto fail;
        }

      /* Find the last byte in the range that's also in the same
       * memory handle as our current starting byte.  We start with
       * the farthest away address because it will usually be in the
       * same memory handle, and we don't need to check any
       * intermediate addresses once we know the far away address is
       * in the same handle.
       */
      for (handle_end_addr = end - 0x1000;
	   handle_end_addr > p;
	   handle_end_addr -= 0x1000)
	{
	  const __djgpp_sbrk_handle *d2;

	  /* Find the memory handle corresponding to this test byte. */
	  d2 = __djgpp_memory_handle (handle_end_addr);
	  if (d2 == NULL)
            {
              errno = EINVAL;
              goto fail;
            }

	  /* Is this test byte in the same handle as the first byte? */
	  if (d2->handle == d->handle)
	    break;
	}
      handle_end_addr += 0x1000;

      /* Map the appropriate physical addresses into this handle. */
      num_pages2 = (handle_end_addr - p) / 0x1000;
      meminfo.handle  = d->handle;
      meminfo.size    = num_pages2;
      meminfo.address = p - d->address;
      if (__dpmi_set_page_attributes (&meminfo, attr))
        {
          switch (__dpmi_error)
            {
              case 0x0507: /* Unsupported function (returned by DPMI 0.9 host, error number is same as DPMI function number) */
              case 0x8001: /* Unsupported function (returned by DPMI 1.0 host) */
                errno = ENOSYS;
                break;
              case 0x8010: /* Resource unavailable (DPMI host cannot allocate internal resources to complete an operation) */
              case 0x8013: /* Physical memory unavailable */
              case 0x8014: /* Backing store unavailable */
                errno = ENOMEM;
                break;
              case 0x8002: /* Invalid state (page in wrong state for request) */
              case 0x8021: /* Invalid value (illegal request in bits 0-2 of one or more page attribute words) */
              case 0x8023: /* Invalid handle (in ESI) */
              case 0x8025: /* Invalid linear address (specified range is not within specified block) */
                errno = EINVAL;
                break;
              default: /* Other unspecified error */
                errno = EACCES;
                break;
            }
          goto fail;
        }

      /* Move on to the next memory handle. */
      p = handle_end_addr;
    }

  /* success! */
  return 0;

 fail:
  return -1;
}

/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <dpmi.h>
#include <crt0.h>
#include <errno.h>
#include <stddef.h>


/* Maps the specified number of bytes at a given address into our
 * address space.  All arguments must be page-aligned.  Returns 0 on
 * success, -1 on failure.  This routine isn't as fast as it could be,
 * but it shouldn't get called all that often.
 */
int
__djgpp_map_physical_memory (void *_our_addr, unsigned long _num_bytes,
			     unsigned long _phys_addr)
{
  unsigned long p, end;

  /* Make sure all arguments are page aligned. */
  if (((unsigned long) _our_addr & 0xfff )
      || (_phys_addr & 0xfff)
      || (_num_bytes & 0xfff))
    {
      errno = EINVAL;
      goto fail;
    }

  /* Loop through the memory range, identify individual handles
   * that intersect the range, and map the appropriate memory
   * within each handle.
   */
  for (p = (unsigned long) _our_addr, end = p + _num_bytes; p < end; )
    {
      const __djgpp_sbrk_handle *d;
      unsigned long handle_end_addr;
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
      meminfo.handle  = d->handle;
      meminfo.size    = (handle_end_addr - p) / 0x1000;  /* # pages */
      meminfo.address = p - d->address;

      if (__dpmi_map_device_in_memory_block (&meminfo,
					     (_phys_addr
					      + (p - (unsigned) _our_addr))))
        {
          switch (__dpmi_error)
            {
              case 0x0508: /* Unsupported function (returned by DPMI 0.9 host, error number is same as DPMI function number) */
              case 0x8001: /* Unsupported function (returned by DPMI 1.0 host) */
                errno = ENOSYS;
                break;
              case 0x8003: /* System integrity (invalid device address) */
                errno = ENXIO;
                break;
              case 0x8010: /* Resource unavailable (DPMI host cannot allocate internal resources to complete an operation) */
                errno = ENOMEM;
                break;
              case 0x8023: /* Invalid handle (in ESI) */
              case 0x8025: /* Invalid linear address (specified range is not within specified block or EBX/EDX is not page-aligned) */
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

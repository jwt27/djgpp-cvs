/*
 * File getfatsz.c.
 *
 * Copyright (C) 2000 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 * FAT size algorithm according to "Hardware White Paper, FAT: General 
 * Overwiew of On-Disk Format" version 1.02, May 5, 1999, Microsoft 
 * Corporation. Downloadable from <http://www.microsoft.com/hwdev/>.
 *
 */

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <errno.h>
#include <libc/farptrgs.h>

/* Returns number of bits in FAT; -1 == error. */
int 
_get_fat_size( const int drive /* drive number (1=A:). */
	      )
{
  __dpmi_regs r = {{0}};
  int size;
  unsigned long bytes_per_sector, sectors_per_cluster, reserved_sectors;
  unsigned long number_of_fats, root_entries, fat16_size, fat32_size;
  unsigned long total16_sectors, total32_sectors;
  unsigned long root_sectors, fat_size, total_sectors, data_sectors;
  unsigned long number_of_clusters;
  char file_system_string[9];
  
  /* First check that we have a FAT file system. */
  if( _get_fs_type( drive, file_system_string ) 
   || file_system_string[0] != 'F' 
   || file_system_string[1] != 'A'
   || file_system_string[2] != 'T' )
  {
    errno = ENOSYS;
    return( -1 );
  }

  r.x.ax = 0x440d;
  r.h.bl = drive;
  r.h.ch = 0x48; /* First we try a FAT32 disk drive. */
  r.h.cl = 0x60;
  r.x.ds = r.x.si = __tb >>4;
  r.x.dx = r.x.di = __tb & 0x0f;

  __dpmi_int(0x21, &r);
  if( r.x.flags & 0x01 )
  {
    /* Hmmpf! That didn't work; fall back to disk drive. */
    r.x.ax = 0x440d;
    r.h.bl = drive;
    r.h.ch = 0x08; /* Disk drive. */ 
    r.h.cl = 0x60;
    r.x.ds = r.x.si = __tb >>4;
    r.x.dx = r.x.di = __tb & 0x0f;

    __dpmi_int(0x21, &r);
    if( r.x.flags & 0x01 )
    {
      errno = ENOSYS;
      return(-1);
    }
  }

  /* +7 is offset in RBIL, the changing number is offset according to 
     Microsnoft's document and -11 is a corrective offset (the Microsnoft 
     document starts its offset counting 11 to early, freely mixing in the 
     boot sector). */
  bytes_per_sector = _farpeekw(_dos_ds, __tb+7+11-11);
  sectors_per_cluster = _farpeekb(_dos_ds, __tb+7+13-11);
  reserved_sectors = _farpeekw(_dos_ds, __tb+7+14-11);
  number_of_fats = _farpeekb(_dos_ds, __tb+7+16-11);
  root_entries = _farpeekw(_dos_ds, __tb+7+17-11);
  fat16_size = _farpeekw(_dos_ds, __tb+7+22-11);
  fat32_size = _farpeekl(_dos_ds, __tb+7+36-11);
  total16_sectors = _farpeekw(_dos_ds, __tb+7+19-11);
  total32_sectors = _farpeekl(_dos_ds, __tb+7+32-11);
  
  /* Check sectors_per_cluster, which might be zero. */
  if( sectors_per_cluster == 0 )
  {
    errno = ENOSYS;
    return( -1 );
  }

  /* Do the calculations. */
  root_sectors = ( (root_entries * 32) 
		 + (bytes_per_sector - 1)
		   ) / bytes_per_sector;
  fat_size = fat16_size ? fat16_size : fat32_size;
  total_sectors = total16_sectors ? total16_sectors : total32_sectors;
  data_sectors = total_sectors - reserved_sectors - number_of_fats*fat_size
    - root_sectors;
  number_of_clusters = data_sectors / sectors_per_cluster;
  if( number_of_clusters < 4085 )
  {
    size = 12;
  }
  else if( number_of_clusters < 65525 )
  {
    size = 16;
  }
  else
  {
    size = 32;
  }

#if 0
#include <stdio.h>
  fprintf(stderr, "bytes_per_sector = %ld.\n", bytes_per_sector);
  fprintf(stderr, "sectors_per_cluster = %ld.\n", sectors_per_cluster);
  fprintf(stderr, "reserved_sectors = %ld.\n", reserved_sectors);
  fprintf(stderr, "number_of_fats = %ld.\n", number_of_fats);
  fprintf(stderr, "root_entries = %ld.\n", root_entries);
  fprintf(stderr, "fat16_size = %ld.\n", fat16_size);
  fprintf(stderr, "fat32_size = %ld.\n", fat32_size);
  fprintf(stderr, "total16_sectors = %ld.\n", total16_sectors);
  fprintf(stderr, "total32_sectors = %ld.\n", total32_sectors);
  fprintf(stderr, "root_sectors = %ld.\n", root_sectors);
  fprintf(stderr, "data_sectors = %ld.\n", data_sectors);
  fprintf(stderr, "number_of_clusters = %ld.\n", number_of_clusters);
#endif

  return( size );
  
}


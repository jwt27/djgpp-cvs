/*
 * This is the file isfat32.c.
 *
 * Copyright (C) 2000 Martin Str@"omberg <ams@ludd.luth.se>.
 *
 * This software may be used freely so long as this copyright notice is
 * left intact. There is no warranty on this software.
 *
 */

#include <dos.h>
#include <libc/bss.h>

/* Returns 1 if the drive is formatted with FAT32; 0 otherwise. */
int 
_is_fat32( const int drive /* drive number (1=A:). */
	  )
{
  /* Check input. */
  if( 0 <= drive
   && drive <= 32
      )
  {
    return( _get_fat_size( drive ) == 32 );
  }

  /* Drives that don't exist can't be FAT32. */
  return( 0 );
}

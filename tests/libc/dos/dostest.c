/*
 * DOSTEST.C
 *
 * Test for _dos_xxxxx function group.
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * Tested with the following DOS C compilers:
 *   DJGPP v2.0
 *   Borland C\C++ 3.1
 *   Borland C\C++ 4.02
 *   Borland C\C++ 4.5
 *   Microsoft C 6.00A
 *   Microsoft C\C++ 7.00
 *   Microsoft Visual C\C++ 1.5
 *   Symantec C\C++ 6.11
 *   Watcom C\C++ 9.5
 *   Watcom C\C++ 10.0A
 */

#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>

#if defined(__GNUC__) || defined(__DPMI32__) || defined(__WATCOMC__)
#define FILESIZE 135000U    /* For protected mode C compilers */
#else
#define FILESIZE 64000U     /* For real mode C compilers */
#endif

void main(void)
{
  unsigned int drive, drives_available, result, attr, i, fd, ft;
  struct diskfree_t space;
  unsigned long freebytes;
  int handle;
  unsigned char pattern;
  unsigned char *file_buffer;
  struct find_t fs;
  struct _dosdate_t date;
  struct _dostime_t time;


  /* _dos_getdrive/_dos_setdrive test */
  _dos_getdrive(&drive);
  printf("The current drive is %c:\n", 'A' + drive - 1);
  _dos_setdrive(1, &drives_available);
  printf("The current drive is set to A:\n");
  printf("(%u logical drives are available)\n", drives_available);
  _dos_setdrive(drive, &drives_available);
  printf("The current drive is restore to %c:\n", 'A' - 1 + drive);


  /* _dos_getdiskfree test */
  if ( !_dos_getdiskfree(0, &space) )
  {
    freebytes = (unsigned long)space.avail_clusters *
                (unsigned long)space.bytes_per_sector *
                (unsigned long)space.sectors_per_cluster;
    printf("There is %lu free bytes on the current drive.\n", freebytes);
  }
  else
    puts("Unable to get free room of default drive !");


  /* _dos_setfileattr\_dos_getfileattr test */
  if ( !_dos_getfileattr("DOSTEST.C", &attr) )
  {
    printf("Attributes of DOSTEST.C file is: %c%c%c%c%c%c\n",
           ( attr & _A_ARCH )   ? 'A' : '.',
           ( attr & _A_RDONLY ) ? 'R' : '.',
           ( attr & _A_HIDDEN ) ? 'H' : '.',
           ( attr & _A_SYSTEM ) ? 'S' : '.',
           ( attr & _A_VOLID )  ? 'V' : '.',
           ( attr & _A_SUBDIR ) ? 'D' : '.');
  }
  else
    puts("Unable to get attributes of DOSTEST.C file !");


  /* _dos_creat\_dos_close\_dos_write test */
  if ( !_dos_creat("FOO.DAT", _A_ARCH, &handle) )
  {
    puts("FOO.DAT creating was successful.");
    if ( (file_buffer=malloc(FILESIZE)) != NULL )
    {
      memset(file_buffer, 0, FILESIZE);
      for(i=0, pattern=0; i < FILESIZE; i++, pattern++)
        file_buffer[i] = pattern;
      if ( !_dos_write(handle, file_buffer, FILESIZE, &result) )
        printf("%i bytes written into FOO.DAT.\n", result);
      free(file_buffer);
    }
    else
      puts("Not enough memory for file buffer.");
    _dos_getftime(handle, &fd, &ft);
    _dos_close(handle);
  }
  else
    puts("FOO.DAT creating failed.");


  /* _dos_getftime test */
  printf("FOO.DAT date and time is: %04u-%02u-%02u %02u:%02u:%02u.\n",
       ((fd >> 9) & 0x7F) + 1980U, (fd >>  5) & 0x0F, fd & 0x1F,
       (ft >> 11) & 0x1F, (ft >>  5) & 0x3F, (ft & 0x1F) * 2);


  /* _dos_creatnew test */
  if ( !_dos_creatnew("FOO.DAT", _A_ARCH, &handle) )
    puts("FOO.DAT recreating was successful (????).");
  else
    /* This is GOOD ! */
    puts("FOO.DAT recreating failed.");


  /* _dos_open\_dos_read test */
  if ( !_dos_open("FOO.DAT", O_RDWR, &handle) )
  {
    puts("FOO.DAT opening was successful.");
    if ( (file_buffer=malloc(FILESIZE)) != NULL )
    {
      memset(file_buffer, 0, FILESIZE);
      if ( !_dos_read(handle, file_buffer, FILESIZE, &result) )
        printf("%i bytes read from FOO.DAT.\n", result);
      for(i=0, pattern=0; i < result; i++, pattern++)
        if ( file_buffer[i] != pattern )
        {
          printf("Different pattern at %X: %u - %u.\n", i, file_buffer[i], pattern);
          break;
        }
      free(file_buffer);
    }
    else
      puts("Not enough memory for file buffer.");
    _dos_close(handle);
  }
  else
    puts("FOO.DAT opening failed.");


  /* _dos_findfirst\_dos_findnext test */
  puts("Contents of current directory:");
  if ( !_dos_findfirst("*.*", _A_ARCH|_A_RDONLY|_A_HIDDEN|_A_SYSTEM|_A_SUBDIR, &fs) )
  {
    do
    {
      printf("%-14s %10u %02u:%02u:%02u %02u/%02u/%04u\n",
             fs.name, fs.size,
             (fs.wr_time >> 11) & 0x1f,
             (fs.wr_time >>  5) & 0x3f,
             (fs.wr_time & 0x1f) * 2,
             (fs.wr_date >>  5) & 0x0f,
             (fs.wr_date & 0x1f),
             ((fs.wr_date >> 9) & 0x7f) + 1980U);
    } while( !_dos_findnext(&fs) );
  }


  /* _dos_gettime\_dos_getdate test. */
  _dos_getdate(&date);
  _dos_gettime(&time);
  printf("The current date and time is: %04u-%02u-%02u %02u:%02u:%02u\n",
         (unsigned)date.year, (unsigned)date.month, (unsigned)date.day,
         (unsigned)time.hour, (unsigned)time.minute, (unsigned)time.second);
}

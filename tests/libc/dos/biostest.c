/*
 * BIOSTEST.C
 *
 * Test for _bios_xxxxx function group.
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

#include <stdio.h>
#include <bios.h>

static unsigned char diskbuf[2048];

void main(void)
{
  struct diskinfo_t di;
  unsigned i, j, status, port;
  unsigned char *p, linebuf[17];
  union
  {                               /* Access equiment either as:    */
    unsigned u;                   /*   unsigned or                 */
    struct                        /*   bit fields                  */
    {
      unsigned diskflag : 1;      /* Diskette drive installed?     */
      unsigned coprocessor : 1;   /* Coprocessor? (except on PC)   */
      unsigned sysram : 2;        /* RAM on system board           */
      unsigned video : 2;         /* Startup video mode            */
      unsigned disks : 2;         /* Drives 00=1, 01=2, 10=3, 11=4 */
      unsigned dma : 1;           /* 0=Yes, 1=No (1 for PC Jr.)    */
      unsigned comports : 3;      /* Serial ports                  */
      unsigned game : 1;          /* Game adapter installed?       */
      unsigned modem : 1;         /* Internal modem?               */
      unsigned printers : 2;      /* Number of printers            */
    } bits;
  } equip;


  /* _bios_equiplist() test */
  equip.u = _bios_equiplist();
  printf( "Disk drive:          %s\n", equip.bits.diskflag ? "Yes" : "No" );
  printf( "Coprocessor:         %s\n", equip.bits.coprocessor ? "Yes" : "No" );
  printf( "Game adapter:        %s\n", equip.bits.game ? "Yes" : "No" );
  printf( "Serial ports:        %d\n", equip.bits.comports);
  printf( "Number of printers:  %d\n\n", equip.bits.printers);


  /* _bios_memsize() test */
  printf( "Size of memory:      %i K\n\n", _bios_memsize());


  /* _bios_printer() test */
  puts("Test of paralel ports:");
  for(port=0; port < equip.bits.printers; port++)
  {
    status = _bios_printer(_PRINTER_STATUS, port, 0);
    printf("  LPT%c status: PRINTER IS %s\n", '1' + port, ( status == 0x90 ) ? "READY" : "NOT READY");
  }


  /* _bios_serialcom() test */
  puts("\nTest of serial ports:");
  for(port=0; port < equip.bits.comports; port++)
  {
    status = _bios_serialcom(_COM_STATUS, port, 0);

    /* Report status of each serial port and test whether there is a
     * responding device (such as a modem) for each. If data-set-ready
     * and clear-to-send bits are set, a device is responding.
     */
    printf("  COM%c status: DEVICE IS %s\n", '1' + port, (status & 0x0030) ? "ACTIVE" : "NOT ACTIVE" );
  }


  /* _bios_disk() test, read the partition table from C: drive. */
  puts("\nContents of master boot sector:");
  di.drive    = 0x80;
  di.head     = 0;
  di.track    = 0;
  di.sector   = 1;
  di.nsectors = 1;
  di.buffer   = diskbuf;

  /* Try reading disk three times before giving up. */
  for(i=0; i<3; i++)
  {
    status = _bios_disk(_DISK_READ, &di) >> 8;
    if ( !status )
      break;
  }
  if ( status )
    printf("Disk error: 0x%.2x\n", status);
  else
  {
    for(p=diskbuf, i=j=0; i<512; i++, p++)
    {
      linebuf[j++] = (*p > 32) ? *p : '.';
      printf("%.2x ", *p);
      if (j == 16)
      {
        linebuf[j] = '\0';
        printf(" %16s\n", linebuf);
        j = 0;
      }
    }
  }

}

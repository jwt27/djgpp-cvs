/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdarg.h>
#include <stdio.h>
#include <sys/mono.h>
#include <libc/farptrgs.h>
#include <go32.h>

static int initted = 0;

#define TOPROW 5*160
#define BOTROW 20*160
#define LIN(r,c) (0xb0000 + (r)*160 + (c)*2)

static int ofs=0;

void _mono_clear(void)
{
  for (ofs=TOPROW; ofs<BOTROW; ofs += 2)
    _farpokew(_dos_ds, 0xb0000+ofs, 0x0720);
  ofs = TOPROW;
  initted = 1;
}

void _mono_putc(int ch)
{
  if (!initted)
    _mono_clear();
  if (ofs >= BOTROW)
  {
    int i;
    for (i=TOPROW; i<BOTROW-160; i+=2)
      _farpokew(_dos_ds, 0xb0000+i, _farpeekw(_dos_ds, 0xb0000+i+160));
    for (; i<BOTROW; i+=2)
      _farpokew(_dos_ds, 0xb0000+i, 0x0720);
    ofs -= 160;
  }
  switch (ch)
  {
  case '\n':
    ofs += 160;
    break;
  case '\r':
    ofs -= ofs % 160;
    break;
  case '\b':
    ofs -= 2;
    break;
  default:
    _farpokew(_dos_ds, 0xb0000+ofs, 0x0700 | (ch & 0xff));
    ofs += 2;
    break;
  }
}

void _mono_printf(const char *fmt, ...)
{
  int i;
  char buf[1000];
  va_list a = 0;
  va_start(a, fmt);
  vsprintf(buf, fmt, a);
  for (i=0; buf[i]; i++)
    _mono_putc(buf[i]);
  va_end(a);
}

/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <bios.h>
#include <fcntl.h>
#include <io.h>
#include <unistd.h>
#include <pc.h>

#include "oread.h"
#include "zread.h"

#define D_READ  2
#define D_WRITE 3

#define CACHE_SECTORS 18

typedef struct {
  int fd; /* or drive, if cm set */
  int cm, hm, sm;
  long file_ptr;
  void *cache;
  long cache_fptr;
  long cache_fptre;
  int volume_in_drive;
} oread;

void *oread_open(char *fn)
{
  oread *or;
  if ((strlen(fn) == 2) && (fn[1] == ':'))
  {
    char buf[512];
    int status;

    switch (fn[0])
    {
      case 'a':
      case 'A':
        or = (oread *)xmalloc(sizeof(oread));
        or->fd = 0;
        break;
      case 'b':
      case 'B':
        or = (oread *)xmalloc(sizeof(oread));
        or->fd = 1;
        break;
      default:
        fprintf(log_out, "Invalid drive specified: %s\n", fn);
        errno = ENODEV;
        return 0;
    }
    while ((status = biosdisk(D_READ, or->fd, 0, 0, 1, 1, buf)) == 6)
      /* wait for valid read */ ;
    if (status)
    {
      if (errno == 0)
        errno = EIO;
      return 0;
    }

    biosdisk(8, or->fd, 0, 0, 0, 0, buf);
    or->cm = (((buf[0] & 0x00C0) << 2) | (buf[1] & 0xff)) + 1;
    or->hm = buf[3] + 1;
    or->sm = buf[0] & 0x3F;
    or->file_ptr = 0L;
    or->cache = (void *)xmalloc(512 * CACHE_SECTORS);
    or->cache_fptr = -512 * CACHE_SECTORS;
    or->cache_fptre = -512 * CACHE_SECTORS;
    or->volume_in_drive = 0;
    return or;
  }
  else
  {
    int fd;
    if (*fn == '-' && fn[1] == '\0')
    {
      fd = fileno(stdin);
      setmode(fd, O_BINARY);
      fd = dup(fd);
      close(fileno(stdin));
      open("CON", O_RDONLY | O_TEXT); /* should reopen fileno(stdin) */
    }
    else
      fd = _open(fn, O_RDONLY);
    if (fd < 0)
      return 0;
    or = (oread *)xmalloc(sizeof(oread));
    or->fd = fd;
    or->cm = 0;
    return or;
  }
}

int oread_read(void *rv, void *buffer)
{
  oread *r = (oread *)rv;
  if (r->cm)
  {
    int c, h, s, v, sc;
    if ((r->file_ptr >= r->cache_fptr) && (r->file_ptr < r->cache_fptre))
    {
      memcpy(buffer, (char *)(r->cache) + r->file_ptr - r->cache_fptr, 512);
      r->file_ptr += 512;
      return 512;
    }
    s = (unsigned long)(r->file_ptr) / 512;
    h = s / r->sm;
    c = h / r->hm;
    v = c / r->cm;
    s = s % r->sm;
    h = h % r->hm;
    c = c % r->cm;
    if (v != r->volume_in_drive)
    {
      fprintf(log_out, "Please insert volume %03d . . .", v);
      fflush(log_out);
      if (getkey() == 3)
      {
        fprintf(log_out, "^C\n");
        exit(3);
      }
      fprintf(log_out, "\n");
      r->volume_in_drive = v;
      r->cache_fptr = -512*CACHE_SECTORS;
    }
    sc = r->sm - s;
    if (sc > CACHE_SECTORS)
      sc = CACHE_SECTORS;
    fprintf(log_out, "v=%02d c=%02d h=%02d s=%02d n=%02d\r", v, c, h, s, sc);
    fflush(log_out);
    if (biosdisk(D_READ, r->fd, h, c, s+1, sc, r->cache))
      biosdisk(D_READ, r->fd, h, c, s+1, sc, r->cache);
    memcpy(buffer, r->cache, 512);
    r->cache_fptr = r->file_ptr;
    r->cache_fptre = r->cache_fptr + 512 * sc;
    r->file_ptr += 512;
    return 512;
  }
  else
    return _read(r->fd, buffer, 512);
}

#if 0
void oread_skip(void *rv, long skip_bytes)
{
  oread *r = (oread *)rv;
  if (r->cm)
  {
    r->file_ptr += skip_bytes;
  }
  else
  {
    lseek(r->fd, skip_bytes, 1);
  }
}
#endif

void oread_close(void *rv)
{
  oread *r = (oread *)rv;
  if (r->cm == 0)
    _close(r->fd);
  else
    free(r->cache);
  free(r);
}

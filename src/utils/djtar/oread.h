/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef _oread_h_
#define _oread_h_

void *oread_open(char *spec);
int  oread_read(void *r, void *buffer);
void oread_skip(void *r, long skip_bytes);
void oread_close(void *r);

#define OREAD_SIZE 512

#endif

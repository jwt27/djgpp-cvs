#ifndef DOS_H
#define DOS_H

#include "djdev64/stub.h"

unsigned _dos_open(const char *pathname, unsigned mode, int *handle);
unsigned _dos_read(int handle, void *buffer, unsigned count, unsigned *numread);
unsigned _dos_write(int handle, const void *buffer, unsigned count, unsigned *numwrt);
unsigned long _dos_seek(int handle, unsigned long offset, int origin);
int _dos_close(int handle);
int _dos_link_umb(int on);

void register_dosops(struct dos_ops *dops);
void unregister_dosops(void);

#endif

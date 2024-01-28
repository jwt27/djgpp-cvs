#ifndef DJDEV64_H
#define DJDEV64_H

#include "djdev64/dj64init.h"

int djdev64_open(const char *path, const struct dj64_api *api, int api_ver,
    dj64symtab_t *st, void *arg);
int djdev64_call(int handle, int libid, int fn, unsigned char *sp);
int djdev64_ctrl(int handle, int libid, int fn, unsigned char *sp);
void djdev64_close(int handle);

#endif

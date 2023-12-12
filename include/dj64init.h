#ifndef DJ64INIT_H
#define DJ64INIT_H

#include "dj64thnk.h"

typedef int (dj64cdispatch_t)(int handle, int libid, int fn, uint8_t *sp);
#define DJ64_INIT_FN dj64init
typedef dj64cdispatch_t **(dj64init_t)(int handle, dj64dispatch_t *disp,
    dj64symtab_t *st);
dj64cdispatch_t **DJ64_INIT_FN(int handle, dj64dispatch_t *disp,
    dj64symtab_t *st);

#endif

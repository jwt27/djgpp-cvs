#ifndef DJ64INIT_H
#define DJ64INIT_H

#include "dj64thnk.h"

typedef uint32_t (dj64cdispatch_t)(int libid, int fn, uint8_t *sp,
    enum DispStat *r_stat, int *r_len);
#define DJ64_INIT_FN dj64init
typedef dj64cdispatch_t *(dj64init_t)(int handle, dj64dispatch_t *disp);
int DJ64_INIT_FN(int handle, dj64dispatch_t *disp);

#endif

#ifndef DJ64THNK_H
#define DJ64THNK_H

#include <stdint.h>

enum DispStat { DISP_OK, DISP_NORET };
enum { DJ64_RET_ABORT = -1, DJ64_RET_OK, DJ64_RET_NORET };

#define DJ64_DISPATCH_FN dj64dispatch

typedef uint32_t (dj64dispatch_t)(int fn, uint8_t *sp, enum DispStat *r_stat,
    int *r_len);
uint32_t DJ64_DISPATCH_FN(int fn, uint8_t *sp, enum DispStat *r_stat,
    int *r_len);

#endif

#ifndef DJ64THNK_H
#define DJ64THNK_H

#include <stdint.h>

enum DispStat { DISP_OK, DISP_NORET };

#define DJ64_DISPATCH_FN dj64dispatch

typedef uint32_t (dj64dispatch_t)(int fn, enum DispStat *r_stat, uint8_t *sp,
    int *r_len);
uint32_t DJ64_DISPATCH_FN(int fn, enum DispStat *r_stat, uint8_t *sp,
    int *r_len);

#define ARG1(sp)		(*(uint32_t *)(sp + 8))
#define ARG1h(sp)		(*(uint16_t *)(sp + 10))
#define ARG2(sp)		(*(uint32_t *)(sp + 12))
#define ARG2h(sp)		(*(uint16_t *)(sp + 14))
#define ARG3(sp)		(*(uint32_t *)(sp + 16))
#define ARG4(sp)		(*(uint32_t *)(sp + 20))
#define ARG5(sp)		(*(uint32_t *)(sp + 24))
#define ARG6(sp)		(*(uint32_t *)(sp + 28))
#define ARG7(sp)		(*(uint32_t *)(sp + 32))
#define ARG8(sp)		(*(uint32_t *)(sp + 36))

#endif

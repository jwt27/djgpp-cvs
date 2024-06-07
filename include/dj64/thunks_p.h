#include <stddef.h>
#include <stdint.h>
#include "util.h"

uint64_t dj64_asm_call(int num, uint8_t *sp, uint8_t len, int flags);
uint8_t *dj64_clean_stk(size_t len);
uint32_t dj64_obj_init(const void *data, uint16_t len);
void dj64_obj_done(void *data, uint32_t fa, uint16_t len);
void dj64_rm_dosobj(const void *data, uint32_t fa);

#define _TFLG_NONE 0
#define _TFLG_FAR 1
#define _TFLG_NORET 2
#define _TFLG_INIT 4

extern struct athunk _U(asm_pthunks)[];
extern const int _U(num_pthunks);
extern uint32_t _U(asm_tab)[];

uint32_t do_asm_call(int num, uint8_t *sp, uint8_t len, int flags);
uint8_t *clean_stk(size_t len);

typedef void (*_cbk_VOID)(void);
uint32_t alloc_cbk_VOID(_cbk_VOID cbk);

#define _TFLG_NONE 0
#define _TFLG_FAR 1
#define _TFLG_NORET 2
#define _TFLG_INIT 4

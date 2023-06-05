#include "sys/cdefs.h"
#define IN_ASMOBJ 1
#include "asm_incsn.h"

#undef errno
extern int __attribute__((alias("___dj_errno"))) _errno;

#include <stdio.h>
#include <string.h>
#include "asm.h"

int main()
{
    printf("Hello from C\n");
    hello_asm();
    return 0;
}

/* called from asm */
void copy_msg(void)
{
    printf("Called %s from ASM\n", __FUNCTION__);
    /* asm messages a $-terminated */
    strcpy(msg, "Hello from ASM$\n");
}

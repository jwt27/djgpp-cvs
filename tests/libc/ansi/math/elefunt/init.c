/* Initailizes the NPX */
#include <float.h>
#include "elefunt.h"

unsigned int _control87(unsigned, unsigned);

void
init(void)
{
    /* _set_cw87(0x137f); */
    _control87(0x137f, 0xffffffffU);
}

#ifndef DJTHUNKS_H
#define DJTHUNKS_H

#include <stdint.h>

void dj64_init(void);

uint8_t *djaddr2ptr(uint32_t addr);
uint32_t djptr2addr(const uint8_t *ptr);

#endif

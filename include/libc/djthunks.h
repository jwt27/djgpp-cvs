#ifndef DJTHUNKS_H
#define DJTHUNKS_H

#include <stdint.h>

void dj64_init(void);

#define PRINTF(n) __attribute__((format(printf, n, n + 1)))
void djloudprintf(const char *format, ...) PRINTF(1);
uint8_t *djaddr2ptr(uint32_t addr);
uint32_t djptr2addr(const uint8_t *ptr);

#endif

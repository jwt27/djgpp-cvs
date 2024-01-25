#ifndef DJTHUNKS_H
#define DJTHUNKS_H

#define PRINTF(n) __attribute__((format(printf, n, n + 1)))
void djloudprintf(const char *format, ...) PRINTF(1);
uint8_t *djaddr2ptr(uint32_t addr);

#endif

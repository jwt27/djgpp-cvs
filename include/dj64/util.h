#ifndef DJ64UTIL_H
#define DJ64UTIL_H

#define PRINTF(n) __attribute__((format(printf, n, n + 1)))
void djloudprintf(const char *format, ...) PRINTF(1);
void djlogprintf(const char *format, ...) PRINTF(1);

#endif

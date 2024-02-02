#include <stdint.h>

void *elf_open(char *addr, uint32_t size);
void elf_close(void *arg);
uint32_t elf_getsymoff(void *arg, const char *name);

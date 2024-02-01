#include <stdint.h>

void *elf_open(char *addr, uint32_t size, uint32_t load_offs);
void elf_close(void *arg);
uint32_t elf_getsym(void *arg, const char *name);
int elf_getsymoff(void *arg, const char *name);

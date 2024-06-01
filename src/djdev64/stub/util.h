#ifndef UTIL_H
#define UTIL_H

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#define _PAGE_MASK	(~(PAGE_SIZE-1))
/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr)	(((addr)+PAGE_SIZE-1)&_PAGE_MASK)

long _long_read(int file, char *buf, unsigned long offset,
    unsigned long size);

#endif

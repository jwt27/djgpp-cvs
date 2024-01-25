#ifndef DJ64INIT_H
#define DJ64INIT_H

#include <stdarg.h>
#include "dj64thnk.h"

typedef int (dj64cdispatch_t)(int handle, int libid, int fn, uint8_t *sp);
#define DJ64_API_VER 1
enum { DJ64_PRINT_LOG, DJ64_PRINT_TERMINAL };
struct dj64_api {
    uint8_t *(*addr2ptr)(uint32_t addr);
    uint32_t (*ptr2addr)(uint8_t *ptr);
    void (*print)(int prio, const char *format, va_list ap);
};
#define DJ64_INIT_ONCE_FN dj64init_once
typedef int (dj64init_once_t)(const struct dj64_api *api, int api_ver);
int DJ64_INIT_ONCE_FN(const struct dj64_api *api, int api_ver);

#define DJ64_INIT_FN dj64init
typedef dj64cdispatch_t **(dj64init_t)(int handle,
    dj64dispatch_t *disp,
    dj64symtab_t *st);
dj64cdispatch_t **DJ64_INIT_FN(int handle,
    dj64dispatch_t *disp,
    dj64symtab_t *st);

#endif

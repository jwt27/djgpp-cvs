/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 Borca Daniel <dborca@yahoo.com>
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 * Partly based on work by Charles Sandmann and DJ Delorie.
 * Usage of this library is not restricted in any way.  
 * ABSOLUTELY no warranties.  Contributed to the DJGPP project.
 */

#include <errno.h>
#include <string.h>
#include <dlfcn.h>

/* Last-error unresolved symbol count */
extern int _dl_unresolved_count;
/* Last-error unresolved symbol */
extern char _dl_unresolved_symbol[];

/* Desc: return the most recent error
 *
 * In  : -
 * Out : static string
 *
 * Note: each call resets the error string
 */
const char *dlerror (void)
{
 int oerrno = errno;
 errno = 0;
 if (_dl_unresolved_count) {
    static char buff[148];
    strcpy(buff, "unresolved symbol: ");
    strcat(buff, _dl_unresolved_symbol);
    _dl_unresolved_count = 0;
    return buff;
 }
 if (!oerrno) {
    return NULL;
 }
 return strerror(oerrno);
}

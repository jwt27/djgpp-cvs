/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 Borca Daniel <dborca@yahoo.com>
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 * Partly based on work by Charles Sandmann and DJ Delorie.
 * Usage of this library is not restricted in any way.  
 * ABSOLUTELY no warranties.  Contributed to the DJGPP project.
 */

#include <string.h>
#include <sys/dxe.h>


/* The list of symbol tables */
extern dxe_symbol_table **_dl_symtabs;
/* Number of symbol table */
extern int _dl_nsymtab;


/* Desc: unregister symbol table
 *
 * In  : symbol table
 * Out : 0 if success
 *
 * Note: -
 */
int dlunregsym (const dxe_symbol_table *symtab)
{
 int i;
 for (i = 0; i < _dl_nsymtab; i++) {
     if (_dl_symtabs[i] == symtab) {
        memcpy(&_dl_symtabs[i], &_dl_symtabs[i + 1], (--_dl_nsymtab - i) * sizeof(dxe_symbol_table *));
        return i;
     }
 }
 return -1;
}

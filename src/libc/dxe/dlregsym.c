/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 Borca Daniel <dborca@yahoo.com>
 * Copyright (C) 2000 Andrew Zabolotny <bit@eltech.ru>
 * Partly based on work by Charles Sandmann and DJ Delorie.
 * Usage of this library is not restricted in any way.  
 * ABSOLUTELY no warranties.  Contributed to the DJGPP project.
 */

#include <stdlib.h>
#include <sys/dxe.h>


/* The list of symbol tables */
extern const dxe_symbol_table **_dl_symtabs;
/* Number of symbol table & max space allocated for symbol tables */
extern int _dl_nsymtab, _dl_nmaxsymtab;


/* Desc: register symbol table
 *
 * In  : symbol table
 * Out : negative number if error
 *
 * Note: -
 */
int dlregsym (const dxe_symbol_table *symtab)
{
 if (_dl_nsymtab >= _dl_nmaxsymtab) {
    const dxe_symbol_table **p;
    p = realloc(_dl_symtabs, (_dl_nmaxsymtab + 8) * sizeof(dxe_symbol_table *));
    if (p == NULL) {
       return -1;
    }
    _dl_nmaxsymtab += 8;
    _dl_symtabs = p;
 }
 _dl_symtabs[_dl_nsymtab] = symtab;
 return _dl_nsymtab++;
}

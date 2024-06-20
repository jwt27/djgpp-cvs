/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DPMIWRP_H
#define DPMIWRP_H

#include "djdev64/stub.h"

#define DD(r, n, a, ...) \
r __##n a;
#define DDv(r, n) \
r __##n(void);
#define vDD(n, a, ...) \
void __##n a;
#define vDDv(n) \
void __##n(void);

#include "djdev64/dpmi_inc.h"

#undef DD
#undef DDv
#undef vDD
#undef vDDv

void register_dpmiops(struct dpmi_ops *dops);
void unregister_dpmiops(void);

#endif

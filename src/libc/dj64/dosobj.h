/*
 *  FDPP - freedos port to modern C++
 *  Copyright (C) 2018  Stas Sergeev (stsp)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DOSOBJ_H
#define DOSOBJ_H

#include <stdint.h>

void dosobj_init(void *ptr, int size);
void dosobj_reinit(void *ptr, int size);
void dosobj_free(void);
uint32_t mk_dosobj(uint16_t len);
void pr_dosobj(uint32_t fa, const void *data, uint16_t len);
void cp_dosobj(void *data, uint32_t fa, uint16_t len);
void rm_dosobj(uint32_t fa);
void dosobj_dump(void);

#endif

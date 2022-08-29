#ifndef _DEBUG_H_
#define _DEBUG_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS
 *  Copyright (C) 2011-2022  Eduardo Casino
 *
 * debug.h: Enable debug to VMware logs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#ifdef DEBUG
#include <assert.h>
#include <stdint.h>

#include "vmtool.h"

extern rpc_t rpcd;

#define ASSERT(EXPRESSION) assert(EXPRESSION)
#define DPRINTF(FORMAT, ...) dprintf(MY_NAME, __FILE__, __LINE__, FORMAT, __VA_ARGS__)
#define DPUTS(STRING) dprintf(MY_NAME, __FILE__, __LINE__, STRING)
#define DDUMP(PRFX, ADDR, SIZE) ddump(MY_NAME, __FILE__, __LINE__, PRFX, ADDR, SIZE)
extern void dprintf(const char *name, const char *file, int line, const char *format, ...);
extern void ddump(const char *name, const char *file, int line, const char *prfx, void __far *addr, uint16_t size);

#else

#define ASSERT(EXPRESSION)      {}
#define DPRINTF(FORMAT, ...)    {}
#define DPUTS(STRING)           {}
#define DDUMP(PRFX, ADDR, SIZE) {}

#endif /* DEBUG */

#endif /* _DEBUG_H_ */

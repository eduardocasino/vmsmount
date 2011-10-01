#ifndef MINILIBC_H_
#define MINILIBC_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *  Copyright (C) 2011  Eduardo Casino
 *
 * minilibc.h: some libc replacement functions
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
 
#include <stddef.h>
 
 char far *_fstrrchr_local( const char far *str, char c );
 char *strchr_local( const char *str, char c );
 void far *_fmemcpy_local( void far *dst, const void far *src, size_t num );
 char far *_fstrcpy_local( char far *dst, const char far *src );
 void *memcpy_local( void *dst, const void *src, size_t num );
 
#endif /* MINILIBC_H_ */

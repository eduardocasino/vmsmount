/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *  Copyright (C) 2011  Eduardo Casino
 *
 * minilibc.c: some libc replacement functions
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

 /*
  * TODO: Write more efficient versions, specially for *memcpy*
  */
#include <stddef.h>

#pragma data_seg("BEGTEXT", "CODE");
#pragma code_seg("BEGTEXT", "CODE");

char far *_fstrrchr_local( const char far *str, char c )
 {
	int i;
	
	for ( i = 0 ; str[i] != '\0' ; ++i );
	for ( ; i && str[i] != c ; --i );
	
	return (char far *)&str[i];
}

char *strchr_local( const char *str, char c )
 {
	int i;
	
	for ( i = 0 ; str[i] != '\0' && str[i] != c ; ++i );
	
	return (char *)&str[i];
}

void far *_fmemcpy_local( void far *dst, const void far *src, size_t num )
{
	char far *d = (char far *)dst;
	char far *s = (char far *)src;
	
	while ( num-- )
	{
		*d++ = *s++;
	}
	
	return dst;
}

char far *_fstrcpy_local( char far *dst, const char far *src )
{
	int i;
	
	for ( i = 0 ; src[i] ; ++i )
	{
		dst[i] = src[i];
	}
	
	return dst;

}

void *memcpy_local( void *dst, const void *src, size_t num )
{
	char *d = (char *)dst;
	char *s = (char *)src;
	
	while ( num-- )
	{
		*d++ = *s++;
	}
	
	return dst;
}


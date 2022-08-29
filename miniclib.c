/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS
 *  Copyright (C) 2011-2022  Eduardo Casino
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

#include <stddef.h>

char *strrchr_local(const char *str, char c)
{
    int i;

    for (i = 0; str[i] != '\0'; ++i)
        ;
    for (; i && str[i] != c; --i)
        ;

    return (char *)&str[i];
}

char *strchr_local(const char *str, char c)
{
    int i;

    for (i = 0; str[i] != '\0' && str[i] != c; ++i)
        ;

    return (char *)&str[i];
}

char __far *_fstrrchr_local(const char __far *str, char c)
{
    int i;

    for (i = 0; str[i] != '\0'; ++i)
        ;
    for (; i && str[i] != c; --i)
        ;

    return (char __far *)&str[i];
}

char __far *_fstrchr_local(const char __far *str, char c)
{
    int i;

    for (i = 0; str[i] != '\0' && str[i] != c; ++i)
        ;

    return (char __far *)&str[i];
}

void _fmemcpy_local(void __far *dst, const void __far *src, size_t num)
{
    void __far *d = dst;
    const void __far *s = src;

    // fastest implementation so far
    // using rep movsD

    __asm
    {
        push  es;
        push  ds;
        push  si;
        push  di;

        mov   cx, num;
        les   di, d;
        lds   si, s;

        // push/pop strictly not necessary
        // but doing the byte moves at last
        // favors aligned buffers

        shr   cx,1;
        pushf
        shr   cx,1;
        rep   movsd;

        adc   cx,cx;
        rep   movsw;

        popf
        adc   cx,cx;
        rep   movsb;

        pop   di;
        pop   si;
        pop   ds;
        pop   es;
    }

    return;
}

char __far *_fstrcpy_local(char __far *dst, const char __far *src)
{
    while (*src)
    {
        *dst++ = *src++;
    }
    *dst = '\0';

    return dst;
}

void *memcpy_local(void *dst, const void *src, size_t num)
{
    char *d = (char *)dst;
    char *s = (char *)src;

    while (num--)
    {
        *d++ = *s++;
    }

    return dst;
}

int strncmp_local(const char *s1, const char *s2, size_t num)
{
    while (*s1 && num)
    {
        if (*s1 != *s2)
        {
            break;
        }

        s1++, s2++, num--;
    }

    return (num == 0 ? num : *(const unsigned char *)s1 - *(const unsigned char *)s2);
}

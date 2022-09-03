/*
 * VMCHCPD
 *  A DOS device driver for managing code page changes for VMSMOUNT
 *  Copyright (C) 2022, Eduardo Casino (mail@eduardocasino.es)
 *
 * vmchcpd.h: Device driver implementation
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
#ifndef _VMCHCPD_H_
#define _VMCHCPD_H_

#include <stdint.h>

#include "device.h"
#include "dosdefs.h"

#define MY_NAME "VMTOOLSD"

#if ( _M_IX86 >= 0x200 )
#define push_all "pusha"
#define pop_all "popa"
#else
#define push_all "push ax" "push cx" "push dx" "push bx" "push sp" "push bp" "push si" "push di"
#define pop_all "pop di" "pop si" "pop bp" "pop bx" "pop bx" "pop dx" "pop cx" "pop ax"
#endif

#if ( _M_IX86 >= 0x300 )
#define push_segregs "push ds" "push es" "push fs" "push gs"
#define pop_segregs "pop gs" "pop fs" "pop es" "pop ds"
#else
#define push_segregs "push ds" "push es" "push bx" "push bx"
#define pop_segregs "pop bx" "pop bx" "pop es" "pop ds"
#endif

#ifdef USE_INTERNAL_STACK

#ifndef STACK_SIZE
#define STACK_SIZE 500
#endif

extern uint8_t *stack_bottom;
extern uint32_t dos_stack;

extern void switch_stack( void );
#pragma aux switch_stack = \
    "cli" \
    "mov word ptr [cs:dos_stack], sp" \
    "mov word ptr [cs:dos_stack+2], ss" \
    "push cs" \
    "pop ss" \
    "mov sp, word ptr [cs:stack_bottom]" \
    "sti";

extern void restore_stack( void );
#pragma aux restore_stack = \
    "cli" \
    "mov sp, word ptr [cs:dos_stack]" \
    "mov ss, word ptr [cs:dos_stack+2]" \
    "sti";

#endif /* USE_INTERNAL_STACK */

extern void push_regs( void );
#pragma aux push_regs = \
    "pushf" \
    push_all \
    push_segregs \
    "push cs" "pop ds";

extern void pop_regs( void );
#pragma aux pop_regs = \
    pop_segregs \
    pop_all \
    "popf";

extern __segment getCS( void );
#pragma aux getCS = \
    "mov ax, cs";

extern void printMsg( const char * );
#pragma aux printMsg = \
    "mov    ah, 0x9" \
    "int    0x21" \
    __parm [__dx] \
    __modify [__ax __di __es];

#define set_dos_stack_for_internal_call \
    "cli" \
    "mov    bp, sp" \
    "mov    sp, word ptr [cs:dos_stack]" \
    "mov    ss, word ptr [cs:dos_stack+2]" \
    "sti"

#define restore_stack_after_internal_call \
    "cli" \
    "push   cs" \
    "pop    ss" \
    "mov    sp, bp" \
    "sti"

extern uint16_t _open( const char *fname );
#pragma aux _open = \
    push_all \
    push_segregs \
    set_dos_stack_for_internal_call \
    "xor    cx, cx" \
    "mov    ax, 0x1226" \
    "push   bp" \
    "int    0x2f" \
    "pop    bp" \
    restore_stack_after_internal_call \
    "jnc    return" \
    "mov    ax, 0xffff" \
  "return:" \
    "mov word ptr [bp+22], ax" \
    pop_segregs \
    pop_all \
    __parm [__dx] \
    __value [__ax];

extern void _close(uint16_t handle);
#pragma aux _close = \
    push_all \
    push_segregs \
    set_dos_stack_for_internal_call \
    "mov    ax, 0x1227" \
    "push   bp" \
    "int    0x2f" \
    "pop bp" \
    restore_stack_after_internal_call \
    pop_segregs \
    pop_all \
    __parm [__bx];

extern uint16_t _read(uint16_t handle, uint16_t count, void __far *buffer);
#pragma aux _read = \
    push_all \
    push_segregs \
    set_dos_stack_for_internal_call \
    "xchg   ax, dx" \
    "mov    ds, ax" \
    "mov    ax, 0x1229" \
    "push   bp" \
    "int    0x2f" \
    "pop    bp" \
    restore_stack_after_internal_call \
    "jnc    return" \
    "mov    ax, 0xffff" \
  "return:" \
    "mov word ptr [bp+22], ax" \
    pop_segregs \
    pop_all \
    __value [__ax] \
    __parm [__bx] [__cx] [__dx __ax];

typedef uint16_t (*driverFunction_t)(void);

extern request __far *fpRequest;

extern SysVars __far *fpSysVars;

#endif /* _VMCHCPD_H_ */

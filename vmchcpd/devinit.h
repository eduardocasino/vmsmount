/*
 * VMCHCPD
 *  A DOS device driver for managing code page changes for VMSMOUNT
 *  Copyright (C) 2022, Eduardo Casino (mail@eduardocasino.es)
 *
 * devinit.h: Device driver initialization
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
#ifndef _DEVINIT_H_
#define _DEVINIT_H_

#include "dosdefs.h"

extern SysVars __far *GetSysvars(void);
#pragma aux GetSysvars = \
    "mov ah, 0x52" \
    "int 0x21" \
    "sub bx, 12" \
    __value [__es __bx];

// CPU identification routine
// (Info from http://www.ukcpu.net/Programming/Hardware/x86/CPUID/x86-ID.asp)
//
// in 808x and 8018x, flags 12-15 are always set.
// in 286, flags 12-15 are always clear in real mode.
// in 32-bit processors, bit 15 is always clear in real mode.
//                       bits 12-14 have the last value loaded into them.
//
static uint8_t RunningIn386OrHigher(void);
#pragma aux RunningIn386OrHigher =                                   \
    "pushf"          /* Save current flags */                        \
    "xor ax, ax"                                                     \
    "push ax"                                                        \
    "popf"           /* Load all flags cleared */                    \
    "pushf"                                                          \
    "pop ax"         /* Load flags back to ax */                     \
    "and ax, 0xf000" /* If 86/186, flags 12-15 will be set */        \
    "cmp ax, 0xf000"                                                 \
    "je return"                                                      \
    "mov ax, 0xf000"                                                 \
    "push ax"                                                        \
    "popf"           /* Load flags 12-15 set */                      \
    "pushf"                                                          \
    "pop ax"         /* Load flags back to ax */                     \
    "and ax, 0xf000" /* If 286, flags 12-15 will be cleared */       \
    "jz return"                                                      \
    "mov al, 0x01"                                                   \
    "return:"                                                        \
    "popf" /* Restore flags */                                       \
    __value [__al];

extern void *transient_data ;

extern uint16_t DeviceInit(void);

#endif /* _DEVINIT_H_ */

#ifndef _VMINT_H_
#define _VMINT_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS
 *  Copyright (C) 2011-2022  Eduardo Casino
 *
 * vmint.h: Basic 64bit integer arithmetic
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

#include <stdint.h>

void u64mul32(uint64_t *result, uint64_t multiplicand, uint32_t multiplier);
#pragma aux u64mul32 =                                                                     \
    "mov bx, word ptr [esp]"        /* Pointer to result */                                \
    "mov eax, dword ptr [esp+0x02]" /* Low word of multiplicand */                         \
    "mov ecx, dword ptr [esp+0x0a]" /* Multiplier */                                       \
    "mul ecx"                                                                              \
    "mov dword ptr [bx], eax"       /* Low dword of result */                              \
    "mov dword ptr [bx+4], edx"     /* Temporary storage for hogh dword of result */       \
    "mov eax, dword ptr [esp+0x06]" /* High word of multiplicand */                        \
    "mov ecx, dword ptr [esp+0x0a]" /* Multiplier */                                       \
    "mul ecx"                                                                              \
    "add dword ptr [bx+4], eax"                                                            \
    __parm __caller []                                                                     \
    __modify [__ax __bx __cx __dx __es];

void u32mul32(uint32_t *result, uint32_t multiplicand, uint32_t multiplier);
#pragma aux u32mul32 =                                                                     \
    "mov bx, word ptr [esp]"        /* Pointer to result */                                \
    "mov eax, dword ptr [esp+0x02]" /* Multiplicand */                                     \
    "mov ecx, dword ptr [esp+0x06]" /* Multiplier */                                       \
    "mul ecx"                                                                              \
    "mov dword ptr [bx], eax"       /* Low word of result */                               \
    __parm __caller []                                                                     \
    __modify [__ax __bx __cx __dx __es];

uint32_t u64div32(uint64_t *quotient, uint64_t dividend, uint32_t divisor);
#pragma aux u64div32 =                                                                     \
    "mov bx, word ptr [esp]"        /* Pointer to quotient */                              \
    "mov eax, dword ptr [esp+0x06]" /* High dword of dividend */                           \
    "xor edx, edx"                                                                         \
    "mov ecx, dword ptr [esp+0x0a]" /* divisor */                                          \
    "div ecx"                                                                              \
    "mov dword ptr [bx+4], eax"     /* High dword of quotient */                           \
    "mov eax, dword ptr [esp+0x02]" /* Low dword of dividend */                            \
    "div ecx"                                                                              \
    "mov dword ptr [bx], eax"       /* Low dword of quotient */                            \
    "mov ax, dx"                    /* return reminder in dx:ax */                         \
    "ror edx, 16"                                                                          \
    __parm __caller []                                                                     \
    __value [__dx __ax]                                                                    \
    __modify [__ax __bx __cx __dx __es];

uint32_t u32div32(uint32_t *quotient, uint32_t dividend, uint32_t divisor);
#pragma aux u32div32 =                                                                     \
    "mov bx, word ptr [esp]"        /* Pointer to quotient */                              \
    "mov eax, dword ptr [esp+0x02]" /* Dividend */                                         \
    "xor edx, edx"                                                                         \
    "mov ecx, dword ptr [esp+0x06]" /* Divisor */                                          \
    "div ecx"                                                                              \
    "mov dword ptr [bx], eax"       /* Quotient */                                         \
    "mov ax, dx"                    /* Return reminder in dx:ax */                         \
    "ror edx, 16"                                                                          \
    __parm __caller []                                                                     \
    __value [__dx __ax]                                                                    \
    __modify [__ax __bx __cx __dx __es];

void u64add32(uint64_t *result, uint64_t value1, uint32_t value2);
#pragma aux u64add32 =                                                                     \
    "mov bx, word ptr [esp]"                                                               \
    "mov eax, dword ptr [esp+0x02]"                                                        \
    "add eax, dword ptr [esp+0x0a]"                                                        \
    "mov dword ptr [bx], eax"                                                              \
    "mov eax, dword ptr [esp+0x06]"                                                        \
    "adc eax, 0"                                                                           \
    "mov dword ptr [bx+4], eax"                                                            \
    __parm __caller []                                                                     \
    __modify [__ax __bx __cx __dx __es];

void u64sub32(uint64_t *result, uint64_t value1, uint32_t value2);
#pragma aux u64sub32 =                                                                     \
    "mov bx, word ptr [esp]"                                                               \
    "mov eax, dword ptr [esp+0x02]"                                                        \
    "sub eax, dword ptr [esp+0x0a]"                                                        \
    "mov dword ptr [bx], eax"                                                              \
    "mov eax, dword ptr [esp+0x06]"                                                        \
    "sbb eax, 0"                                                                           \
    "mov dword ptr [bx+4], eax"                                                            \
    __parm __caller []                                                                     \
    __modify [__ax __bx __cx __dx __es];

#endif /* _VMINT_H_ */

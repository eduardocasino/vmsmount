#ifndef VMINT_H_
#define VMINT_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *  Copyright (C) 2011  Eduardo Casino
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
 *
 * 2020-08-18  Eduardo           * New support functions u64mul32() and u32mul32()
 */

#include <stdint.h>

void u64mul32( uint64_t *result, uint64_t multiplicand, uint32_t multiplier );
#pragma aux u64mul32 =              \
    "push eax"                       \
    "push ebx"                       \
    "push ecx"                       \
    "push edx"                       \
    "mov bx, word ptr [esp+0x0a]"    \
    "mov eax, dword ptr [esp+0x0c]"  \
    "mov ecx, dword ptr [esp+0x14]"  \
    "mul ecx"                        \
    "mov dword ptr [bx], eax"        \
    "mov dword ptr [bx+4], edx"      \   
    "mov eax, dword ptr [esp+0x10]"  \
    "mov ecx, dword ptr [esp+0x14]"  \
    "mul ecx"                        \
    "add dword ptr [bx+4], eax"      \   
    "pop edx"                        \
    "pop ecx"                        \
    "pop ebx"                        \
    "pop eax"                        \
    parm caller[];

void u32mul32( uint32_t *result, uint32_t multiplicand, uint32_t multiplier );
#pragma aux u32mul32 =              \
    "push eax"                       \
    "push ebx"                       \
    "push ecx"                       \
    "mov bx, word ptr [esp+0x0a]"    \
    "mov eax, dword ptr [esp+0x0c]"  \
    "mov ecx, dword ptr [esp+0x10]"  \
    "mul ecx"                        \
    "mov dword ptr [bx], eax"        \
    "pop ecx"                        \
    "pop ebx"                        \
    "pop eax"                        \
    parm caller[];

uint32_t u64div32( uint64_t *quotient, uint64_t dividend, uint32_t divisor );
#pragma aux u64div32 =              \
    "push ebx"                       \
    "push ecx"                       \
    "mov bx, word ptr [esp+0x0a]"    \
    "mov eax, dword ptr [esp+0x10]"  \
	"xor edx, edx"                   \
    "mov ecx, dword ptr [esp+0x14]"  \
    "div ecx"                        \
    "mov dword ptr [bx+4], eax"      \ 
    "mov eax, dword ptr [esp+0x0c]"  \
    "div ecx"                        \
    "mov dword ptr [bx], eax"        \
	"mov ax, dx"                     \
	"ror edx, 16"                    \
    "pop ecx"                        \
    "pop ebx"                        \
    parm caller[]           \
    value [dx ax];

uint32_t u32div32( uint32_t *quotient, uint32_t dividend, uint32_t divisor );
#pragma aux u32div32 =              \
    "push ebx"                       \
    "push ecx"                       \
    "mov bx, word ptr [esp+0x0a]"    \
    "mov eax, dword ptr [esp+0x0c]"  \
	"xor edx, edx"                   \
    "mov ecx, dword ptr [esp+0x10]"  \
    "div ecx"                        \
    "mov dword ptr [bx], eax"        \
	"mov ax, dx"                     \
	"ror edx, 16"                    \
    "pop ecx"                        \
    "pop ebx"                        \
    parm caller[]           \
    value [dx ax];

void u64add32( uint64_t *result, uint64_t value1, uint32_t value2 );
#pragma aux u64add32 =              \
    "push eax"                       \
    "push ebx"                       \
    "mov bx, word ptr [esp+0x06]"    \
    "mov eax, dword ptr [esp+0x08]"  \
    "add eax, dword ptr [esp+0x10]"  \
    "mov dword ptr [bx], eax"        \
    "mov eax, dword ptr [esp+0x0c]"  \
	"adc eax, 0"                     \
    "mov dword ptr [bx+4], eax"      \   
    "pop ebx"                        \
    "pop eax"                        \
    parm caller[];

void u64sub32( uint64_t *result, uint64_t value1, uint32_t value2 );
#pragma aux u64sub32 =              \
    "push eax"                       \
    "push ebx"                       \
    "mov bx, word ptr [esp+0x06]"    \
    "mov eax, dword ptr [esp+0x08]"  \
    "sub eax, dword ptr [esp+0x10]"  \
    "mov dword ptr [bx], eax"        \
    "mov eax, dword ptr [esp+0x0c]"  \
    "sbb eax, 0"                     \
    "mov dword ptr [bx+4], eax"      \  
    "pop ebx"                        \
    "pop eax"                        \
    parm caller[];


#endif /* VMINT_H_ */

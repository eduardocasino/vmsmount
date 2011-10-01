#ifndef VMCALL_H_
#define VMCALL_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *
 * This file is a derivative work of the VMware Command Line Tools, by Ken Kato
 *        http://sites.google.com/site/chitchatvmback/
 *
 * Copyright (c) 2002-2008,	Ken Kato
 * Copyright (C) 2011  Eduardo Casino
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

#pragma pack(1)
typedef struct Regs {
	union {
		uint8_t byte[4];
		struct {
			uint16_t low;
			uint16_t high;
		} halfs;
		uint32_t word;
	} eax;
	union {
		uint8_t byte[4];
		struct {
			uint16_t low;
			uint16_t high;
		} halfs;
		uint32_t word;
	} ebx;
	union {
		uint8_t byte[4];
		struct {
			uint16_t low;
			uint16_t high;
		} halfs;
		uint32_t word;
	} ecx;
	union {
		uint8_t byte[4];
		struct {
			uint16_t low;
			uint16_t high;
		} halfs;
		uint32_t word;
	} edx;
	union {
		uint8_t byte[4];
		struct {
			uint16_t low;
			uint16_t high;
		} halfs;
		uint32_t word;
	} ebp;
	union {
		uint8_t byte[4];
		struct {
			uint16_t low;
			uint16_t high;
		} halfs;
		uint32_t word;
	} edi;
	union {
		uint8_t byte[4];
		struct {
			uint16_t low;
			uint16_t high;
		} halfs;
		uint32_t word;
	} esi;
} CREGS;

extern void __cdecl VmwCommand(CREGS *);
extern void __cdecl VmwRpcIns(CREGS *);
extern void __cdecl VmwRpcOuts(CREGS *);

#endif	/* VMCALL_H_ */


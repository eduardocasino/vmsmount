; VMware backdoor call functions assembly source for DOS/NASM
;
; Copyright (c) 2002-2008, Ken Kato
;   http://sites.google.com/site/chitchatvmback/
;
;   Ported to NASM by Eduardo Casino
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
; MA  02110-1301, USA.
;

CPU 386
BITS 16

SEGMENT BEGTEXT CLASS=CODE PUBLIC

JUMP_ALIGN EQU 2

GLOBAL _VmwCommand
GLOBAL _VmwRpcIns
GLOBAL _VmwRpcOuts

%macro LOAD_REGS 0
	movzx eax, word [esp+2]
	pushad
	push eax
	mov esi, [eax+18h]
	mov edi, [eax+14h]
	mov ebp, [eax+10h]
	mov edx, [eax+0ch]
	mov ecx, [eax+08h]
	mov ebx, [eax+04h]
	mov eax, [eax]
%endmacro
	
%macro STORE_REGS 0
	xchg [esp], eax
	mov [eax+18h], esi
	mov [eax+14h], edi
	mov [eax+10h], ebp
	mov [eax+0ch], edx
	mov [eax+08h], ecx
	mov [eax+04h], ebx
	pop dword [eax]
	popad
%endmacro

; __cdecl void VmwCommand(reg_t *regs)
;
ALIGN JUMP_ALIGN
_VmwCommand:

	LOAD_REGS

	in eax, dx

	STORE_REGS
	
	ret


; __cdecl void VmwRpcIns(reg_t *regs)
;
ALIGN JUMP_ALIGN
_VmwRpcIns:

	LOAD_REGS
	
	push es
	push ds
	pop es
	pushf
	cld
	rep insb
	popf
	pop es
	
	STORE_REGS
	
	ret

; __cdecl void VmwRpcOuts(reg_t *regs)
;
ALIGN JUMP_ALIGN
_VmwRpcOuts:

	LOAD_REGS
	
	pushf
	cld
	rep outsb
	popf
	
	STORE_REGS

	ret


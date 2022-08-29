#ifndef _REDIR_H_
#define _REDIR_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS
 *  Copyright (C) 2011-2022  Eduardo Casino
 *
 * redir.h: Redirector interface
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

#include <dos.h>
#include <stdint.h>

#include "dosdefs.h"
#include "vmshf.h"

#define VMSMOUNT_MAGIC 'SF'

#pragma pack(push, 1)

typedef struct
{
    char signature[9];                // "VMSMOUNT"
    uint16_t psp;                     // Our PSP
    void __far *ourHandler;           // Our handler (points to Int2fRedirector() )
    void __far *previousHandler;      // Handler we chain to and must be restored when uninstalled
    void __far *ourClockHandler;      // Our clock interrupt handler (points to Int1cHandler() )
    void __far *previousClockHandler; // Handler we chain to and must be restored when uninstalled
    rpc_t __far *fpRpci;              // Pointer to HGFS session data
    rpc_t __far *fpTclo;              // Pointer to TCLO session data
#ifdef DEBUG
    rpc_t __far *fpRpcd;     // Pointer to RPCI channel for debugging
    uint8_t __far *ourStack; // Pointer to internal stack
#endif
} Signature;

#pragma pack(pop)

extern void(__interrupt __far *fpPrevInt2fHandler)();

#define STACK_SIZE 300
extern uint8_t newStack[STACK_SIZE];
extern uint8_t lfn;
extern uint8_t driveNum;
extern CDS __far *fpCDS;
extern SDA __far *fpSDA;
extern SDB __far *fpSDB;
extern FDB __far *fpFDB;
extern SFTT __far *fpFileTable;
extern char __far *fpFcbName1;
extern char __far *fpFcbName2;
extern char __far *fpFileName1;
extern char __far *fpFileName2;
extern char __far *fpCurrentPath;
extern char __far *fpLongFileName1;
extern char __far *fpLongFileName2;

extern __segment myDS;

extern void __declspec(naked) __far Int2fRedirector(void);
extern uint16_t BeginOfTransientBlockNoLfn(void);

extern __segment __far get_tsr_cs(void);

#endif /* _REDIR_H_ */

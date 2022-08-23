#ifndef REDIR_H_
#define REDIR_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *  Copyright (C) 2011  Eduardo Casino
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
 *
 * 2011-10-15  Eduardo           * Support for configurable buffer size using
 *                                 the transient code space
 * 2011-10-17  Eduardo           * Signature struct for uninstallation
 * 2011-11-02  Eduardo           * Add partial Long File Name support
 * 2022-08-23  Eduardo           * Port to OW 2.0
 * 2022-08-23  Eduardo           * Debugging support
 * 2022-08-23  Eduardo           * Implement CloseAll()
 * 2022-08-23  Eduardo           * Make proper use of packing pragmas
 * 
 */

#include <dos.h>
#include <stdint.h>
#include "dosdefs.h"
#include "vmshf.h"

#define VMSMOUNT_MAGIC 'SF'

#pragma pack( push, 1 )

typedef struct {
	char		signature[9];			// "VMSMOUNT"
	uint16_t	psp;					// Our PSP
	void far	*ourHandler;			// Our handler (points to Int2fRedirector() )
	void far	*previousHandler;		// Handler we chain to and must be restored when uninstalled
	rpc_t far	*fpRpci;					// Pointer to HGFS session data
#ifdef DEBUG
	rpc_t far	*fpRpcd;				// Pointer to RPCI channel for debugging
	uint8_t far	*ourStack;				// Pointer to internal stack
#endif
} Signature;

#pragma pack( pop )

extern void (__interrupt __far *fpPrevInt2fHandler)();

#define STACK_SIZE 300
extern uint8_t		newStack[STACK_SIZE];
extern uint8_t		lfn;
extern uint8_t		driveNum;
extern CDS 			far *fpCDS;
extern SDA			far *fpSDA;
extern SDB			far *fpSDB;
extern FDB			far *fpFDB;
extern SFTT			far *fpFileTable;
extern char			far *fpFcbName1;
extern char			far *fpFcbName2;
extern char			far *fpFileName1;
extern char			far *fpFileName2;
extern char			far *fpCurrentPath;
extern char			far *fpLongFileName1;
extern char			far *fpLongFileName2;

extern __segment myDS;

extern void __declspec(naked) far Int2fRedirector( void );
extern uint16_t BeginOfTransientBlockNoLfn( void );

extern __segment __far get_tsr_cs(void);

#endif /* REDIR_H_ */

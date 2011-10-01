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
 */

#include <dos.h>
#include <stdint.h>
#include "dosdefs.h"
#include "vmshf.h"

#define VMSMOUNT_MAGIC 'SF'

extern void (__interrupt __far *fpPrevInt2fHandler)();
 
extern uint8_t		driveNum;
extern CDS 			far *fpCDS;
extern SDA			far *fpSDA;
extern SDB			far *fpSDB;
extern FDB			far *fpFDB;
extern char			far *fpFcbName1;
extern char			far *fpFcbName2;
extern char			far *fpFileName1;
extern char			far *fpFileName2;
extern char			far *fpCurrentPath;

extern __segment myDS;

void __interrupt far Int2fRedirector(union INTPACK);
uint16_t GetSizeOfResidentSegmentInParagraphs(void);
	
#endif /* REDIR_H_ */

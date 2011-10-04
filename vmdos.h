#ifndef VMDOS_H_
#define VMDOS_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *  Copyright (C) 2011  Eduardo Casino
 *
 * vmdos.h: Conversions from/to vmware and DOS
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
 * 2011-10-04  Eduardo           * New parameter for FNameToFcbName()
 *                                 (Omit '.' and '..' if root dir)
 *
 */
 
#include <stdint.h>
#include "dosdefs.h"
#include "vmshf.h"

extern int32_t gmtOffset;
extern uint8_t far *fpFUcase;
extern FChar far *fpFChar;

uint32_t FTimeToFatTime( uint64_t );
uint8_t FModeToFatAttr( VMShfAttr * );
VMShfAttr *FatAttrToFMode( uint8_t );
uint32_t DosExtActionToOpenAction( uint16_t );
int FNameToFcbName( char *fcbName, char *fName, uint32_t fNameLen, uint8_t isRoot );
int DosPathToPortable(uint8_t *dst, uint8_t far *src);
uint16_t VmshfStatusToDosError( uint32_t );

#endif /* VMDOS_H_ */

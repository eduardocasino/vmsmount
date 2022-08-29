#ifndef _VMAUX_H_
#define _VMAUX_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS
 *  Copyright (C) 2011-2022  Eduardo Casino
 *
 * vmaux.h: VM functions to be called BEFORE going resident
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

#include "vmshf.h"

#include <stdint.h>

int VMAuxCheckVirtual(void);
int VMAuxBeginSession(rpc_t __far *, rpc_t __far *
#ifdef DEBUG
                      ,
                      rpc_t __far *
#endif
);
void VMAuxEndSession(rpc_t __far *, rpc_t __far *
#ifdef DEBUG
                     ,
                     rpc_t __far *
#endif
);

int VMAuxSharedFolders(rpc_t __far *);

#endif /* _VMAUX_H_ */

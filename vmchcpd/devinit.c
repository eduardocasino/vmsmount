/*
 * VMCHCPD
 *  A DOS device driver for managing code page changes for VMSMOUNT
 *  Copyright (C) 2022, Eduardo Casino (mail@eduardocasino.es)
 *
 * devinit.c: Device driver initialization
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

#include <i86.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "debug.h"
#include "devinit.h"
#include "vmchcpd.h"
#include "vmfunc.h"

#pragma data_seg("_CODE")

//
// Place here any variables or constants that should go away after initialization
//
static char hellomsg[] = "\r\nVMCHCPD Version 0.1 - (C) 2022 Eduardo Casino - GNU GPL Ver. 2.0\r\n$";
static char errormsg[] = "ERROR: VMware not detected. Not installing.\r\n$";

uint16_t DeviceInit(void)
{
    printMsg(hellomsg);

    if (RunningIn386OrHigher() && VMCheckVirtual())
    {
#ifdef DEBUG
        VMRpcOpen();
#endif

        fpSysVars = GetSysvars();

        fpRequest->r_endaddr = MK_FP(getCS(), &transient_data);
    }
    else
    {
        printMsg(errormsg);

        fpRequest->r_endaddr = MK_FP(getCS(), 0);
    }

    return 0;
}

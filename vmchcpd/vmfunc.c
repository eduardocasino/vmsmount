/*
 * VMCHCPD
 *  A DOS device driver for managing code page changes for VMSMOUNT
 *  Copyright (C) 2022, Eduardo Casino (mail@eduardocasino.es)
 *
 * vmfunc.c: VMware interface functions
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

#include <stdbool.h>

#include "debug.h"
#include "vmcall.h"
#include "vmtool.h"

#pragma data_seg("_CODE")

#ifdef DEBUG
void VMRpcOpen(void)
{
    CREGS r;

    r.eax.word = VMWARE_MAGIC;
    r.ebx.word = VMRPC_OPEN_RPCI | VMRPC_ENHANCED;
    r.ecx.word = VMCMD_INVOKE_RPC | VMRPC_OPEN;
    r.edx.word = VMWARE_CMD_PORT;
    r.ebp.word = r.edi.word = r.esi.word = 0;

    _VmwCommand(&r);

    if (r.eax.word || r.ecx.word != 0x00010000L || r.edx.halfs.low)
    {

        rpcd.channel = VMRPC_INVALID_CHANNEL;
    }
    else
    {
        rpcd.channel = r.edx.word;
        rpcd.cookie1 = r.esi.word;
        rpcd.cookie2 = r.edi.word;
    }

    return;
}
#endif

bool VMCheckVirtual(void)
{
    CREGS r;

    r.eax.word = VMWARE_MAGIC;
    r.ebx.word = ~VMWARE_MAGIC;
    r.ecx.halfs.high = 0xFFFF;
    r.ecx.halfs.low = VMCMD_GET_VERSION;
    r.edx.word = VMWARE_CMD_PORT;
    r.ebp.word = r.edi.word = r.esi.word = 0;

    _VmwCommand(&r);

    if (r.ebx.word == VMWARE_MAGIC)
    {
        return true;
    }
    else
    {
        return false;
    }
}

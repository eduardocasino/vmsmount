/*
 * VMCHCPD
 *  A DOS device driver for managing code page changes for VMSMOUNT
 *  Copyright (C) 2022, Eduardo Casino (mail@eduardocasino.es)
 *
 * vmchcpd.c: Device driver implementation
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

// TODO: Check also if VBSF is running
//

#include <dos.h>
#include <stddef.h>
#include <stdint.h>

#include "debug.h"
#include "device.h"
#include "devinit.h"
#include "nlscmds.h"
#include "redir.h"
#include "vmcall.h"
#include "vmchcpd.h"
#include "vmtool.h"

static uint8_t our_stack[STACK_SIZE];
uint8_t *stack_bottom = our_stack + STACK_SIZE;
uint32_t dos_stack;

request __far *fpRequest = (request __far *)0;

SysVars __far *fpSysVars = (SysVars __far *)0;

static uint16_t __far *fpUnicodeTbl = (uint16_t __far *)0;

#define FILE_NAME_SIZE 13  // leading '\' + Name + '.' + ext
#define MAX_PATH_LEN   80  // Including terminating null
static char unicodeTblPath[MAX_PATH_LEN] = {0};

static inline uint16_t CmdSetNls(SETNLS __far *command)
{
    if (command->ptr_size == 4 && command->path_size != 0 && command->path_size <= MAX_PATH_LEN - FILE_NAME_SIZE)
    {
        register uint8_t i;

        fpUnicodeTbl = command->unicode_table;

        for (i = 0; i < command->path_size; ++i)
        {
            unicodeTblPath[i] = command->table_path[i];
        }

        return S_DONE;
    }

    return (S_DONE | S_ERROR | E_WRITE);
}

int _fstrncmp_local(const char __far *s1, const char *s2, size_t num)
{
    while (*s1 && num)
    {
        if (*s1 != *s2)
        {
            break;
        }

        s1++, s2++, num--;
    }

    return (num == 0 ? num : *(const unsigned char __far *)s1 - *(const unsigned char *)s2);
}

static bool IsVMRedirectorInstalled(void)
{
    int driveNum;
    CDS __far *currDir;

    for (driveNum = 4; driveNum < fpSysVars->lastDrive; ++driveNum)
    {
        currDir = &fpSysVars->currDir[driveNum];
        if (currDir->u.Net.parameter == VMSMOUNT_MAGIC)
        {
            break;
        }
    }

    // I'm going to be a bit paranoid here
    //
    if (driveNum < fpSysVars->lastDrive)
    {
        if ((uint32_t)currDir->u.Net.redirIFSRecordPtr != 0ui32 &&
            (uint32_t)currDir->u.Net.redirIFSRecordPtr != 0xffffffffui32)
        {
            Signature __far *sig = (Signature __far *)currDir->u.Net.redirIFSRecordPtr;

            if (!_fstrncmp_local(sig->signature, "VMSMOUNT", 9))
            {
                // Double check that this is actually the instance we know of
                //
                if (FP_SEG(sig->ourHandler) == FP_SEG(fpUnicodeTbl))
                {
                    return true;
                }
            }
        }
    }

    DPUTS("VMSMOUNT NOT found");
    return false;
}

static inline uint16_t loadUnicodeTable(void)
{
    NLSDATA __far *nlsData = (NLSDATA __far *)fpRequest->r_data;

    uint16_t retCode = S_DONE | S_ERROR | E_WRITE;

    if (fpUnicodeTbl == (uint16_t __far *)0 || !IsVMRedirectorInstalled())
    {
        return S_DONE;
    }

    if (nlsData->length >= 2)
    {
        int i, nc;

        for (i = 0; unicodeTblPath[i] != '\0' && i < MAX_PATH_LEN - FILE_NAME_SIZE; ++i)
            ;

        if (unicodeTblPath[i] == '\0')
        {
            int cp = nlsData->codePage;
            uint16_t handle;
            nc = i;

            unicodeTblPath[i++] = '\\';
            unicodeTblPath[i++] = 'c';
            if (cp < 1000)
            {
                unicodeTblPath[i++] = 'p';
            }
            else
            {
                unicodeTblPath[i++] = (cp / 1000) + '0';
                cp %= 1000;
            }
            unicodeTblPath[i++] = (cp / 100) + '0';
            cp %= 100;
            unicodeTblPath[i++] = (cp / 10) + '0';
            cp %= 10;
            unicodeTblPath[i++] = cp + '0';
            unicodeTblPath[i++] = 'u';
            unicodeTblPath[i++] = 'n';
            unicodeTblPath[i++] = 'i';
            unicodeTblPath[i++] = '.';
            unicodeTblPath[i++] = 't';
            unicodeTblPath[i++] = 'b';
            unicodeTblPath[i++] = 'l';
            unicodeTblPath[i++] = '\0';

            handle = _open(unicodeTblPath);
            DPRINTF("handle = %d", handle);

            unicodeTblPath[nc] = '\0';    // Restore path

            if (handle != 0xffff)
            {
                static uint8_t c;

                for (i = 0; i < 256 && 1 == _read(handle, 1, &c) && c != 0x01; ++i)
                    ;

                if (c == 0x01)
                {
                    if (256 == _read(handle, 256, fpUnicodeTbl))
                    {
                        DDUMP("fpUnicodeTbl", fpUnicodeTbl, 256);
                        retCode = S_DONE;
                    }
                }

                _close(handle);
            }
        }
    }
    
    return retCode;
}

static uint16_t GenericIOCTL(void)
{
    DPUTS("GenericIOCTL()");

    if (fpRequest->r_fun == 0x004A)
    {
        return loadUnicodeTable();
    }

    return (S_DONE | S_ERROR | E_CMD);
}

static uint16_t IOCTLOutput(void)
{
    CHCPCMD __far *command = (CHCPCMD __far *)fpRequest->r_trans;

    DPUTS("IOCTLOutput()");

    if (fpRequest->r_count >= sizeof(CHCPCMD))
    {
        switch (command->num)
        {
        case DEV_CTRL_SET_NLS:
            return CmdSetNls(&command->cmd.set_nls);
            break;

        default:
            return (S_DONE | S_ERROR | E_CMD);
            break;
        }
    }

    return (S_DONE | S_ERROR | E_LENGTH);
}

static uint16_t Open(void)
{
    DPUTS("Open()");

    return S_DONE;
}

static uint16_t Close(void)
{
    DPUTS("Close()");

    return S_DONE;
}

static driverFunction_t dispatchTable[] = {
    DeviceInit,     // 0x00 Initialize
    NULL,           // 0x01 MEDIA Check
    NULL,           // 0x02 Build BPB
    NULL,           // 0x03 Ioctl In
    NULL,           // 0x04 Input (Read)
    NULL,           // 0x05 Non-destructive Read
    NULL,           // 0x06 Input Status
    NULL,           // 0x07 Input Flush
    NULL,           // 0x08 Output (Write)
    NULL,           // 0x09 Output with verify
    NULL,           // 0x0A Output Status
    NULL,           // 0x0B Output Flush
    IOCTLOutput,    // 0x0C Ioctl Out
    Open,           // 0x0D Device Open
    Close,          // 0x0E Device Close
    NULL,           // 0x0F Removable MEDIA
    NULL,           // 0x10 Output till busy
    NULL,           // 0x11 Unused
    NULL,           // 0x12 Unused
    GenericIOCTL,   // 0x13 Generic Ioctl
    NULL,           // 0x14 Unused
    NULL,           // 0x15 Unused
    NULL,           // 0x16 Unused
    NULL,           // 0x17 Get Logical Device
    NULL,           // 0x18 Set Logical Device
    NULL            // 0x19 Ioctl Query
};

static driverFunction_t currentFunction;

void __far DeviceInterrupt(void)
#pragma aux DeviceInterrupt __parm []
{
    switch_stack();

    push_regs();

    if (fpRequest->r_command > C_MAXCMD || NULL == (currentFunction = dispatchTable[fpRequest->r_command]))
    {
        fpRequest->r_status = S_DONE | S_ERROR | E_CMD;
    }
    else
    {
        fpRequest->r_status = currentFunction();
    }

    DDUMP("our_stack", our_stack, STACK_SIZE);

    pop_regs();

    restore_stack();
}

void __far DeviceStrategy(request __far *req)
#pragma aux DeviceStrategy __parm [__es __bx]
{
    fpRequest = req;
}

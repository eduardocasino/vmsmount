/*
 * VMSMOUNT - A network redirector for mounting VMware's Shared Folders in DOS 
 * Copyright (C) 2011-2022  Eduardo Casino
 *
 * toolsd.c: VMWare tools daemon
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
 */

#include <dos.h>
#include <stdint.h>

#include "globals.h"
#include "vmtool.h"
#include "miniclib.h"
#include "debug.h"

#define HEARTBEAT_INTERVAL 18	// (1 sec)

#define TCLO_PING				"ping"
#define TCLO_RESET				"reset"
#define TCLO_VIX_CMD			"Vix_1_"
#define TCLO_VIX_MOUNT			TCLO_VIX_CMD "Mount_Volumes"
#define TCLO_SUCCESS			"OK "
#define TCLO_ERROR				"ERROR "
#define TCLO_RST_SUCCESS		TCLO_SUCCESS "ATR toolbox"
#define TCLO_UNKNOWN_CMD		TCLO_ERROR "Unknown command"
#define TCLO_VIX_SUCCESS		TCLO_SUCCESS "0 0"
#define TCLO_VIX_UNSUPPORTED	TCLO_ERROR "6 0"

#define TCLO_CMD_MAX_LEN	160
typedef union {
	struct {
		uint8_t command[TCLO_CMD_MAX_LEN+1];
		uint32_t commandLen;
	} c;
	struct {
		uint8_t reply[TCLO_CMD_MAX_LEN+1];
		uint32_t replyLen;
	} r;
} TCLOMsg;

#ifdef DEBUG
#define DEB_STACK_SIZE 256
uint8_t debStack[DEB_STACK_SIZE] = {0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 
									0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 
									0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 
									0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 
									0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC };

static uint16_t dosSS;
static uint16_t dosSP;
#endif

PUBLIC rpc_t tclo =
{
	0,				// channel
	0,				// cookie1
	0				// cookie2
};

void (__interrupt __far *fpPrevInt1cHandler)();

static void Int1cToolsd(union INTPACK regset)
#pragma aux Int1cToolsd "*" parm caller []  modify [ax bx cx dx si di es gs fs]
{
	// Do not use stack (see how to fix it -zu)
	static TCLOMsg msg = { 0 };
	static ticks = HEARTBEAT_INTERVAL;
	static uint16_t id;
	static int ret;

#ifdef DEBUG
	// static int dump = 0;

	// Save ss:sp and switch to our internal stack.
	// We need this because the debug functions expect SS==DS
	//
	_asm
	{
		mov dosSS, ss
		mov dosSP, sp
		mov ax, ds
		mov ss, ax
		mov sp, (offset debStack) + DEB_STACK_SIZE
	};
/*
	if (dump++ > 360)
	{
		dump= 0;
		DDUMP(&debStack[0], DEB_STACK_SIZE);
	}
*/
#endif	

	if ( msg.r.replyLen || ++ticks > HEARTBEAT_INTERVAL )
	{
		ticks = 0;	// Reset heartbeat counter

		ret = VMRpcSend( &tclo, &msg.r.reply, msg.r.replyLen );
		msg.r.replyLen = 0;	// This also sets msg.c.commandLen to 0

		if ( ret )
		{
			goto chain;
		}

		ret = VMRpcRecvLen( &tclo, &msg.c.commandLen, &id );

		if ( ret || msg.c.commandLen == 0 )
		{
			goto chain;
		}

		if ( msg.c.commandLen > TCLO_CMD_MAX_LEN) msg.c.commandLen = TCLO_CMD_MAX_LEN;

		if ( VMRpcRecvDat( &tclo, &msg.c.command, msg.c.commandLen, id ) == 0 )
		{
			if ( !strncmp_local( msg.c.command, TCLO_RESET, sizeof(TCLO_RESET)-1 ) )
			{
				msg.r.replyLen = sizeof(TCLO_RST_SUCCESS) - 1;
				memcpy_local( &msg.r.reply, TCLO_RST_SUCCESS, msg.r.replyLen );
			}
			else if ( !strncmp_local( msg.c.command, TCLO_PING, sizeof(TCLO_PING)-1 ) )
			{
				msg.r.replyLen = sizeof(TCLO_SUCCESS) - 1;
				memcpy_local( &msg.r.reply, TCLO_SUCCESS, msg.r.replyLen );
			}
			else if ( !strncmp_local( msg.c.command, TCLO_VIX_MOUNT, sizeof(TCLO_VIX_MOUNT)-1 ) )
			{
				msg.r.replyLen = sizeof(TCLO_VIX_SUCCESS) - 1;
				DPRINTF("Received TCLO Vix command: %s", msg.c.command);
				memcpy_local( &msg.r.reply, TCLO_VIX_SUCCESS, msg.r.replyLen );
			}
			else if ( !strncmp_local( msg.c.command, TCLO_VIX_CMD, sizeof(TCLO_VIX_CMD)-1 ) )
			{
				msg.r.replyLen = sizeof(TCLO_VIX_UNSUPPORTED) - 1;
				DPRINTF("Received unsupported TCLO Vix command: %s", msg.c.command);
				memcpy_local( &msg.r.reply, TCLO_VIX_UNSUPPORTED, msg.r.replyLen );
			}			
			else
			{
#ifdef DEBUG
				msg.c.command[msg.c.commandLen] = '\0';
				DPRINTF("Received unsupported TCLO command: %s", msg.c.command);
#endif
				msg.r.replyLen = sizeof(TCLO_UNKNOWN_CMD) - 1;
				memcpy_local( &msg.r.reply, TCLO_UNKNOWN_CMD, msg.r.replyLen );
			}
		}
	}

chain:
#ifdef DEBUG
	_asm
	{
		mov ss, dosSS
		mov sp, dosSP
	};
#endif
	return;

}

void __declspec(naked) __far Int1cHandler(void)
{
	__asm {
		pusha
		push ds
		push es
		push fs
		push gs

		mov bp, sp
		push cs
		pop ds

		call Int1cToolsd
		
        mov  bx, word ptr [bp+28]     ; restore flags
        and  bx, 0fcffh               ; except for IF and TF
        push bx
		popf
		pop gs
		pop fs
		pop es
		pop ds
		popa

		jmp dword ptr cs:fpPrevInt1cHandler
	}
}


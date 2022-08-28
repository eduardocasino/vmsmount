/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *
 * vmaux.c: VM functions to be called BEFORE going resident
 *
 * This file is a derivative work of the VMware Command Line Tools, by Ken Kato
 *        http://sites.google.com/site/chitchatvmback/
 *
 * Copyright (c) 2002-2008, Ken Kato
 * Copyright (c) 2011,      Eduardo Casino
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
 *  2011-10-11  Eduardo           * Use new inlined RPC backdoor functions
 *  2011-10-15  Eduardo           * New verbosity options and code cleanup
 *  2011-10-17  Eduardo           * Pass session info as parameter to
 *                                  VMAuxBeginSession() and VMAuxEndSession()
 *  2011-11-06  Eduardo           * New message printing macros
 *  2022-08-23  Eduardo           * Debugging support
 *  2022-08-23  Eduardo           * Use inlines for virtual machine communication
 *  2022-08-25  Eduardo           * Separate channel initialization and VMSH session
 */
 
/*
 *  I've yet to find a way to do all this initialisation stuff without duplicating code.
 *  Roughly the same functions go into the resident part, but in a different segment
 *  they assume that CS == DS, so calling them from the non-resident part is a lot
 *  of trouble.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "messages.h"
#include "vmtool.h"
#include "vmshf.h"
#include "vmcall.h"

typedef enum {
   VMX_TYPE_UNSET,
   VMX_TYPE_EXPRESS,
   VMX_TYPE_SCALABLE_SERVER,
   VMX_TYPE_WGS,
   VMX_TYPE_WORKSTATION,
   VMX_TYPE_WORKSTATION_ENTERPRISE
} VMX_Type;

static struct {
	int	set;
	int number;
	char *name;
} prodName[] = {
		9, 1, "Unknown",
		9, 2, "Express",
		9, 3, "ESX Server",
		9, 4, "GSX Server",
		9, 5, "Workstation"
};

/*
	open RPC channel
*/
static int VMAuxRpcOpen( rpc_t *rpc, uint32_t type )
{
	CREGS r;

	r.eax.word = VMWARE_MAGIC;
	r.ebx.word = type | VMRPC_ENHANCED;
	r.ecx.word = VMCMD_INVOKE_RPC | VMRPC_OPEN;
	r.edx.word = VMWARE_CMD_PORT;
	r.ebp.word = r.edi.word = r.esi.word = 0;
	
	_VmwCommand( &r );

	if ( r.eax.word || r.ecx.word != 0x00010000L || r.edx.halfs.low )
		return -1;

	rpc->channel = r.edx.word;
	rpc->cookie1 = r.esi.word;
	rpc->cookie2 = r.edi.word;

	return 0;
}

/*
	send RPC command
*/
static int VMAuxRpcSend( rpc_t *rpc, const uint8_t *command, uint32_t length )
{
	CREGS r;
	
	/* send command length */
	
	r.eax.word = VMWARE_MAGIC;
	r.ebx.word = length;
	r.ecx.word = VMCMD_INVOKE_RPC | VMRPC_SENDLEN;
	r.edx.word = rpc->channel | VMWARE_CMD_PORT;
	r.ebp.word = 0;
	r.esi.word = rpc->cookie1;
	r.edi.word = rpc->cookie2;

	_VmwCommand( &r );

	if ( r.eax.word || r.ecx.halfs.high == 0 )
		return -1;

	if ( !length )
		return 0;

	/* send command string */

	if ( rpc->cookie1 && rpc->cookie2 ) {

		/* enhanced RPC */

		r.eax.word = VMWARE_MAGIC;
		r.ebx.word = VMRPC_ENH_DATA;
		r.ecx.word = length;
		r.edx.word = rpc->channel | VMWARE_RPC_PORT;
		r.esi.word = (uint32_t)command;
		r.edi.word = rpc->cookie2;
		r.ebp.word = rpc->cookie1;

		_VmwRpcOuts( &r );

		if ( r.ebx.word != VMRPC_ENH_DATA)
			return -1;
	}
	else {
		int i;
		/* conventional RPC */

		for (;;) {
			
			r.eax.word = VMWARE_MAGIC;
			r.ebx.word = 0;
			for ( i = 0; i < length && i < 4; ++i )
				r.ebx.byte[i] = command[i];
			r.ecx.word = VMCMD_INVOKE_RPC | VMRPC_SENDDAT;
			r.edx.word = rpc->channel | VMWARE_CMD_PORT;
			r.ebp.word = r.edi.word = r.esi.word = 0;

			_VmwCommand( &r );

			if ( r.eax.word || r.ecx.halfs.high == 0 )
				return -1;

			if ( length <= 4 )
				break;

			length -= 4;
			command += 4;
		}
	}

	return 0;
}

static int VMAuxRpcRecvLen(rpc_t *rpc, uint32_t *length, uint16_t *dataid )
{
	CREGS r;
	
	/* receive result length */

	r.eax.word = VMWARE_MAGIC;
	r.ebx.word = 0;
	r.ecx.word = VMCMD_INVOKE_RPC | VMRPC_RECVLEN;
	r.edx.word = rpc->channel | VMWARE_CMD_PORT;
	r.ebp.word = 0;
	r.esi.word = rpc->cookie1;
	r.edi.word = rpc->cookie2;

	_VmwCommand( &r );

	if ( r.eax.word || r.ecx.halfs.high == 0 )
		return -1;

	*length = r.ebx.word;
	*dataid = r.edx.halfs.high;

	return 0;
}

static int VMAuxRpcRecvDat( rpc_t *rpc, unsigned char *data, uint32_t length, uint16_t dataid )
{
	CREGS r;
	
	if ( rpc->cookie1 && rpc->cookie2 ) {
		
		/* enhanced RPC */
				
		r.eax.word = VMWARE_MAGIC;
		r.ebx.word = VMRPC_ENH_DATA;
		r.ecx.word = length;
		r.edx.word = rpc->channel | VMWARE_RPC_PORT;
		r.esi.word = rpc->cookie1;
		r.edi.word = (uint32_t)(data);
		r.ebp.word = rpc->cookie2;

		_VmwRpcIns( &r );

		if ( r.ebx.word != VMRPC_ENH_DATA )
			return -1;
	}
	else {
		/* conventional RPC */


		for (;;) {

			r.eax.word = VMWARE_MAGIC;
			r.ebx.word = dataid;
			r.ecx.word = VMCMD_INVOKE_RPC | VMRPC_RECVDAT;
			r.edx.word = rpc->channel | VMWARE_CMD_PORT;
			r.ebp.word = r.edi.word = r.esi.word = 0;
			
			_VmwCommand( &r );

			if ( r.eax.word || r.ecx.halfs.high == 0 )
				return -1;

			*(data++) = r.ebx.word;

			if (length <= 4)
				break;

			length -= 4;
		}
	}

	r.eax.word = VMWARE_MAGIC;
	r.ebx.word = dataid;
	r.ecx.word = VMCMD_INVOKE_RPC | VMRPC_RECVEND;
	r.edx.word = rpc->channel | VMWARE_CMD_PORT;
	r.ebp.word = 0;
	r.esi.word = rpc->cookie1;
	r.edi.word = rpc->cookie2;

	_VmwCommand( &r );

	if (r.eax.word || r.ecx.halfs.high == 0)
		return -1;

	return 0;
}


/*
	close an rpc channel with command 0x1e, subcommand 6
*/
static int VMAuxRpcClose(rpc_t *rpc)
{
	CREGS r;
	
	r.eax.word = VMWARE_MAGIC;
	r.ebx.word = 0;
	r.ecx.word = VMCMD_INVOKE_RPC | VMRPC_CLOSE;
	r.edx.word = rpc->channel | VMWARE_CMD_PORT;
	r.ebp.word = 0;
	r.esi.word = rpc->cookie1;
	r.edi.word = rpc->cookie2;

	_VmwCommand( &r );

	if ( r.eax.word || r.ecx.halfs.high == 0 )
		return -1;

	return 0;
}

static int VMAuxGetVersion(uint32_t *version, uint32_t *product)
{	
	CREGS r;
		
	r.eax.word = VMWARE_MAGIC;
	r.ebx.word = ~VMWARE_MAGIC;
	r.ecx.halfs.high = 0xFFFF;
	r.ecx.halfs.low = VMCMD_GET_VERSION;
	r.edx.word = VMWARE_CMD_PORT;
	r.ebp.word = r.edi.word = r.esi.word = 0;
	
	_VmwCommand( &r );

	if ( r.eax.word == 0xFFFFFFFF || r.ebx.word != VMWARE_MAGIC )
		return -1;

	*version = r.eax.word;

	if ( r.ecx.halfs.high == 0xFFFF )
		*product = VMX_TYPE_UNSET;
	else
		*product = r.ecx.word;
	
	return 0;
	
}

int VMAuxCheckVirtual(void)
{
	uint32_t product, version;
	int ret;
	
	ret = VMAuxGetVersion( &version, &product );
	
	if ( ret )
		return ret;
	
	if ( product > 4)
		product = 0;
		
	VERB_FPRINTF( VERBOSE, stdout, catgets( cat, 9, 0, MSG_INFO_VMVERS ),
		catgets( cat, prodName[product].set, prodName[product].number, prodName[product].name ),
		version );
	
	return 0;
}

/*
	open an RPC channel for communicating with the VM
*/
int VMAuxBeginSession( rpc_t far *fpRpci, rpc_t far *fpTclo
#ifdef DEBUG
	, rpc_t far *fpRpcd
#endif
)
{
	rpc_t rpc;

	/* open an rpc channel */
	if ( VMAuxRpcOpen( &rpc, VMRPC_OPEN_RPCI ) )
	{
		return -1;
	}

	fpRpci->channel = rpc.channel;
	fpRpci->cookie1 = rpc.cookie1;
	fpRpci->cookie2 = rpc.cookie2;

	/* open a tclo channel */
	if ( VMAuxRpcOpen( &rpc, VMRPC_OPEN_TCLO ) )
	{
		return -1;
	}

	fpTclo->channel = rpc.channel;
	fpTclo->cookie1 = rpc.cookie1;
	fpTclo->cookie2 = rpc.cookie2;

#ifdef DEBUG
	/* open a rpci channel for debugging*/
	if ( VMAuxRpcOpen( &rpc, VMRPC_OPEN_RPCI ) )
	{
		rpc.channel = VMRPC_INVALID_CHANNEL;
	}
	else
	{
		fpRpcd->channel = rpc.channel;
		fpRpcd->cookie1 = rpc.cookie1;
		fpRpcd->cookie2 = rpc.cookie2;
	}
#endif

	/* success */
	return 0;
	
}

/*
	release shared folder context
*/
void VMAuxEndSession( rpc_t far *fpRpci, rpc_t far *fpTclo
#ifdef DEBUG
	, rpc_t far *fpRpcd
#endif
)
{
	rpc_t rpc;
	
	rpc.channel = fpRpci->channel;
	rpc.cookie1 = fpRpci->cookie1;
	rpc.cookie2 = fpRpci->cookie2;
		
	VMAuxRpcClose( &rpc );

	rpc.channel = fpTclo->channel;
	rpc.cookie1 = fpTclo->cookie1;
	rpc.cookie2 = fpTclo->cookie2;

	VMAuxRpcClose( &rpc );

#ifdef DEBUG
	rpc.channel = fpRpcd->channel;
	rpc.cookie1 = fpRpcd->cookie1;
	rpc.cookie2 = fpRpcd->cookie2;
		
	VMAuxRpcClose( &rpc );
#endif
}

/*
	Check for shared folders
*/
int VMAuxSharedFolders( rpc_t far *fpRpci )
{
	uint32_t length;
	uint16_t id;
	const uint8_t shfH[] =
	{ 'f', ' ', 0, 0, 0, 0 };
	unsigned char buffer[32]= { 0 };
	rpc_t rpc;

	rpc.channel = fpRpci->channel;
	rpc.cookie1 = fpRpci->cookie1;
	rpc.cookie2 = fpRpci->cookie2;

	/* send &rpc command "f " */
	if ( !VMAuxRpcSend( &rpc, shfH, 2 ) )
	{
		/* get reply length */
		if ( !VMAuxRpcRecvLen( &rpc, &length, &id ) )
		{
			/* get reply data */
			if ( !VMAuxRpcRecvDat( &rpc, buffer, length, id ) )
			{
				/* check reply status */
				if ( buffer[0] == '1' && buffer[1] == ' ' )
				{
					return 0;
				}
			}
		}
	}

	return -1;

}

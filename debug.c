/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *  Copyright (C) 2011-2022  Eduardo Casino
 *
 * debug.c: Enable debug to VMware logs
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

#include <i86.h>
#include <stddef.h>
#include <stdint.h>
#include "globals.h"
#include "vmtool.h"
#include "printf.h"

PUBLIC rpc_t rpcd =
{
	0,				// channel
	0,				// cookie1
	0				// cookie2
};

#define MSG_LEN 160

static char msg_buf[MSG_LEN+5] = "log ";
static char *msg = &msg_buf[4];


static inline size_t strnlen_local(const char *s, size_t maxlen)
{
#if 0
	size_t len;
	for ( len= 0; s[len] && len < maxlen; ++len );
	return len;
#else
	return (size_t) _strnlen_s(s, maxlen);
#endif
}

static inline int isprint_local(int c)
{
	return (unsigned)c-0x20 < 0x5f;
}


static void dlog(void)
{
	uint32_t len;
	int ret;
	uint16_t id;

	if ( rpcd.channel == VMRPC_INVALID_CHANNEL)
	{
		return;

	}
	len = strnlen_local(msg_buf, sizeof(msg_buf));
	ret = VMRpcSend( &rpcd, msg_buf, len );

	if ( ret != VMTOOL_SUCCESS )
	{
		return;
	}

	ret = VMRpcRecvLen( &rpcd, &len, &id );

	if ( ret != VMTOOL_SUCCESS || len == 0 )
	{
		return;
	}

	if ( len > MSG_LEN ) len = MSG_LEN;

	ret = VMRpcRecvDat( &rpcd, msg, len, id );

	return;
}

void dprintf(const char *name, const char *file, int line, const char* format,  ...)
{
	va_list va;
	int len;

	snprintf(msg, MSG_LEN, "%s: [%s:%d]: ", name, file, line);
	len = strnlen_local(msg, MSG_LEN);

	va_start(va, format);
	vsnprintf(msg+len, MSG_LEN-len, format, va);
	va_end(va);
	dlog();

	return;
}

void ddump(const char *name, const char *file, int line, const char *prfx, void __far *addr, uint16_t size)
{
	char __far *c = (char far *)addr;
	while (size)
	{
		int len, i;
		uint16_t s;

		snprintf(msg, MSG_LEN, "%s: [%s:%d]: %s: %4.4X:%4.4X >", name, file, line, prfx, FP_SEG(c), FP_OFF(c));
		len = strnlen_local(msg, MSG_LEN);

		s = size;
		for (i = 0; i < 16 && s > 0; ++i, --s, len += 3)
		{
			if (i == 8)
			{
				msg[len++] = ' ';
			}
			snprintf(msg+len, MSG_LEN-len, " %2.2x", c[i]);
		}
		while ( i++ < 16 )
		{
			msg[len++] = ' '; msg[len++] = ' '; msg[len++] = ' ';
		}
		msg[len++] = ' '; msg[len++] = '|'; msg[len++] = ' ';
		for (i = 0; i < 16 && size > 0; ++i, ++c, --size, ++len)
		{
			snprintf(msg+len, MSG_LEN-len, "%c", isprint_local(*c) ? *c : '.');
		}
		while ( i++ < 16 )
		{
			msg[len++] = ' ';
		}
		msg[len++] = ' '; msg[len++] = '|';

		dlog();
	}
}


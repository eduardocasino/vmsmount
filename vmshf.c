/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *
 * vmshf.c: file transfer via VMware Shared Folders
 *
 * This file is a derivative work of the VMware Command Line Tools, by Ken Kato
 *        http://sites.google.com/site/chitchatvmback/
 *
 * Copyright (c) 2002-2008,	Ken Kato
 * Copyright (c) 2011,		Eduardo Casino
 *		- Updated to protocol version 3
 *		- Minor modifications to work with the redirector
 *		- Unicode support for DOS
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

#include <stdlib.h>

#include "miniclib.h"
#include "vmint.h"
#include "vmtool.h"
#include "vmshf.h"
#include "vmdos.h"
#include "redir.h"

#pragma data_seg("BEGTEXT", "CODE");
#pragma code_seg("BEGTEXT", "CODE");

static uint8_t buffer[VMSHF_BLOCK_SIZE] = {0};

shf_t shf = {
	{
		0,				// channel
		0,				// cookie1
		0				// cookie2
	},							// rpc
	(uint8_t *)&buffer[0]
};

/*
	execute a backdoor RPC command
*/
static int ExecuteRpc( uint32_t *length )
{
	int ret;
	uint16_t id;

	ret = VMRpcSend( &(shf.rpc), shf.buf, *length );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	ret = VMRpcRecvLen( &(shf.rpc), length, &id );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	if ( *length > VMSHF_BLOCK_SIZE )
	{
		return VMTOOL_RPC_ERROR;
	}

	ret = VMRpcRecvDat( &(shf.rpc), shf.buf, *length, id );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	if ( *length < 10 || (char)shf.buf[0] != '1' || (char)shf.buf[1] != ' ' )
	{
		return VMTOOL_RPC_ERROR;
	}	
		
	return VMTOOL_SUCCESS;
}


/*
	open a host file
*/
#define Request	((VMShfOpenFileRequest *)&shf.buf[0])
#define Reply ((VMShfOpenFileReply *)&shf.buf[0])

int VMShfOpenFile(
	uint32_t	access,			/* one of VMSHF_ACCESS_* values		*/
	uint32_t	action,			/* one of VMSHF_ACTION_* values	*/
	uint8_t		filemode,		/* VMSHF_FILEMODE_* mask values		*/
	uint32_t	fileattr,		/* one of VMSHF_ATTR_* values		*/
	char far	*filename,		/* variable length file name		*/
	uint32_t	*status,		/* status							*/
	uint32_t	*handle)		/* file handle						*/
{
	uint32_t namelen, datalen;
	int ret;
	
	*status = VMSHF_VMTOOLERR;

	Request->header.command	= ' f';
	Request->header.id		= 0;
	Request->header.op		= VMSHF_OPEN_FILE_V3;
	
	Request->data.mask		= VMSHF_OPEN_VALID_FIELDS;
	Request->data.access	= access;
	Request->data.action	= action;
	Request->data.fmode		= filemode;
	Request->data.attr		= fileattr;
	Request->data.lock		= VMSHF_LOCK_DEFAULT;
	Request->data.reserved1	= 0;
	Request->data.reserved2	= 0;
	
	namelen = DosPathToPortable( &Request->data.file.name, filename );
	Request->data.file.length	= namelen;
	Request->data.file.flags	= VMSHF_FILE_NAME_USE_NAME;
	Request->data.file.caseType	= VMSHF_CASE_INSENSITIVE;
	Request->data.file.handle	= VMSHF_INVALID_HANDLE;

	datalen = sizeof( VMShfOpenFileRequest ) + namelen;

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;

	if ( datalen >= sizeof( VMShfOpenFileReply ) )
	{
		*handle = Reply->data.handle;
	}
	else
	{
		*handle = VMSHF_INVALID_HANDLE;
	}

	return VMTOOL_SUCCESS;
}

/*
	read data from a host file
*/
#undef Request
#undef Reply
#define Request	((VMShfReadFileRequest *)&shf.buf[0])
#define Reply ((VMShfReadFileReply *)&shf.buf[0])

int VMShfReadFile(
	uint32_t	handle,		/* 10=> file handle returned by OPEN	*/
	uint64_t	offset,		/* 14=> byte offset to stat reading		*/
	uint32_t	*length,	/* 22-> number of bytes to read			*/
	char		**data,
	uint32_t	*status)
{
	uint32_t datalen;
	int ret;
	
	*status = VMSHF_VMTOOLERR;

	Request->header.command	= ' f';
	Request->header.id		= 0;
	Request->header.op		= VMSHF_READ_FILE_V3;
	
	Request->data.handle	= handle;
	Request->data.offset	= offset;
	Request->data.size		= *length > VMSHF_MAX_DATA_SIZE ? VMSHF_MAX_DATA_SIZE : *length;

	datalen = sizeof( VMShfReadFileRequest );

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;

	if ( datalen >= sizeof( VMShfReadFileReply ) )
	{
		*length = Reply->data.size;
		*data	= &Reply->data.data;
	}
	else
	{
		*length = 0;
	}

	return VMTOOL_SUCCESS;
}

/*
	write data to a host file
*/
#undef Request
#undef Reply
#define Request	((VMShfWriteFileRequest *)&shf.buf[0])
#define Reply ((VMShfWriteFileReply *)&shf.buf[0])

int VMShfWriteFile(
	uint32_t	handle,			/* 10: file handle returned by OPEN */
	uint8_t		flags,
	uint64_t	offset,			/* 15: byte offset to start writing	*/
	uint32_t	*length,		/* 23: number of bytes to write		*/
	char far	*data,
	uint32_t	*status)
{
	uint32_t datalen;
	int ret;

	*status = VMSHF_VMTOOLERR;

	Request->header.command	= ' f';
	Request->header.id		= 0;
	Request->header.op		= VMSHF_WRITE_FILE_V3;
	
	Request->data.handle	= handle;
	Request->data.flags		= flags;
	Request->data.offset	= offset;
	Request->data.reserved	= 0;
	
	datalen = *length > VMSHF_MAX_DATA_SIZE ? VMSHF_MAX_DATA_SIZE : *length;
	
	Request->data.size		= datalen;
	_fmemcpy_local( MK_FP( myDS, &Request->data.data ), data, datalen ); 

	datalen += sizeof( VMShfWriteFileRequest );

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;

	if ( datalen >= sizeof( VMShfWriteFileReply ) )
	{
		*length = Reply->data.size;
	}
	else
	{
		*length = 0;
	}

	return VMTOOL_SUCCESS;
}

/*
	close a host file/directory
*/
#undef Request
#undef Reply
#define Request	((VMShfCloseFileDirRequest *)&shf.buf[0])
#define Reply ((VMShfCloseFileDirReply *)&shf.buf[0])

int VMShfCloseFileDir(
	uint32_t	op,
	uint32_t	handle,		/* 10: file handle returned by OPEN */
	uint32_t	*status)
{
	uint32_t datalen;
	int ret;

	*status = VMSHF_VMTOOLERR;

	Request->header.command	= ' f';
	Request->header.id		= 0;
	Request->header.op		= op;
	
	Request->data.handle	= handle;
	Request->data.reserved	= 0;

	datalen = sizeof( VMShfCloseFileDirRequest );

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;

	return VMTOOL_SUCCESS;
}

/*
	open directory (protocol V3)
*/
#undef Request
#undef Reply
#define Request	((VMShfOpenDirRequest *)&shf.buf[0])
#define Reply ((VMShfOpenDirReply *)&shf.buf[0])

int VMShfOpenDir(
	char far	*dirname,
	uint32_t	*status,
	uint32_t	*handle)
{
	uint32_t namelen, datalen;
	int ret;

	*status = VMSHF_VMTOOLERR;

	Request->header.command		= ' f';
	Request->header.id			= 0;
	Request->header.op			= VMSHF_OPEN_DIR_V3;

	Request->data.reserved		= 0;

	namelen = DosPathToPortable( &Request->data.dir.name, dirname );
	Request->data.dir.length	= namelen;
	Request->data.dir.flags		= VMSHF_FILE_NAME_USE_NAME;
	Request->data.dir.caseType	= VMSHF_CASE_INSENSITIVE;
	Request->data.dir.handle	= VMSHF_INVALID_HANDLE;
	
	datalen = sizeof( VMShfOpenDirRequest ) + namelen;

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}
	
	*status = Reply->header.status;

	if ( datalen >= sizeof( VMShfOpenDirReply ) )
	{
		*handle = Reply->data.handle;
	}
	else
	{
		*handle = VMSHF_INVALID_HANDLE;
	}
	
	return VMTOOL_SUCCESS;
}

/*
	get a directory entry (protocol V3)
*/
#undef Request
#undef Reply
#define Request	((VMShfReadDirRequest *)&shf.buf[0])
#define Reply ((VMShfReadDirReply *)&shf.buf[0])

int VMShfReadDir(
	uint32_t	handle,		/* 10: file handle returned by OPEN */
	uint32_t	index,
	uint32_t	*status,
	VMShfAttr	**fileattr,
	char		**filename,
	uint32_t	*namelen)
{
	uint32_t datalen;
	int ret;

	*status = VMSHF_VMTOOLERR;

	Request->header.command	= ' f';
	Request->header.id			= 0;
	Request->header.op			= VMSHF_READ_DIR_V3;

	Request->data.handle	= handle;
	Request->data.index		= index;
	Request->data.flags		= 0;
	Request->data.reserved	= 0;

	datalen = sizeof( VMShfReadDirRequest );

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status		= Reply->header.status;
	*filename	= (char *)-1;

	if ( datalen > sizeof( VMShfReadDirReply ) )
	{
		*namelen = Reply->data.file.length;

		if ( *namelen )
		{
			*fileattr = &Reply->data.attr;
			*filename = &Reply->data.file.name;
			(*filename)[*namelen] = '\0';
		}
		else
		{
			*filename = 0;
		}
	}

	return VMTOOL_SUCCESS;
}

/*
	get file attributes
*/
#undef Request
#undef Reply
#define Request	((VMShfGetAttrRequest *)&shf.buf[0])
#define Reply ((VMShfGetAttrReply *)&shf.buf[0])

int VMShfGetAttr(
	char far	*filename,
	uint32_t	handle,
	uint32_t	*status,
	VMShfAttr	**fileattr)
{
	uint32_t namelen, datalen;
	int ret;

	*status = VMSHF_VMTOOLERR;

	Request->header.command	= ' f';
	Request->header.id		= 0;
	Request->header.op		= VMSHF_GET_ATTR_V3;

	if ( handle != VMSHF_INVALID_HANDLE )
	{
		Request->data.hints			= VMSHF_ATTR_HINT_USE_HANDLE;
		Request->data.file.flags	= VMSHF_FILE_NAME_USE_HANDLE;
		namelen 					= 0;
		Request->data.file.name[0]	= '\0';
	}
	else
	{
		Request->data.hints			= VMSHF_ATTR_HINT_NONE;
		Request->data.file.flags	= VMSHF_FILE_NAME_USE_NAME;
		namelen = DosPathToPortable( &Request->data.file.name, filename );
	}
	Request->data.reserved		= 0;
	Request->data.file.length	= namelen;
	Request->data.file.caseType	= VMSHF_CASE_INSENSITIVE;
	Request->data.file.handle	= handle;
	
	datalen = sizeof( VMShfGetAttrRequest ) + namelen;
	
	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;

	if ( datalen >= sizeof( VMShfGetAttrReply ) )
	{
		*fileattr = &Reply->data.attr;
		// linkedFile ignored
	}
	
	return VMTOOL_SUCCESS;

}

/*
	set file attributes
*/
#undef Request
#undef Reply
#define Request	((VMShfSetAttrRequest *)&shf.buf[0])
#define Reply ((VMShfSetAttrReply *)&shf.buf[0])

int VMShfSetAttr(
	VMShfAttr	*fileattr,
	char far	*filename,
	uint32_t	*status)
{
	uint32_t namelen, datalen;
	int ret;

	*status = VMSHF_VMTOOLERR;

	Request->header.command	= ' f';
	Request->header.id			= 0;
	Request->header.op			= VMSHF_SET_ATTR_V3;

	Request->data.hints			= VMSHF_ATTR_HINT_NONE;
	Request->data.reserved		= 0;
	
	memcpy_local( &Request->data.attr, fileattr, sizeof ( VMShfAttr ) );
	
	namelen = DosPathToPortable( &Request->data.file.name, filename );
	Request->data.file.length	= namelen;
	Request->data.file.flags	= VMSHF_FILE_NAME_USE_NAME;
	Request->data.file.caseType	= VMSHF_CASE_INSENSITIVE;
	Request->data.file.handle	= VMSHF_INVALID_HANDLE;
	
	datalen = sizeof( VMShfSetAttrRequest ) + namelen;

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;

	return VMTOOL_SUCCESS;
}

/*
	create a host directory
*/
#undef Request
#undef Reply
#define Request	((VMShfCreateDirRequest *)&shf.buf[0])
#define Reply ((VMShfCreateDirReply *)&shf.buf[0])

int VMShfCreateDir(
	uint8_t		dirmode,
	char far	*dirname,
	uint32_t	*status)
{
	uint32_t namelen, datalen;
	int ret, i;

	*status = VMSHF_VMTOOLERR;

	Request->header.command	= ' f';
	Request->header.id		= 0;
	Request->header.op		= VMSHF_CREATE_DIR_V3;
	
	Request->data.mask		= VMSHF_CREATE_DIR_VALID_FIELDS;
	Request->data.fmode		= dirmode;
	
	namelen = DosPathToPortable( &Request->data.dir.name, dirname );
	Request->data.dir.length	= namelen;
	Request->data.dir.flags		= VMSHF_FILE_NAME_USE_NAME;
	Request->data.dir.caseType	= VMSHF_CASE_INSENSITIVE;
	Request->data.dir.handle	= VMSHF_INVALID_HANDLE;

	datalen = sizeof( VMShfCreateDirRequest ) + namelen;

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;

	return VMTOOL_SUCCESS;
}

/*
	delete a host file or directory
*/
#undef Request
#undef Reply
#define Request	((VMShfDeleteFileRequest *)&shf.buf[0])
#define Reply ((VMShfDeleteFileReply *)&shf.buf[0])
	
int VMShfDeleteFileDir(
	uint32_t	op,
	char far	*filename,
	uint32_t	*status)
{
	uint32_t namelen, datalen;
	int ret;

	*status = VMSHF_VMTOOLERR;

	Request->header.command	= ' f';
	Request->header.id			= 0;
	Request->header.op			= op;

	Request->data.hints			= VMSHF_DELETE_HINT_NONE;
	Request->data.reserved		= 0;
	namelen = DosPathToPortable( &Request->data.file.name, filename );
	Request->data.file.length	= namelen;
	Request->data.file.flags	= VMSHF_FILE_NAME_USE_NAME;
	Request->data.file.caseType = VMSHF_CASE_INSENSITIVE;
	Request->data.file.handle	= VMSHF_INVALID_HANDLE;
	
	datalen = sizeof( VMShfDeleteFileRequest ) + namelen;

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;

	return VMTOOL_SUCCESS;
}

/*
	move/rename a host file/directory
*/
#undef Request
#undef Reply
#define Request	((VMShfMoveFileRequest *)&shf.buf[0])
#define Reply ((VMShfMoveFileReply *)&shf.buf[0])

int VMShfMoveFile(
	char far	*srcname,
	char far	*dstname,
	uint64_t	hints,
	uint32_t	*status)
{
	uint32_t srclen, dstlen, datalen;
	VMShfFileName *dst;
	int ret;

	*status = VMSHF_VMTOOLERR;

	Request->header.command		= ' f';
	Request->header.id			= 0;
	Request->header.op			= VMSHF_MOVE_FILE_V3;

	Request->data.hints			= hints;
	Request->data.reserved		= 0;

	srclen = DosPathToPortable( &Request->data.src.name, srcname );
	Request->data.src.length	= srclen;
	Request->data.src.flags		= VMSHF_FILE_NAME_USE_NAME;
	Request->data.src.caseType 	= VMSHF_CASE_INSENSITIVE;
	Request->data.src.handle	= VMSHF_INVALID_HANDLE;

	// This is a bit weird, but it is the way that VMWARE encodes it. We have to move forward srclen bytes
	// the destination file structure as the file name is already occupying the space.
	//
    dst = (VMShfFileName *)( (char *)&Request->data.src + sizeof( VMShfFileName ) + srclen );

	dstlen = DosPathToPortable( &dst->name, dstname );
	dst->length		= dstlen;
	dst->flags		= VMSHF_FILE_NAME_USE_NAME;
	dst->caseType 	= VMSHF_CASE_INSENSITIVE;
	dst->handle		= VMSHF_INVALID_HANDLE;

	datalen = sizeof( VMShfMoveFileRequest ) + srclen + dstlen;

	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;
	
	return VMTOOL_SUCCESS;
}

/*
	get host drive free space
*/
#undef Request
#undef Reply
#define Request	((VMShfGetDirSizeRequest *)&shf.buf[0])
#define Reply ((VMShfGetDirSizeReply *)&shf.buf[0])

int VMShfGetDirSize(
	char far	*dirname,
	uint32_t	*status,
	uint64_t	*avail,
	uint64_t	*total)
{
	uint32_t namelen, datalen;
	int ret;

	*status = VMSHF_VMTOOLERR;

	Request->header.command		= ' f';
	Request->header.id			= 0;
	Request->header.op			= VMSHF_GET_DIRSIZE_V3;

	if ( !dirname )
	{
		namelen = 0;
		Request->data.dir.name[0] = '\0';
	}
	else
	{
		namelen = DosPathToPortable( &Request->data.dir.name, dirname );
	}
	Request->data.dir.length	= namelen;
	
	datalen = sizeof( VMShfGetDirSizeRequest ) + namelen;
	
	ret = ExecuteRpc( &datalen );

	if ( ret != VMTOOL_SUCCESS )
	{
		return ret;
	}

	*status = Reply->header.status;

	if ( datalen >= sizeof( VMShfGetDirSizeReply ) )
	{
		*avail = Reply->data.freeBytes;
		*total = Reply->data.totalBytes;
	}
	else
	{
		*avail = 0;
		*total = 0;
	}

	return VMTOOL_SUCCESS;
}


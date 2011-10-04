/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *  Copyright (C) 2011  Eduardo Casino
 *
 * vmdos.c: Conversions from/to vmware and DOS
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
 * 2011-10-04  Eduardo           * Omit '.' and '..' from listing if root dir
 *
 */
 
#include <dos.h>
#include "dosdefs.h"
#include "vmint.h"
#include "vmshf.h"
#include "miniclib.h"
#include "unicode.h"

#pragma data_seg("BEGTEXT", "CODE");
#pragma code_seg("BEGTEXT", "CODE");

uint8_t far *fpFUcase;
FChar far *fpFChar;
int32_t gmtOffset;

static int IllegalChar( unsigned char c )
{
	int i= 0;
	
	for ( i= 0; i < fpFChar->nIllegal; ++i )
	{
		if ( c == fpFChar->illegal[i] )
		{
			return 1;
		}
	}
	if ( 	 ( c < fpFChar->lowest || c > fpFChar->highest ) ||
			!( c < fpFChar->firstX || c > fpFChar->lastX ) )
	{
		return 1;
	}
	
	return 0;
}

inline unsigned char toupper_local ( unsigned char c )
{
	if ( c > 0x60 && c < 0x7b )
	{
		return c & 0xDF;
	}
	
	return ( c < 0x80 ? c : fpFUcase[c - 0x80] );
}

// fcbName must be 12 bytes long (we NULL-terminate it)
//
//  Returns TRUE if fName is a valid DOS 8.3 name, FALSE otherwise
//
int FNameToFcbName( 
	char *fcbName,		// out : File name in FCB format. Must be 12 bytes long (we NULL-terminate it)
	char *fName,		// in/out : File name in in UTF-8 on input, DOS filename on output
	uint32_t fNameLen,	// in : File name length
	uint8_t isRoot		// in : Flag: is root search?
	)
{
	char *p;
	char *d;
	int i;
	
	*(uint32_t *)(&fcbName[0]) = 0x20202020;
	*(uint32_t *)(&fcbName[4]) = 0x20202020;
	*(uint32_t *)(&fcbName[8]) = 0X00202020;

	// Special case for '.' and '..'
	//
	if ( ! isRoot )
	{
		if ( *(uint16_t *)fName == '..' && fName[2] == '\0' )
		{
			*(uint16_t *)fcbName = *(uint16_t *)fName;
			return 1;
		}
		if ( *(uint16_t *)fName == '\0.' )
		{
			*fcbName = *fName;
			return 1;
		}
	}
	
	if ( *fName == '.' )
	{
		return 0;
	}
	
	(void) Utf8ToLocal( fName, fName );
	
	p = strchr_local( fName, '.' );

	for ( i = 0; fName[i] != '\0' && fName[i] != '.' && i < 8; ++i )
	{
		if ( IllegalChar( fName[i] ) )
		{
			return 0;
		}
		else
		{
			fcbName[i] = toupper_local( fName[i] );
		}
	}

	if ( fName[i] != '.' && fName[i] != '\0' )
	{
		return 0;
	}
	
	if ( *p == '.' )
	{					// WARNING!!! This works because _fstrchr_local ALWAYS returns a valid
						// pointer. The real _fstrchr can return a NULL pointer
		d = &fcbName[8];
		
		++p;
		for ( i = 0; p[i] != '\0' && i < 3; ++i )
		{
			if ( IllegalChar( p[i] ) )
			{
				return 0;
			}
			else
			{
				*(d++) = toupper_local( p[i] );
			}
		}
		if (  p[i] != '\0' )
		{
			return 0;
		}
	}
	
	return 1;
	
}

VMShfAttr *FatAttrToFMode( uint8_t fileAttr )
{
	static VMShfAttr fAttr;

	fAttr.mask	= VMSHF_VALID_ATTR_FTYPE | VMSHF_VALID_ATTR_FMODE | VMSHF_VALID_ATTR_FLAGS;
	// fAttr.fmode	= VMSHF_FILEMODE_EXEC | VMSHF_FILEMODE_READ;
	fAttr.fmode	= VMSHF_FILEMODE_READ;
	fAttr.attr	= 0;
	fAttr.ftype	= VMSHF_TYPE_FILE;
	
	if ( !(fileAttr & _A_RDONLY) )
	{
		fAttr.fmode |= VMSHF_FILEMODE_WRITE;
	}
	if ( fileAttr & _A_HIDDEN )
	{
		fAttr.attr |= VMSHF_ATTR_HIDDEN;
	}
	if ( fileAttr & _A_SYSTEM )
	{
		fAttr.attr |= VMSHF_ATTR_SYSTEM;
	}
	if ( fileAttr & _A_ARCH )
	{
		fAttr.attr |= VMSHF_ATTR_ARCHIVE;
	}
	if ( fileAttr & _A_SUBDIR )
	{
		fAttr.ftype = VMSHF_TYPE_DIRECTORY;
	}
	
	return &fAttr;
	
}


uint8_t FModeToFatAttr( VMShfAttr * fAttr )
{
	uint8_t fileAttr;
	
	fileAttr = _A_NORMAL;
	
	if ( !(fAttr->fmode & VMSHF_FILEMODE_WRITE) )
	{
		fileAttr |= _A_RDONLY;
	}
	if ( fAttr->attr & VMSHF_ATTR_HIDDEN )
	{
		fileAttr |= _A_HIDDEN;
	}
	if ( fAttr->attr & VMSHF_ATTR_SYSTEM )
	{
		fileAttr |= _A_SYSTEM;
	}
	if ( fAttr->attr & VMSHF_ATTR_ARCHIVE )
	{
		fileAttr |= _A_ARCH;
	}	
	if ( fAttr->ftype & VMSHF_TYPE_DIRECTORY )
	{
		fileAttr |= _A_SUBDIR;
	}
	
	return fileAttr;

}


uint32_t DosExtActionToOpenAction(uint16_t extAction)
{
	uint32_t action;
	
	if ( extAction & EXTENDED_CREATE )
	{
		if ( extAction & EXTENDED_REPLACE )
		{
			action = VMSHF_ACTION_C_ALWAYS;
		}
		else
		{
			if ( extAction & EXTENDED_OPEN )
			{
				action = VMSHF_ACTION_O_ALWAYS;
			}
			else
			{
				action = VMSHF_ACTION_C_NEW;
			}
		}
	}
	else
	{
		if ( extAction & EXTENDED_REPLACE )
		{
			action = VMSHF_ACTION_O_TRUNC;
		}
		else 
		{
			action = VMSHF_ACTION_O_EXIST;
		}
	}

	return action;

}
			
  
// MS-DOS date format can only represent dates between 1980/1/1 and 2107/12/31
//   For simplicity, I'm not taking into account that 2100 will not be a leap year, so dates
//   beyond that will not be correct ( contact me for a fix then )
//

// Difference between 1980/1/1 0:0:0 UTC and 1601/1/1 0:0:0 UTC in days
// 
#define FILETIME_DELTA 138426

#define SECSTO100NSECS	10000000
#define SECSPERDAY		86400

uint32_t FTimeToFatTime( uint64_t filetime )
{
	uint32_t year	= 0;			// 1980
	uint32_t month	= 1;
	uint32_t day	= 1;
	uint32_t hour;
	uint32_t min;
	uint32_t days;
	uint32_t secs;
	uint32_t fatTime;

	(void) u64div32( &filetime, filetime, SECSTO100NSECS );

	if ( gmtOffset < 0 )
	{
		u64sub32( &filetime, filetime, (uint32_t)-gmtOffset );
	}
	else
	{
		u64add32( &filetime, filetime, (uint32_t)gmtOffset );
	}

	secs = u64div32( &filetime, filetime, SECSPERDAY );
	
	days = (uint32_t) filetime;

	// DOS doesn't support dates before 1980/1/1
	//
	if ( days < FILETIME_DELTA )
	{
		days = FILETIME_DELTA;
	}
	else
	{
		days -= FILETIME_DELTA;
	}	

	while ( ! (days < 365) )
	{
		if ( !(year & 3) && year != 120 )
		{	// leap year
			if ( days < 366 )
			{
				break;
			}
			days -= 1;
		}
		days -= 365;
		++year;
	}
	
	if ( days >= 31 )
	{									// Jan
		days -= 31;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( (year & 3) && days >= 28 )
	{									// Feb
		days -= 28;
		++month;
	}
	else if ( days >= 29 )
	{
		days -= 29;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 31 )
	{									// Mar
		days -= 31;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 30 )
	{									// Apr
		days -= 30;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 31 )
	{									// May
		days -= 31;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 30 )
	{									// Jun
		days -= 30;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 31 )
	{									// Jul
		days -= 31;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 31 )
	{									// Aug
		days -= 31;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 30 )
	{									// Sep
		days -= 30;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 31 )
	{									// Oct
		days -= 31;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 30 )
	{									// Nov
		days -= 30;
		++month;
	}
	else
	{
		goto days;
	}
	
	if ( days >= 31 )
	{									// Dec
		days -= 31;
		++month;
	}

days:
	day += days;

	secs = u32div32( &hour,	secs, 3600 );
	secs = u32div32( &min,  secs, 60 );

	fatTime = (secs >> 1) + (min << 5) + (hour << 11) + (day << 16) + (month << 21) + (year << 25);

	return fatTime;
}

int DosPathToPortable( uint8_t *dst, uint8_t far *src )
{
	int len, i;
	
	while ( '\\' == *src )						// Skip leading '\\'
	{
		++src;
	}

	len = LocalToUtf8( dst, src );					// Convert to Utf

	while ( len && '\\' == *(dst + len - 1) )		// Remove trailing '\\'
	{
		*(dst + len - 1) = '\0';
		--len;
	}
	
	i = len;
	
	while ( i-- )								// Substitute '\0' for '\\'
	{
		if ( *(dst + i) == '\\' )
		{
			*(dst + i) = '\0';
		}
	}

	return len;
	
}

uint16_t VmshfStatusToDosError( uint32_t status )
{
	uint16_t errcode;
	
	switch( status )
	{
		case VMSHF_SUCCESS:
			errcode = DOS_SUCCESS;
			break;
		case VMSHF_ENOTEXIST:
		case VMSHF_ENAME:
		case VMSHF_ETOOLONG:
			errcode = DOS_FILENOTFND;
			break;
		case VMSHF_ENODIR:
			errcode = DOS_PATHNOTFND;
			break;
		case VMSHF_EINVALID:
			errcode = DOS_INVLDHNDL;
			break;
		case VMSHF_ENOTPERM:
		case VMSHF_EACCESS:
		case VMSHF_ENOTEMPTY:
		case VMSHF_EGENERIC:
		case VMSHF_EEXIST:
			errcode = DOS_ACCESS;
			break;
		case VMSHF_ESHARING:
			errcode = DOS_SHARE;
			break;
		case VMSHF_ENOSPACE:
			errcode = DOS_ENOSPACE;
			break;
		case VMSHF_EUNSUPPORTED:
			errcode = DOS_UNKCMD;
			break;
		case VMSHF_EPARAMETER:
			errcode = DOS_INVLDPARM;
			break;
		case VMSHF_EDEVICE:
			errcode = DOS_DEVICE;
			break;
		case VMSHF_EPROTOCOL:
		case VMSHF_VMTOOLERR:
		default:
			errcode = DOS_GENERAL;
	}
	return errcode;
}


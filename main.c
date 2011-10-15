/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *  Copyright (C) 2011  Eduardo Casino
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
 * Acknowledgements:
 *    "Undocumented DOS 2nd ed." by Andrew Schulman et al.
 *    Ken Kato's VMBack info and Command Line Tools
 *                    (http://sites.google.com/site/chitchatvmback/)
 *    VMware's Open Virtual Machine Tools
 *                    (http://open-vm-tools.sourceforge.net/)
 *    Tom Tilli's <aitotat@gmail.com> TSR example in Watcom C for the 
 *                    Vintage Computer Forum (www.vintage-computer.com)
 *  
 * TODO:
 *
 *   Read and write optimizations
 *   Uninstallation
 *   Codepage change detection
 *   Long file names (really?)
 *
 *
 * 2011-10-01  Eduardo           * Fixed a bug in when printing default error messages
 *                               * New errorlevels: If successful, returns drive number
 *                                 (starting with A == 1)
 * 2011-10-09  Eduardo           * Add a CPU test
 * 2011-10-15  Eduardo           * Configurable buffer size
 *                               * New verbosity options
 *             Tom Ehlert        * Replace fscanf_s() with a much lighter implementation
 *
 */ 
#define __STDC_WANT_LIB_EXT1__ 1
#include <process.h>
#include <dos.h>
#include <env.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>

#include "globals.h"
#include "kitten.h"
#include "messages.h"
#include "vmaux.h"
#include "dosdefs.h"
#include "redir.h"
#include "endtext.h"
#include "unicode.h"

PUBLIC nl_catd cat;
PUBLIC int verbosity = 1;


// Far pointers to resident data
// These are set by GetFarPointersToResidentData()
//
static uint8_t	far * fpDriveNum;
static CDS 		far * far * fpfpCDS;
static SDA		far * far * fpfpSDA;
static SDB		far * far * fpfpSDB;
static FDB		far * far * fpfpFDB;
static char		far * far * fpfpFcbName1;
static char		far * far * fpfpFcbName2;
static char		far * far * fpfpFileName1;
static char		far * far * fpfpFileName2;
static char		far * far * fpfpCurrentPath;
static void		(__interrupt __far * far * fpfpPrevInt2fHandler)();
static void 	(__interrupt __far *fpNewInt2fHandler)();

static uint16_t far *fpUnicodeTbl;

static uint8_t	far * far *fpfpFUcase;
static FChar	far * far *fpfpFChar;
static int32_t	far *fpGmtOffset;

PUBLIC rpc_t	far *fpRpc;
static uint16_t	far *fpBufferSize;
static uint16_t	far *fpMaxDataSize;

static SysVars far *fpSysVars;
static char far *rootPath = " :\\";

// CPU identification routine
// (Info from http://www.ukcpu.net/Programming/Hardware/x86/CPUID/x86-ID.asp)
//
// in 808x and 8018x, flags 12-15 are always set.
// in 286, flags 12-15 are always clear in real mode.
// in 32-bit processors, bit 15 is always clear in real mode.
//                       bits 12-14 have the last value loaded into them.
//
static uint8_t RunningIn386OrHigher( void );
#pragma aux RunningIn386OrHigher =									\
	"pushf"				/* Save current flags */					\
	"xor ax, ax"													\
	"push ax"														\
	"popf"				/* Load all flags cleared */				\
	"pushf"															\
	"pop ax"			/* Load flags back to ax */					\
	"and ax, 0xf000"	/* If 86/186, flags 12-15 will be set */	\
	"cmp ax, 0xf000"												\
	"je return"														\
	"mov ax, 0xf000"												\
	"push ax"														\
	"popf"				/* Load flags 12-15 set */					\
	"pushf"															\
	"pop ax"			/* Load flags back to ax */					\
	"and ax, 0xf000"	/* If 286, flags 12-15 will be cleared */	\
	"jz return"														\
	"mov al, 0x01"													\
	"return:"														\
	"popf"				/* Restore flags */							\
	value [al];

//	00h not installed, OK to install
//	01h not installed, not OK to install 
//	FFh:
//       *magic == ~VMSMOUNT_MAGIC, installed
//       *magic == VMSMOUNT_MAGIC, OK to install
//
static uint8_t InstallationCheck( uint16_t *magic );
#pragma aux InstallationCheck =							\
	"push word ptr [bx]"								\
	"mov ax, 1100h"	/* Installation check */			\
	"int 2fh"											\
	"pop word ptr [bx]"									\
	parm [bx]											\
	value [al];

static void GetFarPointersToResidentData( void )
{
	struct SREGS s;

	segread( &s );	

	// From redir.c
	fpNewInt2fHandler		= s.cs:>Int2fRedirector;
	fpfpPrevInt2fHandler	= s.cs:>&fpPrevInt2fHandler;
	fpDriveNum = s.cs:>&driveNum;
	fpfpSDA = s.cs:>&fpSDA;
	fpfpCDS = s.cs:>&fpCDS;
	fpfpSDB = s.cs:>&fpSDB;
	fpfpFDB = s.cs:>&fpFDB;
	fpfpFcbName1	= s.cs:>&fpFcbName1;
	fpfpFcbName2	= s.cs:>&fpFcbName2;
	fpfpFileName1	= s.cs:>&fpFileName1;
	fpfpFileName2	= s.cs:>&fpFileName2;
	fpfpCurrentPath	= s.cs:>&fpCurrentPath;

	// From unicode.c
	fpUnicodeTbl = s.cs:>&unicodeTbl;
	
	// From vmdos.c
	fpfpFUcase = s.cs:>&fpFUcase;
	fpfpFChar = s.cs:>&fpFChar;
	fpGmtOffset = s.cs:>&gmtOffset;
	
	// From vmshf.c
	fpRpc = s.cs:>&rpc;
	fpBufferSize = s.cs:>&bufferSize;
	fpMaxDataSize = s.cs:>&maxDataSize;

	return;
	
}

static void PrintUsageAndExit(int err)
{
	fprintf( stderr, MSG_COPYRIGHT );
	fputs( catgets( cat, 0, 1, MSG_HELP_1 ), stderr );
	fputs( catgets( cat, 0, 2, MSG_HELP_2 ), stderr );
	fputs( catgets( cat, 0, 3, MSG_HELP_3 ), stderr );
	fputs( catgets( cat, 0, 4, MSG_HELP_4 ), stderr );
	fputs( catgets( cat, 0, 5, MSG_HELP_5 ), stderr );
	fputs( catgets( cat, 0, 6, MSG_HELP_5 ), stderr );
	fputs( catgets( cat, 0, 7, MSG_HELP_5 ), stderr );
	fputs( catgets( cat, 0, 8, MSG_HELP_5 ), stderr );		
	
	exit( err );
	
}

static int GetOptions(char *argString)
{
	int argIndex, i;
	uint32_t d, bufsiz= 0;
	char c, *s;
	int vset= 0;
	int bset= 0;
	int lset= 0;
	
	for ( argIndex= 0; argString[argIndex] != '\0'; ++argIndex )
	{
		switch ( argString[argIndex] )
		{
			case ' ':
			case '\t':
				break;	// Skip spaces
			
			case '/':
			case '-':
				switch ( c= toupper(argString[++argIndex]) )
				{
					case 'H':
					case '?':
						return 1;
						break;
					case 'Q':
						if ( vset++ )
						{
							return 2;	// /Q or /V already set
						}
						verbosity = 0;
						break;
					case 'V':
						if ( vset++ )
						{
							return 2;	// /Q or /V already set
						}
						verbosity = 2;
						break;
					case 'B':
						if ( bset++ || argString[++argIndex] != ':' )
						{
							return 2; // Already been here or option needs a number
						}
						s = &argString[++argIndex];
						for ( i= 0; isdigit( s[i] ) ; ++i, ++argIndex );
						if ( i != 0 )
						{
							d = 1;
							while ( i-- )
							{
								bufsiz += ( s[i] - 0x30 ) * d;
								if ( bufsiz > UINT16_MAX )
								{
									bufsiz = UINT16_MAX;
									break; // bufsiz too large
								}
								d *= 10;
							}
							if ( toupper( argString[argIndex] ) == 'K' )
							{
								if ( bufsiz > ( UINT16_MAX >> 10 ) )
								{
									bufsiz = UINT16_MAX;
								}
								else
								{
									bufsiz <<= 10;
								}
							}
							else
							{
								--argIndex;
							}
						}
						else
						{
							return 2; // Not a number
						}
						*fpBufferSize = (uint16_t) bufsiz;
						break;
					case 'L':
						if ( lset++ || argString[++argIndex] != ':' )
						{
							return 2; // /L already set or needs a drive letter
						}
						if ( !isalpha( c= toupper( argString[++argIndex] ) ) )
						{
							return 2; // Missing or invalid drive letter
						}
						*fpDriveNum = c - 'A';
						break;
					default:
						return 2; // Invalid option
				}
				break;
	
			default:
				return 2; // Non switch
		}
	}
	
	return 0;
}

static int GetNLS(void)
{
	union REGS r;
	struct SREGS s;
	static NLSTable nlsTable;

	segread( &s );
	
	// Get FUCASE (File Uppercase Table)
	//
	r.w.ax = 0x6504;
	r.x.bx	= r.x.dx = 0xffff;
	r.x.cx	= 5;

	s.es	= s.ds;
	r.x.di	= (uint16_t) &nlsTable;
		
	intdosx( &r, &r, &s );

	if ( r.x.cx != 5 )
		return 1;

	*fpfpFUcase = (uint8_t far *)nlsTable.pTableData + 2;	// Skip size word

	// Get FCHAR (File Terminator Table)
	//
	r.w.ax	= 0x6505;
	
	intdosx( &r, &r, &s );

	if ( r.x.cx != 5 )
		return 1;

	*fpfpFChar = (FChar far *)nlsTable.pTableData ;
	
	return 0;

}

static int GetSDA(void)
{
	union REGS r;
	struct SREGS s;
	
	r.w.ax = 0x5D06;
	segread( &s );
	s.ds = r.x.si = 0;
			
	intdosx( &r, &r, &s );
	
	if ( !s.ds && !r.x.si )
		return 1;
		
	*fpfpSDA = (SDA far *) MK_FP( s.ds, r.x.si );
	*fpfpSDB = &(*fpfpSDA)->findFirst;
	*fpfpFDB = &(*fpfpSDA)->foundEntry;

	return 0;

}

static int GetSysVars(void)
{
	union REGS r;
	struct SREGS s;

	r.h.ah = 0x52;
	segread( &s );
	s.es = r.x.bx = 0;
	
	intdosx(&r, &r, &s);
	
	if ( !s.es && !r.x.bx )
		return 1;

	fpSysVars = (SysVars far *) MK_FP( s.es, r.x.bx - SYSVARS_DECR );
	
	return 0;

}

static int SetCDS(void)
{
	static CDS far *currDir;
	int rootPathLen;
	
	if ( *fpDriveNum != 0xFF ) {
		currDir = &fpSysVars->currDir[*fpDriveNum];
		if ( currDir->flags & 0xC000 ) {
			fprintf(stderr, catgets( cat, 1, 0, MSG_ERROR_INUSE ), *fpDriveNum + 'A');
			return 1;
		}
	}
	else {
		// Skip from A to D
		//
		for ( *fpDriveNum = 4;
				( (currDir = &fpSysVars->currDir[*fpDriveNum])->flags & (NETWORK|PHYSICAL) )
												&& *fpDriveNum < fpSysVars->lastDrive; ++*fpDriveNum );
		
		if ( *fpDriveNum == fpSysVars->lastDrive ) {
			fprintf(stderr, catgets( cat, 1, 1, MSG_ERROR_NO_DRIVES ), fpSysVars->lastDrive + '@');
			return 1;
		}
	}
	
	// Undocumented DOS 2nd Ed. p. 511
	//   Set Physical (0x4000), Network (0x8000) and REDIR_NOT_NET (0x80) bits
	//
	currDir->flags = NETWORK|PHYSICAL|REDIR_NOT_NET;
	
	// Undocumented DOS 2nd Ed. p. 514
	//   Set Network UserVal
	//
	currDir->u.Net.parameter = VMSMOUNT_MAGIC;
	
	rootPathLen = _fstrlen( rootPath );
	rootPath[rootPathLen - 3] = *fpDriveNum + 'A';
	_fstrcpy( currDir->currentPath, rootPath );
	currDir->backslashOffset = rootPathLen - 1;

	*fpfpCDS = currDir;
	*fpfpCurrentPath = currDir->currentPath + currDir->backslashOffset;
	*fpfpFcbName1 = &(*fpfpSDA)->fcbName1;
	*fpfpFcbName2 = &(*fpfpSDA)->fcbName2;
	*fpfpFileName1 = &(*fpfpSDA)->fileName1[rootPathLen-1];	
	*fpfpFileName2 = &(*fpfpSDA)->fileName2[rootPathLen-1];	
	
	return 0;
}

static void LoadUnicodeConversionTable(void)
{
	union REGS r;
	struct SREGS s;
	char filename[13];
	char fullpath[_MAX_PATH];
	char buffer[256];
	struct stat filestat;
	FILE *f;
	int i, ret;
	
	// get current Code Page
	//
	//	AX = 6601h
	//	  Return: CF set on error
	//	    AX = error code (see #01680 at AH=59h/BX=0000h)
	//	  CF clear if successful
	//      BX = active code page (see #01757) <---
	//      DX = system code page (see #01757)
	//
	r.w.ax = 0x6601;
			
	intdos( &r, &r );

	if ( r.x.cflag ) {
		// Can't get codepage. Use ASCII only
		//
		fputs( catgets( cat, 1, 15, MSG_WARN_CP ), stderr );
		goto error;
	}
	
	sprintf( filename, r.x.bx > 999 ? "c%duni.tbl" : "cp%duni.tbl", r.x.bx );
	
	VERB_PRINTF( 2, catgets( cat, 9, 2, MSG_INFO_TBL ), r.x.bx, filename );
	
	_searchenv( filename, "PATH", fullpath);
	if ( '\0' == fullpath[0] )
	{
		fprintf( stderr, catgets( cat, 1, 12, MSG_WARN_NOTBL ), filename );
		goto error;
	}
	
	f = fopen( fullpath, "rb" );
	
	if ( NULL == f )
	{
		fprintf( stderr, catgets( cat, 1, 13, MSG_WARN_UNICODE ), filename );
		goto error;
	}
	
	// Tom: this fscanf_s implementation is VERY handy - but costs 5 K .exe size (3 K packed)
	// therefore I'm tempted to implement this manually
#if 0	
	if ( EOF == fscanf_s( f, "Unicode (%s)", buffer, sizeof( buffer) ) )
	{
		fprintf( stderr, catgets( cat, 1, 14, MSG_WARN_TBLFORMAT ), filename );
		goto close;
	}
#else
	if ( fread( buffer, 1, 9, f ) != 9 ||     // "Unicode (
						memcmp( buffer, "Unicode (", 9 ) != 0 )
	{
		fprintf( stderr, catgets( cat, 1, 14, MSG_WARN_TBLFORMAT ), filename );
		goto close;
	}   

	memset( buffer, 0, sizeof( buffer ) );
	                                          
	for ( i = 0; i < sizeof( buffer ); i++ )
	{
		if ( fread( buffer+i, 1, 1, f ) != 1 )
		{
			fprintf( stderr, catgets( cat, 1, 14, MSG_WARN_TBLFORMAT ), filename );
			goto close;
		}
		if ( buffer[i] == ')' )  
		{
			buffer[i] = 0;
			break;	
		}
	}
#endif	

	ret = fread( buffer, 1, 3, f );

	if ( ret != 3 || buffer[0] != '\r' || buffer[1] != '\n' || buffer[2] != 1 )
	{
		fprintf( stderr, catgets( cat, 1, 14, MSG_WARN_TBLFORMAT ), filename );
		goto close;
	}
	
	if ( 256 != (ret = fread( buffer, 1, 256, f )) )
	{
		fprintf( stderr, catgets( cat, 1, 13, MSG_WARN_UNICODE ), filename );
		goto close;
	}
	
	segread( &s );
	_fmemcpy( fpUnicodeTbl, s.ds:>buffer, 256 );
	
	return;

close:
	fclose( f );
error:
	fputs( catgets( cat, 1, 16, MSG_WARN_437 ), stderr );
	
}

static void GetTimezoneOffset(void)
{
	char *tz;
	time_t utcTime, localTime;
	auto struct tm tmbuf;
	
	tz  = getenv( "TZ" );
	
	if ( NULL == tz )
	{
		fputs( catgets( cat, 1, 11, MSG_WARN_TIMEZONE ), stderr );
	}
	else
	{
		// Compute offset between UTC and local times
		//
		localTime = time( NULL );
		_gmtime( &localTime, &tmbuf );
		tmbuf.tm_isdst = daylight;
		utcTime = mktime( &tmbuf );
	
		*fpGmtOffset = localTime - utcTime;
	
		VERB_PRINTF( 2, catgets( cat, 9, 1, MSG_INFO_TZ ), *fpGmtOffset );
	}
	return;
}

static uint16_t SetActualBufferSize( void )
{
	uint16_t transientSize = (uint16_t) EndOfTransientBlock - (uint16_t) BeginOfTransientBlock;
	
	if (*fpBufferSize > transientSize || *fpBufferSize > VMSHF_MAX_BLOCK_SIZE
													|| *fpBufferSize < VMSHF_MIN_BLOCK_SIZE )
	{
		return ( ( transientSize > VMSHF_MAX_BLOCK_SIZE ) ? VMSHF_MAX_BLOCK_SIZE : transientSize );
	}

	*fpMaxDataSize = VMSHF_MAX_DATA_SIZE( *fpBufferSize );
	
	return 0;
}

static uint16_t SizeOfResidentSegmentInParagraphs( void )
{
	uint16_t	sizeInBytes;
	
	sizeInBytes	= (uint16_t) BeginOfTransientBlock + *fpBufferSize;
	sizeInBytes	+= 0x100;	// Size of PSP (do not add when building .COM executable)
	sizeInBytes	+= 15;		// Make sure nothing gets lost in partial paragraph

	return sizeInBytes >> 4;
}

int main(int argc, char **argv)
{

	char		argString[128];
	uint16_t	magic= VMSMOUNT_MAGIC;
	int			tblSize;
	uint16_t	ret;
	uint16_t	paragraphs;
	
	cat = catopen( "VMSMOUNT", 0 );
		
	fprintf( stderr, MSG_MY_NAME, VERSION_MAJOR, VERSION_MINOR);

	// Check that CPU is x386 or higher (to avoid nasty things if somebody
	// tries to run this in a REAL machine
	//
	if ( ! RunningIn386OrHigher() )
	{
		fputs( catgets(cat, 1, 3, MSG_ERROR_NOVIRT ), stderr );
		return( ERR_NOVIRT );
	}

	// Check OS version. Only DOS >= 5.0 is supported
	//
	if ( _osmajor < 5 )
	{
		fprintf(stderr, catgets( cat, 1, 2, MSG_ERROR_BADOS ), _osmajor, _osminor );
		return( ERR_WRONGOS );
	}
	
	GetFarPointersToResidentData();
	
	ret = GetOptions( getcmd( argString ) );
	
	if ( ret == 1 )		// User requested help
	{
		PrintUsageAndExit( ERR_SUCCESS );
	}
	if ( ret == 2 )		// Invalid option
	{
		PrintUsageAndExit( ERR_BADOPTS );
	}

	VERB_PRINTF( 1, MSG_COPYRIGHT );

	if ( VMAuxCheckVirtual() )
	{
		fputs( catgets(cat, 1, 3, MSG_ERROR_NOVIRT ), stderr );
		return( ERR_NOVIRT );
	}
	
	if ( VMAuxBeginSession() )
	{
		fputs( catgets( cat, 1, 8, MSG_ERROR_NOSHF ), stderr );
		return( ERR_NOSHF );
	}
		
	switch ( InstallationCheck( &magic ) )
	{
		case 0x01:
			fputs( catgets(cat, 1, 6, MSG_ERROR_REDIR_NOT_ALLOWED ), stderr );
			ret = ERR_NOINST;
			goto err_close;
			/* NOTREACHED */
			
		case 0xff:
			if ( magic == ~VMSMOUNT_MAGIC )
			{
				fputs( catgets(cat, 1, 7, MSG_ERROR_INSTALLED ), stderr );
				ret = ERR_INSTLLD;
				goto err_close;
			}
			break;
	}
		
	if ( GetSysVars() )
	{
		fputs( catgets( cat, 1, 5, MSG_ERROR_LOL ), stderr );
		ret = ERR_SYSTEM;
		goto err_close;
	}
	
	if ( GetSDA() )
	{
		fputs( catgets( cat, 1, 9, MSG_ERROR_SDA ), stderr );
		ret = ERR_SYSTEM;
		goto err_close;
	}
	
	if ( GetNLS() )
	{
		fputs( catgets( cat, 1, 10, MSG_ERROR_NLSINFO ), stderr );
		ret = ERR_SYSTEM;
		goto err_close;
	}
	
	LoadUnicodeConversionTable();
	GetTimezoneOffset();

	ret = SetActualBufferSize();

	if ( ret )
	{
		fprintf( stderr, catgets( cat, 1, 17, MSG_ERROR_BUFFER ), VMSHF_MIN_BLOCK_SIZE, ret );
		ret = ERR_BUFFER;
		goto err_close;
	}
			
	if ( *fpDriveNum != 0xFF && !( *fpDriveNum < fpSysVars->lastDrive ) )
	{
		fprintf(stderr, catgets( cat, 1, 4, MSG_ERROR_INVALID_DRIVE ),
							*fpDriveNum + 'A', fpSysVars->lastDrive + '@');
		ret = ERR_BADDRV;
		goto err_close;
	}
	
	if ( SetCDS() )
	{
		ret = ERR_SYSTEM;
		goto err_close;
	}

	*fpfpPrevInt2fHandler = _dos_getvect( 0x2F );
	_dos_setvect( 0x2f, fpNewInt2fHandler );	

	paragraphs = SizeOfResidentSegmentInParagraphs();
	VERB_PRINTF( 2, catgets( cat, 9, 3, MSG_INFO_LOAD ), paragraphs << 4 );

	fprintf( stderr, catgets( cat, 2, 0, MSG_INFO_MOUNT ), *fpDriveNum + 'A' );
	
	catclose( cat );
	flushall();		// Flush all streams before returning to DOS
	_dos_keep( *fpDriveNum + 1, paragraphs );

err_close:
	VMAuxEndSession();
	return ret;
}

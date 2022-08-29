#ifndef _DOSDEFS_H_
#define _DOSDEFS_H_
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS
 *  Copyright (C) 2011-2022  Eduardo Casino
 *
 * dosdefs.h: various DOS structures and constants
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

#include <stdint.h>

#define DOS_SUCCESS     0 // Function was successful
#define DOS_INVLDFUNC   1 // Invalid function number
#define DOS_FILENOTFND  2 // File not found
#define DOS_PATHNOTFND  3 // Path not found
#define DOS_TOOMANY     4 // Too many open files
#define DOS_ACCESS      5 // Access denied
#define DOS_INVLDHNDL   6 // Invalid handle
#define DOS_MCBDESTRY   7 // Memory control blocks shot
#define DOS_NOMEM       8 // Insufficient memory
#define DOS_INVLDMCB    9 // Invalid memory control block
#define DOS_INVLDENV   10 // Invalid enviornment
#define DOS_INVLDFMT   11 // Invalid format
#define DOS_INVLDACC   12 // Invalid access
#define DOS_INVLDDATA  13 // Invalid data
#define DOS_INVLDDRV   15 // Invalid drive
#define DOS_RMVCUDIR   16 // Attempt remove current dir
#define DOS_DEVICE     17 // Not same device
#define DOS_NFILES     18 // No more files
#define DOS_WRTPRTCT   19 // Write protected
#define DOS_UNKUNIT    20 // Unknown unit     (FreeDOS defines this as DE_BLKINVLD "Invalid block")
#define DOS_NOTREADY   21 // Drive not ready
#define DOS_UNKCMD     22 // Unknown command
#define DOS_CRCERR     23 // Data error (CRC)
#define DOS_INVLDBUF   24 // invalid buffer size, ext fnc
#define DOS_SEEK       25 // error on file seek
#define DOS_GENERAL    31 // General failure
#define DOS_SHARE      32 // Sharing violation
#define DOS_ENOSPACE   39 // Disk full (In FreeDOS it's 28)
#define DOS_FILEEXISTS 80
#define DOS_INVLDPARM  87 // Invalid parameter

// Extended Open actions
//
#define EXTENDED_CREATE  0x10
#define EXTENDED_OPEN    0x01
#define EXTENDED_REPLACE 0x02

#pragma pack(push, 1)

// NLS Table Structure (for use with int21 0x65nn functions)
//
typedef struct
{
    uint8_t tableId;
    void __far *pTableData;
} NLSTable;

// FCHAR Table Structure
//
typedef struct
{
    uint16_t size;      // table size (not counting this word)
    uint8_t unk1;       // ??? (01h for MS-DOS 3.30-6.00)
    uint8_t lowest;     // lowest permissible character value for filename
    uint8_t highest;    // highest permissible character value for filename
    uint8_t unk2;       // ??? (00h for MS-DOS 3.30-6.00)
    uint8_t firstX;     // first excluded character in range \ all characters in this
    uint8_t lastX;      // last excluded character in range  / range are illegal
    uint8_t unk3;       // ??? (02h for MS-DOS 3.30-6.00)
    uint8_t nIllegal;   // number of illegal (terminator) characters
    uint8_t illegal[1]; // characters which terminate a filename:    ."/\[]:|<>+=;,
} FChar;

// Current Directory Structure
//
#define MAX_CDSPATH 67

#define NETWORK       (1 << 15)
#define PHYSICAL      (1 << 14)
#define JOIN          (1 << 13)
#define SUBST         (1 << 12)
#define REDIR_NOT_NET (1 <<  7) // CDROM  (We'll use this, as Shared Folders aren't network drives either)
#define UNWRITTEN     (1 <<  6)

typedef struct
{
    uint8_t currentPath[MAX_CDSPATH];
    uint16_t flags;
    uint8_t __far *dpb;
    union {
        struct
        {
            uint16_t startCluster;
            uint32_t unknown;
        } Local;
        struct
        {
            uint32_t redirIFSRecordPtr;
            uint16_t parameter;
        } Net;
    } u;
    uint16_t backslashOffset;
} CDS_V3;

typedef struct
{
    uint8_t currentPath[MAX_CDSPATH];
    uint16_t flags;
    uint8_t __far *dpb;
    union {
        struct
        {
            uint16_t startCluster;
            uint32_t unknown;
        } Local;
        struct
        {
            void __far *redirIFSRecordPtr;
            uint16_t parameter;
        } Net;
    } u;
    uint16_t backslashOffset;
    uint8_t cdsNetFlag1;
    uint8_t __far *cdsIfs;
    uint16_t cdsNetFlags2;
} CDS;

// Custom DOS System File Table entry
// Some of the fields are not used by DOS and are specific to this redirector
//
#define SFT_FDATE  0x4000 // File date set
#define SFT_FCLEAN 0x0040 // File has not been written to
typedef struct
{
    uint16_t handleCount;
    uint16_t openMode;
    uint8_t fileAttr;
    uint16_t flags;
    uint8_t __far *devDrvrPtr;
    uint16_t unused1;
    uint32_t fileTime;
    uint32_t fileSize;
    uint32_t filePos;
    uint32_t handle; // File handle
    uint16_t unused2;
    uint8_t unused3;
    char file_name[11];
} SFT;

// DOS System File Table List
//
typedef struct SFTT
{
    struct SFTT __far *nextSFTT;
    uint16_t sfttCount;
    SFT entries[];
} SFTT;

// MS-DOS List-Of-Lists
//
#define SYSVARS_DECR 12

typedef struct
{
    uint16_t shareRetryCount;
    uint16_t shareRetryDelay;
    void __far *currDiskBuff;
    uint16_t unreadCon;
    uint16_t MCB;
    void __far *DPB;
    SFTT __far *fileTable;
    void __far *clock;
    void __far *con;
    uint16_t maxBytes;
    void __far *diskBuff;
    CDS __far *currDir;
    void __far *FCB;
    uint16_t numProtFCB;
    uint8_t numBlkDev;
    uint8_t lastDrive;
    uint8_t nul[18];
    uint16_t numJoin;
} SysVars;

// Custom FindFirst/FindNext data block
// Some of the fields are not used by DOS and are specific to this redirector
//
typedef struct
{
    uint8_t driveNumber;
    char searchMask[11];
    uint8_t attrMask;
    uint16_t dirEntryNum;
    uint32_t dirHandle;
    uint8_t isRoot;
    uint8_t reserved;
} SDB;

// Directory entry for found file
//
typedef struct
{
    char fileName[11];
    uint8_t fileAttr;
    uint8_t filler[10];
    uint32_t fileTime;
    uint16_t unused;
    uint32_t fileSize;
} FDB;

// MS-DOS Swappable DOS Area (V3)
//
typedef struct
{
    uint8_t criticalErrorFlag;
    uint8_t inDOSFlag;
    uint8_t errorDrive;
    uint8_t errorLocus;
    uint16_t extendedErrorCode;
    uint8_t suggestedAction;
    uint8_t errorClass;
    void __far *errorEsDi;
    uint8_t __far *fpCurrentDTA;
    uint16_t currentPSP;
    uint16_t int23SP;
    uint16_t waitStatus;
    uint8_t currentDrive;
    uint8_t breakFlag;
    uint16_t int21AX;
    uint16_t netPSP;
    uint16_t netNumber;
    uint16_t firstMem;
    uint16_t bestMem;
    uint16_t lastMem;
    uint8_t unknown2[10];
    uint8_t monthDay;
    uint8_t month;
    uint16_t year1980;
    uint16_t days;
    uint8_t weekDay;
    uint8_t unknown3[3];
    uint8_t driverRequestHeader[30];
    void __far *driverEntryPoint;
    uint8_t driverRequestHeader2[22];
    uint8_t driverRequestHeader3[30];
    uint8_t PSPType;
    uint8_t unknown4[7];
    uint8_t clockTransfer[6];
    uint8_t unknown5[2];
    uint8_t fileName1[128];
    uint8_t fileName2[128];
    SDB findFirst;
    FDB foundEntry;
    CDS currentCDSCopy;
    char fcbName1[11];
    uint8_t unknown6;
    char fcbName2[11];
    uint8_t unknown7[9];
    uint8_t attrMask;
    uint8_t FCBType;
    uint8_t extAttr;
    uint8_t openMode;
    uint8_t unknown8[3];
    uint8_t dosFlag;
    uint8_t unknown9[9];
    uint8_t termType;
    uint8_t unknown10;
    uint8_t replaceByte;
    void __far *errorDPB;
    void __far *int21StackFrame;
    uint16_t storedSP;
    void __far *dosDPB;
    uint16_t unknown11[4];
    uint8_t mediaID;
    uint16_t unknown12[5];
    SFT __far *currentSFT;
    CDS __far *currentCDS;
    void __far *callersFCB;
    uint16_t unknown14[2];
    void __far *jft;
    uint16_t fileName1Off;
    uint16_t fileName2Off;
    uint8_t unknown15[18];
    uint32_t currOffset;
    uint8_t unknown16[12];
    uint32_t appendedBytes;
    void __far *diskBuffer;
    void __far *sft;
    uint16_t int21StoredBX;
    uint16_t int21StoredDS;
    uint16_t temporary;
    void __far *prevCallFrame;
    SDB RenFindFirst;
    FDB RenFoundEntry;
    uint8_t errorStack[331];
    uint8_t diskStack[384];
    uint8_t ioStack[384];
    uint8_t drvrLookAheadFlag;
    uint8_t volChangeFlag;
    uint16_t unknown17;
} SDA_V3;

// MS-DOS Swappable DOS Area (V4)
//
typedef struct
{
    uint8_t criticalErrorFlag;
    uint8_t inDOSFlag;
    uint8_t errorDrive;
    uint8_t errorLocus;
    uint16_t extendedErrorCode;
    uint8_t suggestedAction;
    uint8_t errorClass;
    void __far *errorEsDi;
    uint8_t __far *fpCurrentDTA;
    uint16_t currentPSP;
    uint16_t int23SP;
    uint16_t waitStatus;
    uint8_t currentDrive;
    uint8_t breakFlag;
    uint8_t unknown1[2];
    uint16_t int21AX;
    uint16_t netPSP;
    uint16_t netNumber;
    uint16_t firstMem;
    uint16_t bestMem;
    uint16_t lastMem;
    uint8_t unknown2[10];
    uint8_t monthDay;
    uint8_t month;
    uint16_t year1980;
    uint16_t days;
    uint8_t weekDay;
    uint8_t unknown3[3];
    uint8_t driverRequestHeader[30];
    void __far *driverEntryPoint;
    uint8_t driverRequestHeader2[22];
    uint8_t driverRequestHeader3[30];
    uint8_t unknown4[6];
    uint8_t clockTransfer[6];
    uint8_t unknown5[2];
    uint8_t fileName1[128];
    uint8_t fileName2[128];
    SDB findFirst;
    FDB foundEntry;
    CDS currentCDSCopy;
    char fcbName1[11];
    uint8_t unknown6;
    char fcbName2[11];
    uint8_t unknown7[11];
    uint8_t attrMask;
    uint8_t openMode;
    uint8_t unknown8[3];
    uint8_t dosFlag;
    uint8_t unknown9[9];
    uint8_t termType;
    uint8_t unknown10[3];
    void __far *errorDPB;
    void __far *int21StackFrame;
    uint16_t storedSP;
    void __far *dosDPB;
    uint16_t diskBufferSegment;
    uint16_t unknown11[4];
    uint8_t mediaID;
    uint8_t unknown12;
    void __far *unknown13;
    SFT __far *currentSFT;
    CDS __far *currentCDS;
    void __far *callersFCB;
    uint16_t unknown14[2];
    void __far *jft;
    uint16_t fileName1Off;
    uint16_t fileName2Off;
    uint16_t unknown15[12];
    uint32_t fileOffset;
    uint16_t unknown16;
    uint16_t partialBytes;
    uint16_t numSectors;
    uint16_t unknown17[3];
    uint32_t appendedBytes;
    void __far *diskBuffer;
    void __far *sft;
    uint16_t int21StoredBX;
    uint16_t int21StoredDS;
    uint16_t temporary;
    void __far *prevCallFrame;
    uint8_t unknown18[9];
    uint16_t extAction;
    uint16_t extAttr;
    uint16_t extMode;
    uint8_t unknown19[9];
    uint16_t lolDS;
    uint8_t unknown20[5];
    char __far *userFnamePtr;
    void __far *unknown21;
    uint16_t lolSS;
    uint16_t lolSP;
    uint8_t stackSwitch;
    SDB RenFindFirst;
    FDB RenFoundEntry;
    uint8_t errorStack[331];
    uint8_t diskStack[384];
    uint8_t ioStack[384];
    uint8_t drvrLookAheadFlag;
    uint8_t volChangeFlag;
    uint8_t virtFOpenFlag;
    uint8_t fastSeekDrive;
    uint16_t fastSeekFirst;
    uint16_t fastSeekLogical;
    uint16_t fastSeekReturned;
    uint16_t tempDOSSysInit;
} SDA;

// PSP
//
typedef struct
{
    uint16_t int20hInstruction;
    uint16_t wSizeOfMemoryInParagraphs;
    uint8_t reservedAt4h;
    uint8_t callToDosFunctionDispatcher[5];
    void __far *fpInt22hTerminate;
    void __far *fpInt23hCtrlC;
    void __far *fpInt24hCriticalError;
    uint16_t parentPSP;
    uint8_t jft[20];
    uint16_t wEnvironmentSegment; // Can be freed
    void __far *stackOnEntry;
    uint16_t jftSize;
    uint8_t __far *fpExtendedJFT;
    uint8_t reservedAt2Eh[24];
    uint8_t int21hAndRetfInstructions[3];
    uint8_t reservedAt53h[9];
    uint8_t FCB1[16];
    uint8_t FCB2[20];
    uint8_t bCommandLineLength; // Can be used for storage
    uint8_t szCommandLine[127];
} PSP;

#pragma pack(pop)

#endif /* _DOSDEFS_H_ */

/*
 * VMSMOUNT - A network redirector for mounting VMware's Shared Folders in DOS
 * Copyright (C) 2011-2022  Eduardo Casino
 *
 * redir.c: Redirector interface
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

#include <dos.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dosdefs.h"
#include "globals.h"
#include "redir.h"
#include "vmint.h"
#include "vmshf.h"

#include "debug.h"
#include "lfn.h"
#include "miniclib.h"
#include "vmcall.h"
#include "vmdos.h"
#include "vmtool.h"

#define SECTOR_SIZE 32768

typedef void (*redirFunction)(void);

// We place it here so if there is a stack overflow, we'll notice at the
// first "DIR" :-)
//
static char volLabel[12] = "SharedFldrs";

// Stack management
//
uint8_t newStack[STACK_SIZE] = {
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC,
    0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC, 0xEC};
static uint16_t __far *fpStackParam; // Far pointer to top os stack at entry
static uint16_t dosSS;
static uint16_t dosSP;
static uint16_t dosBP;

static uint16_t curSP;

void(__interrupt __far *fpPrevInt2fHandler)();
static union INTPACK __far *r;
static redirFunction currFunction;
static __segment dosDS;
__segment myDS;

uint8_t lfn = 0;
uint8_t driveNum = 0xFF;
CDS __far *fpCDS; // CDS for this drive
SDA __far *fpSDA;
SDB __far *fpSDB;
FDB __far *fpFDB;
SFTT __far *fpFileTable;

char __far *fpFcbName1;
char __far *fpFcbName2;
char __far *fpFileName1;
char __far *fpFileName2;
char __far *fpCurrentPath;
char __far *fpLongFileName1;
char __far *fpLongFileName2;

static char fcbName[12];

static void SetSftOwner(void);
#pragma aux SetSftOwner = \
    /* Set DOS DS and stack */                                  \
    "mov   curSP, sp"     /* Save current SP */                 \
    "mov   ss, dosSS"                                           \
    "mov   sp, dosSP"                                           \
    "push  ds"                                                  \
    "mov   ds, dosDS"                                           \
                                                                \
    /* Call int2F120C "OPEN DEVICE AND SET SFT OWNER/MODE"  */  \
    /*   needs DS = DOS DS and a DOS internal stack         */  \
    /*   seems that it destroys ES and DI                   */  \
    "mov   ax, 0x120C"                                          \
    "int   0x2F"                                                \
                                                                \
    /* Restore our DS and internal stack */                     \
    "pop   bx"            /* Restore saved DS             */    \
    "mov   ds, bx"                                              \
    "mov   ss, bx"        /* Restore saved SS, same as DS */    \
    "mov   sp, curSP"     /* Restore saved SP             */    \
    __modify [__ax __bx __es __di];

static uint32_t GetDosTime(void);
#pragma aux GetDosTime = /* Set DOS DS and stack */             \
    "mov   curSP, sp"      /* Save current SP */                \
    "mov   ss, dosSS"                                           \
    "mov   sp, dosSP"                                           \
    "push  ds"                                                  \
    "mov   ds, dosDS"                                           \
                                                                \
    /* Call int2F120D "GET DATE AND TIME"           */          \
    /*   needs DS = DOS DS and a DOS internal stack */          \
    "mov   ax, 0x120D"                                          \
    "int   0x2F"                                                \
    "xchg  ax, dx"                                              \
                                                                \
    /* Restore our DS and internal stack */                     \
    "pop   bx"             /* Restore saved DS             */   \
    "mov   ds, bx"                                              \
    "mov   ss, bx"         /* Restore saved SS, same as DS */   \
    "mov   sp, curSP"      /* Restore saved SP             */   \
    __value [__ax __dx]                                         \
    __modify [__bx];

static inline void Success(void)
{
    r->w.flags &= ~INTR_CF;
    r->w.ax = 0;

    return;
}

static inline void Failure(uint16_t error)
{
    r->w.flags |= INTR_CF;
    r->w.ax = error;

    return;
}

inline void FillFcbName(char __far *fcbName, char __far *fileName)
{
    int i;
    char __far *p = _fstrrchr_local(fileName, '\\') + 1;

    for (i = 0; *p; ++p)
    {
        if (*p == '.')
        {
            while (i < 8)
            {
                fcbName[i++] = ' ';
            }
        }
        else
        {
            fcbName[i++] = *p;
        }
    }
    while (i < 11)
    {
        fcbName[i++] = ' ';
    }

    return;
}

static void InstallationCheck(void)
{
    DPUTS("InstallationCheck()");

    r->w.ax = (uint16_t)0x00FF; // Installed

    return;
}

static void RmDir(void)
{
    int ret;
    uint32_t status;

    DPUTS("RmDir()");

    ret = VMShfDeleteDir(lfn ? LfnGetTrueLongName(fpLongFileName1, fpFileName1) : fpFileName1, lfn, &status);

    if (ret != VMTOOL_SUCCESS)
    {
        Failure(DOS_NOTREADY);
    }
    else if (status != VMSHF_SUCCESS)
    {
        Failure(VmshfStatusToDosError(status));

        if (r->w.ax == DOS_FILENOTFND)
        {
            r->w.ax = DOS_PATHNOTFND;
        }
    }

    return;
}

static void MkDir(void)
{
    int ret;
    uint32_t status;

    DPUTS("MkDir()");

    ret = VMShfCreateDir(VMSHF_FILEMODE_ALL, lfn ? LfnGetTrueLongName(fpLongFileName1, fpFileName1) : fpFileName1,
                         lfn, &status);

    if (ret != VMTOOL_SUCCESS)
    {
        Failure(DOS_NOTREADY);
    }
    else if (status != VMSHF_SUCCESS)
    {
        Failure(VmshfStatusToDosError(status));

        if (r->w.ax == DOS_FILENOTFND)
        {
            r->w.ax = DOS_PATHNOTFND;
        }
    }

    return;
}

static void ChDir(void)
{
    uint32_t status;
    VMShfAttr *fAttr;
    uint8_t fatAttr;
    int ret;

    DPUTS("ChDir()");

    if (*(uint16_t __far *)fpFileName1 != '\0\\')
    {
        // Check if path exists and is a directory
        //
        ret = VMShfGetAttr(lfn ? LfnGetTrueLongName(fpLongFileName1, fpFileName1) : fpFileName1, lfn,
                           VMSHF_INVALID_HANDLE, &status, &fAttr);

        if (ret != VMTOOL_SUCCESS)
        {
            Failure(DOS_NOTREADY);
        }
        else if (status != VMSHF_SUCCESS)
        {
            Failure(VmshfStatusToDosError(status));

            if (r->w.ax == DOS_FILENOTFND)
            {
                r->w.ax = DOS_PATHNOTFND;
            }
            return;
        }
        else
        {
            fatAttr = FModeToFatAttr(fAttr);

            if (!(fatAttr & _A_SUBDIR))
            {
                Failure(DOS_PATHNOTFND);
                return;
            }
        }
    }
    _fstrcpy_local(fpCurrentPath, fpFileName1);

    return;
}

static void _CloseFile(SFT __far *fpSFT)
{
    uint32_t status;
    static VMShfAttr newAttr = {0};

    // Check if date/time is to be set from clock at CLOSE.
    if (!(fpSFT->flags & SFT_FCLEAN))
    {
        if (!(fpSFT->flags & SFT_FDATE))
        {
            fpSFT->fileTime = GetDosTime();
        }
    }

    if ((fpSFT->flags & (SFT_FCLEAN | SFT_FDATE)) != SFT_FCLEAN)
    {
        // Set modification time
        newAttr.mask = VMSHF_VALID_ATTR_UTIME;
        newAttr.utime = FatTimeToFTime(fpSFT->fileTime);

        if (!(fpSFT->fileAttr & _A_RDONLY))
        {
            (void)VMShfSetAttr(&newAttr, NULL, 0, fpSFT->handle, &status);
        }
        else
        {
            (void)VMShfSetAttr(&newAttr, lfn ? LfnGetTrueLongName(fpLongFileName1, fpFileName1) : fpFileName1,
                               lfn, VMSHF_INVALID_HANDLE, &status);
        }
    }

    if (fpSFT->handleCount) // Decrement handle count
    {
        --fpSFT->handleCount;
    }

    (void)VMShfCloseFile(fpSFT->handle, &status);

    return;
}

static void CloseFile(void)
{
    SFT __far *fpSFT = (SFT __far *)MK_FP(r->w.es, r->w.di);

    DPUTS("CloseFile()");

    _CloseFile(fpSFT);
}

static void CommitFile(void)
{
    DPUTS("CommitFile()");

    // Just succeed

    return;
}

static void ReadFile(void)
{
    int ret;
    uint32_t status;
    uint32_t length, toread;
    char *data;
    char __far *buffer;

    SFT __far *fpSFT = (SFT __far *)MK_FP(r->w.es, r->w.di);

    DPUTS("ReadFile()");

    if (fpSFT->openMode & O_WRONLY)
    {
        Failure(DOS_ACCESS);
        return;
    }

    if ((fpSFT->filePos + r->w.cx) > fpSFT->fileSize)
    {
        // Can't read past the EOF
        r->w.cx = (uint16_t)(fpSFT->fileSize - fpSFT->filePos);
    }

    if (!r->w.cx) // Anything to read?
    {
        return;
    }

    buffer = fpSDA->fpCurrentDTA;
    toread = (uint32_t)r->w.cx;

    do
    {
        length = toread;

        ret = VMShfReadFile(fpSFT->handle, (uint64_t)fpSFT->filePos, &length, &data, &status);

        if (ret != VMTOOL_SUCCESS)
        {
            Failure(DOS_NOTREADY);
        }
        else if (status != VMSHF_SUCCESS)
        {
            Failure(VmshfStatusToDosError(status));
            break;
        }
        else
        {
            if (!length) // Can't read more data
            {
                break;
            }
            _fmemcpy_local(buffer, MK_FP(myDS, data), length);
            buffer += length;
            fpSFT->filePos += length;
            toread -= length;
        }
    } while (toread);

    r->w.cx -= (uint16_t)toread;

    return;
}

static void WriteFile(void)
{
    int ret;
    uint32_t status;
    uint32_t length, towrite;
    char __far *buffer;
    static VMShfAttr newAttr;

    SFT __far *fpSFT = (SFT __far *)MK_FP(r->w.es, r->w.di);

    DPRINTF("WriteFile(), r->w.cx = %d", r->w.cx);

    if (!(fpSFT->openMode & (O_WRONLY | O_RDWR)))
    {
        Failure(DOS_ACCESS);
        return;
    }
    // mark file as modified and set date not valid any more
    fpSFT->flags &= ~(SFT_FCLEAN | SFT_FDATE);

    // Tricky: If size is 0, truncate to current file position
    //
    if (!r->w.cx)
    {
        newAttr.fsize = (uint64_t)fpSFT->filePos;
        newAttr.mask = VMSHF_VALID_ATTR_FSIZE;

        ret = VMShfSetAttr(&newAttr, NULL, 0, fpSFT->handle, &status);

        if (ret != VMTOOL_SUCCESS)
        {
            Failure(DOS_NOTREADY);
        }
        else if (status != VMSHF_SUCCESS)
        {
            Failure(DOS_ACCESS);
            return;
        }

        fpSFT->fileSize = fpSFT->filePos;

        return;
    }

    buffer = fpSDA->fpCurrentDTA;
    towrite = (uint32_t)r->w.cx;

    do
    {
        length = towrite;

        ret = VMShfWriteFile(fpSFT->handle, 0, (uint64_t)fpSFT->filePos, &length, buffer, &status);

        if (ret != VMTOOL_SUCCESS)
        {
            Failure(DOS_NOTREADY);
        }
        else if (status != VMSHF_SUCCESS)
        {
            Failure(VmshfStatusToDosError(status));
            break;
        }
        else
        {
            if (!length) // Can't write more data
            {
                break;
            }
            buffer += length;
            fpSFT->filePos += length;
            towrite -= length;
        }
    } while (towrite);

    if (fpSFT->filePos > fpSFT->fileSize)
    {
        fpSFT->fileSize = fpSFT->filePos;
    }

    r->w.cx -= (uint16_t)towrite;

    return;
}

static void LockFile(void)
{
    DPUTS("LockFile()");

    return;
}

static void UnlockFile(void)
{
    DPUTS("UnlockFile()");

    return;
}

static void GetDiskSpace(void)
{
    char __far *path;
    uint32_t status;
    uint64_t avail;
    uint64_t total;
    uint32_t sectSize = SECTOR_SIZE;

    DPUTS("GetDiskSpace()");

    if (MK_FP(r->w.es, r->w.di) == fpSDA->currentCDS)
    {
        // Called on current drive
        path = lfn ? LfnGetTrueLongName(fpLongFileName2, fpCurrentPath) : fpCurrentPath;
    }
    else
    {
        path = (char __far *)NULL;
    }

    if (VMShfGetDirSize(path, lfn, &status, &avail, &total))
    {
        Failure(r->w.ax);
    }
    else
    {
        (void)u64div32(&avail, avail, sectSize);
        (void)u64div32(&total, total, sectSize);

        r->w.ax = 1;                                                 // Sectors per cluster
        r->w.bx = (total > 0xffffui64) ? 0xffff : (uint16_t)(total); // Total sectors
        r->w.cx = sectSize;                                          // Sector size
        r->w.dx = (avail > 0xffffui64) ? 0xffff : (uint16_t)(avail); // Sectors free
    }

    return;
}

static void _GetFileAttrib(void)
{
    int ret;
    uint32_t status;
    VMShfAttr *fAttr;

    ret = VMShfGetAttr(lfn ? LfnGetTrueLongName(fpLongFileName1, fpFileName1) : fpFileName1, lfn, VMSHF_INVALID_HANDLE,
                       &status, &fAttr);

    if (ret != VMTOOL_SUCCESS)
    {
        Failure(DOS_NOTREADY);
    }
    else if (status != VMSHF_SUCCESS)
    {
        Failure(VmshfStatusToDosError(status));
    }
    else
    {
        r->w.ax = (uint16_t)FModeToFatAttr(fAttr);
    }

    return;
}

static void GetFileAttrib(void)
{
    DPUTS("GetFileAttrib()");

    _GetFileAttrib();
}

static void SetFileAttrib(void)
{
    int ret;
    uint32_t status;

    DPUTS("SetFileAttrib()");

    _GetFileAttrib();

    if (r->w.flags & INTR_CF)
    {
        return;
    }

    // Check if file is not a directory and we are trying to set the _A_SUBDIR attribute
    //
    if ((!(r->w.ax & _A_SUBDIR)) && (r->w.cx & _A_SUBDIR))
    {
        Failure(DOS_ACCESS);
        return;
    }

    ret = VMShfSetAttr(FatAttrToFMode((uint8_t)*fpStackParam),
                       lfn ? LfnGetTrueLongName(fpLongFileName1, fpFileName1) : fpFileName1, lfn, VMSHF_INVALID_HANDLE,
                       &status);

    if (ret != VMTOOL_SUCCESS)
    {
        Failure(DOS_NOTREADY);
    }
    else if (status != VMSHF_SUCCESS)
    {
        Failure(VmshfStatusToDosError(status));
    }

    return;
}

static void OpenOrCreateFile(uint16_t accessMode, uint32_t action, VMShfAttr *openAttr)
{
    int ret;
    uint32_t status;
    uint32_t handle;
    VMShfAttr *fAttr;
    uint8_t fatAttr;
    SFT __far *fpSFT = (SFT __far *)MK_FP(r->w.es, r->w.di);

    ret = VMShfOpenFile((uint32_t)accessMode, action, openAttr->fmode, openAttr->attr,
                        lfn ? LfnGetTrueLongName(fpLongFileName1, fpFileName1) : fpFileName1, lfn, &status, &handle);

    if (ret != VMTOOL_SUCCESS)
    {
        Failure(DOS_NOTREADY);
    }
    else if (status != VMSHF_SUCCESS)
    {
        Failure(VmshfStatusToDosError(status));
    }
    else
    {
        fpSFT->flags &= ~SFT_FDATE;
        // use FCLEAN even on replaced/created files: the bit is reset
        // if the file is written to later
        fpSFT->flags |= SFT_FCLEAN;

        // Get file attributes using file handle
        //
        ret = VMShfGetAttr(NULL, 0, handle, &status, &fAttr);

        if (ret != VMTOOL_SUCCESS)
        {
            Failure(DOS_NOTREADY);
        }
        else if (status != VMSHF_SUCCESS)
        {
            Failure(VmshfStatusToDosError(status));
            (void)VMShfCloseFile(handle, &status);
        }
        else
        {
            fatAttr = FModeToFatAttr(fAttr);
            if (fatAttr & (_A_SUBDIR | _A_VOLID))
            {
                Failure(DOS_ACCESS);
                (void)VMShfCloseFile(handle, &status);
            }
            else
            {
                fpSFT->openMode = accessMode;
                fpSFT->fileAttr = fatAttr;
                fpSFT->flags = (NETWORK | UNWRITTEN | driveNum);
                fpSFT->devDrvrPtr = (char __far *)NULL;
                fpSFT->fileTime = FTimeToFatTime(fAttr->utime);
                fpSFT->fileSize = (fAttr->fsize > 0xffffffffui64) ? 0xffffffff : (uint32_t)fAttr->fsize;

                fpSFT->filePos = 0;
                fpSFT->handle = handle;
                _fmemcpy_local(&fpSFT->file_name, fpFcbName1, 11);

                if (fpSFT->openMode & 0x8000)
                {
                    /* File opened via FCB */
                    fpSFT->openMode |= 0x00F0;
                    SetSftOwner();
                }
                else
                {
                    fpSFT->openMode &= 0x000F;
                }
            } // ! fatAttr & _A_SUBDIR
        }     // VMShfGetAttr() OK
    }         // VMShfOpenFile() OK

    return;
}

static void OpenFile(void)
{
    DPUTS("OpenFile()");

    OpenOrCreateFile(fpSDA->openMode & 3, VMSHF_ACTION_O_EXIST,
                     FatAttrToFMode(_A_NORMAL | _A_ARCH | _A_RDONLY | _A_HIDDEN | _A_SYSTEM));

    return;
}

static void CreateFile(void)
{
    DPUTS("CreateFile()");

    OpenOrCreateFile(VMSHF_ACCESS_READWRITE, VMSHF_ACTION_C_ALWAYS, FatAttrToFMode((uint8_t)*fpStackParam));

    return;
}

inline int MatchAttrMask(uint8_t fatAttr, uint8_t mask)
{
    // _A_RDONLY and _A_ARCH should always match
    //
    return !( ((fatAttr & _A_HIDDEN) && (!(mask & _A_HIDDEN))) || ((fatAttr & _A_SYSTEM) && (!(mask & _A_SYSTEM))) ||
              ((fatAttr & _A_VOLID)  && (!(mask & _A_VOLID)))  || ((fatAttr & _A_SUBDIR) && (!(mask & _A_SUBDIR))));
}

inline int MatchNameMask(char *fname, char __far *mask)
{
    int i;

    for (i = 0; i < 11; ++i)
    {
        if ((fname[i] != mask[i]) && (mask[i] != '?'))
        {
            return 0;
        }
    }
    return 1;
}

inline int FcbNameHasWildCards(char __far *fcbName)
{
    int i;

    for (i = 0; i < 11; ++i)
    {
        if (fcbName[i] == '?')
        {
            return 1;
        }
    }
    return 0;
}

static void _FindNext(void)
{
    int ret;
    uint32_t status = VMSHF_SUCCESS;
    VMShfAttr *fAttr;
    uint8_t fatAttr;
    uint32_t fNameLen;
    char *fName;

    int i = fpSDB->dirEntryNum;

    if (fpSDB->dirHandle == VMSHF_INVALID_HANDLE)
    {
        Failure(DOS_NFILES);
        return;
    }

    for (;;)
    {
        ret = VMShfReadDir(fpSDB->dirHandle, ++i, &status, &fAttr, &fName, &fNameLen);

        if (ret != VMTOOL_SUCCESS)
        {
            Failure(DOS_NOTREADY);
        }
        else if (status != VMSHF_SUCCESS || 0 == fName)
        {
            Failure((0 == fName) ? DOS_NFILES : VmshfStatusToDosError(status));
            (void)VMShfCloseDir(fpSDB->dirHandle, &status);
            fpSDB->dirHandle = VMSHF_INVALID_HANDLE;
            break;
        }
        fatAttr = FModeToFatAttr(fAttr);

        if (FNameToFcbName(fcbName, fName, (uint16_t)fNameLen, fpSDB->isRoot, lfn) &&
            MatchNameMask(fcbName, fpSDB->searchMask) && MatchAttrMask(fatAttr, fpSDB->attrMask))
        {

            _fmemcpy_local(fpFDB->fileName, MK_FP(myDS, fcbName), 11);

            fpFDB->fileAttr = fatAttr;
            fpFDB->fileTime = FTimeToFatTime(fAttr->utime);
            fpFDB->fileSize = (fAttr->fsize > 0xffffffffui64) ? 0xffffffff : (uint32_t)fAttr->fsize;
            fpFDB->unused = 0;

            fpSDB->dirEntryNum = i;

            if (!FcbNameHasWildCards(fpSDB->searchMask))
            {
                // If attrMask has no wildcards, then this will be the last match
                // close directory search
                (void)VMShfCloseDir(fpSDB->dirHandle, &status);
                fpSDB->dirHandle = VMSHF_INVALID_HANDLE;
            }

            break;
        }
    }

    return;
}

static void FindNext(void)
{
    DPUTS("FindNext()");

    _FindNext();
}

static void _FindFirst(void)
{
    uint32_t status = VMSHF_SUCCESS;
    char __far *p;
    VMShfAttr *fAttr;
    uint8_t fatAttr;
    uint32_t handle;
    int ret;

    // Special case: Get Volume ID
    //
    if (fpSDA->attrMask == _A_VOLID)
    {

        fpSDB->driveNumber = driveNum | 0x80;
        fpSDB->dirEntryNum = 0;
        _fmemcpy_local(fpSDB->searchMask, fpFcbName1, 11);

        fpFDB->fileAttr = _A_VOLID;
        fpFDB->fileTime = GetDosTime();
        fpFDB->fileSize = 0;
        _fmemcpy_local(fpFDB->fileName, MK_FP(myDS, volLabel), 11);

        return;
    }

    // Open dir to perform search
    //
    *(p = _fstrrchr_local(fpFileName1, '\\')) = 0; // Get the directory part

    fpSDB->isRoot = (*fpFileName1 == '\0') ? 1 : 0;

    ret = VMShfOpenDir(lfn ? LfnGetTrueLongName(fpLongFileName1, fpFileName1) : fpFileName1, lfn, &status, &handle);

    *p = '\\';

    if (ret != VMTOOL_SUCCESS)
    {
        Failure(DOS_NOTREADY);
    }
    else if (status != VMSHF_SUCCESS)
    {
        Failure(VmshfStatusToDosError(status));

        if (r->w.ax == DOS_FILENOTFND)
        {
            r->w.ax = DOS_PATHNOTFND;
        }
    }
    else
    {
        // Initialise FindFirst/FindNext data block
        //
        _fmemcpy_local(fpSDB->searchMask, fpFcbName1, 11);
        fpSDB->driveNumber = driveNum | 0xC0;
        fpSDB->attrMask = fpSDA->attrMask;
        fpSDB->dirEntryNum = -1;
        fpSDB->dirHandle = handle;

        _FindNext();

        if (DOS_NFILES == r->w.ax)
        {
            r->w.ax = DOS_FILENOTFND;
        }
    }

    return;
}

static void FindFirst(void)
{
    DPUTS("FindFirst()");

    _FindFirst();
}

// Index to SFT functions. Copied from FreeDOS kernel, lazy me
//
static inline int idx_to_sft_(int sftIndex, SFT __far **fpCurSFT)
{
    SFTT __far *sp;

    *fpCurSFT = (SFT __far *)-1;
    if (sftIndex < 0)
    {
        return -1;
    }

    /* Get the SFT block that contains the SFT      */
    for (sp = fpFileTable; sp != (SFTT __far *)-1; sp = sp->nextSFTT)
    {
        if (sftIndex < sp->sfttCount)
        {
            /* finally, point to the right entry            */
            *fpCurSFT = (SFT __far *)&(sp->entries[sftIndex]);
            return sftIndex;
        }
        sftIndex -= sp->sfttCount;
    }

    /* If not found, return an error                */
    return -1;
}

SFT __far *idx_to_sft(int sftIndex)
{
    SFT __far *fpCurSFT;

    /* called internally only */
    sftIndex = idx_to_sft_(sftIndex, &fpCurSFT);

    /* if not opened, the SFT is useless            */
    if (sftIndex == -1 || fpCurSFT->handleCount == 0)
    {
        return (SFT __far *)-1;
    }
    return fpCurSFT;
}

static void CloseAll(void)
{
    PSP __far *fpPSP = (PSP __far *)MK_FP(fpSDA->currentPSP, 0);
    int i;

    DPUTS("CloseAll()");

    if (fpPSP == NULL)
    {
        return;
    }

    for (i = 5; i < fpPSP->jftSize; ++i)
    {
        uint8_t idx = fpPSP->fpExtendedJFT[i];
        if (idx != 0xFF)
        {
            SFT __far *fpSFT = idx_to_sft(idx);
            if (fpSFT != (SFT __far *)-1)
            {
                _CloseFile(fpSFT);
            }
        }
    }
}

static void ProcessEnd(void)
{
    DPUTS("ProcessEnd()");
}

static void MakeFullPath(char __far *fullPath, char __far *dirName, char __far *fcbName)
{
    int i;

    while (*dirName)
    {
        *(fullPath++) = *(dirName++);
    }
    *(fullPath++) = '\\';

    for (i = 0; i < 11; ++i)
    {
        if (fcbName[i] != ' ')
        {
            if (i == 8)
            {
                *(fullPath++) = '.';
            }
            *(fullPath++) = fcbName[i];
        }
    }
    *fullPath = '\0';

    return;
}

static void DeleteFile(void)
{
    int ret, error = DOS_SUCCESS;
    uint32_t status;
    char __far *p;

    DPUTS("DeleteFile()");

    fpSDA->attrMask = _A_NORMAL | _A_RDONLY | _A_ARCH;

    _FindFirst();

    while (!r->w.ax)
    {
        if (fpFDB->fileAttr & _A_RDONLY)
        {
            error = DOS_ACCESS;
        }
        else
        {
            *(p = _fstrrchr_local(fpFileName1, '\\')) = 0; // Get the directory part

            MakeFullPath(fpFileName2, fpFileName1, &fpFDB->fileName[0]);

            *p = '\\';

            ret = VMShfDeleteFile(lfn ? LfnGetTrueLongName(fpLongFileName2, fpFileName2) : fpFileName2, lfn, &status);

            if (ret != VMTOOL_SUCCESS)
            {
                Failure(DOS_NOTREADY);
            }
            else if (status != VMSHF_SUCCESS)
            {
                Failure(VmshfStatusToDosError(status));
                return;
            }
        }

        _FindNext();
    }

    if (r->w.ax != DOS_NFILES)
    {
        error = r->w.ax;
    }

    if (!error)
    {
        Success();
    }
    else
    {
        Failure(error);
    }

    return;
}

static void RenameFile(void)
{
    int i, ret, error = DOS_SUCCESS;
    uint32_t status;
    char __far *p;

    DPUTS("RenameFile()");

    if (*(uint16_t __far *)(fpFileName1 - 2) != *(uint16_t __far *)(fpFileName2 - 2))
    {
        Failure(DOS_DEVICE);
        return;
    }

    FillFcbName(fpFcbName2, fpFileName2);

    fpSDA->attrMask = _A_NORMAL | _A_RDONLY | _A_ARCH | _A_SUBDIR;

    _FindFirst();

    if (r->w.ax)
    {
        return;
    }

    while (!r->w.ax)
    {

        *(p = _fstrrchr_local(fpFileName1, '\\')) = 0; // Get the directory part

        MakeFullPath(fpFileName1, fpFileName1, &fpFDB->fileName[0]); // Make full path for source

        for (i = 0; i < 11; ++i) // Generate destination name
        {
            if (fpFcbName2[i] != '?')
            {
                fpFDB->fileName[i] = fpFcbName2[i];
            }
        }

        *(p = _fstrrchr_local(fpFileName2, '\\')) = 0; // Get the directory part

        MakeFullPath(fpFileName2, fpFileName2, &fpFDB->fileName[0]); // Make full path for source

        ret = VMShfRenameFile(lfn ? LfnGetTrueLongName(fpLongFileName1, fpFileName1) : fpFileName1,
                              lfn ? LfnGetTrueLongName(fpLongFileName2, fpFileName2) : fpFileName2, lfn, &status);

        if (ret != VMTOOL_SUCCESS)
        {
            Failure(DOS_NOTREADY);
        }
        else if (status != VMSHF_SUCCESS)
        {
            Failure(VmshfStatusToDosError(status));
            return;
        }

        _FindNext();
    }

    if (r->w.ax != DOS_NFILES)
    {
        error = r->w.ax;
    }

    if (!error)
    {
        Success();
    }
    else
    {
        Failure(error);
    }

    return;
}

static void SpecialOpen(void)
{
    DPUTS("SpecialOpen()");

    OpenOrCreateFile(fpSDA->extMode & 0x7f, DosExtActionToOpenAction(fpSDA->extAction), FatAttrToFMode(fpSDA->extAttr));

    return;
}

static int IsCallForUs(uint8_t function)
{

    SFT __far *fpSFT = (SFT __far *)MK_FP(r->w.es, r->w.di);

    if (function == 0x00)
    {
        return 1;
    }

    if (function == 0x21 || (function >= 0x06 && function <= 0x0B))
    {
        return ((fpSFT->flags & 0x3F) == driveNum);
    }

    if (function == 0x1C) // FindNext
    {
        return ((fpSDB->driveNumber & 0x40) && ((fpSDB->driveNumber & 0x1F) == driveNum));
    }

    // This is not mentioned in "Undocumented DOS". GetDiskSpace can be called for
    // a drive other than current and CDS is passed in es:di
    //
    if (function == 0x0C) // GetDiskSpace
    {
        CDS __far *fpCDS = (CDS __far *)MK_FP(r->w.es, r->w.di);

        if (fpCDS->u.Net.parameter == VMSMOUNT_MAGIC)
        {
            return 1;
        }
    }

    if (fpSDA->currentCDS->u.Net.parameter == VMSMOUNT_MAGIC)
    {
        FillFcbName(fpFcbName1, fpFileName1);
        return 1;
    }
    else
    {
        return 0;
    }
}

static redirFunction dispatchTable[] = {
    InstallationCheck, /* 0x00 */
    RmDir,             /* 0x01 */
    NULL,              /* 0x02 */
    MkDir,             /* 0x03 */
    NULL,              /* 0x04 */
    ChDir,             /* 0x05 */
    CloseFile,         /* 0x06 */
    CommitFile,        /* 0x07        Does nothing */
    ReadFile,          /* 0x08 */
    WriteFile,         /* 0x09 */
    LockFile,          /* 0x0A        Does nothing */
    UnlockFile,        /* 0x0B        Does nothing */
    GetDiskSpace,      /* 0x0C */
    NULL,              /* 0x0D */
    SetFileAttrib,     /* 0x0E */
    GetFileAttrib,     /* 0x0F */
    NULL,              /* 0x10 */
    RenameFile,        /* 0x11 */
    NULL,              /* 0x12 */
    DeleteFile,        /* 0x13 */
    NULL,              /* 0x14 */
    NULL,              /* 0x15 */
    OpenFile,          /* 0x16 */
    CreateFile,        /* 0x17 */
    NULL,              /* 0x18 */
    NULL,              /* 0x19 */
    NULL,              /* 0x1A */
    FindFirst,         /* 0x1B */
    FindNext,          /* 0x1C */
    CloseAll,          /* 0x1D */
    NULL,              /* 0x1E */
    NULL,              /* 0x1F */
    NULL,              /* 0x20 */
    NULL,              /* 0x21        SeekFromdEnd() Unsupported */
    ProcessEnd,        /* 0x22        Process Termination Hook */
    NULL,              /* 0x23 */
    NULL,              /* 0x24 */
    NULL,              /* 0x25 */
    NULL,              /* 0x26 */
    NULL,              /* 0x27 */
    NULL,              /* 0x28 */
    NULL,              /* 0x29 */
    NULL,              /* 0x2A */
    NULL,              /* 0x2B */
    NULL,              /* 0x2C */
    NULL,              /* 0x2D */
    SpecialOpen        /* 0x2E */
};

#define MAX_FUNCTION 0x2E

static bool __near Int2fHandler(union INTPACK regset)
#pragma aux Int2fHandler "*" __parm __caller [] __value [__al] __modify [__ax __bx __cx __dx __si __di __es __gs __fs]
{
    if (regset.h.ah != 0x11 || regset.h.al > MAX_FUNCTION)
    {
        return false;
    }

    r = &regset;

    currFunction = dispatchTable[regset.h.al];

    if (NULL == currFunction || !IsCallForUs(regset.h.al))
    {
        return false;
    }

    dosDS = r->w.ds; // We'll use this in some inline assembly functions because
                     // OpenWatcom does not support struct references in ASM

    // Save ss:sp and switch to our internal stack.
    // Save bp so we can get at parameters at the top of the stack.
    // Also get current DS
    //
    __asm
    {
        mov   dosSS, ss
        mov   dosSP, sp
        mov   dosBP, bp
        mov   ax, ds
        mov   myDS, ax
        mov   ss, ax
        mov   sp, (offset newStack) + STACK_SIZE
    }

    fpStackParam = (uint16_t __far *)MK_FP(dosSS, dosBP + sizeof(union INTPACK) + 4 );

    Success();

    currFunction();

    __asm
    {
        mov ss,   dosSS
        mov sp,   dosSP
    }

    return true;
}

void __declspec(naked) __far Int2fRedirector(void)
{
    __asm
    {
        pusha
        push  ds
        push  es
        push  fs
        push  gs

        mov   bp, sp
        push  cs
        pop   ds

        call  Int2fHandler

        test  al, al
        jnz   handled

        mov    bx, word ptr [bp+28]     ; restore flags
        and    bx, 0fcffh               ; except for IF and TF
        push   bx
        popf
        pop   gs
        pop   fs
        pop   es
        pop   ds
        popa

        ; Jump to the next handler in the chain
        ;
        jmp dword ptr [cs:fpPrevInt2fHandler]

    handled:
        pop   gs
        pop   fs
        pop   es
        pop   ds
        popa
        
        iret
    }
}

__segment __far get_tsr_cs(void)
{
    return get_cs();
}

/**
 * This function must the the last one in the BEGTEXT segment!
 *
 * BeginOfTransientBlock() must locate after all other resident functions and global variables.
 * This function must be defined in the last source file if there are more than
 * one source file containing resident functions.
 */
uint16_t BeginOfTransientBlockNoLfn(void)
{
    return (uint16_t)BeginOfTransientBlockNoLfn; // Force some code
}

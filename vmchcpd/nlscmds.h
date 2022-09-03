/*
 * VMCHCPD
 *  A DOS device driver for managing code page changes for VMSMOUNT
 *
 * Copyright (c) 2022,  Eduardo Casino
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

#ifndef _NLSCMDS_
#define _NLSCMDS_

#include <stdint.h>

// Device Control Channel commands (just one for now)
#define DEV_CTRL_SET_NLS       0
#define DEV_CTRL_LAST_COMMAND  DEV_CTRL_SET_NLS

#pragma pack(push, 1)
typedef struct {
    uint16_t length;
    uint16_t codePage;
} NLSDATA;

typedef struct {
    uint8_t ptr_size;
    uint16_t __far *unicode_table;
    uint8_t path_size;
    char table_path[1];
} SETNLS;

typedef struct {
    uint8_t num;
    union {
        SETNLS set_nls;
    } cmd;
} CHCPCMD;

#pragma pack(pop)

#endif /* _NLSCMDS_ */

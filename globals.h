#ifndef _GLOBALS_H
#define _GLOBALS_H
#pragma once
/*
 * VMSMOUNT
 *  A network redirector for mounting VMware's Shared Folders in DOS 
 *  Copyright (C) 2011  Eduardo Casino
 *
 * globals.h: Some global definitions used in various modules
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

#include "kitten.h"
#include "vmdos.h"
#include "vmshf.h"

#define PUBLIC

#define VERSION_MAJOR	0
#define VERSION_MINOR	1

#define TRUE	1
#define FALSE	0

#define ERR_SUCCESS	0
#define ERR_LOADED	1
#define ERR_WRONGOS	2
#define ERR_NOVIRT	3
#define ERR_NOSHF	4
#define ERR_BADOPTS	5
#define ERR_SYSTEM	6
#define ERR_INSTLLD	7

#define VERB_PRINTF(...) if (verbose) printf(__VA_ARGS__)

extern nl_catd cat;
extern int verbose;

extern shf_t far *fpShf;

#endif /* _GLOBALS_H */

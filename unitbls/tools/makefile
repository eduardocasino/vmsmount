# vi: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab
#
# VMSMOUNT
#  A network redirector for mounting VMware's Shared Folders in DOS 
#  Copyright (C) 2022  Eduardo Casino (mail@eduardocasino.es)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
#
# NOTE:
#   As license is unclear, download and build Henrik Haftmann's
#   mk_table from its original site. No part of its code is used in
#   VMSMOUNT, it is just used for generating the conversion tables
#
#

# Can't get it to compile with Open Watcom
#
# CC = wcl386
CC = gcc
RM = rm -f
!ifeq CC wcl386
CFLAGS  = -bcl=linux
!else
CFLAGS =
!endif

DOSLFN = doslfn.zip
LINK = https://www-user.tu-chemnitz.de/~heha/hsn/dos/doslfn/$(DOSLFN)

# Jason Hood's version:
# LINK = http://adoxa.altervista.org/doslfn/dl.php?f=doslfn

TARGET = mk_table

all : $(TARGET)

clean : .SYMBOLIC
	$(RM) $(TARGET) $(DOSLFN) mk_table.c

!ifeq CC wcl386

$(TARGET) : mk_table.c
	$(CC) $(CFLAGS) -fe $@ $<

!else

$(TARGET) : mk_table.c
	$(CC) $(CFLAGS) -o $@ $<

!endif

mk_table.c : $(DOSLFN)
	unzip -L -a -o $< $@
	patch -p0 < $@.patch

$(DOSLFN) : .EXISTSONLY
	wget -O $@ $(LINK)


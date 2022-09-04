# vi: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab
#
# VMCHCPD
#  A DOS device driver for managing code page changes for VMSMOUNT
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

CC = wcc
AS = wasm
LD = wlink
RM = rm -f
CFLAGS  = -I.. -bt=dos -ms -q -s -osh -DUSE_INTERNAL_STACK -DSTACK_SIZE=300
ASFLAGS = -bt=DOS -zq -mt -0
LDFLAGS = SYSTEM dos &
			ORDER clname HEADER clname DATA clname CODE clname BSS clname INIT &
			DISABLE 1014 OPTION QUIET, STATICS, MAP=vmchcpd.map

!ifdef DEBUG
CFLAGS += -DDEBUG
DBGOBJ = vmtool.obj printf.obj debug.obj
!endif

TARGET = vmchcpd.sys

OBJ = cstrtsys.obj vmfunc.obj devinit.obj $(DBGOBJ) vmchcpd.obj

all : $(TARGET)

clean : .SYMBOLIC
	$(RM) $(OBJ) $(TARGET) *.map *.err

distclean : clean .SYMBOLIC

$(TARGET) : $(OBJ)
	$(LD) $(LDFLAGS) NAME $(TARGET) FILE {$(OBJ)}

devinit.obj : devinit.c .AUTODEPEND
	$(CC) -0 $(CFLAGS) -nt=_INIT -nc=INIT -fo=$@ $<

vmfunc.obj : vmfunc.c .AUTODEPEND
	$(CC) -3 $(CFLAGS) -nt=_INIT -nc=INIT -fo=$@ $<

!ifdef DEBUG
vmtool.obj : ../vmtool.c .AUTODEPEND
	$(CC) -3 $(CFLAGS) -fo=$@ $<

printf.obj : ../printf.c .AUTODEPEND
	$(CC) -3 $(CFLAGS) -fo=$@ $<

debug.obj : ../debug.c .AUTODEPEND
	$(CC) -3 $(CFLAGS) -fo=$@ $<
!endif

.asm.obj : .AUTODEPEND
	$(AS) $(ASFLAGS) -fo=$@ $<

.c.obj : .AUTODEPEND
	$(CC) -3 $(CFLAGS) -fo=$@ $<

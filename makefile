# vi: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab
#
# VMSMOUNT
#  A network redirector for mounting VMware's Shared Folders in DOS 
#  Copyright (C) 2011-2022  Eduardo Casino
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
LD = wlink
UPX = upx
RM = rm -f
CFLAGS  = -bt=dos -ms -q -s -oh -os -DREVERSE_HASH
LDFLAGS =	SYSTEM dos &
			ORDER &
				clname FAR_DATA &
				clname RES_CODE &
				clname CODE segment BEGTEXT segment _TEXT &
				clname BEGDATA &
				clname DATA &
				clname BSS &
			OPTION QUIET, STATICS, MAP=vmsmount.map
UPXFLAGS = -9

TARGET = vmsmount.exe

!ifdef DEBUG
CFLAGS += -DDEBUG
DBGOBJ = printf.obj debug.obj
!endif

OBJ =	kitten.obj vmaux.obj main.obj $(DBGOBJ) miniclib.obj unicode.obj &
		vmdos.obj vmtool.obj vmshf.obj toolsd.obj redir.obj lfn.obj &
		endtext.obj

all : $(TARGET)

clean : .SYMBOLIC
	$(RM) $(OBJ) $(TARGET) *.map *.err

$(TARGET) : $(OBJ)
	$(LD) $(LDFLAGS) NAME $(TARGET) FILE {$(OBJ)} $(LIBPATH) $(LIBRARY)
	$(UPX) $(UPXFLAGS) $(TARGET)

# main.obj and kitten.obj must be compiled with 8086 instructions only to gracefully
#  execute the processor check in real, older machines
#
main.obj : main.c
	$(CC) -0 $(CFLAGS) -fo=$@ $<

kitten.obj : kitten.c
	$(CC) -0 $(CFLAGS) -fo=$@ $<

vmaux.obj : vmaux.c
	$(CC) -3 $(CFLAGS) -fo=$@ $<

endtext.obj : endtext.c
	$(CC) -3 $(CFLAGS) -nt=_TEXT_END -fo=$@ $<

kitten.obj: .AUTODEPEND

main.obj : .AUTODEPEND

vmaux.obj : .AUTODEPEND

endtext.obj : .AUTODEPEND

redir.obj : .AUTODEPEND

toolsd.obj: .AUTODEPEND

vmtool.obj : .AUTODEPEND

vmshf.obj : .AUTODEPEND

vmdos.obj : .AUTODEPEND

lfn.obj : .AUTODEPEND

!ifdef DEBUG

debug.obj: .AUTODEPEND

printf.obj: .AUTODEPEND

!endif

.c.obj :
	$(CC) -3 $(CFLAGS) -g=RES_GROUP -nt=RES_TEXT -nc=RES_CODE -nd=RES -fo=$@ $<

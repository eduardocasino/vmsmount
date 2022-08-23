#
# VMSMOUNT
#  A network redirector for mounting VMware's Shared Folders in DOS 
#  Copyright (C) 2011  Eduardo Casino
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
# 2011-10-01  Eduardo           Add UPX compression
# 2011-10-05  Tom Ehlert        Use Pentium optims for smaller code
# 2011-10-14  Eduardo           Back to -3 optims as they generate the same
#                               code. Use -0 for main.c and kitten.c to allow
#                               execution of processor test.
#                               Reorder segments to mark end of transient part
# 2011-11-01  Eduardo           Add LFN (new object and segment order)
# 2022-08-23  Eduardo           Make integer functions inlines (remove vmint.asm)
# 2022-08-23  Eduardo           Port to OW 2.0
# 2022-08-23  Eduardo           Port to wmake syntax
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
				OPTION QUIET &
				OPTION STATICS &
				OPTION MAP=vmsmount.map
UPXFLAGS = -9

TARGET = vmsmount.exe

OBJ =	kitten.obj vmaux.obj main.obj miniclib.obj unicode.obj vmdos.obj &
		vmcall.obj vmtool.obj vmshf.obj redir.obj lfn.obj &
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
main.obj : main.c .AUTODEPEND
	$(CC) -0 $(CFLAGS) -fo=$@ $<

kitten.obj : kitten.c .AUTODEPEND
	$(CC) -0 $(CFLAGS) -fo=$@ $<

vmaux.obj : vmaux.c .AUTODEPEND
	$(CC) -3 $(CFLAGS) -fo=$@ $<

endtext.obj : endtext.c .AUTODEPEND
	$(CC) -3 $(CFLAGS) -nt=_TEXT_END -fo=$@ $<

kitten.obj: .AUTODEPEND

main.obj : .AUTODEPEND

vmcall.obj : .AUTODEPEND 

vmaux.obj : .AUTODEPEND

redir.obj : .AUTODEPEND

vmtool.obj : .AUTODEPEND

vmshf.obj : .AUTODEPEND

vmdos.obj : .AUTODEPEND

lfn.obj : .AUTODEPEND

.c.obj :
	$(CC) -3 $(CFLAGS) -g=RES_GROUP -nt=RES_TEXT -nc=RES_CODE -nd=RES -fo=$@ $<

	

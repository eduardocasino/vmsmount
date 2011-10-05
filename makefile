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
#

CC = wcc
AS = nasm
LD = wlink
UPX = upx
RM = rm -f
CFLAGS  = -5 -bt=dos -ms -q -s -oh -os
ASFLAGS = -f obj -Worphan-labels -O9
LDFLAGS = SYSTEM dos OPTION QUIET
UPXFLAGS = -9

TARGET = vmsmount.exe

OBJ = kitten.obj vmaux.obj main.obj miniclib.obj vmint.obj unicode.obj vmdos.obj vmcall.obj vmtool.obj vmshf.obj redir.obj


all: $(TARGET)

clean:
	$(RM) $(OBJ) $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) NAME $(TARGET) FILE {$(OBJ)} $(LIBPATH) $(LIBRARY)
	$(UPX) $(UPXFLAGS) $(TARGET)

kitten.obj:: kitten.h

main.obj:: globals.h kitten.h messages.h vmaux.h vmshf.h vmtool.h dosdefs.h redir.h unicode.h

vmaux.obj:: vmcall.h vmtool.h globals.h messages.h vmshf.h kitten.h

redir.obj:: globals.h redir.h dosdefs.h vmshf.h vmtool.h vmcall.h vmdos.h vmint.h miniclib.h

vmtool:: vmtool.h vmcall.h

vmshf:: vmtool.h vmshf.h vmcall.h vmint.h vmdos.h redir.h miniclib.h

vmdos:: vmint.h dosdefs.h vmdos.h vmint.h vmshf.h vmtool.h vmcall.h miniclib.h unicode.h


%.obj : %.c
	$(CC) $(CFLAGS) -fo=$@ $<

%.obj : %.asm
	$(AS) $(ASFLAGS) -o $@ $<
	

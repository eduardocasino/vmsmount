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

MKTABLE = tools/mk_table
MAKE = wmake
RM = rm -f

TABLES = &
		c1258uni.tbl cp437uni.tbl cp720uni.tbl cp737uni.tbl cp775uni.tbl cp850uni.tbl &
		cp852uni.tbl cp855uni.tbl cp857uni.tbl cp858uni.tbl cp860uni.tbl cp861uni.tbl &
		cp862uni.tbl cp863uni.tbl cp864uni.tbl cp865uni.tbl cp866uni.tbl cp869uni.tbl &
		cp874uni.tbl

all : $(MKTABLE) $(TABLES)

$(MKTABLE) : .SYMBOLIC
	(cd tools; $(MAKE) all)

distclean: .SYMBOLIC clean
	(cd tools; $(MAKE) clean)

clean : .SYMBOLIC
	$(RM) *.tbl
	$(RM) -r build

c1258uni.tbl : cp1258.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp437uni.tbl : cp437.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp720uni.tbl : cp720.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp737uni.tbl : cp737.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp775uni.tbl : cp775.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp850uni.tbl : cp850.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp852uni.tbl : cp852.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp855uni.tbl : cp855.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp857uni.tbl : cp857.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp858uni.tbl : cp858.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp860uni.tbl : cp860.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp861uni.tbl : cp861.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp862uni.tbl : cp862.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp863uni.tbl : cp863.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp864uni.tbl : cp864.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp865uni.tbl : cp865.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp866uni.tbl : cp866.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp869uni.tbl : cp869.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<

cp874uni.tbl : cp874.txt
	@[ -d build ] || mkdir build
	@cp $< build
	@-[ -f $<.patch ] && (cd build; patch -p0 $< < ../$<.patch)
	$(MKTABLE) build/$<


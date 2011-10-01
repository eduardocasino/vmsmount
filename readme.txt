VMSMOUNT - A DOS redirector for mounting VMware's Shared Folders
(C) 2011 Eduardo Casino <eduardo.casino@gmail.com>

WARNING AND DISCLAIMER

 This should be considered a beta version and, as such, may contain
 bugs that could cause data loss. THIS PROGRAM IS PROVIDED "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED
 TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE.  THE ENTIRE RISK ASTO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS
 WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL
 NECESSARY SERVICING, REPAIR OR CORRECTION.
 
USAGE

 VMSMOUNT [/H][/V] [/L:<drive letter>]
    /H                 - Prints help and exits
	/V                 - Verbose: Prints information on system resources
	/L:<drive letter>  - Drive letter to assign
                         (if omitted, use the first available)

ENVIRONMENT

    TZ      - Valid POSIX timezone. If omitted, file times will be in UTC
              (see http://www.gnu.org/s/hello/manual/libc/TZ-Variable.html)
	          Example: TZ=CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00
	LANG    - Used by the Kitten library to show messages in the correct
	          language. Currently only available in English and Spanish.
	NLSPATH - Used by the Kitten library to find the message catalogs.
	PATH    - VMSMOUNT searchs in the PATH for the unicode conversion
	          tables.
	
	VMSMOUNT gets the current NLS settings from the kernel and translates
	VMware's UTF encoding to the correct code page, provided that the
	necessary conversion table is found.  It uses the same table format
	as Volkov Commander and DOSLFN. Please refer to TBL.TXT in the DOSLFN
	source package for details(http://adoxa.110mb.com/doslfn/index.html)
	For convenience, I'm distributing with this package the translation
	tables, which can be generated from the ASCII code tables provided at
	www.unicode.orgw using the MK_TABLE program from DOSLFN
	
LIMITATIONS

 * Does not work with DOS < 5 (Tested with latest FreeDOS kernel,
   MS-DOS 6.22 and MS-DOS 7 (win95)
 * Does not support old versions of VMWare Workstation (Tested with
   VMWare Player 3) 
 * Shared folder names MUST be uppercase.
 * Does not support long names (long names and/or with illegal characters 
   are ignored)
 * Does not detect code page changes
 * Can't be uninstalled

ACKNOWLEDGEMENTS

 * "Undocumented DOS 2nd ed." by Andrew Schulman et al.
   Enlightening, but with some inaccuracies that have driven me mad
   
 * Ken Kato's VMBack info and Command Line Tools
                    (http://sites.google.com/site/chitchatvmback/)
   The HGFS code is fully based on his excellent work
   
 * VMware's Open Virtual Machine Tools
                    (http://open-vm-tools.sourceforge.net/)
   Info on the HGFS V3 protocol
   
 * Tom Tilli's <aitotat@gmail.com> TSR example in Watcom C for the 
                    Vintage Computer Forum (www.vintage-computer.com)
   The idea and some code for a lightweight TSR written in C
 
 * Jason Hood/Henrik Haftmann for the unicode translation tables in
   DOSLFN. Jason again for SHSUCDX, it's source code has been very
   helpful.
 
 * And, of course, Pat Villani for the FreeDOS kernel.
   

BUILD

  VMSMOUNT was built with OpenWatcom 1.9, NASM 2.09.10 and GNU Make 3.81
  
BUGS

 * When used with DOSLFN in FreeDOS, "." and ".." appear corrupted
   in the long name listing with DIR /LFN. I don't know if it is a
   FreeDOS kernel, FreeCOM, DOSLFN or VMSMOUNT issue. It does not
   happen with MS-DOS 7.

TODO
 
 * Uninstallation
 * Some optimizations in the read and write functions
 * Code Page change detection (maybe)
 * Long file names (don't even know if I'll try)
 
LICENSE

  VMSMOUNT is GPLd software. See LICENSE.TXT for the details.

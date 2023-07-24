##############################################################################
##
##       Copyright (c) 2007-2018 Frank Eskesen.
##
##       This file is free content, distributed under creative commons CC0,
##       explicitly released into the Public Domain.
##       (See accompanying html file LICENSE.ZERO or the original contained
##       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
##
##############################################################################
##
## Title-
##       Makefile
##
## Purpose-
##       Makefile for C++/Assembler programs
##
## Preparation-
##       The CYGWIN DLL and binaries must be included in %PATH%.
##
##       The Makefile  in your object directory must set default paths:
##         INCDIR      Include directory
##         LIBDIR      Library directory
##         MAKDIR      Makefile directory
##         OBJDIR      Object directory
##         SRCDIR      Source directory
##       Then include this Makefile.
##
## Commands-
##       make depend   Create an updated dependency file
##       make clean    Remove garbage files and executables.
##       make pristine "make clean" plus remove listing and object files.
##
## Reserved file names-
##       *WARNING* These files are subject to automatic deletion.
##
##       *.exe         Executable files.
##       *.lst         Listing files.
##       *.obj         Object files.
##       *.map         Linker files.
##       core          System core dump.
##       debug.out     Debugging output.
##       errs          Error output from certain script files.
##       Makeproj.inp  Makeproj input file.
##       Makeproj.out  Makeproj output file.
##       Makeproj.dep  Makeproj dependency file.
##
## Microsoft compiler control defaults-
##       /nologo Suppress logo
##       /EHsc   Enable C++ exception handling
##       /GR     Enable polymorphic runtime operators (dynamic_cast)
##       /Gs     Disable stack checking
##       /I<dir> Add include directory
##       /MD     MultiThreaded DLL
##       /MT     MultiThreaded library
##       /O2     Optimize(Speed)
##       /Ob2    Inline(AnySuitable)
##       /W1     Warnings Level(1) (Simple warnings)
##       /WX     Warnings as errors
##       /Zc:inline
##               Remove unused inline
##
## Microsoft compiler control options-
##       /FAsc   Generate source/assembler listing (Set if LISTING != "")
##       /Fa     Generate listing (Set if LISTING != "")
##       /Fd     Specify .PDB file
##       /Fo     Specifies output directory
##       /Fp     Specifies precompiled file header
##       /fp     Specifies floating point model
##       /G3     Target 80386
##       /GL     Enable link-time code generatation
##       /GM     Enable minimal rebuild
##       /GS     Enable security checks
##       /GX     Enable exception handling (/EHsc used instead)
##       /Gy     Separate functions for linker
##       /Oi     Enable intrinsic functions
##       /sdl    Enable additional security features and warnings
##       /W3     Warnings Level(3)
##       /Zc:<option>
##               Language conformance options
##       /Ze     Microsoft Extentions
##       /Zi     Full debug info
##       /Zp1    1 byte struct alignment (Conflicts with windows library)
##
## Microsoft linker defaults-
##       /nologo Suprress logo message
##       /subsystem:console
##               WIN character mode application
##       /incremental:no
##               Suppress incremental build
##       /opt:ref
##               Remove duplicate references
##
## Microsoft linker options-
##       /debug  Create debugging information
##       /info   Generate detailed information
##       /LTCG   Link-time code generation
##       /MACHINE  {ARM | EBC | X64 | X86}
##               Specifies the target platform (Fixes LNK1113 error.)
##       /map    Generate map
##       /nodefaultlib
##               No default libraries
##       /nodefaultlib:lib
##               Exclude default library "lib"
##       /PDB    Create Program DataBase file
##       /subsystem:native
##               Device driver application
##       /subsystem:posix
##               POSIX application
##       /subsystem:windows
##               Application which does not require a console
##
##############################################################################

##############################################################################
## Prefix local options
CDEFS  += $(LDEFS)################# C defines
CINCL  += $(LINCL)################# C includes
COPTS  += $(LOPTS)################# C options
CLIBS  += $(LLIBS)################# Link libraries
CLOAD  += $(LLOAD)################# Link options

##############################################################################
## Include compiler option control file
include $(MAKDIR)/Makefile.MSC##### Compiler control file

##############################################################################
## Include standard option control file
INCSRC :=Makefile.OPT############## Default Control file
ifeq "$(findstring $(OBJDIR)/$(INCSRC),$(wildcard $(OBJDIR)/Makefile.*))" "$(OBJDIR)/$(INCSRC)"
INCSRC := $(OBJDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

##############################################################################
## Default rules
vpath

.SUFFIXES:
.SUFFIXES: .exe .obj
Make% : ;
make% : ;

%.exe : %.obj
	$(LD) /out:"$@" $(CLOAD) $< $(CLIBS)

%.obj : $(SRCDIR)/%.c
	$(CC) /Fo"$(OBJDIR)/$@" $(CFLAGS) /c $<

%.obj : $(SRCDIR)/%.cpp
	$(CC) /Fo"$(OBJDIR)/$@" $(CFLAGS) /c $<

%.obj : $(SRCDIR)/%.s
	m4 $< | as -o $@

##############################################################################
## Default source and object files
MAKSRC := $(wildcard $(SRCDIR)/*.cpp)
MAKEXE := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.exe,$(MAKSRC))
MAKOBJ := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.obj,$(MAKSRC))

##############################################################################
## Include implementation file
INCSRC :=Makefile.WIN############## Default Control file
ifeq "$(findstring $(SRCDIR)/$(INCSRC),$(wildcard $(SRCDIR)/Makefile.*))" "$(SRCDIR)/$(INCSRC)"
INCSRC := $(SRCDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

##############################################################################
## TARGET: nothing (Default when no default.)
.PHONY: nothing
nothing: ;
	-@echo "No default target specified"

##############################################################################
## TARGET: CLEAN, PRISTINE
.PHONY: pristine clean
pristine: clean
	-@rm *.lst *.obj                   >/dev/null 2>/dev/null
	-@rm Makeproj.inp Makeproj.out     >/dev/null 2>/dev/null
	-@rm Makeproj.dep                  >/dev/null 2>/dev/null

clean: ;
	-@rm $(MAKEXE)                     >/dev/null 2>/dev/null
	-@rm *.exe *.map                   >/dev/null 2>/dev/null
	-@rm core debug.out errs           >/dev/null 2>/dev/null

##############################################################################
## Include the dependency file
INCSRC :=Makefile.DEP################# Default Control file
ifeq "$(findstring $(SRCDIR)/$(INCSRC),$(wildcard $(SRCDIR)/Makefile.*))" "$(SRCDIR)/$(INCSRC)"
INCSRC := $(SRCDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)


##############################################################################
##
##       Copyright (C) 2015-2024 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
##
## Title-
##       ~/src/mcs/ctl/WIN/Makefile
##
## Purpose-
##       Makefile for C# programs
##
## Last change date-
##       2024/03/01
##
## Preparation-
##       The CYGWIN DLL and binaries must be included in %PATH%.
##
##       The Makefile  in your object directory must set default paths:
##         LIBDIR      Library directory
##         MAKDIR      Makefile directory
##         SRCDIR      Source directory
##       Then include this Makefile.
##
## Commands-
##       make clean    Remove garbage files and executables.
##       make pristine "make clean" plus remove listing and object files.
##
## Environment controls-
##       OPTIMIZE      Override optimization controls
##
## Reserved file names-
##       *WARNING* These files are subject to automatic deletion.
##
##       *.dll         Libraries.
##       *.exe         Executables.
##       *.obj         Intermediate executables.
##       *.mdb         (Compiler generated)
##       *.pdb         (Compiler generated)
##       *.stackdump   System stack dump.
##       core          System core dump.
##       debug.out     Debugging output.
##       errs          Error output from certain script files.
##       __temp.*      Intermediate executables.
##
## ILMerge.exe-
##       When ILMerge.exe is available in the working path, by default
##       library file references are merged into the executable.
##       This allows the executable file to run in standalone mode.
##       That is, the DLLs used to compile the program are included
##       within it. They are not needed at runtime.
##
## Obtaining ILMerge.exe-
##       See: https://www.nuget.org/packages/ilmerge
##       1) In visual studio open a project.
##       2) Click Tools, Library Package Manager, Open Package Manager Console
##       3) In this console, Install-Package ilmerge
##       You will then find the excutable in:
##         <Documents>\Visual Studio 2013\Projects\<project-name>\packages\
##         ilmerge.2.14.1208\tools\ILMerge.exe
##         (Your Visual Studio and ilmerge version numbers may vary)
##       4) Copy that executable into your executable path.
##
##############################################################################

##############################################################################
## Set default paths
OBJDIR = .######################### Object directory

MAKSRC := $(wildcard $(SRCDIR)/*.cs)
MAKEXE := $(patsubst $(SRCDIR)/%.cs,$(OBJDIR)/%.exe,$(MAKSRC))
MAKOBJ := $(patsubst $(SRCDIR)/%.cs,$(OBJDIR)/%.obj,$(MAKSRC))

MAKMAK := OBJ2EXE
ifeq "" "$(shell whence $(LD))"
MAKMAK := SRC2EXE
endif

.SUFFIXES:
vpath

Make%: ;
make%: ;

##############################################################################
## Include compiler option control file
include $(MAKDIR)/Makefile.MCS##### Compiler control file

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
## Include implementation file
CREFS  := $(patsubst %,/reference:"%",$(shell cygpath -w -i $(CLIBS)))

INCSRC :=Makefile.WIN############## Default Control file
ifeq "$(findstring $(SRCDIR)/$(INCSRC),$(wildcard $(SRCDIR)/Makefile.*))" "$(SRCDIR)/$(INCSRC)"
INCSRC := $(SRCDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

UREFS  := $(patsubst %,/reference:"%",$(shell cygpath -w -i $(ULIBS)))

##############################################################################
## Ruleset: EXEONLY (Omits $(CLIB) files from the excutable and directory)
ifeq "EXEONLY" "$(MAKMAK)"
%.exe: $(SRCDIR)/%.cs
	$(CC) /out:$@ $(CFLAGS) "$(shell cygpath -w $<)" $(CREFS) $(UREFS)
endif

##############################################################################
## Ruleset: OBJ2EXE (Merges $(CLIB) references into executable)
ifeq "OBJ2EXE" "$(MAKMAK)"
%.exe: %.obj
	$(LD) /out:__temp.exe /target:exe $< $(patsubst %,"%",$(shell cygpath -w -i $(CLIBS) $(ULIBS)))
	mv -f __temp.exe $@
	rm __temp.pdb

%.obj: $(SRCDIR)/%.cs
	$(CC) /out:$@ $(CFLAGS) "$(shell cygpath -w $<)" $(CREFS) $(UREFS)
endif

##############################################################################
## Ruleset: SRC2EXE (Copies $(CLIB) files into object directory)
ifeq "SRC2EXE" "$(MAKMAK)"
%.dll: $(LIBDIR)/%.dll
	cp -f $< $@

$(MAKEXE): $(patsubst $(LIBDIR)/%.dll,$(OBJDIR)/%.dll,$(wildcard $(LIBDIR)/*.dll))
%.exe: $(SRCDIR)/%.cs
	$(CC) /out:$@ $(CFLAGS) "$(shell cygpath -w $<)" $(CREFS) $(UREFS)
endif

##############################################################################
## TARGET: nothing (Default when no default.)
.PHONY: nothing
nothing: ;
	-@echo "No default target specified"

##############################################################################
## TARGET: pristine, clean
.PHONY: pristine clean
pristine: clean
	@rm -f *.stackdump

clean: ;
	@rm -f *.exe *.dll *.obj *.mdb *.pdb
	@rm -f core debug.out errs

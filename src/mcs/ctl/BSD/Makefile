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
##       ~/src/mcs/ctl/BSD/Makefile
##
## Purpose-
##       Makefile for C# programs
##
## Last change date-
##       2024/03/01
##
## Preparation-
##       In your object directory,
##         ln -s <Library_directory> L
##         ln -s <This_directory>    M
##         ln -s <Source_directory>  S
##         ln -s M/Makefile Makefile
##
## Commands-
##       make clean    Remove garbage files and executables.
##       make pristine "make clean" plus remove listing and object files.
##
## Environment controls-
##       OPTIMIZE      Override optimization controls
##
## Installation-
##       Install Microsoft Visual Studio [SDK not required]
##       Update ~/bat/mcs (Visual Studio C# locator)
##
## Reserved file names-
##       *WARNING* These files are subject to automatic deletion.
##
##       *.dll         -reference: Libraries.
##       *.exe         (mono) executables.
##       *.mdb         (mcs compiler generated debug database.)
##       *.net         -addmodule: Libraries.
##       *.pdb         (csc compiler generated debug database.)
##       *.stackdump   System stack dump.
##       core          System core dump.
##       debug.log     Debugging output.
##       debug.out     Debugging output.
##       errs          Sometimes used, e.g. make 2>errs
##
## Make variables-
##       CLIBS         List of library DLLs
##       LLIBS         List of local library DLLs
##       LNETS         List of local library NETs (netmodule)
##
## Generated make variables-
##       CREFS         List of -reference:CLIBS.dll (generated)
##       LREFS         List of -reference:LLIBS.dll (generated)
##       LMODS         List of -addmodule:LNETS.net (generated)
##
##############################################################################

##############################################################################
## Set defaults
LIBDIR = L######################### Library directory
MAKDIR = M######################### Makefile directory
OBJDIR = .######################### Object directory
SRCDIR = S######################### Source directory

MAKSRC := $(wildcard $(SRCDIR)/*.cs)
MAKEXE := $(patsubst $(SRCDIR)/%.cs,$(OBJDIR)/%.exe,$(MAKSRC))

##############################################################################
## Makefile.MCS: Include compiler option control file
include $(MAKDIR)/Makefile.MCS##### Compiler control file

##############################################################################
## Makefile.BSD: Implementation file
INCSRC :=Makefile.BSD############## Default Control file
ifeq "$(findstring $(SRCDIR)/$(INCSRC),$(wildcard $(SRCDIR)/Makefile.*))" "$(SRCDIR)/$(INCSRC)"
INCSRC := $(SRCDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

##############################################################################
## Makefile.OPT: Standard option control file
INCSRC :=Makefile.OPT############## Default Control file
ifeq "$(findstring $(OBJDIR)/$(INCSRC),$(wildcard $(OBJDIR)/Makefile.*))" "$(OBJDIR)/$(INCSRC)"
INCSRC := $(OBJDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

##############################################################################
## Default rules
.SUFFIXES:
vpath

Make%: ;
make%: ;

%.exe: $(SRCDIR)/%.cs
	$(CC) -target:exe -out:$@ $(CFLAGS) $(XFLAGS) "$(shell cygpath -w $<)"

CREFS  += $(patsubst %,-reference:"%.dll",$(CLIBS))
LREFS  += $(patsubst %,-reference:"%.dll",$(LLIBS))
LMODS  += $(patsubst %,-addmodule:"%.net",$(LNETS))
XFLAGS := $(LREFS) $(LMODS) $(CREFS)

##############################################################################
## Default dependencies
ifeq "Linux" "$(findstring Linux,$(shell uname))"
$(OBJDIR)/%.dll: L/%.dll ############# Keep -reference lib files up to date
	ln -sf $< $@

$(OBJDIR)/%.net: L/%.net ############# Keep -addmodule lib files up to data
	ln -sf $< $@

else ################################# (If copy required)
$(OBJDIR)/%.dll: L/%.dll ############# Keep -reference lib files up to date
	cp -Lpf $< $@
	chmod a=rx,u+w $@

$(OBJDIR)/%.net: L/%.net ############# Keep -addmodule lib files up to data
	cp -Lpf $< $@
	chmod a=rx,u+w $@
endif

ifneq "" "$(LLIBS)$(LNETS)$(CLIBS)" ## Keep executables up to date
$(MAKEXE): $(patsubst %,%.dll,$(CLIBS) $(LLIBS)) $(patsubst %,%.net,$(LNETS))
endif

##############################################################################
## TARGET: nothing (Default when no default.)
.PHONY: nothing
nothing: ;
	-@echo "No default target specified"

##############################################################################
## TARGET: debug
.PHONY: debug
debug: ;
	-@echo 'MAKSRC: "$(MAKSRC)"'
	-@echo 'MAKEXE: "$(MAKEXE)"'
	-@echo ' CDEFS: "$(CDEFS)"'
	-@echo ' CLIBS: "$(CLIBS)"'
	-@echo ' COPTS: "$(COPTS)"'
	-@echo ' CREFS: "$(CREFS)"'
	-@echo ' LLIBS: "$(LLIBS)"'
	-@echo ' LREFS: "$(LREFS)"'
	-@echo ' LMODS: "$(LMODS)"'
	-@echo ' LNETS: "$(LNETS)"'
	-@echo 'CFLAGS: "$(CFLAGS)"'

##############################################################################
## TARGET: pristine, clean
.PHONY: pristine clean
pristine: clean
	@rm -f *.stackdump

clean: ;
	@rm -f *.exe *.dll *.net *.mdb *.pdb
	@rm -f core debug.out debug.log errs

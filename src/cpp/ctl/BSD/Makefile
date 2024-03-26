##############################################################################
##
##       Copyright (c) 2007-2024 Frank Eskesen.
##
##       This file is free content, distributed under creative commons CC0,
##       explicitly released into the Public Domain.
##       (See accompanying html file LICENSE.ZERO or the original contained
##       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
##
##############################################################################
##
## Title-
##       ~/src/cpp/ctl/BSD/Makefile
##
## Purpose-
##       Makefile for C++/Assembler programs
##
## Last change date-
##       2024/03/01
##
## Preparation-
##       In your object directory,
##         ln -s <Source_directory>  S
##         ln -s <Include_directory> H
##         ln -s <Library_directory> L
##         ln -s <This_directory>    M
##         ln -s M/Makefile Makefile
##
## Commands-
##       make depend   Create an updated dependency file
##       make clean    Remove garbage files and executables.
##       make pristine "make clean" plus remove listing and object files.
##
## Environment controls-
##       COMPILER      Override default (gcc) compiler
##       LINKMAP       Non-null to create link map
##       LISTING       Non-null to create listing files
##       OPTIMIZE      Override optimization controls
##
## Reserved file names-
##       *WARNING* These files are subject to automatic deletion.
##
##       *.exe         Executable files.
##       *.lst         Listing files.
##       *.o           Object files.
##       *.map         Linker files.
##       *.stackdump   System stack dump
##       core          System core dump.
##       debug.*       Debugging output.
##       errs          Error output from certain script files.
##       Makeproj.inp  Makeproj input file.
##       Makeproj.out  Makeproj output file.
##       Makeproj.dep  Makeproj dependency file.
##
##############################################################################

##############################################################################
## Remove builtin rules
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

vpath

.SUFFIXES:
.SUFFIXES: .o
## Make% : ;
## make% : ;

##############################################################################
## Set default paths
INCDIR = H######################### Include directory
LIBDIR = L######################### (Static) library directory
MAKDIR = M######################### Makefile directory
OBJDIR = .######################### Object directory
SHRDIR = D######################### (Shared) library directory
SRCDIR = S######################### Source directory

##############################################################################
## Include compiler option control file
ifeq "" "$(COMPILER)"
COMPILER := GCC
endif

INCSRC :=Makefile.$(COMPILER)###### Compiler Control file
ifeq "$(findstring $(OBJDIR)/$(INCSRC),$(wildcard $(OBJDIR)/Makefile.*))" "$(OBJDIR)/$(INCSRC)"
INCSRC := $(OBJDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

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
## Default patterns
% : %.o
	$(LD) -o $@ $< $(CLOAD) $(LLIBS) $(CLIBS)

%.o : $(SRCDIR)/%.cpp
	$(CC) -o $@ -c $<  $(CFLAGS)

%.o : $(SRCDIR)/%.C
	$(CC) -o $@ -c $< $(CFLAGS)

%.o : $(SRCDIR)/%.c
	$(CC) -o $@ -c $<  $(CFLAGS)

%.o : $(SRCDIR)/%.s
	m4 $< | as -o $@ $(AFLAGS)

##############################################################################
## Default source, executable and object files
MAKSRC := $(wildcard $(SRCDIR)/*.cpp)
MAKEXE := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%,$(MAKSRC))
MAKOBJ := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(MAKSRC))

##############################################################################
## Include implementation file
INCSRC :=Makefile.BSD############## Default Control file
ifeq "$(findstring $(OBJDIR)/$(INCSRC),$(wildcard $(OBJDIR)/Makefile.*))" "$(OBJDIR)/$(INCSRC)"
INCSRC := $(OBJDIR)/$(INCSRC)
else
ifeq "$(findstring $(SRCDIR)/$(INCSRC),$(wildcard $(SRCDIR)/Makefile.*))" "$(SRCDIR)/$(INCSRC)"
INCSRC := $(SRCDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
endif
include $(INCSRC)

##############################################################################
## TARGET: nothing (Default when no default.)
.PHONY: nothing
nothing: ;
	-@echo "No default target specified"

##############################################################################
## TARGET: pristine, clean
.PHONY: pristine clean
pristine: clean
	@rm -f *.lst *.o *.stackdump
	@rm -f Makeproj.*

clean: ;
	@rm -f $(MAKEXE)
	@rm -f *.exe *.map
	@rm -f core debug.* errs

##############################################################################
## Include the dependency file
INCSRC :=Makefile.DEP################# Default Control file
ifeq "$(findstring $(SRCDIR)/$(INCSRC),$(wildcard $(SRCDIR)/Makefile.*))" "$(SRCDIR)/$(INCSRC)"
INCSRC := $(SRCDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

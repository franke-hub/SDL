##############################################################################
##
##       Copyright (c) 2007-2021 Frank Eskesen.
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
## Last change date-
##       2021/09/06
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
## Set default paths
INCDIR = H######################### Include directory
LIBDIR = L######################### Library directory
MAKDIR = M######################### Makefile directory
OBJDIR = .######################### Object directory
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
## Default rules
vpath

.SUFFIXES:
.SUFFIXES: .o
## Make% : ;
## make% : ;

% : %.o
	$(LD) -o $@ $< $(CLOAD) $(CLIBS)

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
	-@rm *.lst *.o *.stackdump         >/dev/null 2>/dev/null
	-@rm Makeproj.dep                  >/dev/null 2>/dev/null
	-@rm Makeproj.inp Makeproj.out     >/dev/null 2>/dev/null

clean: ;
	-@rm $(MAKEXE)                     >/dev/null 2>/dev/null
	-@rm *.exe *.map                   >/dev/null 2>/dev/null
	-@rm core debug.* errs             >/dev/null 2>/dev/null

##############################################################################
## Include the dependency file
INCSRC :=Makefile.DEP################# Default Control file
ifeq "$(findstring $(SRCDIR)/$(INCSRC),$(wildcard $(SRCDIR)/Makefile.*))" "$(SRCDIR)/$(INCSRC)"
INCSRC := $(SRCDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)


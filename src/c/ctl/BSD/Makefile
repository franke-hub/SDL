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
##       Makefile
##
## Purpose-
##       Makefile for C/Assembler programs
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
## Reserved file names-
##       *WARNING* These files are subject to automatic deletion.
##
##       *.exe         Executable files.
##       *.lst         Listing files.
##       *.o           Object files.
##       *.map         Linker files.
##       core          System core dump.
##       debug.out     Debugging output.
##       errs          Error output from certain script files.
##       Makeproj.inp  Makeproj input file.
##       Makeproj.out  Makeproj output file.
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
INCSRC :=Makefile.GCC############## Default Control file
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
	$(CC) -o $@ $(CLOAD) $< $(CLIBS)

%.o : $(SRCDIR)/%.c
	$(CC) -o $@ $(CFLAGS) -c $<

%.o : $(SRCDIR)/%.s
	m4 $< | as -o $@ $(AFLAGS)

##############################################################################
## Default source, executable and object files
MAKSRC := $(wildcard $(SRCDIR)/*.c)
MAKEXE := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%,$(MAKSRC))
MAKOBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(MAKSRC))

##############################################################################
## Include implementation file
INCSRC :=Makefile.BSD############## Default Control file
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
	@rm -f *.lst *.o
	@rm -f Makeproj.inp Makeproj.out

clean: ;
	@rm -f $(MAKEXE)
	@rm -f *.exe *.map
	@rm -f core debug.out errs

##############################################################################
## Include the dependency file
INCSRC :=Makefile.DEP################# Default Control file
ifeq "$(findstring $(SRCDIR)/$(INCSRC),$(wildcard $(SRCDIR)/Makefile.*))" "$(SRCDIR)/$(INCSRC)"
INCSRC := $(SRCDIR)/$(INCSRC)
else
INCSRC := $(MAKDIR)/$(INCSRC)
endif
include $(INCSRC)

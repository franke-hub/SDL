##############################################################################
##
##       Copyright (c) 2019-2023 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
##
## Title-
##       Makefile
##
## Purpose-
##       Makefile for Python programs
##
## Last change date-
##       2023/08/13
##
## Preparation-
##       In your object directory,
##         ln -s <Source_directory>  S
##         ln -s <This_directory>    M
##         ln -s M/Makefile Makefile
##         ln -s S/Main.py _Main (Usually)
##
## Commands-
##       make clean    Remove standard garbage
##       make pristine "make clean" plus remove object files.
##
## Reserved file names-
##       *WARNING* These files are subject to automatic deletion.
##       debug.*       Debugging output.
##       *.stackdump   System dump.
##
##############################################################################

##############################################################################
## Remove *ALL* implicit rules and builtin patterns
MAKEFLAGS += --no-implicit-rules
.SUFFIXES:

## Builtin patterns ----------------------------------------------------------
%: %,v
%: RCS/%,v
%: RCS/%
%: s.%
%: SCCS/s.%

##############################################################################
## Set default paths
MAKDIR = M######################### Makefile directory
OBJDIR = .######################### Object directory
SRCDIR = S######################### Source directory

##############################################################################
## Include implementation file
INCSRC :=Makefile.STD############## Default Control file
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
## Standard cleanup file
.PHONY: clean pristine
pristine: clean
	-(cd M/; ./.Cleanup)

clean: ;
	-@rm -f debug.*
	-@rm -f *.stackdump

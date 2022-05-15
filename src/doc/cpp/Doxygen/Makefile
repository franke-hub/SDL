##############################################################################
##
##       Copyright (c) 2022 Frank Eskesen.
##
##       This file is free content, distributed under the "un-license,"
##       explicitly released into the Public Domain.
##       (See accompanying file LICENSE.UNLICENSE or the original
##       contained within http://unlicense.org)
##
##############################################################################
##
## Title-
##       Makefile
##
## Purpose-
##       Makefile for ~/obj/doc/cpp/Doxygen
##
## Last change date-
##       2022/05/15
##
## Commands-
##       make clean    Remove temporary files.
##       make pristine Remove all created files.
##
##############################################################################

##############################################################################
## Set default paths
OBJDIR = .######################### Object directory
SRCDIR = S######################### Source directory

##############################################################################
## Set default target
.PHONY: default
default: run-in-object-directory
	doxygen

##############################################################################
## TARGET: run-in-object-directory (Make sure we are in object subdirectory)
.PHONY: run-in-object-directory
run-in-object-directory: S ######## Link to source only in object subdirectory

##############################################################################
## TARGET: pristine, clean
.PHONY: pristine clean
pristine: clean
	-@rm -Rf html/*                    >/dev/null 2>/dev/null
	-@rm -Rf latex/*                   >/dev/null 2>/dev/null

clean: run-in-object-directory
	-@rm -Rf html/*                    >/dev/null 2>/dev/null
	-@rm -Rf latex/*                   >/dev/null 2>/dev/null

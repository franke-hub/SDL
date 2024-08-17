##############################################################################
##
##       Copyright (c) 2024 Frank Eskesen.
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
##       Makefile for ~/obj/doc/cpp/Doxygen
##
## Last change date-
##       2024/08/15
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
default: ;
	doxygen

##############################################################################
## TARGET: pristine, clean
.PHONY: pristine clean
pristine: clean
## 	-@rm -Rf html/*                    >/dev/null 2>/dev/null
## 	-@rm -Rf latex/*                   >/dev/null 2>/dev/null

clean: ;
	-@rm -Rf html/*                    >/dev/null 2>/dev/null
	-@rm -Rf latex/*                   >/dev/null 2>/dev/null

##############################################################################
##
##       Copyright (c) 2023 Frank Eskesen.
##
##       This file is free content, distributed under creative commons CC0,
##       explicitly released into the Public Domain.
##       (See accompanying html file LICENSE.ZERO or the original contained
##       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
##
##############################################################################
##
## Title-
##       Makefile.MAK
##
## Purpose-
##       Default target Makefile for Python programs
##
## Last change date-
##       2023/08/13
##
##############################################################################

##############################################################################
## Set default target
.PHONY: build
build: ;
	@_Make

##############################################################################
## Built-in targets
.PHONY: do
do: ;
	-_Main 2>debug.err
	@echo -e "\n\nSTDERR:"
	@cat debug.err

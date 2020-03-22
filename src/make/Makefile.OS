##############################################################################
##
##       Copyright (C) 2015 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
## Makefile
##############################################################################

##############################################################################
## Set paths
INCDIR = ../../../../data/src/cpp/inc
LIBDIR = ../lib
MAKDIR = ../../../../data/src/cpp/ctl/WIN
OBJDIR = .
SRCDIR = ../../../../data/src/cpp/Hello

##############################################################################
## Test osWINDOWS, etc
.PHONY: default

MYOS:="UNKNOWN"
ifneq "" "$(shell osWINDOWS)"
  MYOS:="WINDOWS"
else
  ifneq "" "$(findstring CYGWIN,$(shell uname))"
    MYOS:="CYGWIN"
  else
    MYOS:="BSD"
  endif
endif

default: ;
	@echo MYOS: $(MYOS)


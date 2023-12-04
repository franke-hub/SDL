#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2023 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       _exists.sh
##
## Purpose-
##       Makefile: does a file exist?
##
## Last change date-
##       2023/11/27
##
## Usage-
##       EXISTS := $(shell _exists.sh "filename") ## Returns "true" or "false"
##       :
##       ifeq "$(EXISTS)" "true"
##         :
##       endif
##
## Implementation notes-
##       ifeq "$(shell _exists.sh \"filename\")" "true" ## Syntax error!
##
##############################################################################
[[ -e "$1" ]] && { echo "true" ; exit 0 ; }
echo "false"
exit 0

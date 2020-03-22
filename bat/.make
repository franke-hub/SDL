#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2010-2019 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       .make
##
## Function-
##       Stop following reverse links, then make.
##
## Last change date-
##       2019/10/07
##
## Usage-
##       .make   (This script is probably unused now.)
##
##############################################################################
. setupJAVA                         ## Add java to the environment
set -x                              ## Trace on
set -P                              ## Stop following reverse links
cd $PWD                             ## Get into this directory
/usr/bin/make $*                    ## The real make


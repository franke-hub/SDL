#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2020 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       mod.f
##
## Purpose-
##       Chmod for files
##
## Last change date-
##       2020/01/14
##
##############################################################################
chmod u=rw,go=r "$@"

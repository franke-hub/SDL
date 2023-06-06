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
##       .timing.sh
##
## Function-
##       Run multiple TestDisp timing tests
##
## Last change date-
##       2023/06/09
##
##############################################################################

##############################################################################
## Run timing tests
set -x
time TestDisp --timing
time TestDisp --timing
time TestDisp --timing

#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2022-2026 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/license/mit)
##
## SPDX-License-Identifier: MIT
##----------------------------------------------------------------------------
##
## Title-
##       test_time.sh
##
## Function-
##       Run timing tests
##
## Last change date-
##       2026/01/20
##
##############################################################################

##############################################################################
## Function cmd: Run test, success expected
function cmd
{
  echo -e "\nTEST: $1 (started)"
  "$@"
  rc=$?
  if [ $rc == 0 ] ; then
    echo "PASS: $1"
    return
  fi

  echo "FAIL: $1"
  exit 1
}

##############################################################################
## Test started
echo "TEST: test_time.sh Timing tests (started)"

##############################################################################
## Run timing tests
cmd TimeDisp --verbose
cmd TimeDisp --verbose --items=4096 --tasks=32
cmd TimeDisp --verbose --size=2

cmd TestDisp --timing
cmd TestSock --runtime=30 --verbose --packet --stream --thread --worker
cmd TestSock --runtime=30 --verbose --stream --thread --worker --ssl

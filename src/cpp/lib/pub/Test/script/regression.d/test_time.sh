#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2022 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       test_time.sh
##
## Function-
##       Run timing tests
##
## Last change date-
##       2022/05/29
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
cmd TestDisp --timing
cmd TestSock --runtime=30 --verbose --packet --stream --thread --worker

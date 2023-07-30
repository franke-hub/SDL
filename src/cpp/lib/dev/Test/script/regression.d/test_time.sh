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
##       test_time.sh
##
## Function-
##       Run timing tests
##
## Last change date-
##       2023/07/29
##
##############################################################################

##############################################################################
## Function cmd: Run test, success expected
function cmd
{
  echo -e "\nTEST: $1 (started)"
  echo "$@"
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
## Run timing/performance tests
cmd T_Stream --runtime=30 --stress=1  --verbose
cmd T_Stream --runtime=30 --stress=16 --verbose
cmd T_Stream --runtime=30 --stress=1  --verbose --major
cmd T_Stream --runtime=30 --stress=16 --verbose --major

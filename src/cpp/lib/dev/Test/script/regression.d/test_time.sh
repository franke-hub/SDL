#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2023-2026 Frank Eskesen.
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
##       Run timing tests, logging to test_time.out
##
## Last change date-
##       2026/04/22
##
##############################################################################

##############################################################################
## Definitions
logfile="test_time.out"

##############################################################################
## Function log: Display command, run logging output
function log
{
  echo "$@"
  echo -e "\n$@" >>$logfile
  "$@" >>$logfile
  rc=$?
  if [[ $rc != 0 ]] ; then
    echo "FAIL: ./$1, rc $rc" >>$logfile
    echo "FAIL: ./$1, rc $rc"
    exit $rc
  fi

}

##############################################################################
## Test started
echo "TEST: test_time.sh (started)"
echo "`date` TEST: test_time.sh (started) on $HOST" >$logfile

##############################################################################
## Run informational test
log T_Stream --bringup

##############################################################################
## Run timing tests
log T_Stream --runtime=5  --stress=1  --verbose
log T_Stream --runtime=5  --stress=1  --verbose --major
log T_Stream --runtime=30 --stress=16 --verbose
log T_Stream --runtime=30 --stress=16 --verbose --major

##############################################################################
echo "PASS: test_time.sh" >>$logfile

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
##       2026/01/23
##
##############################################################################

##############################################################################
## Definitions
logfile="test_time.out"

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
## Function log: Display command, run logging output
function log
{
  echo "$@"
  echo -e "\n$@" >>$logfile
  "$@" >>$logfile
}

##############################################################################
## Test started
echo "TEST: test_time.sh (started)"
echo "`date` TEST: test_time.sh (started) on $HOST" >$logfile

##############################################################################
## Run timing tests
log TimeDisp --verbose
log TimeDisp --verbose --items=4096 --tasks=32
log TimeDisp --verbose --size=2 --retest ## Regression test

log TestDisp --timing
log TestSock --runtime=30 --verbose --packet --stream --thread --worker
log TestSock --runtime=30 --verbose --stream --thread --worker --ssl
echo "PASS: test_time.sh" >>$logfile

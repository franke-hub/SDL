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
##       test_misc.sh
##
## Function-
##       Run verbose tests
##
## Last change date-
##       2023/06/04
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
## Run verbose tests
cmd T_Quick  --verbose
cmd TestIoda --verbose

cmd T_Stream --verbose --bringup --client
cmd T_Stream --verbose --server
cmd T_Stream --verbose --stress
cmd T_Stream --verbose --stress=1
cmd T_Stream --verbose --stress=10

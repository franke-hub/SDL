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
##       2023/07/30
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
    sleep 5
    return
  fi

  echo "FAIL: $1"
  exit 1
}

##############################################################################
## Run verbose tests
cmd T_Quick  --verbose

cmd T_Stream --verbose --bringup --client
cmd T_Stream --verbose --server

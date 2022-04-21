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
##       Unit-test.sh
##
## Function-
##       Run all executables with default options
##
## Last change date-
##       2022/03/04
##
##############################################################################
F=$(basename "$0")
echo -e "\nRunning $F ======================================================="

##############################################################################
## Function OK: Run test, success expected
function OK
{
  $@
  rc=$?
  if [ $rc == 0 ] ; then
    echo "OK $@"
    return
  fi

  echo "$@ returned $rc, but 0 expected"
  exit 1
}

##############################################################################
## Run executables
echo -e "\nUnit tests"
OK Quick
OK TestDisp
OK TestList
OK TestLock
OK TestMisc
OK Test_Num
OK Test_Thr
OK Test_bug

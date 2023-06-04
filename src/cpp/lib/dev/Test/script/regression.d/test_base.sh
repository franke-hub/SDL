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
##       test_base.sh
##
## Function-
##       Run executables with default options
##
## Last change date-
##       2023/06/04
##
##############################################################################

##############################################################################
## Run executables
test_set="T_Option T_Quick TestIoda"
for test in $test_set
do
  ./$test
  rc=$?
  if [[ $rc == 0 ]] ; then
    echo "PASS: ./$test"
  else
    echo "FAIL: ./$test, rc $rc"
    exit $rc
  fi
done

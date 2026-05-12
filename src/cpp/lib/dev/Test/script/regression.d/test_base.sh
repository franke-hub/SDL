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
##       test_base.sh
##
## Function-
##       Run executables with default options
##
## Last change date-
##       2026/04/22
##
##############################################################################

##############################################################################
## Run executables
test_set="T_Option T_Quick T_Stream"
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

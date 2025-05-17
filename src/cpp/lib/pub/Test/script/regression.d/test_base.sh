#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2022-2025 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
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
##       2025/03/30
##
##############################################################################

##############################################################################
## Insure TestLock semaphore is reset
TestLock --reset >/dev/null 2>/dev/null

##############################################################################
## Run executables
test_set="Quick    TestTime"
for test in $test_set
do
  ./$test --all
  rc=$?
  if [[ $rc == 0 ]] ; then
    echo "PASS: ./$test --all"
  else
    echo "FAIL: ./$test --all, rc $rc"
    exit $rc
  fi
done

test_set="TestIoda TestList TestLock TestMisc"
test_set="$test_set Test_num Test_thr Test_utf"
for test in $test_set
do
  [[ "$test" == "Test_num" ]] && echo "TEST: ./$test (started)"
  [[ "$test" == "Test_thr" ]] && echo "TEST: ./$test (started)"
  ./$test
  rc=$?
  if [[ $rc == 0 ]] ; then
    echo "PASS: ./$test"
  else
    echo "FAIL: ./$test, rc $rc"
    exit $rc
  fi
done

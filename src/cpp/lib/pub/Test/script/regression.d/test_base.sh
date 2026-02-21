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
##       test_base.sh
##
## Function-
##       Run executables with default options
##
## Last change date-
##       2026/02/19
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

## Removed Test_thr, now in test_time.sh
test_set="TestIoda TestList TestLock TestMisc"
test_set="$test_set Test_num Test_utf"
for test in $test_set
do
  [[ "$test" == "TestMisc" ]] && echo "TEST: ./$test (started)"
  [[ "$test" == "Test_num" ]] && echo "TEST: ./$test (started)"
  ./$test
  rc=$?
  if [[ $rc == 0 ]] ; then
    echo "PASS: ./$test"
  else
    echo "FAIL: ./$test, rc $rc"
    exit $rc
  fi
done

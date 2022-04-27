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
##       test_base.sh
##
## Function-
##       Run all executables with default options
##
## Last change date-
##       2022/04/22
##
##############################################################################

##############################################################################
## Function OK: Run test, success expected
function OK
{
  $@
  rc=$?
  if [ $rc == 0 ] ; then
    return
  fi

  echo "$@ returned $rc, but 0 expected"
  exit 1
}

##############################################################################
## Insure TestLock semaphore is reset
TestLock --reset >/dev/null 2>/dev/null

##############################################################################
## Run executables
test="Quick --all"
./$test
rc=$?
if [[ $rc == 0 ]] ; then
  echo "PASS: ./$test"
else
  echo "FAIL: ./$test, rc $rc"
fi

test_set="TestDisp TestList TestLock TestMisc Test_num Test_thr"
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
  fi
done

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
##       test_Parser.sh
##
## Function-
##       Parser regression set
##
## Last change date-
##       2022/04/19
##
##############################################################################
F=$(basename "$0")
echo -e "\nRunning $F ======================================================="

##############################################################################
## Run test, check output
ng=0
Test_Parser 1>parser.out 2>parser.err

diff parser.out S/script/out/parser.out >/dev/null 2>/dev/null
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: stdout not as expected"
  diff parser.out S/script/out/parser.out
  ng=1
fi

diff parser.err S/script/out/parser.err >/dev/null 2>/dev/null
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: stderr not as expected"
  diff parser.err S/script/out/parser.err
  ng=1
fi

if [[ $ng == 0 ]] ; then
  echo "OK: test_Parser.sh"
else
  echo "NG: test_Parser.sh (Unexpected error)"
fi

##############################################################################
## Remove temporaries
rm -f parser.err parser.out
exit $ng

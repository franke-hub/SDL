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
##       test_parser.sh
##
## Function-
##       Parser regression set
##
## Last change date-
##       2022/04/22
##
##############################################################################

##############################################################################
## Run test, check output
ng=0
Test_parser 1>parser.out 2>parser.err

diff parser.out S/script/out/parser.out >/dev/null 2>/dev/null
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: stdout data mismatch"
  diff parser.out S/script/out/parser.out
  ng=1
fi

diff parser.err S/script/out/parser.err >/dev/null 2>/dev/null
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: stderr data mismatch"
  diff parser.err S/script/out/parser.err
  ng=1
fi

##############################################################################
## Remove temporaries
rm -f parser.err parser.out
exit $ng

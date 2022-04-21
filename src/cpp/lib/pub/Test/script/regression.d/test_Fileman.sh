#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2020-2022 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       test_Fileman.sh
##
## Function-
##       Fileman regression set
##
## Last change date-
##       2022/03/04
##
##############################################################################
F=$(basename "$0")
echo -e "\nRunning $F ======================================================="

DO=TestFile

##############################################################################
## Function OK: Run test, success expected
function OK
{
  $DO $@
  rc=$?
  [ $rc == 0 ] && return

  echo "$DO $1 returned $rc, but 0 expected"
  exit 1
}

##############################################################################
## Function NG: Run test, failure expected
function NG
{
  $DO $@
  rc=$?
  [ $rc != 0 ] && return

  echo "$DO $1 returned 0, but non-zero expected"
  exit 1
}

##############################################################################
## Remove/create links
rm -f junk loop zero
ln -s S/junk junk
ln -s loop loop
ln -s /../zero zero

##############################################################################
echo -e "\n\n================================================================"
echo "OK Regression tests"
## set -x
OK ./../.                           ## Resolves to ../.
OK ./././.                          ## Remove trailing blanks
OK H resolve utf8.sh                ## Link resolution
OK junk                             ## Link to missing file S/junk (OK)
OK *.xxyyz                          ## Non-existent (OK)
## set +x

##############################################################################
echo -e "\n\n================================================================"
echo "NG Regression tests"
## set -x
NG ./ .//. S/.//.                   ## Empty file names
NG zero                             ## Name /..
NG loop                             ## SYMLINK_MAX
NG miss/ing S/miss/ing              ## miss missing
## set +x

##############################################################################
## Remove links
rm -f junk loop zero

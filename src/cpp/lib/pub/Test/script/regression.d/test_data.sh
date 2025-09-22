#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2020-2025 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
## SPDX-License-Identifier: MIT
##----------------------------------------------------------------------------
##
## Title-
##       test_data.sh
##
## Function-
##       Data regression set
##
## Last change date-
##       2025/09/20
##
##############################################################################
DO=TestData

##############################################################################
## Function OK: Run test, success expected
function OK
{
  $DO name $parm $@
  rc=$?
  [ $rc == 0 ] && return

  echo "$DO $1 returned $rc, but 0 expected"
  exit 1
}

##############################################################################
## Function NG: Run test, failure expected
function NG
{
  $DO name $parm $@
  rc=$?
  [ $rc != 0 ] && return

  echo "$DO $1 returned 0, but non-zero expected"
  exit 1
}

##############################################################################
## Parameter analysis
parm=
verb=0
for arg in "$@"
do
  case $arg in
    --verbose*)
      verb=1
      parm="$arg $parm"
      shift
      ;;

    -*)
      echo "Parameter '$arg' ignored"
      shift
      ;;

    *)
      break
      ;;
  esac
done

##############################################################################
## Remove/create links
rm -f junk loop zero
ln -s S/junk junk
ln -s loop loop
ln -s /../zero zero

##############################################################################
[[ $verb > 0 ]] && echo "Regression tests, expect success"
## set -x
OK /../etc                          ## Resolves to /etc
OK ./../.                           ## Resolves to ../.
OK ./././.                          ## Remove trailing blanks
OK H resolve utf8.sh                ## Link resolution
OK junk                             ## Link to missing file S/junk (OK)
OK *.xxyyz                          ## Non-existent (OK)
OK zero                             ## Past filesystem origin (OK, now)
## set +x

##############################################################################
[[ $verb > 0 ]] && echo "Regression tests, expect failure"
## set -x
NG ./ .//. S/.//.                   ## Empty file names
NG loop                             ## SYMLINK_MAX
NG miss/ing S/miss/ing              ## Path miss missing
## set +x

##############################################################################
if [[ $verb > 0 ]] ; then
  echo "Regression tests: verbose"
  set -x
  $DO  $parm path .
  $DO  $parm path S
  $DO  $parm path S/
  set +x
fi

##############################################################################
## Remove links
rm -f junk loop zero

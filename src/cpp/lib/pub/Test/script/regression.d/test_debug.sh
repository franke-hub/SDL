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
##       test_debug.sh
##
## Function-
##       Debug.h regression set
##
## Last change date-
##       2022/04/22
##
##############################################################################

##############################################################################
## Information exit
function info
{
  echo "Cannot test Debug.h, $HOME/bin/filecomp not installed"
  echo "Install filecomp using '~/obj/cpp/Tools/Compare/make install'"
  exit 1
}

##############################################################################
## Prerequisite: Tools/Compare/filecomp.cpp installed
locate=`whence filecomp`
if [[ $? != 0 ]] ; then
  if [[ ! -x "$HOME/bin/filecomp" ]] ; then
    info
  fi
else
  if [[ -z "$locate" ]] ; then
    info
  fi
fi

##############################################################################
## Parameter analysis
keep=0
parm=
verb=0
for arg in "$@"
do
  case $arg in
    --keep)
      keep=1
      shift
      ;;

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
## Run test, check output
ng=0
Test_bug 1>test_bug.out 2>test_bug.err
[[ -f "debug.out" ]] && mv debug.out test_bug.log

filecomp $parm S/script/out/test_bug.out test_bug.out
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: stdout data mismatch"
  ng=1
fi

filecomp $parm S/script/out/test_bug.err test_bug.err
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: stderr data mismatch"
  ng=1
fi

filecomp $parm S/script/out/test_bug.log test_bug.log
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: debug.out data mismatch"
  ng=1
fi

##############################################################################
## Remove temporaries
[[ $keep == 0 ]] && rm -f debug.out test_bug.err test_bug.log test_bug.out
exit $ng

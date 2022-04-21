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
##       test_Debug.sh
##
## Function-
##       Debug.h regression set
##
## Last change date-
##       2022/04/22
##
##############################################################################
F=$(basename "$0")
echo -e "\nRunning $F ======================================================="

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
parm=
for arg in "$@"
do
  case $arg in
    --verbose*)
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

filecomp $parm S/script/out/test_bug.out test_bug.out
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: stdout not as expected"
  ng=1
fi

filecomp $parm S/script/out/test_bug.err test_bug.err
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: stderr not as expected"
  ng=1
fi

filecomp $parm S/script/out/test_bug.log debug.out
rc=$?
if [[ $rc != 0 ]] ; then
  echo "Error: stderr not as expected"
  ng=1
fi

if [[ $ng == 0 ]] ; then
  echo "OK: test_Debug.sh"
else
  echo "NG: test_Debug.sh (Unexpected error)"
fi

##############################################################################
## Remove temporaries
## rm -f debug.out filecomp.info test_bug.err test_bug.log test_bug.out
exit $ng

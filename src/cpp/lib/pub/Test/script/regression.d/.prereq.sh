#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2022-2023 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       .prereq.sh
##
## Function-
##       Verify test prequisites
##
## Last change date-
##       2023/06/05
##
##############################################################################

##############################################################################
## Prerequisite: filecomp
type -p filecomp >/dev/null 2>/dev/null
if [[ $? != 0 ]] ; then
  echo -e "\n\nInstalling missing prerequite: filecomp (into ~/bin)\n"
  pushd ~/obj/cpp/sys >/dev/null
  make install
  popd >/dev/null

  echo ""
  type -p filecomp >/dev/null 2>/dev/null
  if [[ $? != 0 ]] ; then
    echo "Install failed or ~/bin is not in \$PATH"
    exit 1
  fi
fi

##############################################################################
## Prerequisite: libraries
pushd ~/obj/cpp/lib/com >/dev/null
echo make: $PWD
make
rc=$?
if [[ $rc == 0 ]] ; then
  cd ../pub
  echo make: $PWD
  make
  rc=$?
fi
popd >/dev/null
[[ $rc != 0 ]] && exit $rc

##############################################################################
## Prerequisite: test executables
echo make: $PWD
make all
rc=$?
exit $rc

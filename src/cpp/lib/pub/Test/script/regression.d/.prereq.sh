#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2022-2024 Frank Eskesen.
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
##       2024/03/04
##
##############################################################################

##############################################################################
## Prerequisite: filecomp
type -p filecomp >/dev/null 2>/dev/null
if [[ $? != 0 ]] ; then
  echo -e "\n\nInstalling missing prerequite: filecomp (into $ROOT/bin)\n"
  pushd $ROOT/obj/cpp/sys >/dev/null
  make install-filecomp
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
pushd $ROOT/obj/cpp/lib >/dev/null
echo make: $PWD
make
rc=$?
popd >/dev/null
[[ $rc != 0 ]] && exit $rc

##############################################################################
## Prerequisite: test executables
echo make: $PWD
make all
rc=$?
exit $rc

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
##       _prereq.sh
##
## Function-
##       Verify test prequisites
##
## Last change date-
##       2022/04/24
##
##############################################################################

##############################################################################
## Prerequisite: filecomp
type -p filecomp >/dev/null 2>/dev/null
if [[ $? != 0 ]] ; then
  echo -e "\n\nInstalling missing prerequite: filecomp (into ~/bin)\n"
  pushd ~/obj/cpp/Tools/Compare >/dev/null
  make install
  popd >/dev/null

  echo ""
  type -p filecomp >/dev/null 2>/dev/null
  if [[ $? != 0 ]] ; then
    echo "Install failed or ~/bin is not in \$PATH"
    exit 1
  fi
fi

exit 0

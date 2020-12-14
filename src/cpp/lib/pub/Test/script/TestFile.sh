#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2020 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       TestFile.sh
##
## Function-
##       TestFile regression set
##
## Last change date-
##       2020/12/13
##
##############################################################################

DO=TestFile
make $DO
if [[ $? != 0 ]] ; then
  exit $?
fi

##############################################################################
## Remove/create links
rm -f junk loop zero
ln -s S/junk junk
ln -s loop loop
ln -s /../zero zero

##############################################################################
echo -e "\n\n================================================================"
echo "OK Regression tests"
set -x
$DO ./../.                   ## Resolves to ../.
$DO ./././.                  ## Remove trailing blanks
$DO H resolve utf8.sh        ## Link resolution
$DO junk                     ## Link to missing file S/junk (OK)
$DO *.xxyyz                  ## Non-existent (OK)
set +x

##############################################################################
echo -e "\n\n================================================================"
echo "NG Regression tests"
set -x
$DO ./ .//. S/.//.           ## Empty file names
$DO zero                     ## Name /..
$DO loop                     ## SYMLINK_MAX
$DO miss/ing S/miss/ing      ## miss missing
set +x

##############################################################################
## Remove links
rm -f junk loop zero

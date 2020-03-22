#!/bin/sh
##----------------------------------------------------------------------------
##
##       Copyright (C) 2016-2020 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       cygwin-cleanup.sh
##
## Purpose-
##       Remove unreachable cygwin installation files.
##
## Last change date-
##       2020/03/21
##
## Usage-
##       cd <cygwin install directory>
##       cygwin-cleanup.sh ftp%3a%2f%2fmirror.mcs.anl.gov%2fpub%2fcygwin%2f/
##       cygwin-cleanup.sh http%3a%2f%2fmirrors.kernel.org%2fsourceware%2fcygwin%2f/
##       cygwin-cleanup.sh <etc>
##
##############################################################################
cat "$1setup.ini" | sed -n '/release\//p' | sed 's/^.*release\///g' |\
sed 's/\( [a-f0-9]*\)*$//g' | sort | uniq                        > setup.1
find "$1release/" -type f | sed -e 's/^.*release\///g' | sort    > setup.2
diff setup.1 setup.2 | grep ">"                                  > setup.3
cat setup.3 | sed -e 's:> :rm -vfr "'"$1"'release\/:g ; s/$/"/g' | bash
#cat setup.3 | sed -e 's:> :rm -vfr "'"$1"'release\/:g ; s/$/"/g' > setup.4
rm setup.1 setup.2 setup.3


#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2023 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       ~/bat/sys/configure.sh
##
## Purpose-
##       If version is out of date, make pristine.
##
## Last change date-
##       2024/02/01
##
## Usage-
##       ~/bat/sys/configure.sh library-name
##       ~/bat/sys/_want.version contains a list of library names and versions:
##          library-name1 = date
##
##       Make files invoke configure.sh, testing to see if they need to
##       be recompiled outside of dependency controls. Configure.sh uses
##       the file "_have.version" to test whether or not 'make pristine'
##       needs to be invoked. If the dates match, configure.sh does nothing.
##       If the dates don't match, configure.sh invokes 'make.pristine'
##
##       Programs usually include library functions using 'include <name.h>'.
##       This means that name.h is *not* included as a change dependency.
##       Library functions usually change without updating their function
##       signatures or structure order, so this is OK. However, when a
##       library's function signature or structure order *does* change, an
##       application needs to be recompiled.
##
##       That's why ~/bat/sys/configure.sh and ~/bat/sys/_want.version
##       (and the anciliary files ~/bat/sys/rdconfig and ~/bat/sys/wrconfig)
##       exist.
##
##############################################################################

##############################################################################
## Parameter and environment verification
[[ -z "$1" ]] && { echo `basename "$0"`: Missing parameter ; exit 1 ; }
here=$(dirname $0)
pushd $here >/dev/null
here=$(pwd -P)
popd >/dev/null

if [[ -z "$SDL_ROOT" ]] ; then
   pushd $here/../.. >/dev/null
   SDL_ROOT=$(pwd -P)
   popd >/dev/null
   export SDL_ROOT=$SDL_ROOT
fi

##############################################################################
## Parameterization: Program names and actions
RDCONFIG="$SDL_ROOT/bat/sys/rdconfig"
WRCONFIG="$SDL_ROOT/bat/sys/wrconfig"
WANTFILE="$SDL_ROOT/bat/sys/_want.version"
HAVEFILE="./_have.version"
THISFILE=`basename "$0"`

##############################################################################
## If we are up to date, do nothing
want=`$RDCONFIG "$WANTFILE" "$1"`
if [[ -r "$HAVEFILE" ]] ; then
   have=`$RDCONFIG $HAVEFILE "$1"`
   [[ "$have" == "$want" ]] && exit 0

   echo "$THISFILE: _have $have, _want $want"
else
   echo "$THISFILE: $HAVEFILE NOT READABLE"
fi

##############################################################################
## We are not up to date, remake
make pristine
rm -f $HAVEFILE
$WRCONFIG $HAVEFILE "$1" $want
exit 0

##############################################################################
##
##       Copyright (C) 2023 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
##
## Title-
##       ~/bat/sys/configure.sh
##
## Purpose-
##       If version is out of date, make pristine.
##
## Last change date-
##       2023/12/01
##
## Implementation notes-
##       (Currently) only used for C++
##
##############################################################################

##############################################################################
## Parameter and environment verification
[[ -z "$1" ]] && { echo "configure.sh: Missing parameter" ; exit 1 ; }

if [[ -z "$SDL_ROOT" ]] ; then
   ## Parse the parameter. For each xx in xx/xx/xx create ../../..
   for path in ${1//\// } ; do
     if [[ -n "$SDL_ROOT" ]] ; then
       SDL_ROOT=$SDL_ROOT/..
     else
       SDL_ROOT=..
     fi
   done
   export SDL_ROOT=$SDL_ROOT
fi

##############################################################################
## If we are up to date, do nothing
want=`"$SDL_ROOT/bat/sys/rdconfig" "$SDL_ROOT/bat/sys/_want.version" "$1"`
if [[ -r "./_have.version" ]] ; then
   have=`$SDL_ROOT/bat/sys/rdconfig ./_have.version "$1"`
   [[ "$want" == "$have" ]] && exit 0

   echo "configure.sh: _have $have, _want $want"
fi

##############################################################################
## We are not up to date, remake
make pristine
rm -f ./_have.version
$SDL_ROOT/bat/sys/wrconfig ./_have.version "$1" "$want"
exit 0

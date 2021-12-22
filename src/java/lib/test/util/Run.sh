#!/bin/bash
##############################################################################
##
##       Copyright (c) 2007-2013 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##############################################################################
##
## Title-
##       Run
##
## Purpose-
##       Run a test.
##
## Last change date-
##       2013/01/01
##
##############################################################################
target=test.util.Main
if [[ ! -z "$1" ]] ; then
  target=$1
  shift
fi

WINPATH=`cygpath --path --windows $CLASSPATH`
java -Xms32M -Xmx64M -classpath "$WINPATH" $target $*


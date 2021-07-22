#!/bin/bash
##----------------------------------------------------------------------------
##
##       Copyright (C) 2021 Frank Eskesen.
##
##       This file is free content, distributed under the MIT license.
##       (See accompanying file LICENSE.MIT or the original contained
##       within https://opensource.org/licenses/MIT)
##
##----------------------------------------------------------------------------
##
## Title-
##       semaphore.sh
##
## Purpose-
##       Semaphore.cpp stress test.
##
## Last change date-
##       2021/07/21
##
## Usage-
##       ./semaphore.sh (From obj directory)
##
##############################################################################

##############################################################################
## Compile
make Semaphore
rc=$?
if [[ $rc != 0 ]] ; then
   echo "Compile failure"
   exit $rc
fi

##############################################################################
## Script options
MM=256
[[ -n "$1" ]] && { MM=$1; shift; }

##############################################################################
## Stress test
II=0
while [ $II -lt $MM ] ; do
   Semaphore $*
   rc=$?
   if [[ $rc != 0 ]] ; then
     echo "Test failure, iteration $II"
     exit $rc
   fi
   let II=$II+1
done
echo "Test complete, iteration $II"

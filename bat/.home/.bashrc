##
##============================================================================
##
##       Copyright (C) 2020-2024 Frank Eskesen.
##
##       This file is free content, distributed under creative commons CC0,
##       explicitly released into the Public Domain.
##       (See accompanying html file LICENSE.ZERO or the original contained
##       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
##
##============================================================================
##
## Title-
##       .bashrc
##
## Function-
##       Shell customization script, sourced for each shell invocation
##
## Last change date-
##        2024/01/26
##
##############################################################################

##############################################################################
## Debugging hook
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) begin .bashrc $$ $0" >>$debugging

##############################################################################
## If not running bash interactively, don't do anything
if [ -z "`uname | grep CYGWIN`" ] ; then ## Ignore interactive check in CYGWIN
   [ -z "$BASH_VERSION" ] && return
   [[ $- != *i* ]] && return
fi

##############################################################################
## Common initialization
[ -r $HOME/bat/bash_common ] && source $HOME/bat/bash_common

export EXECIGNORE="*.dll"           ## Exclude *.dlls from TAB expansion

##############################################################################
## XTERM decoration
[ "${TERM:0:5}" == "xterm" -a -r $HOME/bat/setPROMPT ] && source $HOME/bat/setPROMPT

##############################################################################
## Debugging hook
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) *end* .bashrc $$ $0" >>$debugging

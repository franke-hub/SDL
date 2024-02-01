##############################################################################
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
##       .bash_profile
##
## Function-
##       Shell startup script, sourced during login
##
## Last change date-
##        2024/01/26
##
##############################################################################

##############################################################################
## Debugging hook
## export debugging=$HOME/.local/log/user.log
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) begin .bash_profile $$ $0" >>$debugging

##############################################################################
## Set PATH
## export PATH=/usr/local/bin:usr/bin/:/usr/local/sbin:/usr/sbin

. $HOME/bat/f.appendstring

## Prepend in reverse order
[ -d $HOME/bin ] && PrependString PATH "$HOME/bin"
[ -d $HOME/bat ] && PrependString PATH "$HOME/bat"
[ -d $HOME/.local/bin ] && PrependString PATH "$HOME/.local/bin"
[ -d $HOME/.local/bat ] && PrependString PATH "$HOME/.local/bat"
PrependString PATH "."

## Append in normal order
[ -d /usr/local/bin ] && AppendString PATH "/usr/local/bin"

##############################################################################
## Set LD_LIBRARY_PATH

## Prepend in reverse order
[ -d $HOME/.local/lib ] && PrependString LD_LIBRARY_PATH "$HOME/.local/lib"
[ -d $HOME/.local/lib/usr ] && PrependString LD_LIBRARY_PATH "$HOME/.local/lib/usr"
PrependString LD_LIBRARY_PATH "."

## Append in normal order
[ -d /usr/local/lib ] && AppendString LD_LIBRARY_PATH "/usr/local/lib"

##############################################################################
## CYGWIN version code
isCYGWIN=`uname | grep CYGWIN`
if [ -n "$isCYGWIN" ] ; then
  export CYGWIN="winsymlinks:native"
fi

##############################################################################
## Other initialization
[ -z "$HOST" ] && export HOST=`hostname`
export LANG=$(locale -uU 2>/dev/null)
[ -z "$LANG" ] && export LANG=en_US.UTF-8

unset EDITOR
source $HOME/bat/setupEDIT

##############################################################################
## Shell initialization
[ -r $HOME/.bashrc ] && source $HOME/.bashrc

##############################################################################
## Environment checks.
[ -s $HOME/.cvspass ] && echo WARNING: $HOME/.cvspass exists
[ -s $HOME/.local/log/error.log ] && echo WARNING: $HOME/.local/log/error.log exists

unset -f AppendString PrependString

##############################################################################
## Debugging hook
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) *end* .bash_profile $$ $0" >>$debugging

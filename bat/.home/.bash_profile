##############################################################################
##
##       Copyright (C) 2020-2021 Frank Eskesen.
##
##       This file is free content, distributed under the "un-license,"
##       explicitly released into the Public Domain.
##       (See accompanying file LICENSE.UNLICENSE or the original
##       contained within http://unlicense.org)
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
##        2021/04/20
##
##############################################################################

##############################################################################
# Debugging hook
# export debugging=~/.local/log/user.log
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) begin .bash_profile $$ $0" >>$debugging

##############################################################################
# Set PATH
. $HOME/bat/f.appendstring

## Prepend in reverse order
[ -d $HOME/.local/bin ] && PrependString PATH "$HOME/.local/bin"
[ -d $HOME/bin ] && PrependString PATH "$HOME/bin"
[ -d $HOME/bat ] && PrependString PATH "$HOME/bat"
PrependString PATH "."

## Append in normal order
[ -d /usr/local/bin ] && AppendString PATH "/usr/local/bin"

##############################################################################
# LINUX/CYGWIN version code
isCYGWIN=`uname | grep CYGWIN`
if [ -n "$isCYGWIN" ] ; then
  if [ -z "$HOST" ] ; then
    export HOST=`hostname`
  fi

  export CYGWIN="winsymlinks:nativestrict"
else
  [ -d /usr/local/lib ] && PrependString LD_LIBRARY_PATH "/usr/local/lib"
  PrependString LD_LIBRARY_PATH "."

##[ ! -d /run/user/$UID/thumbnails ] && mkdir /run/user/$UID/thumbnails
fi

##############################################################################
# Shell initialization
[ -r ~/.bashrc ] && source ~/.bashrc

##############################################################################
# Environment checks.
[ -s ~/.cvspass ] && echo WARNING: ~/.cvspass exists
[ -s ~/.local/log/error.log ] && echo WARNING: ~/.local/log/error.log exists

unset -f AppendString PrependString

##############################################################################
# Debugging hook
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) *end* .bash_profile $$ $0" >>$debugging

##############################################################################
##
##       Copyright (C) 2020-2021 Frank Eskesen.
##
##       This file is free content, distributed under the "un-license,"
##       explicitly released into the Public Domain.
##       (See accompanying file LICENSE.UNLICENSE or the original
##       contained within http://unlicense.org)
##
##############################################################################
##
## Title-
##       .profile
##
## Function-
##       Sourced during user login on certain systems.
##
## Last change date-
##        2021/04/22
##
## Implementation notes-
##       This is not *SUPPOSED* to be invoked when .bash_profile is present.
##
##############################################################################

##############################################################################
# Debugging hook
# export debugging=$HOME/.local/log/user.log
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) begin .profile $$ $0" >>$debugging

##############################################################################
# Profile initialization
[ -r ~/.bash_profile ] && source .bash_profile

##############################################################################
# Debugging hook
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) *end* .profile $$ $0" >>$debugging

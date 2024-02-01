##############################################################################
##
##       Copyright (C) 2020-2024 Frank Eskesen.
##
##       This file is free content, distributed under creative commons CC0,
##       explicitly released into the Public Domain.
##       (See accompanying html file LICENSE.ZERO or the original contained
##       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
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
##        2024/01/26
##
## Implementation notes-
##       This is not *SUPPOSED* to be invoked when .bash_profile is present.
##
##############################################################################

##############################################################################
## Debugging hook
## export debugging=$HOME/.local/log/user.log
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) begin .profile $$ $0" >>$debugging

##############################################################################
## Profile initialization
[ -r $HOME/.bash_profile ] && source .bash_profile

##############################################################################
## Debugging hook
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) *end* .profile $$ $0" >>$debugging

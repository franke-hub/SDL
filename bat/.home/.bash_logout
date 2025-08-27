##
##============================================================================
##
##       Copyright (C) 2025 Frank Eskesen.
##
##       This file is free content, distributed under creative commons CC0,
##       explicitly released into the Public Domain.
##       (See accompanying html file LICENSE.ZERO or the original contained
##       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
##
## SPDX-License-Identifier: CC0-1.0
##============================================================================
##
## Title-
##       .bash_logout
##
## Function-
##       Shell customization script, sourced during logout
##
## Last change date-
##        2025/08/27
##
##############################################################################

##############################################################################
## Debugging hook
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) begin .bash_logout $$ $0" >>$debugging

##############################################################################
## Debugging hook
[ -n "$debugging" ] && date "+%s.%N HOST($HOST) USER($USER) *end* .bash_logout $$ $0" >>$debugging

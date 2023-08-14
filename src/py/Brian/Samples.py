#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2016-2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Samples.py
##
## Purpose-
##       Brian AI: Sample replacement packages
##
## Last change date-
##       2018/01/01
##
##############################################################################
from lib.Control import control
from lib.Dispatch import TAB

##############################################################################
## Sample control['post-server'] handler
##############################################################################
if True:
    class __Handler(TAB):
        def work(self, uow):
            inp = uow.work
            inp += '\n'
            if getattr(uow, 'sess', '*') == 'Default/Server':
                inp += '<br>'

            uow.work = "You sayeth: " + inp
            uow.work += "I sayeth: 'I've got nothing to sayeth.'"
            uow.done()

    control['post-server'] = __Handler()

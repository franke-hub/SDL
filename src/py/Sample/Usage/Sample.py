##############################################################################
##
##       Copyright (C) 2017 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Sample.py
##
## Purpose-
##       Sample operator Commands
##
## Last change date-
##       2017/01/01
##
##############################################################################
from __future__ import print_function

from lib.Command import command
from lib.Debug import Debug, debugf, tracef

##############################################################################
## __Command class (Sample command)
##############################################################################
class __Command:
    @staticmethod
    def run(argv):
        debugf("Sample command")
        return 0

command['sample'] = __Command


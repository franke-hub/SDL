#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2019 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Dirty.py
##
## Purpose-
##       Quick and dirty test.
##
## Last change date-
##       2019/08/18
##
##############################################################################
import sys

#### PyQt5 ###################################################################
from PyQt5.QtCore    import *
from PyQt5.QtGui     import *
from PyQt5.QtWidgets import *

#### lib #####################################################################
from lib.Command     import *
from lib.Debug       import *
from lib.Utility     import *

#### Golf ####################################################################
from DbServer        import *
from GolfApplet      import *
from HoleInfo        import *

##############################################################################
## Dirty test
##############################################################################
class Dirty_fail():
    @staticmethod
    def run(*args):
        raise RuntimeError('Induced failure')

class Dirty_test():
    @staticmethod
    def run(*args):                 ## Test DbServer
        debugf(dbError('dbError test'))
        debugf(dbMissing(FIND_PLAYER, 'nobody'))

command['dirty'] = Dirty_fail       ## Last one wins
command['dirty'] = Dirty_test       ## Last one wins

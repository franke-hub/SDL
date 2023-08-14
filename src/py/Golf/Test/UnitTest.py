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
##       UnitTest.py
##
## Purpose-
##       Golf: Unit tests.
##
## Last change date-
##       2019/08/18
##
##############################################################################

#### Import ##################################################################
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
from golf            import *

##############################################################################
## Test GolfApplet
##############################################################################
class GolfApplet_test():
    @staticmethod
    def run(*args):
        debugf('DEFAULT_CI: %s' % loadCourseID(None))
        debugf('C0001: %s' % loadCourseID('C0001'))

        debugf('DEFAULT_EI: %s' % loadEventsID(None))
        debugf('gg2019: %s' % loadEventsID('gg2019'))

        debugf('DEFAULT_PI: %s' % str(loadPlayerName(None)))
        debugf('P0001: %s'      % str(loadPlayerName('P0001')))
        debugf('frank: %s'      % str(loadPlayerName('frank')))
        debugf('P0011: %s'      % str(loadPlayerName('P0011')))
        debugf('yogar: %s'      % str(loadPlayerName('yogar')))

        ######################################################################
        ## Error tests
        try:
            debugf('gg2019: %s' % loadCourseID('gg2019'))
            assert False, 'Expected exception not raised'
        except Exception as X:
            debugf('Expected exception raised: %s' % X)
        return 0

command['golfapplet'] = GolfApplet_test

##############################################################################
## Test HoleInfo
##############################################################################
class HoleInfo_test():
    @staticmethod
    def run(*args):
        info = HoleInfo('18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 Hello')
        info.setText(0, 'Info')
        info.debug()

        print('')
        info = HoleInfoLabel()
        info.debug()
        return 0

command['holeinfo'] = HoleInfo_test

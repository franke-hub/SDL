##############################################################################
##
##       Copyright (C) 2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Types_test.py
##
## Purpose-
##       Types.py bringup test
##
## Last change date-
##       2018/01/01
##
##############################################################################
import sys
import threading
import time

from lib.Command import command
from lib.Debug import debugf
from lib.Types import Bool, Integer, String

##############################################################################
## __Command class (The Types test command)
##############################################################################
class __Command:
    @staticmethod
    def run(*args):
        debugf('lib.Types self-test')
        b = Bool()
        i = Integer()
        s = String()

        b._ = True
        assert b._ == True, 'Failed: Bool=True'
        b._ = False
        assert b._ == False, 'Failed: Bool=False'
        maybe = True
        b._ = maybe
        assert b._ == maybe, 'Failed: Bool=value'

        raised = False
        try:
            b._ = 0
        except ValueError as e:
            raised = True
            debugf('Expected exception raised:', e.__repr__())
        assert raised, 'Bool = 0'

        raised = False
        try:
            b._ = 1
        except ValueError:
            raised = True
        assert raised, 'Bool = 1'

        i._ = 12
        assert i._ == 12, 'Failed: Int=12'
        i._ = -12
        assert i._ == -12, 'Failed: Int=-12'
        maybe = int(True)
        i._ = maybe
        assert i._ == maybe, 'Failed: Int=value'

        raised = False
        try:
            i._ = '1234'
        except ValueError as e:
            raised = True
            debugf('Expected exception raised:', e.__repr__())
        assert raised, 'Int = True'

        raised = False
        try:
            i._ = '1'
        except ValueError:
            raised = True
        assert raised, "Int = '1'"

        s._ = 'alpha'
        assert s._ == 'alpha', "Failed: Str='alpha'"
        s._ = 'beta'
        assert s._ == 'beta',  "Failed: Str='beta'"
        maybe = 'True'
        s._ = maybe
        assert s._ == maybe, 'Failed: Str=value'

        raised = False
        try:
            s._ = True
        except ValueError as e:
            raised = True
            debugf('Expected exception raised:', e.__repr__())
        assert raised, 'Str = True'

        raised = False
        try:
            s._ = 1
        except ValueError:
            raised = True
        assert raised, 'Str = 1'
        debugf('lib.Types self-test completed')
        return 0

command['types'] = __Command


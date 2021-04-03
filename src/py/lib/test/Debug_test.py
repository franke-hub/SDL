##############################################################################
##
##       Copyright (C) 2017-2021 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Debug_test.py
##
## Purpose-
##       Debug.py bringup test
##
## Last change date-
##       2021/03/30
##
##############################################################################
import sys
import threading
import time

from lib.Command import command
from lib.Debug   import *

##############################################################################
## __Command class (The debug test command)
##############################################################################
class __Command:
    @staticmethod
    def run(*args):
        Debug.set(None)
        init = Debug('debug.one')
        debugf('lib.Debug self-test')
        with Debug.lock():
            assert Debug.get() == init, 'one'
            assert Debug.get() is init, 'two'

        init.set_opt('APPEND', True)
        init.set_opt('FLUSH', True)
        init.set_opt('HCDM', True)
        init.set_opt('MODE', Debug.MODE_LOGTNM)

        _raised = False
        try:
            init.set_opt('<invalid-option>', True)
        except Exception as e:
            _raised = True
            debugf('Expected exception raised:', e.__repr__())
        assert _raised, 'Expected exception missing!'

        _raised = False
        try:
            init.set_opt('HCDM', 'True') ## ('True' is str, not bool)
        except Exception as e:
            _raised = True
            debugf('Expected exception raised:', e.__repr__())
        assert _raised, 'Expected exception missing!'

        debugf('Testing: errorf pseudo-function', file=sys.stderr)
        debugf('test1', 'test2', 'test3: ', end='FOOBAR\n')
        debugf('test3', 'test2', 'test1: ', end='BARFOO\n')

        debugf("'%s'" % 'test1', "'%s'" % 'test2', "'%s': " % 'test3', end='FOOBAR\n')
        debugf("'%s'" % 'test3', "'%s'" % 'test2', "'%s': " % 'test1', end='BARFOO\n')

        tini = Logger('debug.two', append=True)
        assert Debug.set(tini) == init, 'Replace return value one'
        assert Debug.get() == tini, 'one'
        assert Debug.get() is tini, 'two'

        tini.set_opt('MODE', Debug.MODE_LOGTNM)
        debugf('Debug self-test: ', end=(time.asctime() + '\n'))
        debugf('NOISY LOGGER TEST')
        tracef('QUIET LOGGER TEST')

        tini._format = tini._format_prt ## Can't use: tini._format = Debug._format_prt
        debugf('Now using Debug._format')
        tini._format = tini._format_log
        debugf('Now using Logger._format')

        printf('If all goes well, this will only appear in stdout')
        writef("If all goes well, this will only appear in 'debug.two'")
        assert Debug.set(None) == tini, 'Replace return value two'

        init.debugf('lib.Debug self-test completed')
        ## del init
        ## del tini
        return 0

command['debug'] = __Command

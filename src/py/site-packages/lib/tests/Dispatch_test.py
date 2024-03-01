##############################################################################
##
##       Copyright (C) 2016-2019 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Dispatch_test.py
##
## Purpose-
##       Dispatch.py bringup test
##
## Last change date-
##       2019/09/05
##
## Implementation notes-
##       python2: 147.593 seconds elapsed time (4X longer)
##       python3:  34.928 seconds elapsed time
##
##############################################################################
import sys
import threading
import time

from lib.Command import command
from lib.Debug import *
from lib.Dispatch import *

#### lib #####################################################################
from lib.Global      import *

##############################################################################
## Local controls
##     _MULTI * _LOOPS (Number of operations started)
##     _MULTI * _HANGS (Number of ignored completions)
##     _MULTI * (_LOOPS-_HANGS) (Number of operation completion waits)
##     _MULTI * _LOOPS * (_TABS+1) (Total number of operations)
##############################################################################
_DEFAULT_LOOPS = 1024   ## sys.argv[1] Number of outer loops
_DEFAULT_HANGS = 2      ## sys.argv[4] Number of elements left hanging
_DEFAULT_MULTI = 16     ## sys.argv[2] Number of elements queued per loop
_DEFAULT_TABS  = 12     ## sys.argv[3] Number of 'pass-along' TABs
_USE_EXTRAS = True      ## Run special case tests?
_USE_EXTRAS = False     ## Run special case tests?
_VERBOSE = 0

##############################################################################
## The dispatcher object
##############################################################################
_OBJ = None

##############################################################################
## Class _BringUpTAB
##############################################################################
class _BringupTAB(TAB):
    def __init__(self, bringup):
        super(_BringupTAB,self).__init__()
        self.bringup = bringup
        self.index = len(bringup.tab)

    def work(self, uow):
        if _VERBOSE:
            debugf('_BringupTAB[%d].work(%s)' % (self.index, str(uow)))
        _OBJ.enqueue(self.bringup.tab[self.index+1], uow)

##############################################################################
## Class _ThrowExceptionTAB
##############################################################################
class _ThrowExceptionTAB(TAB):
    def __init__(self, bringup):
        super(_ThrowExceptionTAB,self).__init__()
        self.bringup = bringup
        self.index = len(bringup.tab)

    def work(self, uow):
        debugf('_ThrowExceptionTAB[%d].work(%s)' % (self.index, str(uow)))
        assert uow == None, 'Expected exception'

##############################################################################
## Class _WaitForeverTAB
##############################################################################
class _WaitForeverTAB(TAB):
    def __init__(self, bringup):
        super(_WaitForeverTAB,self).__init__()
        self.bringup = bringup
        self.index = len(bringup.tab)

    def work(self, uow):
        debugf('_WaitForeverTAB[%d].work(%s)' % (self.index, str(uow)))
        time.sleep(120.0)
        ## The UOW will not complete in time

##############################################################################
## Class _WorkForeverTAB
##############################################################################
class _WorkForeverTAB(TAB):
    def __init__(self, bringup):
        super(_WorkForeverTAB,self).__init__()
        self.bringup = bringup
        self.counter = 0
        self.index = len(bringup.tab)

    def work(self, uow):
        if (self.counter % 1000000) == 0:
            debugf('_WorkForeverTAB[%d].work(%s) %d' % (self.index, str(uow), self.counter))
        self.counter += 1
        _OBJ.enqueue(self, uow)

##############################################################################
## Bringup class (The dispatcher test object)
##############################################################################
class Bringup:
    def run(self, argv):
        ######################################################################
        ## Create the dispatch object
        global _OBJ
        _OBJ = OBJ()

        ######################################################################
        ## Set controls from defaults and arguments
        _LOOPS = _DEFAULT_LOOPS
        _MULTI = _DEFAULT_MULTI
        _TABS  = _DEFAULT_TABS
        _HANGS = _DEFAULT_HANGS
        try:
            _LOOPS = int(argv[1])
            _MULTI = int(argv[2])
            _TABS  = int(argv[3])
            _HANGS = int(argv[4])
        except IndexError:
            pass

        debugf('LOOPS(%d) MULTI(%d) TABS(%d) HANGS(%d)' % (_LOOPS, _MULTI, _TABS, _HANGS))

        ######################################################################
        ## Create test TAB array
        self.tab = []
        for i in range(_TABS):
            self.tab.append(_BringupTAB(self))

        self.tab.append(TAB())

        ## TEST SUCCEEDS WHEN USED
        if _USE_EXTRAS: ## Create a _ThrowExceptionTAB, give it some work
            tab = _ThrowExceptionTAB(self)
            for i in range(2):
                wdw = WDW()
                uow = UOW(WDO=wdw)
                _OBJ.enqueue(tab, uow)
            # self.common.stop()
            # return

        ## TEST SUCCEEDS WHEN USED
        if _USE_EXTRAS: ## Create a _WaitForeverTAB, give it some work
            tab = _WaitForeverTAB(self)
            wdw = WDW()
            uow = UOW(WDO=wdw)
            _OBJ.enqueue(tab, uow)
            # self.common.stop()
            # return

        ## TEST SUCCEEDS WHEN USED
        if _USE_EXTRAS: ## Create a _WorkForeverTAB, give it some work
            tab = _WorkForeverTAB(self)
            wdw = WDW()
            uow = UOW(WDO=wdw)
            _OBJ.enqueue(tab, uow)
            # self.common.stop()
            # return

        ## STRESS TEST SUCCEEDS (Always used)
        start = time.clock()
        for i in range(_LOOPS):
            wdw = []
            uow = []
            for j in range(_MULTI):
                wdw += [WDW()]
                uow += [UOW(WDO=wdw[j])]
                _OBJ.enqueue(self.tab[0], uow[j])
            if _VERBOSE: debugf('Before Bringup.wdw')
            if i < (_LOOPS-_HANGS):
                for j in range(_MULTI):
                    wdw[j].wait()
            if _VERBOSE: debugf('After  Bringup.wdw')
        elapsed = time.clock() - start
        debugf('%8.3f seconds elapsed, test complete' % elapsed)

        _OBJ.stop()
        _OBJ.join()
        elapsed = time.clock() - start

##############################################################################
## __Command class (The dispatcher test command)
##############################################################################
class __Command:
    @staticmethod
    def run(*argv):
        debugf('lib.Dispatch self-test')
        if Global.TESTING != 'dispatch':
            print('SKIPPED, --testing=dispatch not specified')
            return 0

        if len(argv) == 0:
            argv = ('dispatch', '10240')

        bringup = Bringup()
        bringup.run(argv)

        debugf('lib.Dispatch self-test completed')
        return 0
command['dispatch'] = __Command

##############################################################################
## _ADD_TIME class: Extends UOW done method
##############################################################################
class _ADD_TIME(UOW):
    def __init__(self, name, *args, **kwargs):
        super(_ADD_TIME, self).__init__(*args, **kwargs)
        self.post = False
        self.name = name

    def done(self, cc=None):
        debugf('%.3f %.3f _ADD_TIME(%s).done(%s)' %
               (time.time(), self._when, self.name, cc))
        self.post = True
        super(_ADD_TIME, self).done(cc)

class __ADD_TIMER:
    @staticmethod
    def run(*argv):
        global _OBJ
        _OBJ = OBJ()

        uow1 = _ADD_TIME('uow1')
        uow2 = _ADD_TIME('uow2')
        uow3 = _ADD_TIME('uow3')

        _OBJ.add_timer(0.125, uow1)
        _OBJ.add_timer(0.125, uow2)
        _OBJ.add_timer(0.125, uow3)
        _OBJ.del_timer(uow1)
        _OBJ.del_timer(uow2)
        _OBJ.del_timer(uow3)
        assert not uow1.post, 'del_timer1, but post'
        assert not uow2.post, 'del_timer2, but post'
        assert not uow3.post, 'del_timer3, but post'
        if False: ## Requires _OBJ._debug
            _OBJ._debug()
            _OBJ.stop()
            _OBJ.join()
            return

        _OBJ.add_timer(2.0, uow2)
        _OBJ.add_timer(2.0002, _ADD_TIME('beta'))
        _OBJ.add_timer(2.0001, _ADD_TIME('alpha'))
        _OBJ.add_timer(2.0003, _ADD_TIME('delta'))
        _OBJ.add_timer(3.0, uow3)
        _OBJ.add_timer(1.0, uow1)

        debugf('%.3f Waiting __ADD_TIMER...' % time.time())
        _DEL = uow2
        _OBJ.del_timer(_DEL)
        time.sleep(4.0)
        assert not _DEL.post, 'Posted ' + _DEL.name
        if _DEL != uow1: assert uow1.post, 'Missing uow1'
        if _DEL != uow2: assert uow2.post, 'Missing uow2'
        if _DEL != uow3: assert uow3.post, 'Missing uow3'
        debugf('%.3f ...Waiting __ADD_TIMER' % time.time())

        ## Verify stop posts timer
        _OBJ.add_timer(1.0, _DEL)
        _OBJ.stop()
        time.sleep(0.00125)         ## Stop runs on a different thread
        assert _DEL.post, '_OBJ.stop failed to post timer'

        ## Verify add_timer after stop immediate failure
        _DEL.cc = 0
        _DEL.post = False
        _OBJ.add_timer(1.0, _DEL)
        assert _DEL.post, 'add_timer after stop was accepted'
        assert _DEL.cc == UOW.CC_PURGE, 'Incorrect completion code'

        ## Test complete
        _OBJ.join()
        debugf('lib.Dispatch add-timer completed')
        return 0
command['add-timer'] = __ADD_TIMER


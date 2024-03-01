##############################################################################
##
##       Copyright (C) 2016-2021 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Dispatch.py
##
## Purpose-
##       Dispatcher classes.
##
## Last change date-
##       2021/04/03
##
## Usage notes-
##       from lib.Dispatch import OBJ, TAB, UOW, WDO, ...
##
##       OBJ: The Dispatcher itself. Derived from threading.Thread.
##            An implementation normally only uses one Dispatcher.
##            The Dispatcher is responsible for thread allocation
##            for each independent TAB.
##
##            The constructor accepts all threading.Thread arguments
##            The OBJ instance is a parameter to TAB() constructor.
##            .join() Waits for Dispatcher termination to complete.
##            .stop() Terminates the Dispatcher.
##            The Dispatcher MUST be terminated/joined before main exit.
##
##            .enqueue(TAB, UOW) passes a unit of work instance to the TAB.
##                Many threads can enqueue UOW instances but they will
##                be processed one a time, in sequence, as received,
##                by TAB.work(UOW).
##
##       TAB: Task Action Block. Processes units of work, one at a time.
##            Each individual TAB processes work on its own thread, which
##            is dynamically allocated by the Dispatcher. You can think
##            of a TAB as container with a queue that accepts standardized
##            Units of Work (UOWs). Each TAB processes its own queue
##            synchronously, in sequence, but is multi-threaded with
##            respect to all other TABs.
##
##            .work(UOW) Handle one unit of work.
##                This method should be overridden in a subclass as it
##                defines how each unit of work is processed.
##
##                It is inherently single-threaded. Without regard to the
##                number of work units enqueued, it will not be invoked
##                again on the same TAB until it returns.
##
##                When a work unit completes, UOW.done() should be called.
##                Alternatly, an implementation may opt to enqueue the UOW
##                to a different (or event the same) TAB, allowing the UOW
##                to flow through multiple work handlers. The last work
##                handler invokes UOW.done(), completing the work flow and
##                returning ownership of the UOW to the work originator.
##
##       UOW: Unit of Work. Contains a work unit instance.
##            Constructor: uow = UOW(WDO=None, FC=None)
##
##            Data fields:
##            .fc Function code.
##            .cc Completion code, indicates result.
##                None or 0 indicates OK, others usually Exceptions.
##            .work Common parameter for TAB.work() implementation.
##                The default is None
##
##            Methods:
##            .done(cc=*) Called from TAB.work() to indicate completion.
##                if cc specified: uow.cc=cc; if WDO: invokes WDO.done()
##                Do not override this method.
##
##       WDO: When-Done Object.
##            .done(UOW) Invoked from UOW.done()
##                This base class method does nothing. This has the same
##                effect as omitting the WDO from the UOW.
##
##                There are several built-in WDO subclasses, each of
##                which override this method. You can add others.
##
##       WDW: When Done Wait, wait for operation completion.
##            .wait()  ## Wait for operation completion
##
##            Usage example: (with dsp = OBJ(), done elsewhere)
##                tab = TAB() ## (Generic example)
##                uow = UOW(WDO=WDW())
##                dsp.enqueue(tab, uow)
##                uow.wdo.wait() ## Waits for uow.done() call
##
## Implementation notes-
##       The _set_fsm methods are for only for debugging. They are not even
##       implemented unless _HCDM == True during compilation. Since the
##       invocation test cannot similarly be compiled out, _HCDM cannot
##       be False during compliation and True during execution.
##
##       For exact counts, certain statistics need to be protected by a lock.
##       However, adding this locking adds about 10% to Dispatcher overhead.
##       We'll do without the exact counts.
##
## Implementation notes-
##       obj/py/lib/test >> time python3 ./Main dispatch
##       35.241 seconds elapsed
##              inps(2,129,920) outs(2,129,920) drains(133,107)
##       real 0m35.385s
##       user 0m25.506s
##       sys  0m 9.828s
##
##############################################################################
import queue
import threading
import time

from lib.Debug import *

##############################################################################
## Define available imports
##############################################################################
__all__ = ['OBJ', 'TAB', 'UOW', 'WDO', 'WDW', 'CCException', 'FCException']

##############################################################################
## Local controls
##############################################################################
_HCDM = False                       ## Hard Core Debug Mode?
_MAX_THREADS = 32                   ## Maximum number of threads in pool
_MIN_DELTA = 0.001                  ## Minimum event delta time
_USE_RELATIVE_TIME = True           ## Use absolute add_timer time?
_USE_THREAD_POOL = True             ## Use thread pool logic?
_VERBOSE = 1                        ## Verbosity: Larger is noisier

_SERIAL_LOCK = threading.Lock()     ## Serial number lock
_TIMERS_LOCK = threading.RLock()    ## Timers events lock

##############################################################################
## Helper classes
##############################################################################
class CCException(Exception):       ## uow.cc = CCException('description')
    pass                            ## Used to indicate error

class _FCBuiltin(object):           ## Internal uow function codes
    def __init__(self, name):
        self.name = name

class FCException(CCException):     ## uow.cc = FCException('function code')
    pass                            ## Indicates invalid uow.fc

##############################################################################
## Internal functions
##############################################################################
def _debugf(*args, **kwargs):
    M = Debug.get()._format_log(*args)
    printf(M, **kwargs)
    writef(M, **kwargs)

##############################################################################
## Class _Thread: (TAB driver thread)
##############################################################################
class _Thread(threading.Thread):
    if _HCDM: __dict = {}           ## Only used if _HCDM
    __serial = 0

    def __init__(self, _obj, _tab, *args, **kwargs):
        super(_Thread, self).__init__(*args, **kwargs)
        _Thread.__serial += 1
        self.serial = _Thread.__serial
        self.name = 'Dispatch-{}'.format(self.serial)
        if _VERBOSE > 2: debugf('%s.__init__()' % self.name)

        if _HCDM:
            _Thread.__dict[self.serial] = self
            self._fsm = '*init'

        _obj._stat_news += 1
        self.daemon = True
        self._event = threading.Event()
        self._obj = _obj
        self._tab = _tab
        self.start()

    if _HCDM:
        def __repr__(self):
            _tab = self._tab
            if _tab != None: _tab = _tab.serial
            status = 'event.set' if self._event.is_set() else 'event.clear'
            return '<THR[%s],TAB[%s],%s,%s>' % (self.serial, _tab, self._fsm, status)

    def run(self):
        _obj = self._obj
        cc = True
        while cc:
            self._event.wait()
            self._event.clear()
            if _HCDM: self._set_fsm('activ')
            _tab = self._tab
            if _tab:
                try:
                    if _HCDM: _tab._set_fsm('drain')
                    _obj._drain(_tab)
                    if _HCDM: _tab._set_fsm('empty')
                except:
                    Debug.handle_exception()
            cc = _obj._put_thread(self)

        if _HCDM:
            self._set_fsm('*DEL*')
            assert self._tab == None, '%s.exit but TAB present' % (self.name)
            del _Thread.__dict[self.serial]

    if _HCDM: ## NOTE: Caller is responsible for any required locking
        def _set_fsm(self, _fsm):
            old = self._fsm
            self._fsm = _fsm
            if _VERBOSE > 2:
                _tab = self._tab
                if _tab: _tab = _tab.serial
                debugf('THR[%2d] fsm(%s=>%s) TAB[%s]'
                      % (self.serial, old, _fsm, _tab))

    if _HCDM:
        @staticmethod
        def _static_debug(): ## Note: only called with OBJ._lock held
            debugf('Dispatch:_Thread._static_debug() %s' % (len(_Thread.__dict)))
            for key in _Thread.__dict:
                debugf('%s: %s' % (key, _Thread.__dict[key]))

##############################################################################
## Class _Timer: (Timer control thread)
##############################################################################
def _timer_done(_that):
    with _TIMERS_LOCK:
        _owner = _that._owner
        _that._owner = None
        if _owner:
            _owner._timer = None
            _owner._event.set()

class _Timer(object):
    ## For versions < 3.3, we have to base _Timer on object rather than extend
    ## threading.Timer. For compatability, we always use the object base.
    def __init__(self, _owner, _when, *args, **kwargs):
        delta = _when - time.time()
        delta = max(delta, _MIN_DELTA)
        self._that = threading.Timer(delta, _timer_done, [self])

        self._owner = _owner
        self._when  = _when
        self._that.start()

    def cancel(self):
        with _TIMERS_LOCK:
            self._owner = None
            self._that.cancel()

##############################################################################
## Class OBJ: (The Dispatcher)
##############################################################################
class OBJ(threading.Thread):
    _obj = None                     ## The current (last) OBJ

    def __init__(self, *args, **kwargs):
        super(OBJ, self).__init__(*args, **kwargs)
        self.name = 'DispatchMain'
        if _VERBOSE > 2: debugf('%s.__init__' % self.name)

        self._actives = 0           ## Number of active threads
        self._event = threading.Event() ## self.run Event
        self._lock = threading.RLock()  ## Synchronization (reentrant) lock
        self._max_count = 0         ## Maximum value of self._actives
        self._max_pools = 0         ## Maximum size of self._pool
        self._operational = True    ## True while operational
        self._pool = []             ## List of Dispatch._Thread objects
        self._timer = None          ## No current _Timer
        self._when = {}             ## Timed event dictionary
        self._USE_RELATIVE_TIME = _USE_RELATIVE_TIME ## (Configurable)
        self._USE_THREAD_POOL = _USE_THREAD_POOL ## (Set False when terminating)

        ## STATISTICS
        self._stat_dels = 0         ## Number of deleted _Thread objects
        self._stat_news = 0         ## Number of created _Thread objects
        self._stat_gets = 0         ## Number of calls to _get_thread
        self._stat_puts = 0         ## Number of calls to _put_thread
        self._stat_deque = 0        ## Number of work items processed
        self._stat_enque = 0        ## Number of work items enqueued
        self._stat_drain = 0        ## Number of drain operations
        self._stat_reget = 0        ## Number of _get_thread()s from queue
        self._stat_reput = 0        ## Number of _put_thread()s onto queue
        self._stat_unget = 0        ## Number of unneeded _get_thread()s
        self._stat_unput = 0        ## Number of unneeded _put_thread()s

        OBJ._obj = self;            ## The current dispatcher
        self.start()

    def add_timer(self, _when, _uow):
        if self._USE_RELATIVE_TIME:
            _when += time.time()
        _uow._when = _when
        isec = int(_when)
        with _TIMERS_LOCK:
            _operational = self._operational
            if _operational:
                if isec in self._when:
                    self._when[isec] += [ _uow ]
                else:
                    self._when[isec] = [ _uow ]
                self._reset_timer()
                return

        _uow.done(UOW.CC_PURGE)

    if _HCDM:
        def _debug(self, _stats=False):
            with self._lock:
                _debugf('%s._debug() operational(%s)' % (self.name, 'True' if self._operational else 'False'))

                _when = 'None'
                if self._timer: _when = '%.3f %s' % (self._timer._when, 'Valid' if self._timer._owner else 'ERROR')
                debugf('>> Timer(%s)' % _when)

                D = ''
                for isec in sorted(self._when):
                    if len(D) > 0: D += ','
                    D += '%d:[' % isec
                    D += ','.join('%.3f' % uow._when for uow in self._when[isec])
                    D += ']'
                debugf('>> _when({%s})' % D)

                _Thread._static_debug()
                TAB._static_debug()
                if _stats:
                    debugf('%s.stat: %s' % (self.name, self.stats()))

    def del_timer(self, _uow):      ## Remove UOW from timer list
        _when = _uow._when
        isec = int(_when)
        with _TIMERS_LOCK:
            _operational = self._operational
            if _operational:
                try:
                    uows = self._when[isec]
                    uows.remove(_uow)
                    _uow = None
                    if len(uows) == 0:
                       del self._when[isec]
                       self._reset_timer()
                except:
                    Debug.handle_exception()
        if _uow: _uow.done(UOW.CC_ERROR)

    def _drain(self, _tab):
        if _VERBOSE > 2: debugf('%s._drain(%s)' % (self.name, _tab))
        self._stat_drain += 1       ## Inexact (Increment is not atomic)
        while not _tab._queue.empty():
            _uow = _tab._queue.get()
            self._stat_deque += 1   ## Inexact (Increment is not atomic)
            try:
                if isinstance(_uow.fc, _FCBuiltin):
                    if _uow.fc == UOW.FC_CHASE:
                        _uow.done()
                    else:
                        _uow.done(FCException(uow.fc.name))
                else:
                    _tab.work(_uow)
            except Exception as x:
                Debug.handle_exception()
                try:
                    _uow.done(CCException(x))
                except:
                    Debug.handle_exception()
            _tab._queue.task_done()

    def enqueue(self, _tab, _uow):
        if _VERBOSE > 2: debugf('%s.enqueue(%s,%s)' % (self.name, str(_tab), str(_uow)))
        self._stat_enque += 1       ## Inexact (Increment is not atomic)
        if not self._operational:
            _uow.done(UOW.CC_PURGE)
            return

        _tab._queue.put(_uow)
        if not _tab._thread:
            self._get_thread(_tab)

    @staticmethod
    def get():
        return OBJ._obj

    def _get_thread(self, _tab):    ## Only called from self.enqueue
        """Allocate a _Thread for a TAB"""
        with self._lock:
            self._stat_gets += 1
            if _HCDM: _tab._set_fsm('thget')
            if _tab._thread:
                ## The thread may already be assigned due to a race condition
                ## where enqueue for the same TAB is running under multiple
                ## Threads. The test to call _get_thread is not lock protectd.
                if _HCDM: _tab._thread._set_fsm('unget')
                self._stat_unget += 1
            else:
                if self._USE_THREAD_POOL:
                    try:
                        self._stat_reget += 1
                        _thread = self._pool.pop()
                        _thread._tab = _tab
                        if _HCDM: _thread._set_fsm('reget')
                    except IndexError:
                        _thread = _Thread(self, _tab)
                else:
                    _thread = _Thread(self, _tab)

                self._actives += 1
                self._max_count = max(self._max_count, self._actives)

                _tab._thread = _thread
                _thread._event.set()

    def _put_thread(self, _thread):
        """Return a _Thread to the _Thread pool"""
        with self._lock:
            self._stat_puts += 1
            _tab = _thread._tab
            if _tab:
                _tab._thread = None

                ## Special case: elements now exist on the queue.
                ## (They may have been added while _tab._thread was not None.)
                if not _tab._queue.empty():
                    if _HCDM: _thread._set_fsm('unput')
                    self._stat_unput += 1
                    _tab._thread = _thread
                    _thread._event.set()
                    return True
            else:
                if _HCDM: _thread._set_fsm('noTAB')

            _thread._tab = None
            if self._USE_THREAD_POOL:
                if len(self._pool) < _MAX_THREADS:
                    if _HCDM: _thread._set_fsm('reput')
                    self._stat_reput += 1

                    self._pool.append(_thread)
                    self._max_pools = max(self._max_pools, len(self._pool))

                    assert self._actives != 0, 'self._actives going negative'
                    self._actives -= 1
                    return True

            self._stat_dels += 1
            assert self._actives != 0, 'self._actives going negative'
            self._actives -= 1

        if _HCDM: ## NOTE: thread being deleted, locking not required
            _fsm = 'unTAB'
            if not self._operational:
                _fsm = 'final'
            elif self._USE_THREAD_POOL:
                _fsm = 'ofull'
            _thread._set_fsm(_fsm)

        del _thread
        return False

    def _reset_timer(self):
        with _TIMERS_LOCK:
            empty = True
            for isec in sorted(self._when):
                empty = False
                min_when = float(isec + 2)
                for _uow in self._when[isec]:
                    min_when = min(min_when, _uow._when)

                if self._timer:
                    if self._timer._when <= (min_when + _MIN_DELTA):
                        return
                    self._timer.cancel()

                self._timer = _Timer(self, min_when)
                break

            if empty and self._timer:
                self._timer.cancel()
                self._timer = None

    def run(self):
        while self._operational:
            self._event.wait()

            done = []
            now  = time.time() + _MIN_DELTA + _MIN_DELTA
            inow = int(now)
            with _TIMERS_LOCK:
                if not self._operational:
                    break
                self._event.clear() ## Logically earlier, but lock needed
                for isec in sorted(self._when):
                    if isec > inow:
                        break

                    rems = []
                    uows = self._when[isec]
                    for _uow in uows:
                        if _uow._when < now:
                            rems.append(_uow)
                            done.append(_uow)
                    for _uow in rems:
                        uows.remove(_uow)
                    if len(uows) == 0: ## We can do this here because
                        del self._when[isec] ## sorted(self._when) is a copy

            for _uow in done:
                _uow.done(None)

            self._reset_timer()

        ######################################################################
        ## Termination cleanup, self._operational == False
        ######################################################################
        self._term()

    def stats(self):
        return ('threads(%s/%s), actives(%s/%s), pooled(%s/%s)\n'
                'gets/puts(%s/%s), fromq(%s/%s), state(%s/%s)\n'
                'inps(%s) outs(%s) drains(%s)'
               % ( self._stat_dels, self._stat_news   ## _Thread deletes/creates
                 , self._actives, self._max_count     ## Active _Threads (now/max)
                 , len(self._pool), self._max_pools   ## Pooled _Threads (now,max)
                 , self._stat_gets, self._stat_puts   ## Pooled total gets/puts
                 , self._stat_reget, self._stat_reput ## Successful gets/puts
                 , self._stat_unget, self._stat_unput ## Unneeded gets/puts
                 , self._stat_deque, self._stat_enque ## UOW dequeue/enqueues
                 , self._stat_drain                   ## UOW drains
                 )
               )

    def stop(self):
        ## We need the lock here to ensure the event is never cleared after
        ## self._operational is set to False
        with _TIMERS_LOCK:
            self._operational = False
            self._event.set()

    def _term(self):
        """Termination cleanup, self._operational == False"""
        if _VERBOSE > 0:
            if _VERBOSE == 1:
                _debugf('Terminating...')
            else:
                _debugf('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>')
                _debugf('Terminating...\n%s' % self.stats())

        ## Purge the outstanding timers
        for _when in self._when:
            uows = self._when[_when]
            for _uow in uows:
                _uow.done(UOW.CC_PURGE)
        self._when = {}
        if self._timer: self._timer.cancel()

        ## Clean up the thread pool
        if self._USE_THREAD_POOL:
            self._USE_THREAD_POOL = False
            with self._lock:
                for _thread in self._pool:
                    assert _thread._tab == None, '%s._tab != None' % (_thread)
                    if _HCDM: _thread._set_fsm('clean')
                    _thread._event.set()

                self._actives += len(self._pool)
                self._pool = []

        ## Wait for active threads to complete
        time.sleep(0.125)           ## Small delay for the pool _Threads
        self._wait_counter(16)      ## Wait for all active _Threads

        if _HCDM:
            _debugf('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>')
            self._debug()

        if _VERBOSE > 0:
            if _VERBOSE == 1:
                _debugf('...Terminated')
            else:
                _debugf('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>')
                _debugf('...Terminated\n%s' % self.stats())

    def _wait_counter(self, limit, delay=0.25):
        """Wait for active thread completion"""
        counter = 0
        while self._actives > 0:
            with self._lock:
                n = self._actives
                m = self._stat_news - self._stat_dels
            if n > 0:
                counter += 1
                if limit > 0 and counter > limit:
                    if _VERBOSE: debugf(self.name, 'Waiting: ABANDONED')
                    break

                if _VERBOSE: debugf(self.name, 'Waiting: %2d of %2d Threads pending' % (n, m))
                time.sleep(delay * min(15, counter))

##############################################################################
## Class TAB: (Task Action Block) base class
##############################################################################
class TAB(object):
    if _HCDM: __dict = {}           ## Only used if _HCDM
    if _HCDM: __serial = 0          ## Only used if _HCDM

    def __init__(self):
        self._queue = queue.Queue()
        self._thread = None

        if _HCDM:
            with _SERIAL_LOCK:
                TAB.__serial += 1
                self.serial = TAB.__serial
            self._fsm = '*init'
            TAB.__dict[self.serial] = self

    if _HCDM:
        def __repr__(self):
            _thread = self._thread
            if _thread != None: _thread = _thread.serial
            status = 'Empty' if self._queue.empty() else 'Queue'
            return '<TAB[%s],THR[%s],%s,%s>' % (self.serial, _thread, self._fsm, status)

    def isBusy(self):
        return (self._queue.qsize() > 0)

    def isIdle(self):
        return (self._queue.qsize() == 0)

    if _HCDM: ## NOTE: Caller is responsible for any required locking
        def _set_fsm(self, _fsm):
            old = self._fsm
            self._fsm = _fsm
            if _VERBOSE > 1:
                _thread = self._thread
                if _thread: _thread = _thread.serial
                debugf('TAB[%2d] fsm(%s=>%s) THR[%s]' % (self.serial, old, _fsm, _thread))

    if _HCDM:
        @staticmethod
        def _static_debug(): ## Note: only called with OBJ._lock held
            debugf('Dispatch:TAB._static_debug() %s' % (len(TAB.__dict)))
            for key in TAB.__dict:
                debugf('%s: %s' % (key, TAB.__dict[key]))

    def work(self, _uow): ## OVERRIDE this method in your Dispatch.TAB
        if _VERBOSE > 2: debugf('%s.work(%s)' % (self, _uow))
        _uow.done()

##############################################################################
## Class UOW: (Unit of Work) base class
##############################################################################
class UOW(object):
    if _HCDM: __serial = 0          ## Only used if _HCDM
    CC_ERROR = CCException()        ## Generic processing error
    CC_ERRFC = FCException()        ## Generic for invalid function code
    CC_PURGE = CCException('Purged')## Operation purged (OBJ stopped)

    FC_CHASE = _FCBuiltin('chase')  ## Insure prior operations complete
##  FC_TRACE = _FCBuiltin('trace')  ## (TBD)
##  FC_RESET = _FCBuiltin('reset')  ## (TBD)

    def __init__(self, WDO=None, FC=None):
        self.cc = None              ## Condition code
        self.fc = FC                ## Function code
        self.wdo = WDO              ## When Done Object
        self.work = None            ## Work object (User defined usage)

        if _HCDM:
            with _SERIAL_LOCK:
                UOW.__serial += 1
                self.serial = UOW.__serial

    if _HCDM:
        def __repr__(self):
            return '<UOW[%s]>' % (self.serial)

    __marker = object()
    def done(self, cc=__marker):
        """Returns ownership of the WDO to its creator.
        Subclasses override WDO.done(), not this method.
        Implementations invoke this method, not WDO.done().
        """
        ######################################################################
        ## WARNING: To those who override this method:
        ##   Make sure you use try/except handling when calling self.wdo.done.
        ##   Failure to heed this advice can make debugging very difficult.
        ######################################################################
        if _VERBOSE > 2: debugf('UOW(%s).done(%s)' % (self, cc))
        if cc is not self.__marker:
            self.cc = cc

        if self.wdo:
            try:
                self.wdo.done(self)
            except:
                Debug.handle_exception()

##############################################################################
## Class WDO: (When Done Object) base class
##############################################################################
class WDO(object):
    def done(self, _uow):
        """Handle operation completion.
        Subclasses override this method, not UOW.done().
        Implementations invoke UOW.done(), not this method.
        This base class does nothing, the same as a UOW with no WDO.
        """
        pass

##############################################################################
## Class WDW: (When Done Wait) object
##############################################################################
class WDW(WDO):
    """When Done Wait object.
    Callers use WDW objects to wait for work unit completion.
    WDW objects are not thread-safe.
    """

    def __init__(self):
        self._event = threading.Event()

    def done(self, _uow):
        """Handle operation completion"""
        self._event.set()

    def wait(self):
        """Wait for operation completion"""
        self._event.wait()
        self._event.clear()

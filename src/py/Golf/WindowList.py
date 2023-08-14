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
##       WindowList.py
##
## Purpose-
##       Golf: Main window
##
## Last change date-
##       2019/08/15
##
## Implementation notes-
##       TODO: Resolve ??_00001
##
##############################################################################
import sys

import atexit
import threading

#### PyQt5 ###################################################################
from PyQt5.QtCore    import *
from PyQt5.QtGui     import *
from PyQt5.QtWidgets import *

#### lib #####################################################################
from lib.Debug       import *
from lib.Utility     import *

#### Golf ####################################################################
from GolfApplet      import *

##############################################################################
## Internal data areas
##############################################################################
_HCDM        = True                 ## Hard Core Debug Mode

_threadLock  = threading.RLock()    ## Protects _callbackSet and _windowLists
_callbackSet = set()                ## The GLOBAL _Callback set
_windowLists = set()                ## The GLOBAL WindowList set

_registered  = False                ## True iff atexit registered
_terminated  = False                ## True iff atexit invoked

def _atexit():                      ## Termination indicator
    global _terminated              ## Needed here!! (Otherwise local variable)

    print('WindowList._atexit')
    _terminated = True

_logger = logger
def logger(*args, **kwargs):        ## Most logging not needed
   if _HCDM: _logger(*args, **kwargs)

##############################################################################
##
## Class-
##       _Window
##
## Purpose-
##       A Window container.
##
##############################################################################
class _Window():
    def __init__(self, window):     ## Constructor
        self.window = window

    def __eq__(self, that):
        if not isinstance(that, _Window): raise TypeError(str(type(that)))
        return self.window == that.window

    def __hash__(self):
        return self.window.__hash__()

##############################################################################
##
## Class-
##       _Callback
##
## Purpose-
##       Our callback pseudo-window
##
##############################################################################
class _Callback(_Window):
    def __init__(self, window):     ## Constructor
        super().__init__(window)

        logger('_Callback(%s).__init' % self)
        self.closer = window.close  ## The real window.close
        window.close = self.close   ## Replace the window's close method
        logger('old closer(%s) REPLACED' % self.closer)

    def __del__(self):
        logger('_Callback(%s).__del %s' % (self, _terminated))
        if _terminated: return      ## All bets are off

        with _threadLock:
            if self in _callbackSet:
                _logger('OOPS: _callbackSet[%s] PRESENT' % self)
                try:
                    self.close()
                except Exception as X:
                    logger('_Callback._del: Exception ignored: %s' % X)

    def __repr__(self):
        return '<_Callback(%s)>' % self.window

    def close(self):                ## Our close function
        logger('_Callback(%s).close>>> %s' % (self, _terminated))
        if _terminated: return      ## All bets are off

        with _threadLock:
            if self in _callbackSet:
                _callbackSet.remove(self) ## Remove self from _callbackSet
                logger('_callbackSet[%s] REMOVED' % self)
                self.window.close = self.closer ## Restore the real close function
                logger('old closer(%s) RESTORED' % self.closer)

                self.window.close() ## Invoke the real close function
            else:
                _logger('OOPS: _callbackSet[%s] MISSING' % self)
        logger('_Callback(%s).<<<close %s' % (self, _terminated))

##############################################################################
##
## Class-
##       WindowList
##
## Purpose-
##       The list of child windows (to be closed.)
##
##############################################################################
class WindowList(_Window):          ## The List of open windows
    def __init__(self, window):     ## WindowList(window)
        global _registered
        super().__init__(window)
        logger('WindowList(%s).__init(%s)' % (self, window))

        self._windowList = set()    ## The list of appended windows
        with _threadLock:
            if not _registered:
                atexit.register(_atexit)
                _registered = True

            if window in _windowLists: raise IndexError('[%s] Duplicate')
            _windowLists.add(self)

    def __del__(self):
        logger('WindowList(%s).__del %s' % (self, _terminated))
        if _terminated: return      ## All bets are off

        with _threadLock:
            if self in _windowLists:
                _logger('OOPS: _windowLists[%s] PRESENT' % self)
                self.remove()
            else:
                logger('ISOK: _windowLists[%s] MISSING' % self)

    def __repr__(self):
        return '<_WindowList(%s)>' % self.window

    def debug(self):                ## Debugging display
        debugf('WindowList(%s).debug %s' % (self, str(self._windowList)))
        with _threadLock:
            debugf('_callbackSet: %s' % str(_callbackSet))
            debugf('_windowLists: %s' % str(_windowLists))

    def static_debug(self):         ## Global debugging display
        debugf('WindowList(%s).static_debug' % self)
        with _threadLock:
            debugf('_callbackSet: %s' % str(_callbackSet))
            debugf('_windowLists: %s' % str(_windowLists))
            for windowList in _windowLists:
                debugf('[%s] %s' % (windowList, windowList._windowList))

    ##########################################################################
    def append(self, window):       ## Add window to dictionary
        logger('WindowList(%s).append(%s)' % (self, window))

        USE_00001 = False ## ??_00001 only occurs testing exception
        window_ = _Window(window)
        if USE_00001 and False: ## ??_00001 True/False different Exceptions
            window_ = window
        with _threadLock:
            if window_ in _callbackSet: raise KeyError('[%s] Duplicate' % window)
            _callbackSet.add(_Callback(window)) ## Add _Callback to callback set
            if USE_00001: ## ??_00001 (Differing exception types)
                print(' EXCEPTION EXPECTED ')
                if window_ in _callbackSet:
                    raise KeyError('[%s] Duplicate' % window)

        assert window_ not in self._windowList ## Or internal logic error
        self._windowList.add(window) ## Add window to sub-window set

    def close(self):                ## Remove all sub-windows
        logger('WindowList(%s).close>>>' % self)
        targetList = []
        for window in self._windowList:
            targetList += [window]
        self._windowList = set()
        for window in targetList:
            window.close()
        logger('WindowList(%s).<<<close' % self)

    def remove(self):               ## Remove ourself from _windowLists
        logger('WindowList(%s).remove>>>' % self)
        with _threadLock:
            if self in _windowLists:
                _windowLists.remove(self)
                logger('_windowLists[%s] REMOVED' % self)
            else:
                _logger('OOPS: _windowLists[%s] MISSING' % self)

            ## Remove this WindowList from any other WindowList._windowList
            for item in _windowLists:
                if self.window in item._windowList:
                   item._windowList.remove(self.window)
                   logger('ISOK: _windowLists[%s] REMOVED window[%s]'
                         % (item, self.window))
                   break            ## (It can only be in one!)
        self.close()
        logger('WindowList(%s).<<<remove' % self)

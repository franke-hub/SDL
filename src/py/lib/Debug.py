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
##       Debug.py
##
## Purpose-
##       Debugging utility.
##
## Last change date-
##       2021/03/30
##
## Implementation notes-
##       Do not override printf or writef method.
##
##############################################################################
from __future__ import print_function
import sys
import threading
import time
import traceback

from lib.Types import Bool, Integer, String

##############################################################################
## Define available imports
##############################################################################
__all__ = [ 'Debug', 'Logger', 'debugf', 'printf', 'tracef', 'writef' ]

##############################################################################
## Default modes
##############################################################################
_USE_APPEND = False                 ## Append Mode?
_USE_FLUSH  = False                 ## Flush Mode?
_USE_HCDM   = False                 ## Hard Core Debug Mode?

##############################################################################
## Static accessor methods
##
## These functions accept standard print arguments and keywords.
##
## debugf(...) Write to stdout and trace file.
## printf(...) Write to stdout. (Does not use _format.)
## tracef(...) Write to trace file.
## writef(...) Write to trace file. (Does not use _format.)
##
##############################################################################
def _get():
    return Debug.get()

def debugf(*args, **kwargs):
    _get().debugf(*args, **kwargs)

def printf(*args, **kwargs):
    _get().printf(*args, **kwargs)

def tracef(*args, **kwargs):
    _get().tracef(*args, **kwargs)

def writef(*args, **kwargs):
    _get().writef(*args, **kwargs)

##############################################################################
## Class Debug
##############################################################################
_debug_lock = threading.RLock()     ## The global debug lock
_singleton  = None                  ## The global debug singleton

class Debug(object):
    MODE_NORMAL = 0                 ## Log Format Mode (print mode)?
    MODE_LOGGER = 1                 ## Log Format Mode (include time)?
    MODE_LOGTNM = 2                 ## Log Format Mode (include time + thread)?

    def __init__(self, name='debug.out', append=False):
        global _singleton
        if not _singleton:
            with _debug_lock:
                if not _singleton:
                    _singleton = self

        self._APPEND = _USE_APPEND  ## Initial Append Mode
        self._FLUSH  = _USE_FLUSH   ## Initial Flush Mode
        self._HCDM   = _USE_HCDM    ## Initial Hard Core Debug Mode
        self._MODE   = self.MODE_NORMAL ## Initial Log format

        self._file = None           ## The trace file
        self._name = name           ## The trace file name
        if not append:
            ## FX_00002: NameError: 'open' is not defined can occur here too
            try:
                self._open('wb')
            except Exception as X:
                print('open(%s) Exception ignored: %s' % (name, X)
                     , file=sys.stderr)
                print('  This can occur normally during termination'
                     , file=sys.stderr)

    if sys.version_info[0] >= 3:
        def __del__(self):
            global _singleton
            self._close()
            if _singleton is self:
                _singleton = None

    def _close(self):
        with _debug_lock:
            if self._file:
                self._file.close()
            self._file = None

    def debugf(self, *args, **kwargs):
        M = self._format(*args)
        self.printf(M, **kwargs)
        self.writef(M, **kwargs)

    def errorf(self, *args, **kwargs):
        M = self._format(*args)
        self.printf(M, **kwargs, file=sys.stderr)
        self.writef(M, **kwargs)

    def flush(self):
        with _debug_lock:
            if self._file:
                self._file.flush()
            sys.stdout.flush()

    def _format_log(self, *args):   ## Logger style formatting
        M = '%.3f ' % time.time()
        if self._MODE > self.MODE_LOGGER:
            M = M + '[%-12s] ' % threading.current_thread().name
        M = M + ' '.join(str(arg) for arg in args)
        return M

    def _format_prt(self, *args):   ## Print style formatting
        return ' '.join(str(arg) for arg in args)

    def _format(self, *args):
        if self._MODE:
            return self._format_log(*args)
        return self._format_prt(*args)

    @staticmethod
    def get():
        global _singleton
        if not _singleton:
            with _debug_lock:
                if not _singleton:
                    _singleton = Debug()
        return _singleton

    @staticmethod
    def _get():
        return _singleton

    @staticmethod
    def handle_exception():
        with _debug_lock:
            x_type, x_value, x_tb = sys.exc_info()
            traceback.print_exception(x_type, x_value, x_tb)

    @staticmethod
    def lock():
        return _debug_lock

    def _open(self, mode):
        with _debug_lock:
            self._close()
            self._file = open(self._name, mode)

    def printf(self, *args, **kwargs):
        flush = kwargs.pop('flush', False) or self._FLUSH or self._HCDM
        print(' '.join(str(arg) for arg in args), **kwargs)
        if flush:
            sys.stdout.flush()

    @staticmethod
    def set(default):
        global _singleton
        with _debug_lock:
            result = _singleton
            if default != None:
                assert isinstance(default, Debug), 'Invalid type'
            _singleton = default
        return result

    def set_opt(self, name, value=True):
        name = name.upper()
        if name == 'MODE':
            self._MODE = int(value)
            return

        if not isinstance(value, bool):
            raise ValueError('bool(%s(%s))' % (type(value), value))
        if name == 'APPEND':
            self._APPEND = value
            self._close()
        elif name == 'FLUSH':
            self._FLUSH = value
        elif name == 'HCDM':
            self._HCDM = value
        else:
            raise KeyError('set_opt(%s)' % name)

    def tracef(self, *args, **kwargs):
        M = self._format(*args)
        self.writef(M, **kwargs)

    def _writef(self, *args, **kwargs): ## FX_00002: (Renamed from writef)
        data = ' '.join([str(item) for item in args])
        data += kwargs.get('end', '\n')
        with _debug_lock:
            if not self._file:
                self._open('ab')

            self._file.write(data.encode())
            if self._APPEND or self._HCDM:
                self._close()
            else:
                flush = kwargs.get('flush', False) or self._FLUSH
                if flush:
                    self._file.flush()

    ##########################################################################
    ## FX_00002: So far, 'print' has still been available
    def writef(self, *args, **kwargs): ## FX_00002 (Added try/except wrapper)
        try:
            self._writef(*args, **kwargs)
        except Exception as X:
            try:
                if sys.stderr:      ## TODO: Is this always True?
                    print(*args, **kwargs, file=sys.stderr)
                else:
                    print('Debug.writef exception: %s' % X)
            except:
                raise X

##############################################################################
## Class Logger: Extends debugf/tracef to include time and optionally thread.
##############################################################################
class Logger(Debug):
    def __init__(self, name='debug.log', append=False):
        super(Logger, self).__init__(name=name, append=append)

    def _format(self, *args):
        return self._format_log(*args)

#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2017-2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Test_Howto.py
##
## Purpose-
##       "How to" tests.
##
## Last change date-
##       2018/01/01
##
##############################################################################
from __future__ import print_function

import sys
import threading
import time

from lib.Command import command
from lib.Debug import *

##############################################################################
## Local controls
##############################################################################
_howto = {}                         ## The "How to" test list dictionary
error_count = 0                     ## Error counter

def format_count(error_count):
    if error_count:
        return "{} Error{} encountered".format(error_count, "" if error_count == 1 else "s")

    return "No errors encountered"

##############################################################################
## __PrintFile class: Print out a small file in source directory.
##
## Note: Run from object directory, where S links to source directory
##############################################################################
class __PrintFile:
    @staticmethod
    def run(argv):
        file_name = "S/Command.py"
        file_name = "S/NoSuchFile.py" ## Don't print, raise an exception
        try:
            with open(file_name, "r") as file:
                for line in file:
                    debugf(line, end="") ## Line already ends with '/n'
        except FileNotFoundError:
            debugf("Missing file:", file_name)

        return 0

_howto['print-file'] = __PrintFile

##############################################################################
## __SuperClass class: Access super classes from derived class.
## Notes: self.method, used in __init__, always accesses the created class
##   (All class methods are in place before any __init__ is invoked.)
##############################################################################
class _Base(object):
    def __init__(self, *args, **kwargs):
        self.method("_Base.__init__", *args, **kwargs)

    def method(self, *args, **kwargs):
        debugf("_Base.method", *args, **kwargs)
        return 0

    ### This only works if called from an _Upper instance
    def dont_try_this_at_home(self, *args, **kwargs):
        super(_Upper, self).method("Don't try this at home", *args, **kwargs)
        return 0

class _Lower(_Base):
    def __init__(self, *args, **kwargs):
        self.method("_Lower.__init__", *args, **kwargs)
        super(_Lower, self).__init__(*args, **kwargs)

    def method(self, *args, **kwargs):
        debugf("_Lower.method", *args, **kwargs)
        assert super(_Lower, self).method(*args, **kwargs) == 0, "Didn't call _Base.method"
        return 1

class _Upper(_Lower):
    def __init__(self, *args, **kwargs):
        super(_Upper, self).__init__(*args, **kwargs)
        self.method("_Upper.__init__", *args, **kwargs)

    def method(self, *args, **kwargs):
        debugf("_Upper.method", *args, **kwargs)
        return 2

class __SuperClass:
    @staticmethod
    def run(argv):
        obj = _Upper("Creating _Upper")
        assert obj.method("alpha") == 2, "Didn't call _Upper.method"
        assert super(_Upper,obj).method("beta") == 1, "Didn't call _Lower.method"
        assert super(_Lower,obj).method("gamma") == 0, "Didn't call _Base.method"

        try:                        ## Mistake demo
            obj.dont_try_this_at_home("or you'll be sorry you did!")
            jbo = _Lower("Creating _Lower")
            debugf("About to try this at home improperly.")
            jbo.dont_try_this_at_home("or you'll be sorry you did!")
        except Exception as e:
            debugf("Told ya!", e)

        try:                        ## Mistake demo
            debugf("The next statement fails:")
            super(_Base,obj).method("delta") ## This fails, because
        except Exception as e:      ## super(_Base,obj) is the base object,
            debugf("Told ya!", e)

        return 0

_howto['super-class'] = __SuperClass

##############################################################################
## __UseThread class: Create and run a thread
##############################################################################
class _TestThread(threading.Thread):
    def __init__(self, *args, **kwargs):
        super(_TestThread, self).__init__()
        debugf("_TestThread.__init__", *args, **kwargs)
        self.event = threading.Event()
        self.start()
        self.event.wait()
        del self.event

    def run(self):
        self.event.set()

        debugf("_TestThread.run...")
        time.sleep(1.5)
        debugf("...run._TestThread")

class __UseThread:
    @staticmethod
    def run(argv):
        _thread = _TestThread()
        for i in range(5):
            debugf("iteration:", i)
            time.sleep(0.5)
        _thread.join()
        del _thread

        return 0

_howto['use-thread'] = __UseThread

##############################################################################
## _UseTimers class: Create and run a timer Thread
##     The _TEST_TIMERS_LOCK insures that either owner._event.set() or
##     that.cancel() will be called, but not both of them.
##############################################################################
_TEST_TIMERS_LOCK = threading.Lock() ## Some implementations need RLock()
##                                  ## to allow multiple lock usages.

def _TestTimers_done(that):
    with _TEST_TIMERS_LOCK:
        owner = that.owner
        that.owner = None
        if owner: owner._event.set()
    debugf("_TestTimers_done when(%f)" % that.when)

class _TestTimers(object):
    def __init__(self, owner, when, *args, **kwargs):
        debugf("_TestTimers.__init__", when)

        ## if version < 3.3, we have to base on object rather than extend
        ## threading.Timer. For compatability, we always base on object.
        self.that = threading.Timer(when, _TestTimers_done, [self])

        self.owner = owner
        self.when  = time.time() + when
        self.that.start()

    def cancel(self):
        with _TEST_TIMERS_LOCK:
            owner = self.owner
            self.owner = None
            if owner: self.that.cancel()

def _wait4(that, delay):
    debugf("Wait %.3f seconds..." % delay)
    t = _TestTimers(that, delay)
    if False:
        that._event.wait()
        debugf("...Wait %.3f seconds" % delay)
        that._event.clear()
    return t

class _UseTimers:
    def main(self):
        self._event = threading.Event() ## Normally created in __init__

        _wait4(self, 1.25)
        _wait4(self, 2.50)
        t = _wait4(self, 1.00)
        _wait4(self, 1.00)
        t.cancel()
        time.sleep(5.0)

    @staticmethod
    def run(argv):
        debug = Debug._get()
        Debug.set(Logger())

        obj = _UseTimers()
        obj.main()

        Debug.set(debug)
        return 0

_howto['use-timers'] = _UseTimers

##############################################################################
## __Command class
##############################################################################
class __Command:
    @staticmethod
    def run(argv):
        global error_count

        if len(argv) > 1:
            name = argv[1]
            argv = argv[1:]
            try:
                error_count += _howto[name].run(argv)
            except KeyError:
                error_count += 1
                debugf("No How To:" , name)
            except:
                error_count += 1
                Debug.handle_exception()
        else:
            for name in _howto:
                try:
                    error_count += _howto[name].run([name])
                except:
                    error_count += 1
                    Debug.handle_exception()
                debugf("")

        debugf(format_count(error_count))
        return 0

command['howto'] = __Command

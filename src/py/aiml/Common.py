#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2016-2023 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Common.py
##
## Purpose-
##       AIML: Common data area.
##
## Last change date-
##       2023/08/17
##
##############################################################################
import threading
import time

from lib.Control import control
from lib.Debug import *
from lib.Dispatch import OBJ, TAB

##############################################################################
## Internal functions
##############################################################################
def _debugf(*args, **kwargs):
    M = Debug.get()._format_log(*args)
    printf(M, **kwargs)
    writef(M, **kwargs)

def _tracef(*args, **kwargs):
    M = Debug.get()._format_log(*args)
    writef(M, **kwargs)

def _thread_exception(_where, _thread, _op, x): ## FX_00001
    _where = "!!ERROR!! " + _where
    _debugf(_where, "Thread(%s).%s() Exception: %s" % (_thread.name, _op, x))

##############################################################################
## Module Common: The Common singleton
##############################################################################
_event = threading.Event()          ## Termination event
_service = {}                       ## Service dictionary
_threads = []                       ## List of (stopable) threads

dispatcher = None                   ## The Common Dispatcher

def aiml_transmit(sess, mess):
    from AimlServer import transmit
    return transmit(sess, mess)

def add_service(name, tab):
    """Add a ServiceTAB to the list of services."""
    global _service

    assert name not in _service, "add_service duplicate(%s)" % name
    assert isinstance(tab, TAB), "add_service tab(%s)" % type(tab)
    _service[name] = tab

def add_thread(thread):
    """Add a Thread to the managed list.
    A managed Thread must provide the 'stop' method.
    """
    global _threads

    assert isinstance(thread, threading.Thread)
    assert callable(getattr(thread, 'stop')), "Thread must have 'stop' method"
    _threads += [thread]

def del_thread(thread):
    """Remove a Thread from the managed list."""
    try:
        _threads.remove(thread)
    except ValueError:
        debugf("del_thread(%s) ValueError" % thread.name)
    except: ## FX_00001
        Debug.handle_exception()

def enqueue(tab, uow):
    """Shortcut for Common.dispatch.enqueue."""
    dispatcher.enqueue(tab, uow)

def get_service(name):
    """Locate a named ServiceTAB.
    Returns None if the service is not defined.
    """
    return _service.get(name, None)

def join():
    """Wait for all managed threads to complete"""
    global _threads

    _event.wait()
    start_time = time.time()
    alive_time = start_time
    while len(_threads) > 0:
        alert_mess = False
        final_mess = False
        now = time.time()
        if now - start_time > 180.0:
            final_mess = True
        elif now - alive_time > 15.0:
            alert_mess = True
            alive_time = now
        thread_list = _threads[:]
        for thread in thread_list:
            if thread.is_alive():
                try:
                    thread.join(0.125)
                except Exception as x: ## FX_00001
                    _thread_exception("Common.join:", thread, "join", x)
                    Debug.handle_exception()
                    del_thread(thread)
        thread_list = _threads[:]
        for thread in thread_list:
            if thread.is_alive():
                if alert_mess:
                    _debugf("Common.join, thread(%s) still running" % thread.name)
                elif final_mess:
                    _debugf("Common.join, thread(%s) ABANDONED" % thread.name)
            else:
                del_thread(thread)
        if final_mess: break

def stop():
    """Common termination"""

    aiml_transmit('*', '.QUIT')     ## Terminate any active AIML session

    ## Terminate all threads
    thread_list = _threads[:]
    for thread in thread_list:
        try:
            thread.stop()
        except Exception as x: ## FX_00001
            _thread_exception("Common.stop:", thread, "stop", x)
            Debug.handle_exception()

    _event.set()

## (Moved to Main.py)
## dispatcher = OBJ()                  ## The Dispatcher
## add_thread(dispatcher)              ## (The dispatcher is a managed Thread)

#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2016-2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Alarm.py
##
## Purpose-
##       Brian AI: Persistent alarm.
##
## Last change date-
##       2018/01/01
##
##############################################################################
import sys
import pickle
import threading
import time

from lib.Command import command
from lib.Debug import Debug, Logger, debugf, tracef
from lib.Dispatch import TAB, UOW
import Common
from PersistFile import replace, restore

##############################################################################
## _logger: Log "ShouldNotOccur" conditions
##############################################################################
def _logger(*args, **kwargs):
    with Debug.lock():
        logger = Logger("log/exception.log", append=True)
        M = " ".join(str(arg) for arg in args)
        logger.debugf(time.asctime(), "Alarm." + M, **kwargs)
        del logger

##############################################################################
## _AlarmTAB: Handle a single alarm
##############################################################################
_alarm_file = "_temp/alarm-data.pickle"
_alarm_lock = threading.RLock()

class _AlarmTAB(TAB):
    def __init__(self, when, what, *args, **kwargs):
        super(_AlarmTAB, self).__init__(*args, **kwargs)

        self.when = when
        self.what = what

    @classmethod
    def load(cls, attributes):
        obj = cls.__new__(cls)
        when = attributes['when']
        what = attributes['what']
        self.__init__(when, what)
        return obj

    def save(self):
        return (self.__class__, {'when':when, 'what':what})

    def cancel(self):
        return
        raise NotImplementedError('_AlarmTAB.cancel')

    def work(self, uow):
        uow.done()
        return
        raise NotImplementedError('_AlarmTAB.work')

##############################################################################
## _AlarmThread: Alarm controller
##############################################################################
class _AlarmThread(threading.Thread):
    def __init__(self):
        super(_AlarmThread, self).__init__()
        self.name = "AlarmThread"

        self.alarms = []
        try:
            data = restore(_alarm_file)
            self.alarms = pickle.loads(data)
        except FileNotFoundError:
            pass
        except Exception as x:
            _logger("__init__() invalid alarm-data:", x)

        self.event = threading.Event()
        self.start()

    def run(self):
        Common.add_thread(self)

        self.event.wait()
        with _alarm_lock:
            data = pickle.dumps(self.alarms)
            replace(_alarm_file, data)
            for alarm in self.alarms:
                alarm.cancel()

        Common.del_thread(self)

    def stop(self):
        self.event.set()

##############################################################################
## __Command class: Set an alarm
##############################################################################
class __Command:
    ## _thread = _AlarmThread()     ## Create the _AlarmThread

    @staticmethod
    def run(argv):
        raise NotImplementedError('alarm')
        return 0

command['alarm'] = __Command

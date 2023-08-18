#!/usr/bin/env python
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
##       UserServer.py (From Console.py)
##
## Purpose-
##       Manage operator console
##
## Last change date-
##       2021/04/03
##
##############################################################################
from __future__ import print_function

import readline
import sys
import threading
import time

from lib.Command import command, Command
from lib.Console import Console
from lib.Control import control
from lib.Debug import Debug, debugf, printf, tracef
from lib.Dispatch import TAB, UOW, WDW
from lib.Utility import visify
import Common

##############################################################################
## Define available imports
##############################################################################
__all__ = None

##############################################################################
## Controls
##############################################################################
_MULTI_COMMAND = False              ## Allow multiple simultaneous commands?

##############################################################################
## Class _UserServerTAB
##
## With only one _UserServerTAB, commands will be processed one at a time.
## Commands can still be read, but they will be queued for processing on the
## single TAB. The _MULTI_COMMAND control variable, if True, creates a new TAB
## for each command. These commands then run in parallel.
##############################################################################
class _UserServerTAB(TAB):
    def __init__(self):
        ## global command
        super(_UserServerTAB, self).__init__()
        self.command = Command(command)

    def work(self, uow):
        self.command.work(uow)
        return

class _Command:                    ## control['command']('any command string')
    _tab = _UserServerTAB()

    @staticmethod
    def run(inp):
        uow = UOW()
        uow.work = inp
        Common.enqueue(_tab, uow)

control['command'] = _Command.run
control['command'] = None

##############################################################################
## Class _HandleAIML
##
## This class routes input messages to the AIML server.
##############################################################################
class _HandleAIML:
    @staticmethod
    def work(inp):
        if inp.upper() == ".EXIT":
            print("Mode: USER")
            control['Console'] = _HandleUSER
        elif inp.upper() == ".QUIT":
            _HandleUSER.work('.quit')
        else:
            resp = Common.aiml_transmit('Default/Console', inp)
            debugf(resp)

class __AIML_Command:
    @staticmethod
    def run(argv):
        print("Mode: AIML")
        control['Console'] = _HandleAIML
        return 0

command['aiml'] = __AIML_Command

##############################################################################
## Class _HandleUSER
##
## This class routes commands to the _UserServerTAB.
##############################################################################
class _HandleUSER:
    tab = _UserServerTAB()

    @staticmethod
    def work(inp):
        uow = UOW()
        uow.work = inp
        if _MULTI_COMMAND:
            Common.enqueue(_UserServerTAB(), uow) ## Each command gets its own TAB
            time.sleep(.25)
        else:
            _HandleUSER.tab.work(uow) ## Synchronous

control['Console'] = _HandleUSER

##############################################################################
## Add Console to the list of controlled threads
##############################################################################
if False:
    Common.add_thread(Console())

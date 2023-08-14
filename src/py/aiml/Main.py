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
##       Main.py
##
## Purpose-
##       AIML controller.
##
## Last change date-
##       2021/04/03
##
## Usage-
##       ./Main.py
##
##############################################################################
from __future__ import print_function

import os
import sys
import threading
import time

from lib.Command import command
from lib.Control import control
from lib.Debug import Debug, debugf, tracef
import Common
from Config import PROGRAM, VERSION

##############################################################################
## __Echo_Command class (Built-in ECHO command)
##############################################################################
class __Echo_Command:
    @staticmethod
    def run(argv):
        debugf("%s: %s" % (argv[0], " ".join(argv[1:])))
        return 0

command['echo'] = __Echo_Command

##############################################################################
## __Quit_Command class (Built-in QUIT command)
##############################################################################
class __Quit_Command:
    @staticmethod
    def run(argv):
        Common.stop()
        return 0

command['.quit'] = __Quit_Command   ## Adds built-in ".quit" to command list
command['shutdown'] = __Quit_Command ## Delayed quit

##############################################################################
## __Raise_Command class (Built-in THROW command)
##############################################################################
class __Raise_Command:
    @staticmethod
    def run(argv):
        code = "raise command"
        if len(argv) > 1:
            code = " ".join(argv[1:])
        raise Exception(code)

command['raise'] = __Raise_Command

##############################################################################
## Mainline code
##############################################################################
if __name__ == "__main__":
    try:
        debug = Debug("debug.out", append=True)
        tracef("\n")
        tracef(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        tracef(">> RESTART >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        tracef(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        debug.set_opt("append", True) ## Close log after each write
        if False:                   ## Use logger format?
            debug._format = debug._format_log
        now = time.time()
        debugf("%.3f" % now, time.asctime(time.localtime(now)),
               PROGRAM +"/"+ VERSION, "Started")

        import Imports
        Common.join()
        if False:
            debugf("Active threads:")
            for thread in threading.enumerate():
                debugf("{}".format(thread))

    except KeyboardInterrupt:
        debugf("\n%.3f Ctrl-C" % time.time())
        Common.stop()
        try:
            common.join()
        except KeyboardInterrupt:
            debugf("%.3f .... Quit ...." % time.time())
            del debug
            sys.exit()

    except:
        Debug.handle_exception()
        os.kill(os.getpid(), 9)

    debugf("%.3f .... Done ...." % time.time())

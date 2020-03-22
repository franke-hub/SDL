#!/bin/python3
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
##       Main.py
##
## Purpose-
##       Brian AI controller.
##
## Last change date-
##       2018/01/01
##
## Usage-
##       ./Main.py
##
##############################################################################
import os
import sys
import threading
import time

from lib.Command import command
from lib.Debug import *
import Common
import Config
from Console import Console

_debugf = Common._debugf

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

command['.quit'] =    __Quit_Command ## Immediate quit Console built-in
command['shutdown'] = __Quit_Command ## Delayed quit

##############################################################################
## __Raise_Command class (Built-in THROW command)
##############################################################################
class __Raise_Command:
    @staticmethod
    def run(argv):
        raise Exception("raise command")

command['raise'] = __Raise_Command

##############################################################################
## Mainline code
##############################################################################
if __name__ == "__main__":
    try:
        debug = Debug("debug.out", append=True)
        debug.set_opt('flush', True)
        now = time.time()
        if True:                    ## Use logger format?
            tracef("\n\n")
            debug._format = debug._format_log
            debug.set_opt("ltnm")
        _debugf(time.asctime(time.localtime(now)),
                Config.PROGRAM +"/"+ Config.VERSION, "Started")

        import Imports
        Console()
        Common.join()
        if False:
            debugf("Active threads:")
            for thread in threading.enumerate():
                debugf("{}".format(thread))

    except KeyboardInterrupt:
        print("")
        _debugf("Ctrl-C")
        Common.stop()
        try:
            Common.join()
        except KeyboardInterrupt:
            _debugf(".... Quit ....")
            del debug
            sys.exit()

    except:
        Debug.handle_exception()
        os.kill(os.getpid(), 9)

    _debugf(".... Done ....")


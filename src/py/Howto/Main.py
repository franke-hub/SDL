#!/bin/python3
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
##       Command processor.
##
## Last change date-
##       2021/04/03
##
## Usage-
##       ./Main.py
##
##############################################################################
import importlib
import os
import sys
import threading
import time

from lib.Command import command
from lib.Debug import *
from lib.Dispatch import OBJ
from lib.Console import Console

##############################################################################
## Demonstrate override Console.getch, Console.putch
##############################################################################
if False:
    def getch():                    ## (Echos)
        return sys.stdin.read(1)

    def putch(c):                   ## (getch echos, so putch shouldn't)
        pass

    import lib.Console as Override
    Override.getch = getch
    Override.putch = putch

##############################################################################
## Constants
##############################################################################
__PROGRAM = 'Howto'
__VERSION = '0.0.0'

##############################################################################
## Internal functions
##############################################################################
def _debugf(*args, **kwargs):
    M = ' '.join(str(arg) for arg in args)
    printf(M, **kwargs)
    writef(M, **kwargs)

##############################################################################
## __Disp_Command class (Built-in DISP command)
##############################################################################
_Disp_Command_obj = None
class __Disp_Command:
    @staticmethod
    def run(argv):
        global _Disp_Command_obj
        if _Disp_Command_obj == None:
            _Disp_Command_obj = OBJ()
        return 0

command['disp'] = __Disp_Command

##############################################################################
## __Echo_Command class (Built-in ECHO command)
##############################################################################
class __Echo_Command:
    @staticmethod
    def run(argv):
        _debugf('%s: %s' % (argv[0], ' '.join(argv[1:])))
        return 0

command['echo'] = __Echo_Command

##############################################################################
## __Quit_Command class (Built-in QUIT command)
##############################################################################
class __Quit_Command:
    @staticmethod
    def run(argv):
        _console.stop()

        obj = OBJ.get()
        if obj:
            obj.stop()

        return 0

command['.quit'] =    __Quit_Command ## Immediate quit Console built-in
command['shutdown'] = __Quit_Command ## Delayed quit

##############################################################################
## __Sleep_Command class (Built-in SLEEP command)
##############################################################################
class __Sleep_Command:
    @staticmethod
    def run(argv):
        delay = 10.0
        if len(argv) >= 2:
            delay = int(argv[1])
        debugf('sleep %s...' % (delay))
        time.sleep(delay)
        debugf('...sleep %s' % (delay))
        return 0

command['sleep'] = __Sleep_Command  ## Sleep command

##############################################################################
## __Load_Command class (Command Loader)
##############################################################################
class __Load_Command:
    @staticmethod
    def run(argv):
        importlib.import_module(argv[1])

command['load'] = __Load_Command

##############################################################################
## Mainline code
##############################################################################
if __name__ == '__main__':
    try:
        debug = Debug('debug.out', append=True)
        debug.set_opt('flush', True)
        now = time.time()
        if True:                    ## Use logger format?
            tracef('\n\n')
            debug._format = debug._format_log
            debug.set_opt('MODE', Debug.MODE_LOGTNM)
        debugf(time.asctime(time.localtime(now)),
               __PROGRAM +' '+ __VERSION, 'Started')

        _console = Console()
        Console._MULTI_COMMAND = True
        _console.join()
        if False:
            debugf('Active threads:')
            for thread in threading.enumerate():
                debugf('{}'.format(thread))

    except KeyboardInterrupt:
        print('')
        debugf('Ctrl-C')
        _console.stop()
        try:
            _console.join()
        except KeyboardInterrupt:
            debugf('.... Quit ....')
            del debug
            sys.exit()

    except:
        Debug.handle_exception()
        os.kill(os.getpid(), 9)

    ##########################################################################
    ## Termination
    ##########################################################################
    obj = OBJ.get()                 ## Terminate Dispatch, if present
    if obj:
        obj.stop()
        obj.join()
    debugf('.... Done ....')

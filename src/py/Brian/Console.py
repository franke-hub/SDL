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
##       Console.py
##
## Purpose-
##       Brian AI: Manage operator console
##
## Last change date-
##       2018/01/01
##
## Usage notes[backspace handling]-
##       The Console should be run from a terminal which handles the backspace
##       character itself, so that readline does not return it.
##
##       If it isn't, sys.stdin.readline echos the backspace and passes it
##       through to be interpreted. While we correctly remove these backspaces
##       the resulting line will not be displayed correctly if that echoing
##       overwrites the prompt.
##
##       The stdio assembler package provides versions of getch and putch
##       which do not present this problem. The getch method does not echo
##       and all characters, including backspaces, are returned as received.
##       We don't echo backspaces past the original position.
##
## Implementation notes-
##       Neither sys.stdin.readline nor sys.stdin.read(1) returns any input
##       until an end-of-line  character is received. Both of these routines
##       also handle character echoing.
##
##############################################################################
import readline
import sys
import threading
import time

from lib.Command import Command, command
from lib.Debug import *
from lib.Dispatch import TAB, UOW
from lib.Utility import visify
import Common

##############################################################################
## Define available imports
##############################################################################
__all__ = ['Console']

##############################################################################
## Controls
##############################################################################
_MULTI_COMMAND = False              ## Allow multiple simultaneous commands?

##############################################################################
## Import getch/putch (With internal default implementations.)
##############################################################################
try:
    raise ImportError("stdio multithreading conflict with http")
    from stdio import getch, putch
except ImportError:
    print("Warning: stdio not available")
    def getch(): ## FAKE getch (because it echos)
        return sys.stdin.read(1)

    def putch(c): ## FAKE putch (because echo already done)
        pass

##############################################################################
## Class _ConsoleTAB
##
## With only one _ConsoleTAB, commands will be processed one at a time.
## Commands can still be read, but they will be queued for processing on the
## single TAB. The _MULTI_COMMAND control variable, if True, creates a new TAB
## for each command. These commands then run in parallel.
##############################################################################
class _ConsoleTAB(TAB):
    def work(self, uow):
        work = uow.work
        if isinstance(work, str):
            if work == "":
                uow.done(None)
                return

            argv = tuple(work.split())
            work = argv[0]

            try:
                Name = command[work]
            except KeyError:
                debugf("Invalid command: '%s'" % visify(work))
                debugf("Valid commands: %s" % Command._get_list(command))
                uow.done(UOW.CC_ERROR)
                return

            try:
                uow.done(Name.run(argv))
                return
            except:
                Debug.handle_exception()

        uow.done(UOW.CC_ERROR)
        return

##############################################################################
## Class Console
##############################################################################
class Console(threading.Thread):
    __singleton = None

    def __init__(self, *args, **kwargs):
        assert Console.__singleton == None, "Only one console thread allowed"
        Console.__singleton == self

        threading.Thread.__init__(self,name="Console",*args,**kwargs)
        self.name = 'Console'
        self.tab = _ConsoleTAB()

        self.operational = True
        self.start()

    ##########################################################################
    ## This method allows for testing of different input mechanisms.
    ## Note: With Python, the last defined method is used
    def __inp(self):                ## Method[1]: sys.std.readline()
        import lib.Utility
        return lib.Utility.unbs(sys.stdin.readline())

    def __inp(self):                ## Method[2]: Character by character input
        s = ""
        while self.operational:
            c = getch()
            if c == '\b':
                if len(s) > 0:
                    putch('\b')     ## Echo character delete sequence
                    putch(' ')
                    putch('\b')
                    s = s[:-1]
            elif ord(c) == 21:      ## Ctrl-U
                L = len(s)
                for _ in range(L): putch('\b')
                for _ in range(L): putch(' ')
                for _ in range(L): putch('\b')
                s = ""
            else:
                if c == '\t': c = ' '

                putch(c) ## Echo the character
                if c == '\n':
                    break
                s += c

        return s

    def run(self):
        """Operate the Console Thread"""
        Common.add_thread(self)
        while self.operational:
            try:
                printf("%.3f >>>> " % time.time(), end="", flush=True)
                s = self.__inp().strip()
                writef("%.3f >>>> %s" % (time.time(), s))

                uow = UOW()
                uow.work = s
                tab = self.tab
                if _MULTI_COMMAND:
                    tab = _ConsoleTAB()
                Common.enqueue(tab, uow)
                time.sleep(.25)

                if s == '.quit':
                    break

            except Exception as e:
                Debug.handle_exception()
                break

        Common.del_thread(self)
        debugf("Console terminated")

    def stop(self):
        """Terminate the Console Thread"""
        self.operational = False


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
##       UserServer.py (From Console.py)
##
## Purpose-
##       Manage operator console
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
from __future__ import print_function

import readline
import sys
import threading
import time

from lib.Command import command, Command
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
_USE_HISTORY_DEBUG = False          ## History debugging active?

##############################################################################
## Constants
##############################################################################
_ESC = 27                           ## The escape character

##############################################################################
## Function _tracef: tracef that can easily be turned off
##############################################################################
if _USE_HISTORY_DEBUG:
    debugf("WARNING: UserServer.py debugging traces active")
    def _tracef(*args, **kwargs):
        tracef(*args, **kwargs)
else:
    def _tracef(*args, **kwargs):
        pass

##############################################################################
## Import stdio.getch/putch (With internal default implementations.)
##############################################################################
_FAKE_STDIO = True
try:
    raise ImportError("Multi-threading conflict with HttpServer")
    from stdio import getch, putch

    def outch(s): pass
    _FAKE_STDIO = False
except ImportError:
    debugf("Warning: stdio not available")

    def getch(): ## FAKE getch (because it echos)
        return sys.stdin.read(1)

    def putch(c): ## FAKE putch (because echo already done)
        pass

    def outch(s):
        _tracef("outch(%s)" % visify(s))
        sys.stdout.write(s)

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

##############################################################################
## Class _HandleAIML
##
## This class routes input messages to the AIML server.
##############################################################################
class _HandleAIML:
    @staticmethod
    def work(inp):
        resp = Common.aiml_transmit('Default/Console', inp)

        if inp.upper() == ".EXIT":
            print("Mode: USER")
            control['user-server'] = _HandleUSER
        elif inp.upper() != ".QUIT":
            debugf(resp)

class __AIML_Command:
    @staticmethod
    def run(argv):
        print("Mode: AIML")
        control['user-server'] = _HandleAIML
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
        if inp.upper() == ".QUIT":
            return

        uow = UOW()
        uow.work = inp
        if _MULTI_COMMAND:
            Common.enqueue(_UserServerTAB(), uow) ## Each command gets its own TAB
            time.sleep(.25)
        else:
            _HandleUSER.tab.work(uow) ## Synchronous

control['user-server'] = _HandleUSER

##############################################################################
## Class _UserServer
##############################################################################
if _USE_HISTORY_DEBUG:
    class __HIST_Command:
        @staticmethod
        def run(argv):
            server = _UserServer.get()
            debugf(server.h_index, server.history)
            return 0

    command['.hist'] = __HIST_Command

class _UserServer(threading.Thread):
    __singleton = None

    @staticmethod                   ## TODO: REMOVE: Only used in debugging
    def get():
        return _UserServer.__singleton

    def __init__(self,*args,**kwargs):
        assert _UserServer.__singleton == None, "Only one console thread allowed"
        threading.Thread.__init__(self,name="_UserServer",*args,**kwargs)
        _UserServer.__singleton = self
        _UserServer.get()

        self.tab = _UserServerTAB()
        self.h_index = 1            ## Current history line index
        self.history = [""]         ## Input line history

        Common.add_thread(self)
        self.operational = True
        self.start()

    def _ctrl_u(self, s):           ## Clear a line
        _tracef("_ctrl_u[1] '{}'".format(s))
        for c in s:
            putch('\b')
            putch(' ')
            putch('\b')

    ##########################################################################
    ## Duplicate method defintions allow testing of different mechanisms.
    ## Note: With Python, the last defined method is used
    def __inp(self):                ## Method[0]: sys.std.readline()
        import lib.Utility
        return lib.Utility.unbs(sys.stdin.readline())

    ##########################################################################
    ## The up/down arrow processing is not fully debugged.
    ## This remains until debugging is complete.
    def _debug_history(self, history, s):
        if _USE_HISTORY_DEBUG:
            debugf("history({}) h_index({}) history{} s({})".format(history, self.h_index, self.history, s))

    def __inp(self):                ## Method[1]: Character by character input
        s = ""
        history = self.h_index      ## Current history position
        escape = False              ## Not in escape mode
        while self.operational:
            c = getch()

            if ord(c) == _ESC:      ## Escape (sequence)
                c = getch()
                if c == '[':
                    c = getch()
                    if c == 'A':    ## UP arrow
                        escape = True
                        self._ctrl_u(s)

                        if s.strip() != "":
                            history += 1
                            if history > len(self.history):
                                history = len(self.history)

                        s = self.history[-history]
                        for c in s: putch(c)
                        self._debug_history(history, s)
                        continue

                    if c == 'B':    ## DOWN arrow
                        escape = True
                        self._ctrl_u(s)

                        history -= 1
                        if history < 1:
                            history = 1

                        s = self.history[-history]
                        for c in s: putch(c)
                        self._debug_history(history, s)
                        continue

                    _tracef("\\e[:", ord(c), c)
                    putch(chr(_ESC))
                    putch('[')
                    s += chr(_ESC) + '['
                else:
                    _tracef("\\e:", ord(c), c)
                    putch(chr(_ESC))
                    s += chr(_ESC)

            if c == '\b':           ## BackSpace
                if len(s) > 0:      ## Do not backspace past origin
                    putch('\b')     ## Echo character delete sequence
                    putch(' ')
                    putch('\b')
                    s = s[:-1]
            elif ord(c) == 21:      ## Ctrl-U
                self._ctrl_u(s)
                s = ""
            elif c == '\n':         ## NewLine
                if _FAKE_STDIO and escape:
                    escape = False
                    outch('\n' + s + '\r')
                    continue

                putch('\n')         ## Echo the character

                ## Update history
                self._debug_history(history, s)
                if s != self.history[-history]:
                    if s.strip() != "":
                        self.history += [ s ]
                        if len(self.history) > 8:
                            self.history = self.history[1:]
                    self.h_index = 1
                else:
                    self.h_index = history
                break
            else:
                if c == '\t': c = ' '
                putch(c)            ## Echo the character
                s += c

        return s

    def run(self):
        """Operate the _UserServer daemon Thread"""
        if not _FAKE_STDIO:         ## While debugging 'real' stdio
            Debug.get().set_opt("hcdm", True)  ## Set Hard Core Debug Mode
        while self.operational:
            try:
                printf("%.3f >>>> " % time.time(), end="", flush=True)
                s = self.__inp().strip()
                tracef("%.3f >>>> %s" % (time.time(), s))

                control['user-server'].work(s)
                if s.upper() == '.QUIT':
                    break

            except Exception as e:
                Debug.handle_exception()
                break

        Common.stop()
        Common.del_thread(self)
        debugf("UserServer terminated")

    def stop(self):
        """Terminate the _UserServer Thread"""
        self.operational = False

_UserServer()


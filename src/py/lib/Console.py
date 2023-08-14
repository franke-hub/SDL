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
##       Console.py
##
## Purpose-
##       Console utility
##
## Last change date-
##       2023/08/13
##
## Implementation notes: Command definitions-
##       from lib.Command import command
##       class __CommandN:
##           @staticmethod
##           def run(self, argv):
##               ## Handle input string using command dictionary
##
##       command['commandN'] = __CommandN
##
## Usage notes[sys.stdin character handling]-
##       The sys.stdin.read(1) function echos characters as received. The
##       backspace is echoed but the deleted character is not removed. None
##       of the input characters are returned to the caller are returned to
##       the caller until a '\n' (newline) character is received.
##
##       The stdio assembler package provides a version of getchar which
##       returns but does not echo the input character. We clear backspace
##       characters as received and don't move back past the original input
##       column.
##
##       Backspace and up/down arrow are only handled properly when the
##       stdio assembler package is used.
##
## Implementation notes: Input handler definition-
##       Input lines are passed to an input hander, the default is:
##       class __Console(object):
##           def work(self, str):
##               ## Handle input string using command dictionary
##
##       control['Console'] = __Console()
##
##############################################################################
import readline
import sys
import threading
import time

from lib.Command import Command, command
from lib.Control import control
from lib.Debug import *
from lib.Dispatch import OBJ, TAB, UOW
from lib.Utility import visify

##############################################################################
## Define available imports
##############################################################################
__all__ = ['Console']

##############################################################################
## Import getchar/putchar (With internal default implementations.)
##############################################################################
try:
    from stdio import getchar, putchar
except ImportError:
    print('Warning: stdio not available')
    def getchar(): ## FAKE getchar (because it echos)
        return sys.stdin.read(1)

    def putchar(c): ## FAKE putchar (because echo already done)
        pass

    def outch(s):
        sys.stdout.write(s)

##############################################################################
## Constants
##############################################################################
_CTU = 21                           ## CTRL-U character
_ESC = 27                           ## Escape character

##############################################################################
## Utility subroutines
##############################################################################
def _bs():                          ## Echo character delete sequence
    putchar('\b')
    putchar(' ')
    putchar('\b')

def _cu(s):                         ## Echo CTRL-U delete sequence for line
    for _ in s:
        _bs()

##############################################################################
## Class _ConsoleTAB
##
## With only one _ConsoleTAB, commands will be processed one at a time.
## Commands can still be read, but they will be queued for processing on the
## single TAB. The MULTI_COMMAND control variable, if True, creates a new TAB
## for each command. These commands then run in parallel.
##############################################################################
class _ConsoleTAB(TAB):
    def work(self, uow):
        work = uow.work
        if isinstance(work, str):
            if work == '':
                uow.done(None)
                return

            argv = tuple(work.split())
            work = argv[0]

            try:
                Name = command[work]
            except KeyError:
                debugf('Invalid command: "%s"' % visify(work))
                debugf('Valid commands: %s' % Command._get_list(command))
                uow.done(UOW.CC_ERROR)
                return

            try:
                uow.done(Name.run(argv))
                return
            except Exception as X:
                debugf('%s.run%s Exception: %s' % (work, argv, X))
                Debug.handle_exception()

        uow.done(UOW.CC_ERROR)
        return

##############################################################################
## class _Console
##
## Default input handler, handles one input line at at time.
##############################################################################
class _Console(object):
    MULTI_COMMAND = True            ## Allow multiple simultaneous commands?
    MULTI_COMMAND = False           ## Allow multiple simultaneous commands?

    def __init__(self):
        self.tab= _ConsoleTAB()

    def work(self, str):
        uow = UOW()
        uow.work = str
        obj = OBJ.get()
        if obj:
            tab= self.tab
            if _Console.MULTI_COMMAND:
                tab = _ConsoleTAB()
            obj.enqueue(tab, uow)
            time.sleep(.125)
        else:
            self.tab.work(uow)

control['Console'] = _Console()

##############################################################################
## Class _History
##
## Handle command line history.
##############################################################################
class _History(object):
    _MAX_LINES = 8

    def __init__(self):
        self.hist = []
        self.index = 0

    def insert(self, s):
        if s == '':
            return
        self.hist += [s]
        if len(self.hist) > _History._MAX_LINES:
            self.hist = self.hist[1:]
        self.index = len(self.hist)

    def move_down(self):
        if self.index >= len(self.hist):
            return ''

        s = self.hist[self.index]
        self.index += 1
        return s

        history -= 1
        if history < 1:
            history = 1

        s = self.history[-history]
        for c in s: putchar(c)
        self._debug_history(history, s)

    def move_up(self):
        if self.index == 0:
            return ''

        self.index -= 1
        return self.hist[self.index]

##############################################################################
## Class Console
##############################################################################
class Console(threading.Thread):
    __singleton = None

    def __init__(self, *args, **kwargs):
        assert Console.__singleton == None, 'Only one console thread allowed'
        Console.__singleton == self

        threading.Thread.__init__(self,name='Console',*args,**kwargs)
        self.hist = _History()
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
        s = ''
        while self.operational:
            c = getchar()

            if ord(c) == _ESC:      ## Escape (sequence)
                c = getchar()
                if c == '[':
                    c = getchar()
                    if c == 'A':    ## UP arrow
                        _cu(s)
                        s = self.hist.move_up()
                        for c in s: putchar(c)
                        continue

                    if c == 'B':    ## DOWN arrow
                        _cu(s)
                        s = self.hist.move_down()
                        for c in s: putchar(c)
                        continue

                    putchar(chr(_ESC)) ## Unrecognized sequence
                    putchar('[')
                    s += chr(_ESC) + '['
                else:
                    putchar(chr(_ESC)) ## Unrecognized sequence
                    s += chr(_ESC)

            if c == '\b':
                if len(s) > 0:
                    _bs()           ## Echo character delete sequence
                    s = s[:-1]
            elif ord(c) == _CTU:    ## CTRL-U
                _cu(s)
                s = ''
            else:
                if c == '\t':
                    c = ' '

                putchar(c)          ## Echo the character
                if c == '\n':
                    break
                s += c

        return s

    def run(self):
        '''Operate the Console Thread'''
        while self.operational:
            try:
                printf('%.3f >>>> ' % time.time(), end='', flush=True)
                s = self.__inp().strip()
                writef('%.3f >>>> %s' % (time.time(), s))

                s.strip()
                self.hist.insert(s)
                control['Console'].work(s)

            except Exception as X:
                print('Exception: %s' % (X))
                Debug.handle_exception()
                break

        debugf('Console terminated')

    def stop(self):
        '''Terminate the Console Thread'''
        self.operational = False

#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2024 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Curses.py
##
## Purpose-
##       Curses tests.
##
## Last change date-
##       2024/05/16
##
##############################################################################
import curses
import time

from lib.Command import command
from lib.Debug import *

##############################################################################
##
## Class-
##       Logger
##
## Purpose-
##       Extends Debug, adding methods put and log for debugf and tracef
##
##############################################################################
class Logger(Debug):
##    def __init__(self):
##        super().__init__()
##        super().set_opt('MODE', self.MODE_LOGGER)

    def put(self, *args, **kwargs):
        self.debugf(*args, **kwargs)

    def log(self, *args, **kwargs):
        self.tracef(*args, **kwargs)

##############################################################################
##
## Subroutine-
##       test_curses
##
## Purpose-
##       Test curses features
##
##############################################################################
def curses_init(stdscr):
    curses.start_color()
    curses.use_default_colors()
##    tracef("%.3f curses.COLORS(%d)" % (time.time(), curses.COLORS))
##    tracef("%.3f curses.COLOR_PAIRS(%d)" % (time.time(), curses.COLOR_PAIRS))
##    tracef("%.3f curses.COLS(%d)" % (time.time(), curses.COLS))
##    tracef("%.3f curses.LINES(%d)" % (time.time(), curses.LINES))
##    tracef("%.3f %s= curses.can_change_color()" %
##           (time.time(), str(curses.can_change_color())))
##    curses.cbreak()

    tracef("curses.COLORS(%d)" % (curses.COLORS))
    tracef("curses.COLOR_PAIRS(%d)" % ( curses.COLOR_PAIRS))
    tracef("curses.COLS(%d)" % (curses.COLS))
    tracef("curses.LINES(%d)" % (curses.LINES))
    tracef("%s= curses.can_change_color()" % (str(curses.can_change_color())))
    curses.cbreak()

def curses_test(stdscr):
    curses.init(stdscr)
    max_color = curses.COLORS
    if max_color > 16:
        max_color = 16

    for i in range(0, max_color):
        curses.init_pair(i + 1, i, -1)
    stdscr.addstr(0, 0, '{0} colors available'.format(curses.COLORS))
    maxy, maxx = stdscr.getmaxyx()
    maxx = maxx - maxx % 4
    x = 6
    y = 1
    try:
        for i in range(0, max_color):
            stdscr.addstr(y, x, '{0:4}'.format(i), curses.color_pair(0))
            x = (x + 4) % maxx
            if x == 0:
                y += 1
    except curses.ERR:
        pass

    y = 2
    pair = 0
    for i in range(0, max_color):
        for j in range(0, max_color):
            curses.init_pair(pair, i, j)
            pair += 1
    try:
        pair = 0
        while y <= curses.LINES:
            stdscr.addstr(y, 0, "[%3d] " % (pair), curses.color_pair(0))
            x = 6
            for i in range(0, max_color):
                S = "{0:4}".format(pair)
                stdscr.addstr(y, x, S, curses.color_pair(pair))
                pair += 1
                x += 4

            tracef("%.3f Wrote line(%d)" % (time.time(), y))
            if pair >= curses.COLOR_PAIRS:
                break

            y += 1
    except Exception:
        tracef("%.3f FAILED line(%d)" % (time.time(), y))

    Debug._get().flush()
    stdscr.getch()

##############################################################################
## ALTERNATE TEST
##############################################################################
def curses_test(stdscr):
    curses.init(stdscr)
    # for i in range(0, curses.COLORS):
    #    curses.init_pair(i + 1, i, -1)
    y = 0
    x = 0
    maxy, maxx = stdscr.getmaxyx()
    if maxx > 16:
        maxx= 16
    pair = curses.COLOR_PAIRS - 1
    if pair > 255:
        pair = 255
    for i in range(0, curses.COLORS):
        curses.init_pair(pair, i, -1)
        stdscr.addstr(y, x, '{0:3}'.format(i), curses.color_pair(pair))
        x += 4
        if x >= maxx:
            x  = 0
            y += 1
            if y >= maxy:
                break

    stdscr.getch()

##############################################################################
## ALTERNATE TEST
##############################################################################
def curses_test(stdscr):
    curses_init()
    for i in range(0, curses.COLORS):
        curses.init_pair(i + 1, i, -1)
    try:
        for i in range(0, 255):
            stdscr.addstr(str(i), curses.color_pair(i))
    except curses.ERR:
        # End of screen reached
        pass
    stdscr.getch()

##############################################################################
## ALTERNATE TEST
##############################################################################
def curses_test(stdscr):
    curses_init(stdscr)

    maxy, maxx = stdscr.getmaxyx()
    if maxx > 80:
        maxx= 80

    min_color= 0
    while min_color < curses.COLORS:
        max_color = curses.COLORS
        tracef("min_color(%d) max_color(%d)" % (min_color, max_color))
        if max_color > min_color + 256:
            max_color = min_color + 256

        for i in range(min_color, max_color):
            if False:
                curses.init_pair(i, i, -1)
            else:
                curses.init_pair(i, -1, i)

        y = 0
        x = 0
        for i in range(min_color, max_color):
            S = "{0:7} ".format(i)
            stdscr.addstr(y, x, S, curses.color_pair(i))
            x += 8
            if x >= 64:
                y += 1
                if y >= maxy:
                    break
                x = 0;

        min_color += 256
        C = stdscr.getch()
        if C == ord('q') or C == ord('Q'):
            break

##############################################################################
## ALTERNATE TEST: Keyboard test
##############################################################################
def curses_test(stdscr):
    curses_init(stdscr)

    row= 0
    while True:
        stdscr.move(row, 0)
        C= stdscr.getch()
        D= C
        if D == ord('\b') or D == ord('\t') or D == ord('\n') or D >= 128:
            D= '~'
        S= "O:%.4o 0x%.4x '%c'" % (C, C, D)
        tracef(S)
        if C == ord('q'):
            break

        row += 1
        if row >= curses.LINES:
            row= 0
            stdscr.erase()
        stdscr.addstr(row, 0, S, curses.color_pair(0))

##############################################################################
## 'curses' COMMAND (Run _Main curses)
##############################################################################
def test_curses(argv):              ## Test curses (basic)
    version= "test_curses()"
    debugf("Running: %s %s" % (version, time.asctime()))
    try:
        import curses
    except Exception:
        print('Cannot import curses')
        return

    ## for arg in argv:
    ##     if arg == "curses":
    ##         test_curses()
    ##     else:
    ##         usage(arg)

    print("Length: ", len(argv))
    print("argv[0]: ", argv[0])

    curses.wrapper(curses_test)

##############################################################################
##
## Class-
##       __Command
##
## Purpose-
##       Implement 'curses' command
##
##############################################################################
def usage(arg):
    print("Invalid argument '" + arg + "'")
    print("Curses command, ...")
    print("  curses:    curses test")

class __Command:
    @staticmethod
    def run(argv):
        logger = Debug.get()
        logger.set_opt('MODE', Debug.MODE_LOGGER)

        debugf("Curses:", time.asctime())
        test_curses(argv[1:])

        return 0

command['curses'] = __Command

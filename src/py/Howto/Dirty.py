#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2021 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Dirty.py
##
## Purpose-
##       Quick and dirty test.
##
## Last change date-
##       2021/03/30
##
## Usage-
##       ./Dirty.py
##
##############################################################################
import time

from lib.Command import command
from lib.Debug import Debug, debugf, tracef
from lib.Dispatch import OBJ, TAB, UOW
_disp = OBJ()

##############################################################################
## Quick and dirty test command
##   Now testing: Basic Reader functionality
##############################################################################
class _Blip(TAB):                   ## Blip every 10 seconds
    def work(self, uow):
        while True:                 ## True enables blipping
            time.sleep(10)
            print('.', end='', flush=True)

class __Ugly:
    @staticmethod
    def run(self, argv):
        tab = _Blip()
        uow = UOW()
        _disp.enqueue(tab, uow)

command['.blip'] = __Ugly

class _TAB(TAB):
    def work(self, uow):
        print('...working...')
        uow.done()

class __Dirty:
    @staticmethod
    def run(argv):
        print('Running quick and dirty test...')
        from lib.Dispatch import TAB, UOW, WDW

        name = 'http://bigblue:8080'
        if len(argv) > 1:
            name = argv[1]
        work = [name]

        ######################################################################
        ## Test multiple usage of WDW object
        tab = _TAB()
        wdw = WDW()
        uow = UOW(WDO=wdw)
        print('one...')
        _disp.enqueue(tab, uow)
        wdw.wait()
        print('two...')
        _disp.enqueue(tab, uow)
        wdw.wait()

        ######################################################################
        ## Test type function for tuple and list
        obj = ('a', 'b')
        print(type(obj))

        obj = ['a', 'b']
        print(type(obj))

        print('...Test complete')

command['dirty'] = __Dirty

##############################################################################
## Standalone test
##   Now testing: Dictionary ordering
##############################################################################
if __name__ == '__main__':
    print('Running quick and dirty test')

    Debug('debug.out')
    logout('NOISY TEST')
    logger('QUIET TEST')

    dict = {}
    dict['A'] = 'aaah'
    dict['B'] = 'caah'
    dict['C'] = 'baah'

    print('one')
    print(dict)
    print('two')
    print('{%s}' % ', '.join(["'%s': '%s'" % (name, dict[name]) for name in sorted(dict)]))

    quit()

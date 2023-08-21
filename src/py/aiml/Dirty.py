#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2018 Frank Eskesen.
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
##       2018/01/01
##
## Usage-
##       ./Dirty.py
##
##############################################################################

##############################################################################
## Quick and dirty .blip command
##############################################################################
import time

from lib.Command import command
from lib.Dispatch import TAB, UOW
import Common
class _Blip(TAB):                   ## Blip every 10 seconds
    def work(self, uow):
        while True:                 ## True enables blipping
            time.sleep(10)
            print(".", end="", flush=True)

class __Command:
    def run(argv):
        tab = _Blip()
        uow = UOW()
        Common.enqueue(tab, uow)

command['.blip'] = __Command

##############################################################################
## Standalone test
##############################################################################
if __name__ == "__main__":
    print("Running quick and dirty test")
    pass

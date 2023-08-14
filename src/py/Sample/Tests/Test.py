#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2016 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Test.py
##
## Purpose-
##       Python sample programs: Test base class
##
## Last change date-
##       2016/05/04
##
##############################################################################
class Test(object):                 ## NOTE: object base needed for super
    def __init__(self, verbose=False):
        if verbose:
            print("Test.__init__")

    def run(self):
        print("Test.run")

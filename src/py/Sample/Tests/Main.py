#!/usr/bin/env python
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
##       Python sample programs: Mainline code
##
## Last change date-
##       2018/01/25
##
##############################################################################
import sys
import traceback

import Config                       ## Configuration defaults
import Test                         ## Common base class
import Samples                      ## Sample test cases

import sys                          ## For sys.argv

def _get_list(omit):
    list = []
    for key in Samples.dict:
        if key not in omit:
            list += [key]

    return sorted(list)

def _run(test):
    try:
        print("")
        print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        print("Trying: '%s'" % test)
        Name = Samples.dict[test]

    except KeyError:
        print("No such test:", test)
        print("Tests include:", _get_list([]))
        return

    try:
        obj = Name()
        obj.run()
    except Exception as e:
        print("Error running:", test)
        x_type, x_value, x_tb = sys.exc_info()
        traceback.print_exception(x_type, x_value, x_tb)
        raise

##############################################################################
## Mainline code
##############################################################################
class Main(Test.Test):
    def __init__(self,verbose=Config.VERBOSE):
        super(Main,self).__init__(verbose=verbose)
        # Test.Test.__init__(self,verbose=verbose) ## Alternative to super(...

        self.verbose = verbose
        if verbose: print("Main.__init__")

    def run(self):
        if self.verbose: print("Main.run")
        for test in sys.argv[1:]:
            _run(test)

        print(".... Done ....")

##############################################################################
## All tests
##############################################################################
class All(Test.Test):
    def run(self):
        omit = []
        omit += ["all", "compile-only", "dirty", "exception"]
        omit += ["list", "terminal"]
        list = _get_list(omit)
        for test in list:
            _run(test)

Samples.dict['all'] = All

##############################################################################
## List test
##############################################################################
class List(Test.Test):
    def run(self):
        print(_get_list([]))

Samples.dict['list'] = List

##############################################################################
## Mainline code
##############################################################################
if __name__ == '__main__':
    main = Main()
    main.run()

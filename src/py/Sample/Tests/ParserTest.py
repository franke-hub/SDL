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
##       ParserTest.py
##
## Purpose-
##       Python sample programs: Test ArgumentParser
##
## Last change date-
##       2018/01/25
##
##############################################################################
import argparse
import sys
import traceback

import Config                       ## Configuration defaults

##############################################################################
## Constants
##############################################################################
_DEFAULT_X = 128
_DEFAULT_Y = 128
_DEFAULT_Z = 64

##############################################################################
## Mainline code
##############################################################################
class Main(object):
    def __init__(self,verbose=Config.VERBOSE):
        super(Main,self).__init__()
        # object.__init__(self)     ## Alternative to super(...

        self.verbose = verbose
        if verbose: print("Main.__init__")

    def run(self):
        if self.verbose: print("Main.run")
        parser = argparse.ArgumentParser()

        ## Add positional arguments
        parser.add_argument('DEFAULT_X', action='store', default=_DEFAULT_X,
                            type=int, nargs='?')
        parser.add_argument('DEFAULT_Y', action='store', default=_DEFAULT_Y,
                            type=int, nargs='?')
        parser.add_argument('DEFAULT_Z', action='store', default=_DEFAULT_Z,
                            type=int, nargs='?')

        ## Add keyword arguments
        ## NOT CODED YET

        if False:
            print(parser)
        args = parser.parse_args()
        if True:
            print(args)
            print(args.DEFAULT_X)

        print(".... Done ....")

##############################################################################
## Mainline code
##############################################################################
if __name__ == '__main__':
    main = Main()
    try:
        main.run()
    except Exception as x:
        print("Exception!")
        x_type, x_value, x_tb = sys.exc_info()
        traceback.print_exception(x_type, x_value, x_tb)
        raise

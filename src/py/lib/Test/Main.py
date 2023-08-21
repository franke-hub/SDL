#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2017-2023 Frank Eskesen.
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
##       Library tester.
##
## Last change date-
##       2023/08/21
##
##############################################################################
import argparse
import datetime
import pdb
import sys

#### lib #####################################################################
from lib.Global      import *
Global.TESTING = 'lib.Utility.ReminderFunctions'

from lib.Command     import *
from lib.Debug       import *
from lib.Dispatch    import *
from lib.Utility     import *
Global.TESTING = None

## Import the Define command dictionary, imported test commands
import Collections_test
import Debug_test
import Dispatch_test
import Types_test
import Utility_test

##############################################################################
try:
    import Dirty
except ImportError:
    pass

##############################################################################
## Constants (Some may be overridden by parameters)
##############################################################################
USE_LOGGER = False                  ## Use Debug logger?
USE_PDB    = False                  ## Use PDB debugger?

OMIT_TESTS  = ['Misuse:Node', 'dirty', 'list', 'main']

##############################################################################
## The Main application
##############################################################################
def main(*args):
    ## Initialize ############################################################
    if USE_LOGGER:
        log = Logger()              ## Use Debug.Logger
        if Global.HCDM: log.set_opt('flush', True)
        if True:
            now = datetime.datetime.now()
            writef('')
            logger('********************************************************')
            tod = now.strftime('%A, %d %B %Y %I:%M%p')
            logger('Application started: %s' % tod)

    ## Run all applications ##################################################
    runner = Command(command)
    runner.omit = OMIT_TESTS
##  runner.main([])
    runner.main(args[0])

##############################################################################
## Mainline code
##############################################################################
class __Main():
    @staticmethod
    def run(*args):
        main(args)
        sys.exit(0)
command['main'] = __Main

if __name__ == '__main__':
    ## Parameter analysis ####################################################
    parser = argparse.ArgumentParser(description='Personal Database')
    parser.add_argument('--logger', nargs='?', default=False,
                        help='Use Debug.Logger? (default: False)')
    parser.add_argument('--pdb', nargs='?', default='False',
                        help='Use python debugger? (default: False)')
    parser.add_argument('--testing', nargs=1,
                        help='Testing qualifier. (Default: None)')
    text = 'Verbosity, range 0..5 (default: %d)' % Global.VERBOSE
    parser.add_argument('--verbose', nargs=1, type=int, help=text)
    parser.add_argument('argv', metavar='Test', type=str, nargs='*',
                        help='Programs to run (default: "main")')
    parsed = parser.parse_args()

    argv = parsed.argv
    print('argv:', argv)
    if isinstance(parsed.logger, bool):
        assert parsed.logger == False ## Otherwise it's a string
    elif isinstance(parsed.logger, None):
        USE_LOGGER = True
    elif parsed.logger.upper() == 'HCDM':
        USE_LOGGER = True
        Global.HCDM = True
    else:
        raise ValueError('--logger(%s)' % parsed.logger)

    if parsed.pdb == None or parsed.pdb.upper() == 'TRUE': USE_PDB = True

    if parsed.testing != None:
        Global.TESTING = parsed.testing[0]

    if parsed.verbose != None:
        Global.VERBOSE = parsed.verbose[0]
    del parser, parsed, text

    ## Mainline ##############################################################
    if USE_PDB:
        print("\n\n******************************** Type 'continue' to begin")
        pdb.run('main(argv)')
    else:
        main(argv)

#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2019-2021 Frank Eskesen.
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
##       Command processor.
##
## Last change date-
##       2021/04/20
##
##############################################################################
import sys
import argparse
import datetime
import pdb

#### lib #####################################################################
from lib.Command     import *
from lib.Debug       import *
from lib.Global      import *
from lib.Utility     import *

##############################################################################
import CalendarTest
import NittyGritty

##############################################################################
try: ## The Dirty module is not maintained in git, so it might not be here.
    import Dirty
except Exception as X:
    debugf('Exception: %s' % X)

##############################################################################
## Constants (Some may be overridden by parameters)
##############################################################################
_OMIT      = ['dirty', 'list', 'main']
USE_LOGGER = False                  ## Use Debug logger?
USE_PDB    = False                  ## Use PDB debugger?

wasname    = __name__               ## Name when compiled

## Global.Global can easily be accessed outside of Main, but Main.Thing can't
class Thing: ## Test using Main nitty-gritty --testing=main-attributes
    HCDM    = False
    TESTING = None
    VERBOSE = 1

##############################################################################
## The Main application
##############################################################################
def main():
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
    runner.omit = _OMIT
    runner.main([])

##############################################################################
## Mainline code
##############################################################################
class __Main():
    @staticmethod
    def run(*args):
        main()
        sys.exit(0)
command['main'] = __Main

if __name__ == "__main__":
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
        Thing.TESTING = parsed.testing[0]

    if parsed.verbose != None:
        Global.VERBOSE = parsed.verbose[0]
        Thing.VERBOSE = parsed.verbose[0]
    del parser, parsed, text

    if Global.TESTING == 'main-attributes':
        try:
            debugf('\nin %s' % __name__)
            debugf('__name__: When compiled(%s), Now(%s)' % (wasname, __name__))
            debugf('Global.TESTING(%s), Main.Thing.TESTING(%s)' %
                  (Global.TESTING, Main.Thing.TESTING))
            assert False, 'This USED TO raise NameError'
        except NameError as X:
            if Global.VERBOSE > 1: debugf('Expected Exception:', X)
            debugf('Global.TESTING(%s), Thing.TESTING(%s)' %
                 (Global.TESTING, Thing.TESTING))
            debugf('Thing: <%s (%s,%s,%s,%s)>' % \
                   ( type(Thing), Thing.__module__, Thing.__name__
                   , '*' ## 'dir("Thing"):\n' + str(dir("Thing"))
                   , '*' ## 'dir("Main"):\n' + str(dir("Main"))
                   )
                  )
            Global.ADDED_BY = 'Main'
            debugf('\n')

    ## Mainline ##############################################################
    if len(argv) == 0 or argv[0] == 'main':
        if USE_PDB:
            debugf("\n\n************************ Type 'continue' to begin")
            pdb.run('main()')
        else:
            main()
    else:
        runner = Command(command)
        runner.omit = _OMIT
        runner.main(argv)

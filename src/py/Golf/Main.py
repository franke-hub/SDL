#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2019 Frank Eskesen.
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
##       Golf: Main program
##
## Last change date-
##       2019/08/05
##
##############################################################################
import sys

import configparser
import datetime
import pdb

##############################################################################
## Compliation controls
##############################################################################
USE_PDB = False                     ## Use debugger?

#### PyQt5 ###################################################################
from PyQt5.QtCore    import *
from PyQt5.QtGui     import *
from PyQt5.QtWidgets import *

#### lib #####################################################################
from lib.Command     import *
from lib.Debug       import *
from lib.Utility     import *

#### Golf ####################################################################
from GolfApplet      import *
from GolferMain      import *
from WindowList      import *

#### Test ####################################################################
try:
    import Dirty
except Exception as X:
    print('Exception: %s' % X)

##############################################################################
## Constants
##############################################################################
CONFIG_FILE = 'golfer.ini'

##############################################################################
## Internal data areas
##############################################################################
config      = configparser.ConfigParser()

##############################################################################
## The Main application
##############################################################################
def main():
    ## Initialize ############################################################
    log = Logger()                  ## Use Debug.Logger
    log.set_opt('flush', True)
    if True:
        now = datetime.datetime.now()
        writef('')
        logger('************************************************************')
        tod = now.strftime('%A, %d %B %Y %I:%M%p')
        logger('Application started: %s' % tod)

    config.read(CONFIG_FILE)
    database = config['dbserver']['path']
    db = DbServer(database)

    ## Run the Main application ##############################################
    app = QApplication(sys.argv)
    main = GolferMain()
    app.exec_()

    ## Termination debugging #################################################
    if True:
        logger('\n\n')
        logger('MAIN: Normal termination')
        window = WindowList(None)
        window.static_debug()

##############################################################################
## Mainline code
##############################################################################
class Main():
    @staticmethod
    def run(*args):                 ## Test DbServer
        main()
        sys.exit(0)

command['main'] = Main

if __name__ == '__main__':
    argv = sys.argv[1:]
    if len(argv) == 0:
        argv = ['main']
    if len(argv) > 0 and argv[0] == 'main':
        if USE_PDB:
            print("\n\n************************ Type 'continue' to begin")
            pdb.run('Main.run()')
        else:
            Main.run()

    runner = Command(command)
    runner.omit = ['dirty', 'list', 'main']
    runner.main(argv)


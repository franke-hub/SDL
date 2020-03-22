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
##       Golf test driver.
##
## Last change date-
##       2019/08/18
##
## Implementation note-
##       Only this module creates a DbServer object
##
##############################################################################
import sys

#### PyQt5 ###################################################################
from PyQt5.QtCore    import *
from PyQt5.QtGui     import *
from PyQt5.QtWidgets import *

#### lib #####################################################################
from lib.Command     import *
from lib.Debug       import *
from lib.Utility     import *

#### Golf ####################################################################
from DbServer        import *

## Import the test modules
try:
    import test.Dirty
except Exception as X:
    print('Exception: %s' % X)

import test.UnitTest

##############################################################################
## Mainline code
##############################################################################
def runAllTests():
    errorCount = 0
    for test in sorted(command):
        if test == 'dirty': continue
        if test == 'list': continue
        debugf("Running: '%s'" % test)
        try:
            command[test].run()
        except Exception as X:
            errorCount += 1
            debugf('!!!!!!!! Test[%s] Exception:' % test)
            Debug.handle_exception()
        debugf('')
    return errorCount

def main():
    app = QApplication(sys.argv)
    db = DbServer('GOLFER.DB')

    errorCount = 0
    selected = False            ## True iff a test was selected
    for test in sys.argv[1:]:
        if test[0] == '-':      ## If a switch parameter
            continue            ## Ignore it

        selected = True
        try:
            if test == 'all':
                errorCount += runAllTests()
            else:
                debugf("Running: '%s'" % test)
                command[test].run()
        except KeyError:
            errorCount += 1
            debugf('!!!!!!!! Test[%s] Not available:' % test)
        except Exception as X:
            errorCount += 1
            debugf('!!!!!!!! Test[%s] Exception:' % test)
            Debug.handle_exception()
        debugf('')

    if not selected:            ## If no test selected
        print('Default: running all tests')
        errorCount += runAllTests()

    if errorCount:
        if errorCount == 1:
            debugf('!!!!!!!! 1 test failed')
        else:
            debugf('!!!!!!!! %s tests failed' % errorCount)
    else:
        debugf('All tests successful')

class Main():
    def run(self):
        main()

if __name__ == '__main__':
    main = Main()
    main.run()


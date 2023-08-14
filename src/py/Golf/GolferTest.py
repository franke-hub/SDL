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
##       GolferTest.py
##
## Purpose-
##       Golf: Test window
##
## Last change date-
##       2019/08/11
##
## Implementation notes-
##       This program is not maintained in .git
##
##############################################################################
import sys

#### PyQt5 ###################################################################
from PyQt5.QtCore    import *
from PyQt5.QtGui     import *
from PyQt5.QtWidgets import *

#### lib #####################################################################
from lib.Debug       import *
from lib.Utility     import *

#### Golf ####################################################################
from DbServer        import *
from GolfApplet      import *
from ResultEdit      import *
from ResultView      import *
from WindowList      import *

##############################################################################
##
## Function-
##       _keepLeft
##
## Purpose-
##       Keep a layout left when window expanded
##
## Implementation notes-
##       Function not needed when dates are left of label
##
##############################################################################
def _keepLeft(L):
    print('TEST._keepLeft')
    I = L                           ## Save input layout
    I.addStretch(4)                 ## (Keep left function)
    L = QHBoxLayout()
    L.setAlignment(Qt.AlignLeft)
    L.addLayout(I)
    L.addStretch()
    return L

def _keepLeft(L):                   ## This disables the KeepLeft function
    return L

##############################################################################
##
## Function-
##       _warning
##
## Purpose-
##       Create warning: DOES NOT EXIT until OK button pressed!
##
##############################################################################
def _warning(text):                 ## Warning window
    logger('TEST.warning(%s) ****************************************' % text)
    exit()                          ## TODO: Something else

##############################################################################
##
## Class-
##       GolferTest
##
## Purpose-
##       The golfer quick and dirty test window.
##
##############################################################################
class GolferTest(GolfApplet):       ## The application control window
    def __init__(self, *args):
        super().__init__()
        logger('TEST(%s).__init' % self)

        self.windowList = WindowList(self)

        ## Initialize main window
        self.setWindowTitle('GolferTest')

        title = 'GolferTest'
        if len(args):
            title = ','.join(arg for arg in args[1:])
            title = '%s [%s]' % (args[0], title)

        ## Graphic attributes
        self.closeButton        = QPushButton('Close')
        self.debugButton        = QPushButton('Debug')
        self.testButton         = QPushButton('Test')
        self.terminateButton    = QPushButton('Terminate')
        self.titleLabel         = QLabel(title)

        ## Load and initialize the attributes
        self._load()                ## Load the attributes
        self._init()                ## Initialize graphic attributes
        self.show()                 ## Display the window

    def __del__(self):
        logger('TEST(%s).__del HCDM info-only' % self)

    def debug(self):                ## Debugging display
        debugf('TEST(%s).debug' % self)
        debugf('self.close: %s' % self.close) ## HCDM: State of close function
        self.windowList.debug()     ## (includes flush)

    def _init(self):                ## Initialize graphic attributes
        logger('TEST(%s)._init' % self);

        self.closeButton.clicked.connect(self.buttonClose)

        self.debugButton.clicked.connect(self.buttonDebug)

        self.testButton.clicked.connect(self.buttonTest)

        Gt.setBg(self.terminateButton, Qt.red)
        self.terminateButton.clicked.connect(self.buttonTerminate)

        self.titleLabel.setAlignment(Qt.AlignCenter)

    def _load(self):                ## Load the data
        return ## DOES NOTHING, NO MESSAGE
        logger('TEST(%s)._load' % self);

    def buttonClose(self):
        logger('TEST(%s).buttonClose' % self)
        self.close()

    def buttonDebug(self):
        logger('TEST(%s).buttonDebug' % self)
        self.debug()

    def buttonTest(self):
        logger('TEST(%s).buttonTest' % self)

        ## Create new sub-window
        self.windowList.debug()
        W = GolferTest()
        self.windowList.append(W)

    def buttonTerminate(self):
        logger('TEST(%s).buttonTerminate' % self)
        QApplication.instance().quit()

    def close(self):                ## Log close
        logger('TEST(%s).close>>>' % self)

        self.windowList.remove()    ## We're outta here
        logger('TEST(%s).<<<close' % self)
        return super().close()

    def show(self):
        logger('TEST(%s).show' % self);

        main = QVBoxLayout()
        main.setContentsMargins(5, 5, 5, 5)

        main.addWidget(self.titleLabel)
        main.addWidget(self.testButton)
        main.addWidget(self.closeButton)
        main.addWidget(self.debugButton)
        main.addWidget(self.terminateButton)

        self.setLayout(main)
        return super().show()

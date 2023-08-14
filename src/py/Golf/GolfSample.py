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
##       GolfSample.py
##
## Purpose-
##       Golf: Sample golf application window
##
## Last change date-
##       2019/08/16
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
from WindowList      import *

##############################################################################
##
## Class-
##       GolfSample
##
## Purpose-
##       The golfer sample application
##
##############################################################################
class GolfSample(GolfApplet):       ## A sample application window
    def __init__(self):
        super().__init__()

        self.windowList = WindowList(self) ## The active window list

        ## Selection attributes
        self.eventsID   = None      ## (Unused)

        ## Graphic attributes
        self.closeButton = QPushButton('Close sub-windows')
        self.debugButton = QPushButton('Debug')

        ## Load and initialize the attributes
        self._load()                ## Load the attributes
        self._init()                ## Initialize graphic attributes
        self.show()                 ## Display the main window

    def debug(self):                ## Debugging display
        super().debug()
        self.windowList.debug()

    def _init(self):                ## Initialize graphic attributes
        self.closeButton.clicked.connect(self.selectCloseButton)
        self.debugButton.clicked.connect(self.selectDebugButton)

        self.setWindowTitle('GolfSample')

    def _load(self):                ## Load the data
        """Normally data must be loaded before the graphic attributes can be
           properly initialized. This sample does not need this
        """
        pass

    def close(self):                ## Intercept close (for debugging)
        logger('GolfSample.close')
        return super().close()

    def focusCloseButton(self):     ## Focus event: Example only
        logger('GolfSample.focusCloseButton')
        pass                        ## Do something

    def selectCloseButton(self):    ## Select CloseButton
        logger('GolfSample.selectCloseButton')
        self.windowList.close()

    def selectDebugButton(self):    ## Unused sample
        logger('GolfSample.selectDebug')
        self.debug()

    def show(self):                 ## Override this method
        main = QVBoxLayout()
        main.setContentsMargins(5, 5, 5, 5)
##      main.setHorizontalSpacing(0)  ## ONLY FOR QGridLayout
##      main.setVerticalSpacing(0)    ## ONLY FOR QGridLayout

        Gt.setBg(self.closeButton, Qt.yellow)
        main.addWidget(self.closeButton)

        main.addWidget(self.debugButton)

        self.setLayout(main)
        return super().show()

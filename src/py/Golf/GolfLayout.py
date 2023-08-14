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
##       GolfLayout.py
##
## Purpose-
##       Golf: The standard golf layout.
##
## Last change date-
##       2019/08/21
##
## Usage notes-
##       This is a QGridLayout with some enhancement functions.
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
from DatabaseInfo     import *
from DataField        import *
from GolfApplet       import *
from HoleInfo         import *

##############################################################################
##
## Class-
##       GolfLayout
##
## Purpose-
##       Golf: The extended QGridLayout
##
##############################################################################
class GolfLayout(QGridLayout):
    def __init__(self):
        super().__init__()

        ## Set default attributes
        self.setContentsMargins(1, 1, 1, 1)
        self.setHorizontalSpacing(0)
        self.setVerticalSpacing(0)

    def debug(self):
        debugf('GolfLayout(%s).debug' % self)

    def append(self, thing):        ## Add row or rows to grid
        if isinstance(thing, HoleInfo):
            info = thing
            row = self.rowCount()
            info.getField(HOLE_ID).setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
            for hole in range(info.format):
                F = info.getField(info.display(hole))
                self.addWidget(F, row, hole)
        elif isinstance(thing, QLayout):
            self.addLayout(thing, self.rowCount(), 0, 1, -1)
        elif isinstance(thing, QWidget):
            self.addWidget(thing, self.rowCount(), 0, 1, -1)
        else:
            raise TypeError(thing)

    ##########################################################################
    ## This wraps the layout with a thicker outside border
    def wrapper(self):
        if False:                   ## The simplest wrapper
            return self             ## The simplest wrapper

        if True:                    ## Current working version
            G = QGridLayout()
            G.setContentsMargins(5, 5, 5, 5)
            G.setHorizontalSpacing(0)
            G.setVerticalSpacing(0)
            W = QLabel()
            W.setAlignment(Qt.AlignCenter)
            W.setFrameStyle(QFrame.Box | QFrame.Plain)
            G.addWidget(W, 0, 0, 1, 1)
            G.addLayout(self, 0, 0, 1, 1)
            return G

        if False:                   ## Experimental, NOT CODED YET
            F = QFrame()
            F.setFrameStyle(QFrame.WinPanel | QFrame.Plain)
            return None             ## Haven't figured out the rest of it

        raise NotImplementedError('Should not occur') ## Pick one, dammit

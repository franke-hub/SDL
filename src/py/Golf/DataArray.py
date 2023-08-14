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
##       DataArray.py
##
## Purpose-
##       Golf: An array of DataField rows.
##
## Last change date-
##       2019/08/20
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

##############################################################################
##
## Class-
##       DataArray.py
##
## Purpose-
##       Golf: DataField[rows][cols] array
##
##############################################################################
class DataArray(object):
    def __init__(self, rows, cols): ## GridPanel(rows, cols)
        self.cols = cols            ## The number of columns
        self.rows = []              ## The row array

        for R in range(rows):
            row = []
            for C in range(cols):
                row.append(DataField())
            self.rows.append(row)

    def debug(self):
        debugf('DataArray(%s).debug' % self)
        debugf("rows: '%s':" % self.rows)
        debugf("cols: '%s':" % self.cols)
        for R in range(len(self.rows)):
            debugf("[%s]: '%s'" % (R, self.rows[R]))

    def colCount(self):
        return self.cols

    def rowCount(self):
        return len(self.rows)

    def field(self, row, col):      ## Get associated DataField
        return self.rows[row][col]

    #### SCAFFOLDING, BRINGUP ################################################
    def layout(self):               ## Get a nicely wrapped QGridLayout
        grid = QGridLayout()
        USE_THICK_BORDER = True     ## Hack to use thicker outside border
        if USE_THICK_BORDER:
            grid.setContentsMargins(1, 1, 1, 1)
        else:
            grid.setContentsMargins(5, 5, 5, 5)
        grid.setHorizontalSpacing(0)
        grid.setVerticalSpacing(0)
        ######################################################################

        for row in range(len(self.rows)):
            for col in range(self.cols):
                W = self.rows[row][col]
                ## print('%s' % W) ## TODO: REMOVE

                grid.addWidget(W, row, col)

        ######################################################################
        THICK_VID = 0
        if USE_THICK_BORDER:
            THICK_VID = 2
        if THICK_VID == 1: ########### Trying to use QFrame
            F = QFrame()
            F.setFrameStyle(QFrame.WinPanel | QFrame.Plain)
            ## Haven't figured out the rest of it
        elif THICK_VID == 2: ######### This works
            G = QGridLayout()
            G.setContentsMargins(5, 5, 5, 5)
            G.setHorizontalSpacing(0)
            G.setVerticalSpacing(0)
            W = QLabel()
            W.setAlignment(Qt.AlignCenter)
            W.setFrameStyle(QFrame.Box | QFrame.Plain)
            G.addWidget(W, 0, 0, 1, 1)
            G.addLayout(grid, 0, 0, 1, 1)
            return G
        else:
            return grid

    def setField(self, row, col, field): ## Replace the DataField
        assert isinstance(field, DataField)
        self.rows[row][col] = field

    def setText(self, row, col, text): ## Replace the DataField's text
        self.rows[row][col].setText(text)

    def text(self, row, col):       ## Get associated DataField's text
        return rows[row][col].text()

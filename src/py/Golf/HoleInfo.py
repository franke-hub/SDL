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
##       HoleInfo.py
##
## Purpose-
##       Golf: Hole information
##
## Last change date-
##       2019/08/21
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
from GolfApplet       import *
from DatabaseInfo     import *
from DataField        import *

##############################################################################
## Internal data areas
##############################################################################
_CHANGE_HOLES = [ HOLE_LDO, HOLE_CPO, HOLE_LDI, HOLE_CPI, HOLE_FWH, HOLE_GIR ]
                                    ## List of changeable holes
_CHANGE_NAMES = [   'LDO:',   'CPO:',   'LDI:',   'CPI:',   'FWH:',   'GIR:' ]
                                    ## List of changeable hole names

##############################################################################
##
## Class-
##       HoleInfo
##
## Purpose-
##       Hole content information
##
## Usage-
##       HoleInfo([data, [format]])
##           data initializes the hole content
##               If not specified, defaults to an empty list
##               If isinstance(data,str), data is split into a list
##               if len(data) < format, empty fields are initialized to ''
##                   Empty holes (1..18) are initialized to '-'
##
##           format is the number of holes in the data array
##               If not specified, defaults to maximum size.
##
##############################################################################
class HoleInfo(DatabaseInfo):
    def __init__(self, *args):
        self.format = FORMAT_MAX    ## The default format
        self.holes  = [DataField()] ## Initialize, holes[0] empty DataField
        super().__init__()

        if len(args) > 0 and args[0]: ## If not specified, or specified None
            data = args[0]
            if isinstance(data, str):
                data = data.split()
            assert isinstance(data, list)
        else:
            data = []               ## Default, empty list

        if len(args) > 1 and args[1]: ## If format specified with a value
            self.format = int(args[1])
            assert self.format >= FORMAT_MIN and fmt <= FORMAT_MAX, \
                   'Format(%s)' % self.format

        ## TODO: Handle LDO:, LDI:, CPO:, CPI:, FWH:, GIR:, ...
        for hole in range(1, 19):
            if hole <= len(data):
                self.holes.append(DataField(data[hole-1]))
            else:
                self.holes.append(DataField())
        for hole in range(19, self.format):
            self.holes.append(DataField())
        self.sigma()

    def __repr__(self):
        text = ','.join(hole.text() for hole in self.holes)
        return '<HoleInfo(%s:[%s])>' % (self.format, text)

    def debug(self):
        super().debug()
        print(self)

    @staticmethod
    def display(hole):              ## Adjust hole for display panel
        if hole < 10: return hole
        if hole == 10: return HOLE_OUT
        if hole > 10 and hole <= HOLE_OUT: return hole - 1
        return hole

    def export(self):               ## Create export string, reset ischanged
        raise NotImplementedError('HoleInfo.export')

        data = ' '.join(self.holes[hole].text() for hole in range(1, 19))

        ## TODO: Handle LDO:, LDI:, CPO:, CPI:, FWH:, GIR:, ...
        if self.getText(HOLE_LDO):
            data += 'LDO: '
            data += self.getText(HOLE_LDO)
        ## TODO: And so on...

        self.ischanged = False
        self.ispresent = True
        return data

    @property                       ## (Overrides DatabaseInfo.ischanged!)
    def ischanged(self):            ## Only applies to 'important' holes
        for hole in range(1, 19):
            if self.holes[hole].ischanged: return True
        for hole in _CHANGE_HOLES:
            if self.holes[hole].ischanged: return True
        return False

    @ischanged.setter
    def ischanged(self, value):     ## Reset all DataField.ischanged
        for hole in range(len(self.holes)):
            self.holes[hole].ischanged = value

    def getField(self, hole):
        return self.holes[hole]

    def getText(self, hole):
        return self.holes[hole].text()

    def setText(self, hole, value):
        self.holes[hole].setText(str(value))

    def sigma(self):
        pass

class HoleInfoLabel(HoleInfo):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        for hole in range(1,19):    ## Fill hole numbers not set
            if not self.getText(hole): self.setText(hole, str(hole))

        self._setText(HOLE_ID,  'Hole')
        self._setText(HOLE_OUT, 'Out')
        self._setText(HOLE_IN,  'In')
        self._setText(HOLE_TOT, 'Tot')
        self._setText(HOLE_ESC, 'ESC')
        self._setText(HOLE_HCP, 'HCP')
        self._setText(HOLE_NET, 'NET')
        self._setText(HOLE_LDO, 'LDO')
        self._setText(HOLE_CPO, 'CPO')
        self._setText(HOLE_LDI, 'LDI')
        self._setText(HOLE_CPI, 'CPI')
        self._setText(HOLE_GIR, 'GIR')
        self._setText(HOLE_FWH, 'FWH')
        self._setText(HOLE_SKIN, 'Skin')

    def _setText(self, hole, value):
        if self.format > hole: self.setText(hole, value)

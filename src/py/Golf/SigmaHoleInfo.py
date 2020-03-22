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
##       SigmaHoleInfo.py
##
## Purpose-
##       Golf: Hole information, with OUT/IN/TOTAL
##
## Last change date-
##       2019/08/21
##
##############################################################################
import sys

#### PyQt5 ###################################################################
#### PyQt5.QtCore    import *
#### PyQt5.QtGui     import *
#### PyQt5.QtWidgets import *

#### lib #####################################################################
#### lib.Debug       import *
#### lib.Utility     import *

#### Golf ####################################################################
from GolfApplet      import *
from HoleInfo        import *

##############################################################################
##
## Class-
##       SigmaHoleInfo
##
## Purpose-
##       HoleInfo, computing HOLE_OUT, HOLE_IN, and HOLE_TOT
##
## Usage-
##       (Same constructor as HoleInfo)
##
##############################################################################
class SigmaHoleInfo(HoleInfo):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def __repr__(self):
        return '<SigmaHoleInfo(%s)>' % (self.holes[:FORMAT_MIN])

    def setText(self, index, value):
        super().setText(index, value)
        self.sigma()

    def sigma(self):
        print('sigma')
        out = 0
        try:
            for hole in range(1, 10):
                out += int(self.get(hole))
        except:
            out = 'N/A'
        super().setText(HOLE_OUT, str(out))

        inp = 0
        try:
            for hole in range(10, 19):
                inp += int(self.getText(hole))
        except:
            inp = 'N/A'
        super().setText(HOLE_IN, str(inp))

        tot = 0
        try:
            tot = str(int(out) + int(inp))
        except:
            tot = 'N/A'
        super().setText(HOLE_TOT, str(tot))


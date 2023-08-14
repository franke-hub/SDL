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
##       ResultEdit.py
##
## Purpose-
##       Golf: Edit results for event on date
##
## Last change date-
##       2019/08/18
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
from ResultView      import *

##############################################################################
##
## Class-
##       ResultEdit
##
## Purpose-
##       Edit event result, given eventID and date. All times included.
##
##############################################################################
class ResultEdit(ResultView):       ## An ResultEdit window
    def __init__(self, eventsID, eventsDate):
        super().__init__(eventsID, eventsDate)

    def debug(self):                ## Debugging display
        debugf('EventsCard.debug')
        super().debug()

    def _init(self):                ## Initialize the GUI
        logger('EventsCard._init');
        super()._init()

        self.setWindowTitle('ResultEdit')

    def _load(self):                ## Load the data
        logger('EventsCard._load');
        super()._load()

    def show(self):
        logger('EventsCard.show');
        super().show()

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
##       DataField.py
##
## Purpose-
##       Golf: Data field widget
##
## Last change date-
##       2019/08/21
##
## Implementation notes-
##       TODO: Remove debugging asserts??
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

##############################################################################
##
## Class-
##       Listener
##
## Purpose-
##       Golf: Data field Listener, called when text changed
##
##############################################################################
class Listener(object):             ## Data field Listener
    def changed(self, field):       ## DataField change event
        pass                        ## (The default Listener does nothing)

##############################################################################
##
## Class-
##       Validator
##
## Purpose-
##       Golf: Data field Validator
##
##############################################################################
class Validator(object):            ## Data field validator
    def validate(self, field):      ## Validate a DataField
        return True                 ## (The default Validator does nothing)

##############################################################################
##
## Class-
##       DataField
##
## Purpose-
##       Golf: Data field Widget
##
## Implementation notes-
##       Imported from QLabel:      ## (Reminder)
##           setText()
##           text()
##
##############################################################################
class DataField(QLabel, DatabaseInfo):
    def __init__(self, *args, **kwargs):
        super(QLabel,self).__init__()
        super(DatabaseInfo,self).__init__()

        self.listeners  = []        ## The Listener list
        self.validators = []        ## The Validator list

        ######################################################################
        ## Set QLabel default properties and attributes
        self.wordWrap = False
        self.setAlignment(Qt.AlignCenter)
        self.setFrameStyle(QFrame.Box | QFrame.Plain)
        self.setMargin(5)
        self.setTextFormat(Qt.PlainText) ## (No HTML)
        self.setTextInteractionFlags(Qt.NoTextInteraction)

        ######################################################################
        ## Argument analysis
        if len(args):               ## If arguments present
            assert len(args) == 1 and isinstance(args[0], str)
            self.setText(args[0])

        for arg in kwargs:          ## Handle keyword arguments
            if arg == 'rw':         ## Read/Write mode?
                self.setRW(kwargs['rw'])
            elif arg == 'ro':       ## Read/Only mode?
                self.setRW(not kwargs['ro'])
            else:
                raise KeyError(arg)

    def __repr__(self):
        return '<DataField(%s)>' % self.text()

    def debug(self):
        debugf('%s.debug' % self)
        ## TODO: May want to remove __repr__ and display text here
        ## TODO: Readable control flags, etc

    def addListener(self, listener):
        assert isinstance(listener, Listener)
        self.listeners += [listener]

    def addValidator(self, validator):
        assert isinstance(validator, Validator)
        self.validators += [validator]

    def setRW(self, mode):
        if mode:
            self.setTextInteractionFlags(Qt.TextEditorInteraction)
        else:
            self.setTextInteractionFlags(Qt.NoTextInteraction)

    def setText(self, text, changed=True):
        if not changed:
            super().setText(text)
        else:
            if not self.validate(): raise ValueError(self.text())
            super().setText(text)
            for listener in self.listeners:
                listener.changed(self)
            self.ischanged = True

    def validate(self):
        for validator in self.validators:
            if validator.validate(self) == False:
                return False
        return True

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
##       DababaseInfo.py
##
## Purpose-
##       Golf: Base class (database info)
##
## Last change date-
##       2019/08/05
##
##############################################################################
from DebuggingAdaptor import DebuggingAdaptor

class DatabaseInfo(DebuggingAdaptor):
    def __init__(self):
        super().__init__()
        self.ischanged = False
        self.ispresent = False
        self.isremoved = False

    def debug(self):                ## Debug THIS object
        super().debug()
        print('ischanged: %s' % self.ischanged)
        print('ispresent: %s' % self.ischanged)
        print('isremoved: %s' % self.ischanged)

    def debugf(self, *args, **kwargs):
        if self.isdebug(): Debug.debugf(*args, **kwargs)

    def tracef(self, *args, **kwargs):
        if self.isdebug(): Debug.tracef(*args, **kwargs)

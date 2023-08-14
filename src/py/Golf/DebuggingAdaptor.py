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
##       DebuggingAdaptor.py
##
## Purpose-
##       Golf: Base class (debugging methods)
##
## Last change date-
##       2019/08/05
##
##############################################################################
from lib.Debug import *

class DebuggingAdaptor(object):
    def __init__(self):
        pass

    def isdebug(self):              ## OVERRIDE this method
        return False

    def debug(self):                ## Debug THIS object
        print('module: ' + self.__module__)
        print('isdebug: %s' % self.isdebug())

    def debugf(self, *args, **kwargs):
        if self.isdebug(): Debug.debugf(*args, **kwargs)

    def tracef(self, *args, **kwargs):
        if self.isdebug(): Debug.tracef(*args, **kwargs)

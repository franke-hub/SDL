##############################################################################
##
##       Copyright (C) 2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Types.py
##
## Purpose-
##       Strongly typed objects. (See usage notes before using.)
##
## Last change date-
##       2018/01/01
##
## Usage example-
##       s = String('initial value')
##       print(s._)        ## get value
##       s._ = 'new value' ## set value
##       s._ = 1           ## ValueError (Not a string)
##
## Usage notes-
##       These classes impose significant additional overhead:
##           Reading: 4.5x; Writing (with type checking): 2.0x
##
##############################################################################
from __future__ import print_function
import sys
import threading
import time
import traceback

##############################################################################
## Define available imports
##############################################################################
__all__ = [ 'Bool', 'Integer', 'String' ]

##############################################################################
## Class Bool: Boolean value
##############################################################################
class Bool(object):
    def __init__(self, v=False):
        self._ = v

    @property
    def _(self):
        return self.__v

    @_.setter
    def _(self, v):
        if not isinstance(v, bool): raise ValueError('Bool(%s(%s))' % (type(v), v))
        self.__v = v

##############################################################################
## Class Integer: Integer value
##############################################################################
class Integer(object):
    def __init__(self, v=0):
        self._ = v

    @property
    def _(self):
        return self.__v

    @_.setter
    def _(self, v):
        if not isinstance(v, int): raise ValueError('Integer(%s(%s))' % (type(v), v))
        self.__v = int(v)           ## Because isinstance(bool, int) is True

##############################################################################
## Class String: String value
##############################################################################
class String(object):
    def __init__(self, v=''):
        self._ = v

    @property
    def _(self):
        return self.__v

    @_.setter
    def _(self, v):
        if not isinstance(v, str): raise ValueError('String(%s(%s))' % (type(v), v))
        self.__v = v

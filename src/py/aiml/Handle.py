#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2017 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Handle.py
##
## Purpose-
##       Handle exceptions.
##
## Last change date-
##       2017/01/01
##
##############################################################################
from __future__ import print_function

import sys
import traceback

def exception():
    x_type, x_value, x_tb = sys.exc_info()
    traceback.print_exception(x_type, x_value, x_tb)

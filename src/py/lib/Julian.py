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
##       Julian.py
##
## Purpose-
##       Julian date and time.
##
## Last change date-
##       2019/08/27
##
## Implementation notes-
##       ** NOT CODED YET **
##
##############################################################################
import sys
import datetime

##############################################################################
## Constants
##############################################################################
_Gregorian = None                   ## First Gregorian calendar date
                                    ## NOT IMPLEMENTED

##############################################################################
##
## Title-
##       Julian
##
## Purpose-
##       Julian date and time container.
##
## Implementation note-
##       ** NOT IMPLEMENTED YET ** (Only supports float get/set)
##       TO BE implemented as a Calendar date, with a midnight origin.
##       WILL NOT account for leap seconds. Add a subclass if you want.
##
##       Date and time contained as a float. On most systems, this provides
##       more accuracy than can be physically measured:
##           Millisecond for billions of years.
##           Microsecond for millions of years.
##           Nanosecond for thousands of years.
##
##############################################################################
class Julian():                     ## The Julian date and time
    def __init__(self):
        self._julian = 0.0          ## NOT CODED YET

    @property                       ## Returns float
    def date(self):                 ## Get the Julian date
        return self._julian;

    @date.setter
    def date(self, value):          ## Set the Julian date
        if isinstance(value, float):
            self._julian = value
        elif isinstance(value, datetime):
            raise NotImplementedError('type(%s) NOT CODED YET' % type(value))
        else:
            raise TypeError('type(%s) not implemented' % type(value))

    @property
    def datetime(self):             ## Get the Julian calendar date
        raise NotImplementedError('Not coded yet')

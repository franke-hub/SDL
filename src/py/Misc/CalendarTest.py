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
##       CalendarTest.py
##
## Purpose-
##       Test theoretical calendar.
##
## Last change date-
##       2019/09/04
##
## Implementation note-
##       So far: no good.
##
##############################################################################
import sys
import calendar

#### lib #####################################################################
from lib.Command     import *
from lib.Debug       import *
from lib.Global      import *
from lib.Utility     import *

##############################################################################

VERBOSE = None
def initialize():
    global VERBOSE
    VERBOSE = Global.VERBOSE

##############################################################################
## The CalendarTest application
##############################################################################
_DIY = 365.2422                     ## Number of days in a year

###################################### Adjustment factors
USE_METHOD = 1
_000 = 0                            ## Calendar year 0
_NEG = 0                            ## Negative year adjustment
_POS = 0                            ## Positive year adjustment

## Adjustment adjustments
if False: ## USE_METHOD == N        ## (Not implemented)
    _000 = 0                        ## Calendar year 0
    _NEG = 0                        ## Negative year adjustment
    _POS = 0                        ## Positive year adjustment

def tester(year):                   ## Doing the conjectured math
    test = year + _000
    ok = 'OK'

    if USE_METHOD == 0:
        ## 1st century adjustment year: 32, next: 128, .., 2049, 2084
        if test > 0:
           test = year + _POS
           thisYear = (test) * _DIY
           nextYear = (test+1) * _DIY
        else:
            test = year + _NEG
            thisYear = test * _DIY
            nextYear = (test+1) * _DIY
        diffDays = int(nextYear) - int(thisYear)
        isleap = diffDays > 365
        if isleap != calendar.isleap(year): ok = '** NG'
        print('test: %5s %.4d %3d %10.2f %10.2f %s' % \
             (isleap, year, diffDays, thisYear, nextYear, ok))

    else:
        ## Number of years between leap years either 3, 4, or 7. (Mostly 3)
        thisYear = (test) * _DIY
        nextYear = (test+1) * _DIY

        diffDays = int(thisYear)
        isleap = (diffDays % 4) == 0
        if isleap != calendar.isleap(year): ok = '** NG'

        print('test: %5s %.4d %10.2f %10d %d %s' % \
             (isleap, year, thisYear, diffDays, diffDays%4, ok))

    return isleap

class __CalendarTest:
    @staticmethod
    def run(*argv):
        initialize()
        if VERBOSE < 2:
            print('Skipped: VERBOSE < 2:', VERBOSE)
            print('So far, NO good') ## The short story
            return 0                ## (Which is OK)

        print('USE_METHOD:' , USE_METHOD)
        errors = 0

        ok = 'OK'
        oopsie = None

        for year in range(201):
            myleap = tester(year)
            isleap = calendar.isleap(year)
            if myleap != isleap:
                errors += 1
                ok = 'NO'
                if oopsie is None: oopsie = 'Failed year(%s)' % year

        print('')
        min = 1970
        max = 2170
        for year in range(min, max + 1):
            myleap = tester(year)
            isleap = calendar.isleap(year)
            if myleap != isleap:
                errors += 1
                ok = 'NO'
                if oopsie is None: oopsie = 'Failed year(%s)' % year

        if oopsie is None: oopsie = 'Wahoo!'
        printf('So far, %s good: %s' % (ok, oopsie))
        return errors

command['CalendarTest'] = __CalendarTest

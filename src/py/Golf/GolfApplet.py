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
##       GolfApplet.py
##
## Purpose-
##       Golf: Application window and utility functions
##
## Last change date-
##       2019/08/16
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

##############################################################################
## Exports
##############################################################################
__all__  = []                       ## (Classes and funcions add themselves)

##############################################################################
##
## Class-
##       GolfApplet
##
## Purpose-
##       The golfer application base window.
##
##############################################################################
class GolfApplet(QDialog):          ## The golfer application base window
    def __init__(self):             ## Constructor
        super().__init__()          ## INVOKE this method

    def debug(self):                ## Debugging display
        pass                        ## OVERRIDE this method

    def _init(self):                ## Initialize graphic attributes
        pass                        ## OVERRIDE this method

    def _load(self):                ## Load the data
        pass                        ## OVERRIDE this method

##  QDialog methods, almost always overridden ################################
##  def close(self):                ## Default close
##      return super().close()      ## INVOKE this method
##
##  def show(self):                 ## Application layout
##      return super().show()       ## INVOKE this method

__all__ += ['GolfApplet']

##############################################################################
##
## Function-
##       catcon(str, index)
##
## Purpose-
##       Inversion on concat: catcon('AA.BB.CC', 1) == 'BB'
##
##############################################################################
def catcon(S, index):             ## Get string[index] from concat S
    offset = 0                    ## Working offset
    while index > 0:
        X = S.find('.', offset)
        if X < 0: return None
        offset = X + 1
        index -= 1

    X = S.find('.', offset)
    if X > 0:
        return S[offset:X]
    return S[offset:]
__all__ += ['catcon']

##############################################################################
##
## Function-
##       concat(str, str, ...)
##
## Purpose-
##       Concatenate: concat('aa', 'bb', 'cc') == 'aa.bb.cc')
##
##############################################################################
def concat(*args):                ## Concatenate arguments
    return '.'.join(str(arg) for arg in args)
__all__ += ['concat']

##############################################################################
##
## Function-
##       loadCourseID(str)         ## Param: Either courseID or None
##
## Purpose-
##       courseID = loadCourseID(courseID), courseID verified valid
##
##############################################################################
def loadCourseID(courseID):       ## Extract courseID
    if not courseID:              ## If courseID not specified
        courseID = dbGet(CMD_DEFAULT, DEFAULT_CI) ## Get default courseID
        assert courseID, dbMissing(CMD_DEFAULT, DEFAULT_CI)
        courseID = courseID[0]

    resultID = dbGet(FIND_COURSE, courseID) ## Verify the courseID
    if not resultID:
        raise Exception(dbMissing(FIND_COURSE, courseID))
    return resultID[0]
__all__ += ['loadCourseID']

##############################################################################
##
## Function-
##       loadEventsID(str)         ## Param: Either eventsID or None
##
## Purpose-
##       eventsID = loadEventsID(eventsID), eventsID verified valid
##
##############################################################################
def loadEventsID(eventsID):       ## Extract eventsID
    if not eventsID:              ## If eventsID not specified
        eventsID = dbGet(CMD_DEFAULT, DEFAULT_EI) ## Get default eventsID
        assert eventsID, dbMissing(CMD_DEFAULT, DEFAULT_EI)
        eventsID = eventsID[0]

    resultID = dbGet(FIND_EVENTS, eventsID) ## Verify the eventsID
    if not resultID:
        raise Exception(dbMissing(FIND_EVENTS, eventsNN))
    return resultID[0]
__all__ += ['loadEventsID']

##############################################################################
##
## Function-
##       loadPlayerName(str)      ## Param: Either playerID, playerNN, or None
##
## Purpose-
##       (playerID, playerNN) = loadPlayerName(name)
##
##############################################################################
def loadPlayerName(name):         ## Extract playerID and playerNN
    if not name:                  ## If name not specified
        name = dbGet(CMD_DEFAULT, DEFAULT_PI) ## Get default playerID
        assert name, dbMissing(CMD_DEFAULT, DEFAULT_PI)
        name = name[0]

    playerNN = name.upper()       ## Default, name is playerNN
    playerID = dbGet(FIND_PLAYER, playerNN) ## Get the associated playerID
    if not playerID:
        raise Exception(dbMissing(FIND_PLAYER, playerNN))
    playerID = playerID[0]
    if playerID == playerNN:      ## If we only have the playerID
        ITEM = ''                 ## Search for nickname
        while True:
            data = dbNext(FIND_PLAYER, ITEM)
            if not data: break    ## If no nickname

            ITEM = data[1]
            if playerNN != ITEM and playerID == data[2]:
                playerNN = ITEM
                break
    return (playerID, playerNN)
__all__ += ['loadPlayerName']

##############################################################################
##
## Function-
##       logger
##
## Purpose-
##       Write to log
##
## Implementation notes-
##       Using logger within ANY __del__ is problematic; if called from global
##       application termination, data (and features) may be missing.
##       Example: the open built-in may have already been deleted.
##
##       TODO: Use tracef (rather than the current debugf.)
##
##############################################################################
def logger(*args, **kwargs):      ## Logging output
    debugf(*args, **kwargs)       ## FX_00002: Exception handling now in Debug.writef
__all__ += ['logger']

##############################################################################
##
## Section-
##       Time and date utilities
##
## Purpose-
##       fixDate:   Convert 'm/d/yy' to 'mm/dd/yyyy'
##       fixTime:   Convert 'h:m' to 'hh:mm'
##       ################## 0123456789
##       showDate:  Convert yyyy/mm/dd to mm/dd/yyyy
##       sortDate:  Convert mm/dd/yyyy to yyyy/mm/dd
##
##############################################################################
def _dateException(date):       ## Raise date error exception
    raise Exception('Invalid date(%s)' % date)

def _timeException(time):       ## Raise date error exception
    raise Exception('Invalid time(%s)' % time)

def fixDate(date):                ## Convert m/d/yy to mm/dd/yyyy
    _dateException(date) ## NOT CODED YET

def fixTime(time):                ## Convert h:m hh:mm
    _timeException(time) ## NOT CODED YET

def showDate(date):               ## Convert yyyy/mm/dd to mm/dd/yyyy
    return date[5:7] + '/' + date[8:] + '/' + date[0:4]

def sortDate(date):               ## Convert mm/dd/yyyy to yyyy/mm/dd
    return date[6:] + '/' + date[0:2] + '/' + date[3:5]

####### += ['fixDate',  'fixTime'] ## NOT CODED YET
__all__ += ['showDate', 'sortDate']

##############################################################################
##
## Section-
##       Gt: Qt GUI utilities
##
## Functions-
##       Gt.align:     Set a QWidget's (or QLayout's) alignment
##       Gt.setBG:     Set a QWidget's background color
##
##############################################################################
class Gt:
    def align(W, A):              ## Set a QWidget's alignment, return Widget
        W.setAlignment(A)         ## (Also works for QLayout)
        return W

    def setBg(W, C):              ## Set a Widget's background color
        P = W.palette()
        P.setColor(W.backgroundRole(), C)
        W.setPalette(P)
        return W
__all__ += ['Gt']

##############################################################################
##
## Section-
##       Hole number constants
##
##############################################################################
## Hole frame field index
HOLE_ID     =  0 ## Hole title index
HOLE_OUT    = 19 ## OUT index
HOLE_IN     = 20 ## IN  index
HOLE_TOT    = 21 ## TOT index

## Optional fields
HOLE_ESC    = 22 ## ESC index
HOLE_HCP    = 23 ## HCP index
HOLE_NET    = 24 ## NET index

HOLE_LDO    = 25 ## LDO index (Longest drive:  OUT)
HOLE_CPO    = 26 ## CPO index (Closest to pin: OUT)
HOLE_LDI    = 27 ## LDI index (Longest drive:  IN)
HOLE_CPI    = 28 ## CPI index (Closest to pin: IN)

HOLE_FWH    = 29 ## FWH index (Fairways hit)
HOLE_GIR    = 30 ## GIR index (Greens in regulation)

HOLE_SKIN   = 31 ## Skins index

## Format constants (Number of columns in frame)
FORMAT_BASE = HOLE_NET  + 1
FORMAT_EVNT = HOLE_GIR  + 1
FORMAT_SKIN = HOLE_SKIN + 1
FORMAT_USER = HOLE_TOT  + 1

FORMAT_MAX  = FORMAT_SKIN
FORMAT_MIN  = FORMAT_BASE

## Field special values
FIELD_DP    = 'DP'   ## Double par
FIELD_ERR   = '!ERR' ## Error!

## Other constants
CLOSE_MAX   = 100    ## Closest to pin maximum value (in feet)

## Export constants
__all__ += [ 'HOLE_ID',  'HOLE_OUT', 'HOLE_IN',  'HOLE_TOT', 'FORMAT_USER'
           , 'HOLE_ESC', 'HOLE_HCP', 'HOLE_NET',             'FORMAT_BASE'
           , 'HOLE_LDO', 'HOLE_CPO', 'HOLE_LDI', 'HOLE_CPI'
           , 'HOLE_FWH', 'HOLE_GIR',                         'FORMAT_EVNT'
           , 'HOLE_SKIN',                                    'FORMAT_SKIN'
           , 'FORMAT_MAX', 'FORMAT_MIN'
           , 'FIELD_DP',   'FIELD_ERR',  'CLOSE_MAX'
           ]


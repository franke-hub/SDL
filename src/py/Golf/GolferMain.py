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
##       GolferMain.py
##
## Purpose-
##       Golf: Main window
##
## Last change date-
##       2019/08/11
##
## Known problems-
##       Unable to set yellow BG: CourseEdit, PlayerEdit, ResultEdit
##
##############################################################################
import sys

import gc

##############################################################################
## Compliation controls
##############################################################################
TIMER = 10*1000                     ## Tick timer, in milliseconds

#### PyQt5 ###################################################################
from PyQt5.QtCore    import *
from PyQt5.QtGui     import *
from PyQt5.QtWidgets import *

#### lib #####################################################################
from lib.Debug       import *
from lib.Utility     import *

#### Golf ####################################################################
from DbServer        import *
from GolfApplet      import *
from GolferTest      import GolferTest
from ResultEdit      import *
from ResultView      import *
from WindowList      import *

##############################################################################
##
## Function-
##       _keepLeft
##
## Purpose-
##       Keep a layout left when window expanded
##
## Implementation notes-
##       Function not needed when dates are left of label
##
##############################################################################
def _keepLeft(L):
    print('MAIN._keepLeft')
    I = L                           ## Save input layout
    I.addStretch(4)                 ## (Keep left function)
    L = QHBoxLayout()
    L.setAlignment(Qt.AlignLeft)
    L.addLayout(I)
    L.addStretch()
    return L

def _keepLeft(L):                   ## This disables the KeepLeft function
    return L

##############################################################################
##
## Function-
##       _warning
##
## Purpose-
##       Create warning: DOES NOT EXIT until OK button pressed!
##
##############################################################################
def _warning(text):                 ## Warning window
    logger('MAIN.warning(%s) ****************************************' % text)
    exit()                          ## TODO: Something else

##############################################################################
##
## Class-
##       GolferMain
##
## Purpose-
##       The golfer application control window.
##
##############################################################################
class GolferMain(GolfApplet):       ## The application control window
    def __init__(self):
        super().__init__()

        ## Initialize main window
        self.setGeometry(10, 20, -1, -1)
        self.setWindowTitle('GolferMain')

        ## Set background timer
        timer = QTimer(self)
        timer.timeout.connect(self._tick)
        timer.start(TIMER)
        self.timer = timer          ## Just in case we need it later

        ## Non-graphic attributes
        self.courseDict = None      ## The Course dictionary
        self.eventsDate = None      ## The list of dates for selected event
        self.eventsList = None      ## The list of Events
        self.playerDict = None      ## The Player dictionary
        self.windowList = WindowList(self) ## The active window list

        ## Selection attributes
        self.eventsID   = None      ## The selected event

        ## Graphic attributes
        self.addCourseButton    = Gt.setBg(QPushButton('Add Course'), Qt.yellow)
        self.addEventsButton    = Gt.setBg(QPushButton('Add Event'), Qt.yellow)
        self.addPlayerButton    = Gt.setBg(QPushButton('Add Player'), Qt.yellow)
        self.closeButton        = Gt.setBg(QPushButton('Close'), Qt.yellow)
        self.courseEditComboBox = Gt.setBg(QComboBox(), Qt.yellow) ## TODO: FAILS
        self.courseViewComboBox = QComboBox()
        self.dbStartButton      = Gt.setBg(QPushButton('Database Start'), Qt.yellow)
        self.dbStopButton       = Gt.setBg(QPushButton('Database Stop'), Qt.yellow)
        self.debugButton        = Gt.setBg(QPushButton('Debug'), Qt.yellow)
        self.eventsEditButton   = Gt.setBg(QPushButton('Update'), Qt.yellow)
        self.eventsListComboBox = QComboBox()
        self.eventsShow         = QLabel('')
        self.playerEditComboBox = Gt.setBg(QComboBox(), Qt.yellow) ## TODO: FAILS
        self.playerHdcpComboBox = QComboBox()
        self.reportPlayButton   = QPushButton('Player Scores')
        self.reportStatButton   = QPushButton('Statistics')
        self.resultEditComboBox = Gt.setBg(QComboBox(), Qt.yellow) ## TODO: FAILS
        self.resultViewComboBox = QComboBox()
        self.setDefaultsButton   = Gt.setBg(QPushButton('Set Defaults'), Qt.yellow)
        self.testButton         = Gt.setBg(QPushButton('Test'), Qt.yellow)
        self.terminateButton    = Gt.setBg(QPushButton('Terminate'), Qt.red)
        self.timerStartButton   = Gt.setBg(QPushButton('Timer Start'), Qt.yellow)
        self.timerStopButton    = Gt.setBg(QPushButton('Timer Stop'), Qt.yellow)

        ## Load and initialize the attributes
        self._load()                ## Load the attributes
        self._init()                ## Initialize graphic attributes
        self.show()                 ## Display the main window

    def debug(self):                ## Debugging display
        debugf('MAIN.debug')
##      debugf('courseDict: %s' % self.courseDict)
##      debugf('eventsDate: %s' % self.eventsDate)
##      debugf('eventsID: %s'   % self.eventsID)
##      debugf('eventsList: %s' % self.eventsList)
##      debugf('playerDict: %s' % self.playerDict)
        self.windowList.static_debug() ## TODO: Just debug

        debugf('gc.get_count: %s' % str(gc.get_count()))
        debugf('gc.isenabled: %s' % gc.isenabled())
        debugf('gc.collect: ** CALLED **')
        gc.collect()
        debugf('gc.get_count: %s' % str(gc.get_count()))

    def _init(self):                ## Initialize graphic attributes
        logger('MAIN._init');

        self.addCourseButton.clicked.connect(self.selectAddCourse)
        self.addEventsButton.clicked.connect(self.selectAddEvents)
        self.addPlayerButton.clicked.connect(self.selectAddPlayer)

        self.closeButton.clicked.connect(self.selectClose)

        keys = sorted(self.courseDict.keys())
        self.courseEditComboBox.addItems(keys)
        self.courseEditComboBox.activated[str].connect(self.selectCourseEdit)

        self.courseViewComboBox.addItems(keys)
        self.courseViewComboBox.activated[str].connect(self.selectCourseView)

        self.dbStartButton.clicked.connect(self.selectDbStart)
        self.dbStopButton.clicked.connect(self.selectDbStop)

        self.eventsEditButton.clicked.connect(self.selectEditEvents)

        self.eventsListComboBox.addItems(self.eventsList)
        self.eventsListComboBox.activated[str].connect(self.selectEventsList)

        self.eventsShow.setAlignment(Qt.AlignCenter)

        keys = sorted(self.playerDict.keys())
        self.playerEditComboBox.addItems(keys)
        self.playerEditComboBox.activated[str].connect(self.selectPlayerEdit)

        self.playerHdcpComboBox.addItems(keys)
        self.playerHdcpComboBox.activated[str].connect(self.selectPlayerHdcp)

        self.reportPlayButton.clicked.connect(self.selectReportPlay)
        self.reportStatButton.clicked.connect(self.selectReportStat)

        self.resultEditComboBox.addItems(self.eventsDate)
        self.resultEditComboBox.activated[str].connect(self.selectResultEdit)

        self.resultViewComboBox.addItems(self.eventsDate)
        self.resultViewComboBox.activated[str].connect(self.selectResultView)

        self.setDefaultsButton.clicked.connect(self.selectSetDefaults)

        self.debugButton.clicked.connect(self.selectDebug)

        self.testButton.clicked.connect(self.selectTest)

        self.terminateButton.clicked.connect(self.selectTerminate)

        self.timerStartButton.clicked.connect(self.selectTimerStart)
        self.timerStopButton.clicked.connect(self.selectTimerStop)

    def _loadCourseDict(self):      ## Load the course dictionary
        logger('MAIN._loadCourseDict');

        self.courseDict = dict()    ## Empty course dictionary
        ITEM = ''
        while True:
            data = dbNext(FIND_COURSE, ITEM)
            if not data: break

            ITEM = data[1]
            data = dbGet(COURSE_SHOW, ITEM)
            if data:
                data = data[0]
                if data in self.courseDict:
                    dup = self.courseDict[data]
                    text = '%s {%s | %s}' % (COURSE_SHOW, dup, ITEM)
                    text += " == '%s'" % data
                    _warning(dbError(text))
                else:
                    self.courseDict[data] = ITEM
            else:
                _warning(dbMissing(COURSE_SHOW, ITEM))

    def _loadEventsDate(self):      ## Load the event dates
        logger('MAIN._loadEventsDate(%s)' % self.eventsID);

        self.eventsDate = []        ## Empty date list
        DATE = ''
        while True:
            data = dbNext(EVENTS_DATE, concat(self.eventsID,DATE))
            if not data: break

            DATE = data[1]
            if catcon(DATE, 0) != self.eventsID: break

            DATE = catcon(DATE, 1)
            self.eventsDate.append(showDate(DATE))

    def _loadEventsList(self):      ## Load the events list
        logger('MAIN._loadEventsList');

        self.eventsList = []        ## Empty events list
        ITEM = ''
        while True:
            data = dbNext(FIND_EVENTS, ITEM)
            if not data: break

            ITEM = data[1]
            self.eventsList.append(ITEM)
        self.eventsList.reverse()   ## Invesavehe list order

        if len(self.eventsList):
            self.eventsID   = self.eventsList[0]
            self._loadEventsDate()
####        self.eventsShow.setText(dbGet(EVENTS_SHOW, self.eventsID)[0] + '\n')
            self.eventsShow.setText(dbGet(EVENTS_SHOW, self.eventsID)[0])
        else:
            _warning('No events exist')
            self.eventsShow.setText('No events exist')
            self.eventsList = []
            self.eventsDate = []
            self.eventsID   = None

    def _loadPlayerDict(self):      ## Load the player dictionary
        logger('MAIN._loadPlayerDict');

        self.playerDict = dict()    ## Empty player dictionary
        ITEM = ''
        while True:
            data = dbNext(FIND_PLAYER, ITEM)
            if not data: break

            ITEM = data[1]
            if data[2] != ITEM: continue ## Ignore nickname

            data = dbGet(PLAYER_SHOW, ITEM)
            if data:
                data = data[0]
                if data in self.playerDict:
                    dup = self.playerDict[data]
                    text = '%s {%s | %s}' % (PLAYER_SHOW, dup, ITEM)
                    text += " == '%s'" % data
                    _warning(dbError(text))
                else:
                    self.playerDict[data] = ITEM
            else:
                _warning(dbMissing(PLAYER_SHOW, ITEM))

    def _load(self):                ## Load the data
        logger('MAIN._load');

        self._loadCourseDict()      ## Load the course dictionary
        self._loadEventsList()      ## Load the list of events (+ eventsDate)
        self._loadPlayerDict()      ## Load the player dictionary

    def close(self):                ## Log close
        logger('MAIN.close')
        self.windowList.remove()    ## We're outta here
        return super().close()

    def focusEventsList(self):      ## Focus event: event list selector
        logger('MAIN.focusEventList')
        self._loadEventsList()

    def focusResultEdit(self):      ## Focus event: ResultEdit date selector
        logger('MAIN.focusResultEdit')
        self._loadEventsDate()

        self.resultEditComboBox.clear()
        self.resultEditComboBox.addItems(self.eventsDate)
        self.resultEditComboBox.show()

    def focusResultView(self):      ## Focus event: ResultView date selector
        logger('MAIN.focusResultView')
        self._loadEventsDate()

        self.resultViewComboBox.clear()
        self.resultViewComboBox.addItems(self.eventsDate)

        self.resultViewComboBox.show()

    def selectAddCourse(self):      ## Select AddCourse
        logger('MAIN.selectAddCourse')
        ## NOT CODED YET
        self.windowList.append(GolferTest('AddCourse'))

    def selectAddEvents(self):      ## Select AddEvents
        logger('MAIN.selectAddEvents')
        ## NOT CODED YET
        self.windowList.append(GolferTest('AddEvents'))

    def selectAddPlayer(self):      ## Select AddPlayer
        logger('MAIN.selectAddPlayer')
        ## NOT CODED YET
        self.windowList.append(GolferTest('AddPlayer'))

    def selectClose(self):
        logger('MAIN.selectClose')
        self.windowList.close()

    def selectCourseEdit(self, name): ## Select CourseEdit course
        courseID = self.courseDict[name]
        logger('MAIN.selectCourseEdit(%s): %s' % (name, courseID))
        ## NOT CODED YET
        self.windowList.append(GolferTest('CourseEdit', courseID))

    def selectCourseView(self, name): ## Select CourseView course
        courseID = self.courseDict[name]
        logger('MAIN.selectCourseView(%s): %s' % (name, courseID))
        ## NOT CODED YET
        self.windowList.append(GolferTest('CourseView', courseID))

    def selectDbStart(self):
        logger('MAIN.selectDbStart')
        ## NOT CODED YET

    def selectDbStop(self):
        logger('MAIN.selectDbStop')
        ## NOT CODED YET

    def selectDebug(self):
        logger('MAIN.selectDebug')
        self.debug()

    def selectEditEvents(self): ## Select EditEvents
        logger('MAIN.selectEditEvents(%s)' % self.eventsID)
        ## NOT CODED YET
        self.windowList.append(GolferTest('EditEvents', self.eventsID))

    def selectEventsList(self,eventsID): ## Select event
        logger('MAIN.selectEventList(%s)' % eventsID)

        self.eventsID = eventsID    ## Remember selected event
        self.focusResultEdit()      ## Update the associated dates
        self.focusResultView()      ## Update the associated dates

    def selectResultEdit(self,date): ## Select ResultEdit date
        logger('MAIN.selectResultEdit(%s)' % date)
        self.windowList.append(ResultEdit(self.eventsID, sortDate(date)))

    def selectResultView(self,date): ## Select ResultView date
        logger('MAIN.selectResultView(%s)' % date)
        self.windowList.append(ResultView(self.eventsID, sortDate(date)) )

    def selectPlayerEdit(self, name): ## Select PlayerEdit course
        playerID = self.playerDict[name]
        logger('MAIN.selectPlayerEdit(%s): %s' % (name, playerID))
        ## NOT CODED YET
        self.windowList.append(GolferTest('PlayerEdit', playerID))

    def selectPlayerHdcp(self, name): ## Select PlayerHdcp course
        playerID = self.playerDict[name]
        logger('MAIN.selectPlayerHdcp(%s): %s' % (name, playerID))
        ## NOT CODED YET
        self.windowList.append(GolferTest('PlayerHdcp', playerID))

    def selectReportPlay(self):     ## Select ReportPlay
        logger('MAIN.selectReportPlay(%s)' % self.eventsID)
        ## NOT CODED YET
        self.windowList.append(GolferTest('ReportPlay', self.eventsID))

    def selectReportStat(self):     ## Select ReportStat
        logger('MAIN.selectReportStat(%s)' % self.eventsID)
        ## NOT CODED YET
        self.windowList.append(GolferTest('ReportStat', self.eventsID))

    def selectSetDefaults(self):    ## Select SetDefaults
        logger('MAIN.selectSetDefaults')
        ## NOT CODED YET
        self.windowList.append(GolferTest('SetDefaults'))

    def selectTerminate(self):
        logger('MAIN.selectTerminate')
        self.close()

    def selectTest(self):
        logger('MAIN.selectTest')
        W = GolferTest('GolferTest', 'GolferMain')
        self.windowList.append(W)

    def selectTimerStart(self):
        logger('MAIN.selectTimerStart')
        self.timer.start(TIMER)

    def selectTimerStop(self):
        logger('MAIN.selectTimerStop')
        self.timer.stop()

    def show(self):
        logger('MAIN.show');

        main = QVBoxLayout()
        main.setContentsMargins(5, 5, 5, 5)

        main.addWidget(self.eventsShow) ## Event title label

        G = QGridLayout()

        L = QHBoxLayout()
        L.setAlignment(Qt.AlignLeft)
        W = QLabel('View result')
        W.setBuddy(self.resultViewComboBox)
        L.addWidget(self.resultViewComboBox)
        L.addWidget(W)
        L.addStretch()
        G.addLayout(L, 0, 0, 1, 1)

        L = QHBoxLayout()
        L.setAlignment(Qt.AlignLeft)
        W = QLabel('View player hdcp')
        W.setBuddy(self.playerHdcpComboBox)
        L.addWidget(self.playerHdcpComboBox)
        L.addWidget(W)
        L.addStretch()
        G.addLayout(L, 0, 1, 1, 1)

        G.addWidget(self.reportStatButton, 1, 0, 1, 1)
        G.addWidget(self.reportPlayButton, 1, 1, 1, 1)

        L = QHBoxLayout()
        L.setAlignment(Qt.AlignLeft)
        W = QLabel('View course')
        W.setBuddy(self.courseViewComboBox)
        L.addWidget(self.courseViewComboBox)
        L.addWidget(W)
        L.addStretch()
        G.addLayout(L, 2, 0, 1, 1)
        main.addLayout(G)

        main.addWidget(QLabel())    ## Spacer
        main.addStretch()
        main.addWidget(Gt.align(QLabel('Maintenance'), Qt.AlignCenter))

        G = QGridLayout()

        L = Gt.align(QHBoxLayout(), Qt.AlignLeft)
        W = Gt.setBg(QLabel('Edit result'), Qt.yellow) ## TODO: FIX
        W.setBuddy(self.resultEditComboBox)
        L.addWidget(self.resultEditComboBox)
        L.addWidget(W)
        L.addStretch()
        G.addLayout(L, 0, 0, 1, 1)

        L = QHBoxLayout()
        L.setAlignment(Qt.AlignLeft)
        W = QLabel('Select Event')
        W.setBuddy(self.eventsListComboBox)
        L.addWidget(self.eventsListComboBox)
        L.addWidget(W)
        L.addWidget(self.eventsEditButton)
        L.addStretch()
        G.addLayout(L, 0, 1, 1, 1)

        L = QHBoxLayout()
        L.setAlignment(Qt.AlignLeft)
        Gt.setBg(QLabel('Edit course'), Qt.yellow) ## TODO: FIX
        W.setBuddy(self.courseEditComboBox)
        L.addWidget(self.courseEditComboBox)
        L.addWidget(W)
        L.addStretch()
        G.addLayout(L, 1, 0, 1, 1)

        L = QHBoxLayout()
        L.setAlignment(Qt.AlignLeft)
        W = Gt.setBg(QLabel('Edit player'), Qt.yellow) ## TODO: FIX
        W.setBuddy(self.playerEditComboBox)
        Gt.setBg(self.playerEditComboBox, Qt.yellow) ## TODO: FIX/REMOVE
        L.addWidget(self.playerEditComboBox)
        L.addWidget(W)
        L.addStretch()
        G.addLayout(L, 1, 1, 1, 1)

        G.addWidget(self.addPlayerButton, 2, 0, 1, 1)
        G.addWidget(self.addCourseButton, 2, 1, 1, 1)
        G.addWidget(self.addEventsButton, 3, 0, 1, 1)
        G.addWidget(self.setDefaultsButton, 3, 1, 1, 1)

        main.addLayout(G)

        main.addWidget(QLabel())    ## Spacer
        main.addStretch()
        main.addWidget(Gt.align(QLabel('Controls'), Qt.AlignCenter))

        G = QGridLayout()
        G.addWidget(self.testButton,       0, 0, 1, 1)
        G.addWidget(self.closeButton,      0, 1, 1, 1)
        G.addWidget(self.dbStopButton,     1, 0, 1, 1)
        G.addWidget(self.dbStartButton,    1, 1, 1, 1)
        G.addWidget(self.timerStopButton,  2, 0, 1, 1)
        G.addWidget(self.timerStartButton, 2, 1, 1, 1)

        G.addWidget(self.debugButton,     3, 0, 1, 1)
        G.addWidget(self.terminateButton, 3, 1, 1, 1)
        main.addLayout(G)

        self.setLayout(main)
        super().show()
        return True

    def _tick(self):                ## Handle background timer
        writef()
        tracef('MAIN._tick')
        tracef('gc.get_count: %s' % str(gc.get_count()))
        tracef('gc.isenabled: %s' % gc.isenabled())
        tracef('gc.collect: ** CALLED **')
        gc.collect()
        tracef('gc.get_count: %s' % str(gc.get_count()))
        Debug.get().flush()

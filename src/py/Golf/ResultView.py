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
##       ResultView.py
##
## Purpose-
##       Golf: View results for event on date
##
## Last change date-
##       2019/08/21
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
from GolfApplet      import *
from GolfLayout      import *
from HoleInfo        import *
from SigmaHoleInfo   import *
from TeamInfo        import *

##############################################################################
##
## Class-
##       ResultView
##
## Purpose-
##       Edit event result, given eventID and date. All times included.
##
##############################################################################
class ResultView(GolfApplet):       ## An ResultView window
    def __init__(self, eventsID, eventsDate):
        super().__init__()

        self.eventsID = eventsID    ## The event identifier
        self.eventsNN = None        ## The event nickname
        self.skinPane = None        ## The EventsSkinPane (for team coloring)
        self.skinList = None        ## The EventsSkinPane (for beer fund)
        self.teamList = []          ## The EventsTeamInfo list
        self.teeboxID = None        ## The teebox identifier

        self.courseHdcp = None      ## The course handicap infomation
        self.courseHole = None      ## The course hole name infomation
        self.courseLdcp = None      ## The course LD/CP infomation
        self.coursePars = None      ## The course par infomation
        self.courseShow = None      ## The course name
        self.courseTbox = None      ## The course teebox infomation
        self.eventsDate = eventsDate ## The event date
        self.eventsShow = None      ## The event name

        ## Startup
        self._load()                ## Load the data
        self._init()                ## Initialize the GUI
        self.show()                 ## Show the window

    def debug(self):                ## Debugging display
        debugf('EventsView.debug')
        debugf('eventsID: %s' % self.eventsID)
        debugf('eventsNN: %s' % self.eventsNN)
        debugf('skinPane: %s' % self.skinPane)
        debugf('skinList: %s' % self.skinList)
        debugf('teamList: %s' % self.teamList)
        debugf('teeboxID: %s' % self.teeboxID)

        debugf('courseHdcp: %s' % self.courseHdcp)
        debugf('courseHole: %s' % self.courseHole)
        debugf('courseLdcp: %s' % self.courseLdcp)
        debugf('coursePars: %s' % self.coursePars)
        debugf('courseShow: %s' % self.courseShow)
        debugf('courseTbox: %s' % self.courseTbox)
        debugf('eventsDate: %s' % self.eventsDate)
        debugf('eventsShow: %s' % self.eventsShow)

    def _init(self):                ## Initialize graphic objects
        self.setWindowTitle('ResultView')

    def _load(self):                ## Load the data
        logger('EventsView._load');

        if not self.eventsID:
            self.eventsID = dbGet(CMD_DEFAULT, DEFAULT_EI)[0]
        self.eventsNN = dbGet(FIND_EVENTS, self.eventsID)[0]

        if not self.eventsDate:
            datetime = dbGet(CMD_DEFAULT, DEFAULT_DT)
            if not datetime:
                raise Exception(dbMissing(CMD_DEFAULT, DEFAULT_DT))
            self.eventsDate = datetime[0]

        key_ = concat(self.eventsID, self.eventsDate)
        data = dbGet(EVENTS_DATE, key_)
        if not data:
            raise Exception(dbMissing(EVENTS_DATE, key_))
        self.courseID = data[0]
        self.teeboxID = data[1]

        self.courseHdcp = dbGet(COURSE_HDCP, self.courseID)
        self.courseHole = dbGet(COURSE_HOLE, self.courseID)
        self.coursePars = dbGet(COURSE_PARS, self.courseID)
        self.courseShow = dbGet(COURSE_SHOW, self.courseID)[0]
        self.courseTbox = dbGet(COURSE_TBOX, concat(self.courseID, self.teeboxID))
        self.eventsShow = dbGet(EVENTS_SHOW, self.eventsID)[0]

        self.courseHdcp = HoleInfo(self.courseHdcp)
        self.courseHdcp.setText(HOLE_ID, 'Hdcp')

        self.courseHole = HoleInfoLabel(self.courseHole)

        self.coursePars = SigmaHoleInfo(self.coursePars)
        self.coursePars.setText(HOLE_ID, 'Par')

        self.courseShow = showDate(self.eventsDate) + ' -- ' +              \
                          self.courseShow + ' -- ' +                        \
                          self.courseTbox[0] + '/' + self.courseTbox[1]
        self.courseTbox = SigmaHoleInfo(self.courseTbox[4:])
        self.courseTbox.setText(HOLE_ID, self.teeboxID)

        longInfo = dbGet(COURSE_LONG, self.courseID)
        nearInfo = dbGet(COURSE_NEAR, self.courseID)
        self.courseLdcp = None      ## NOT CODED YET

        ## DEBUGGING INFORMATION. TODO: REMOVE
        print('longInfo: %s' % longInfo) ## TODO: REMOVE
        print('nearInfo: %s' % nearInfo) ## TODO: REMOVE
        self.debug()                     ## TODO: REMOVE
        ## DEBUGGING INFORMATION. TODO: REMOVE

        ## Extract the team data. TODO: Consider separate class
        BASE = concat(self.eventsID, self.eventsDate)
        TIME = BASE
        count = 0
        while True:
            data = dbNext(EVENTS_TIME, TIME)
            if not data: break

            TIME = data[1]
            if self.eventsID   != catcon(TIME, 0): break
            if self.eventsDate != catcon(TIME, 1): break

            team = TeamInfo(TIME)
            team.loader(data[2])         ## Load the team data
            team.debug()
            self.teamList.append(team)

    ##########################################################################
    ## show
    def show(self):
        logger('EventsView.show');
        main = GolfLayout()

        W = QLabel(self.eventsShow)
        W.setAlignment(Qt.AlignCenter)
        W.setFrameStyle(QFrame.Box | QFrame.Plain)
        W.setMargin(5)
        main.addWidget(W, main.rowCount(), 0, 1, -1)

        W = QLabel(self.courseShow)
        W.setAlignment(Qt.AlignCenter)
        W.setFrameStyle(QFrame.Box | QFrame.Plain)
        W.setMargin(5)
        main.addWidget(W, main.rowCount(), 0, 1, -1)

        main.append(self.courseHole)
        main.append(self.courseTbox)
        main.append(self.coursePars)
        main.append(self.courseHdcp)

        for team in self.teamList:       ## Scores
            W = QLabel('Tee time: ' + catcon(team.TIME, 2))
            W.setAlignment(Qt.AlignCenter)
            W.setFrameStyle(QFrame.Box | QFrame.Plain)
            main.append(W)

            for card in team.playerCard:
                main.append(card)

        self.setLayout(main.wrapper())
        return super().show()

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
##       TeamInfo.py
##
## Purpose-
##       Golf: Team information container
##
## Last change date-
##       2019/08/21
##
##############################################################################
import sys

#### PyQt5 ###################################################################
#### PyQt5.QtCore    import *
#### PyQt5.QtGui     import *
#### PyQt5.QtWidgets import *

#### lib #####################################################################
from lib.Debug       import *
from lib.Utility     import *

#### Golf ####################################################################
from GolfApplet      import *
from DatabaseInfo    import *
from DbServer        import *
from SigmaHoleInfo   import *

##############################################################################
##
## Class-
##       PlayerData
##
## Purpose-
##       TeamInfo.playerData list item
##
##############################################################################
class PlayerData(object):           ## Player data information container
    def __init__(self, playerID, playerNN):
        self.playerID = playerID    ## Player ID
        self.playerNN = playerNN    ## Player nickname

        self.playerHdcp = None      ## Player handicap (optional)
        self.playerShow = None      ## Player (short) name

    def __repr__(self):
        return 'PlayerData<%s,%s,%s,%s>' % \
               (self.playerID, self.playerNN, self.playerHdcp, self.playerShow)

##############################################################################
##
## Class-
##       TeamInfo
##
## Purpose-
##       Team information container
##
## Implementation notes-
##       catcon(self.TIME, 0): Event identifier
##       catcon(self.TIME, 1): Date, format yyyy/mm/dd
##       catcon(self.TIME, 2): Time, format hh:mm
##
##############################################################################
class TeamInfo(DatabaseInfo):       ## A team information container
    def __init__(self, TIME):       ## Time: event_id.date.time
        super().__init__()

        self.TIME = TIME            ## The event tee time
        self.playerCard = []        ## The list of ESC scores
        self.playerData = []        ## The list of PlayerData elements
        self.playerNets = []        ## The list of net scores
        self.playerPost = []        ## The list of posted scores

    def __repr__(self):
        return 'TeamInfo<%s,%s,...>' % (self.TIME, self.playerCard)

    def debug(self):                ## Debugging display
        super().debug()
        print('TIME: %s' % self.TIME)

        print('playerCard: %s' % self.playerCard)
        print('playerData: %s' % self.playerData)
        print('playerNets: %s' % self.playerNets)
        print('playerPost: %s' % self.playerPost)

    def loader(self, playerList):   ## Load the player data
        playerList = tokenize(playerList)
        eventsID = catcon(self.TIME, 0) ## Get the event identifier
        for playerNN in playerList:
            playerID = dbGet(FIND_PLAYER, playerNN)[0]

            playerHdcp = dbGet(EVENTS_HDCP, concat(eventsID, playerID))
            if playerHdcp: playerHdcp = playerHdcp[0]
            else: playerHdcp = '0'

            playerShow = dbGet(PLAYER_SHOW, playerID)[0]

            playerData = PlayerData(playerID, playerNN)
            playerData.playerHdcp = playerHdcp
            playerData.playerShow = playerShow
            self.playerData.append(playerData)

            playerCard = dbGet(EVENTS_CARD, concat(self.TIME, playerNN))
            playerCard = SigmaHoleInfo(playerCard)
            playerCard.setText(HOLE_ID, playerShow);
            self.playerCard.append(playerCard)

            self.playerNets.append(None) ## TODO: NOT CODED YET
            self.playerPost.append(None) ## TODO: NOT CODED YET


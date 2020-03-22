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
##       DbServer.py
##
## Purpose-
##       Golf: Database server
##
## Last change date-
##       2019/08/11
##
##############################################################################
import sys

import string

from lib.Debug   import *
from lib.Utility import *

from DebuggingAdaptor import DebuggingAdaptor

##############################################################################
## Exports
##############################################################################
__all__  = [ 'DbServer', 'dbCmd', 'dbGet', 'dbNext', 'dbPut'
           , 'dbError', 'dbMissing'
           , 'CMD_COMMENT', 'CMD_DEFAULT', 'DEFAULT_DT'
           , 'DEFAULT_CI' , 'DEFAULT_EI' , 'DEFAULT_PI' , 'DEFAULT_TI'
           , 'FIND_COURSE', 'FIND_EVENTS', 'FIND_PLAYER'
           , 'COURSE_HDCP', 'COURSE_HOLE', 'COURSE_HTTP', 'COURSE_LONG'
           , 'COURSE_NAME', 'COURSE_NEAR', 'COURSE_PARS', 'COURSE_SHOW'
           , 'COURSE_TBOX'
           , 'EVENTS_CARD', 'EVENTS_DATE', 'EVENTS_HDCP', 'EVENTS_NAME'
           , 'EVENTS_PLAY', 'EVENTS_POST', 'EVENTS_SHOW', 'EVENTS_TEAM'
           , 'EVENTS_TIME'
           , 'PLAYER_CARD', 'PLAYER_HDCP', 'PLAYER_MAIL', 'PLAYER_NAME'
           , 'PLAYER_NICK', 'PLAYER_POST', 'PLAYER_SHOW'
           ]

##############################################################################
## Constants
##############################################################################
SEP      = '@'                      ## Seperator character

##############################################################################
## Internal data areas
##############################################################################
DB       = None                     ## the DbServer singleton
dict     = {}                       ## The database dictionary

##############################################################################
## Exported commands
##############################################################################
CMD_COMMENT = '##'
CMD_DEFAULT = 'DEFAULT'
DEFAULT_DT  = 'DATE_TIME'
DEFAULT_CI  = 'COURSE_ID'
DEFAULT_EI  = 'EVENTS_ID'
DEFAULT_PI  = 'PLAYER_ID'
DEFAULT_TI  = 'TEEBOX_ID'

FIND_COURSE = 'COURSE.FIND'
FIND_EVENTS = 'EVENTS.FIND'
FIND_PLAYER = 'PLAYER.FIND'

COURSE_HDCP = 'COURSE.HDCP'
COURSE_HOLE = 'COURSE.HOLE'
COURSE_HTTP = 'COURSE.HTTP'
COURSE_LONG = 'COURSE.LONG'
COURSE_NAME = 'COURSE.NAME'
COURSE_NEAR = 'COURSE.NEAR'
COURSE_PARS = 'COURSE.PARS'
COURSE_SHOW = 'COURSE.SHOW'
COURSE_TBOX = 'COURSE.TBOX'

EVENTS_CARD = 'EVENTS.CARD'
EVENTS_DATE = 'EVENTS.DATE'
EVENTS_HDCP = 'EVENTS.HDCP'
EVENTS_NAME = 'EVENTS.NAME'
EVENTS_PLAY = 'EVENTS.PLAY'
EVENTS_POST = 'EVENTS.POST'
EVENTS_SHOW = 'EVENTS.SHOW'
EVENTS_TEAM = 'EVENTS.TEAM'
EVENTS_TIME = 'EVENTS.TIME'

PLAYER_CARD = 'PLAYER.CARD'
PLAYER_HDCP = 'PLAYER.HDCP'
PLAYER_MAIL = 'PLAYER.MAIL'
PLAYER_NAME = 'PLAYER.NAME'
PLAYER_NICK = 'PLAYER.NICK'
PLAYER_POST = 'PLAYER.POST'
PLAYER_SHOW = 'PLAYER.SHOW'

##############################################################################
## Exported database accessors
##############################################################################
def dbCmd(command, item, qual):
    return tokenize(DB.get(command, concat(item, qual)))

def dbGet(item, qual):
    return tokenize(DB.get(item, qual))

def dbNext(item, qual):
    return DB.next(item, qual)

def dbPut(item, qual, data):
    return tokenize(DB.put(item, qual, data))

##############################################################################
## Exported database exception text
##############################################################################
def dbError(text):
    return 'DB error: %s' % text

def dbMissing(item, qual):
    return 'DB missing: %s %s' % (item, qual)

##############################################################################
##
## Class-
##       DbServer
##
## Purpose-
##       Database accessor
##
##############################################################################
def _key(item, qual):               ## Generate combined key
    if qual:
        item += SEP
        item += qual
    return item.upper()

##############################################################################
## DbServer([filename]), default filename = 'GOLFER.DB'
##############################################################################
class DbServer(DebuggingAdaptor):
    def __init__(self, *args):
        super().__init__()

        global DB
        assert DB == None, 'Multiple DbServer objects exist'
        DB = self

        self.debugging = True
        self.fileName = 'GOLFER.DB'
        if len(args) > 0 and args[0]: self.fileName = args[0]

        self.loader(*args)

    def debug(self):                ## Debug
        global dict

        debugf('DbServer.debug()')
        for key in sorted(dict.keys()):
            debugf('[%s]: %s' % (key, dict[key]))

    def get(self, item, qual):      ## Get associated dictionary entry
        global dict

        try:
            out = dict[_key(item,qual)]
        except KeyError:
            out = None

        if self.debugging: debugf('DB.get(%s,%s): %s' % (item, qual, out))
        return out

    def isdebug(self):             ## Is debugging active?
        return self.debugging

    def loader(self, *args):        ## Load the database
        global dict

        errors = 0                  ## Error counter
        with open(self.fileName, 'rb') as file:
            for line in file:
                line = bytes2str(line).strip()

                tokenizer = Tokenizer(line)
                if not tokenizer.hasNextToken(): continue ## Ignore empty line

                item = tokenizer.nextToken() ## Get command
                if item[0] == '#': continue ## Ignore comment line

                if not tokenizer.hasNextToken():
                    errors += 1
                    debugf('line(%s) missing qualifier' % line)
                    continue

                qual = tokenizer.nextToken() ## Get item
                key = _key(item, qual)
                if tokenizer.hasNextToken(): ## If insert
                    if key in dict: ## If duplicate entry
                        if False:
                            errors += 1
                            debugf('line(%s) duplicate(%s)' % (line, dict[key]))
                    dict[key] = tokenizer.remainder()
                else:               ## If remove
                    if key in dict: ## If entry present
                        del dict[key] ## Remove it
                    else:           ## Remove missing item
                        if False:
                            errors += 1
                            debugf('line(%s) key(%s) missing' % (line, key))

        debugf('Database loaded, %s errors' % errors)

    def next(self, item, qual):     ## Get key following
        global dict

        inp = _key(item, qual)
        out = None
        for key in sorted(dict.keys()):
            if key.__gt__(inp):
                out = key.split(SEP)
                if out and out[0] == item:
                    out.append(dict[key])
                else:
                    out = None
                break

        if self.debugging: debugf('DB.next(%s,%s): %s' % (item, qual, out))
        return out

    ## Returns prior data value
    ## Use data == None remove an item from the database
    def put(self, item, qual, data): ## Set/update dictionary entry
        global dict

        ## Update the external dictionary
        line = item + ' ' + qual
        if data: line = line + ' ' + data
        with open(self.fileName, 'ab') as file:
            file.write(str2bytes(line + '\n'))

        ## Update the internal dictionary
        if item == CMD_COMMENT: return None ## Comments do not replace data

        key_ = _key(item, qual)
        print("key_: '%s'" % key_)
        out = None
        if key_ in dict:
            out = dict[key_]

        if data:
            print("DATA: '%s'" % data)
            dict[key_] = data
        else:
            print("DEL: '%s'" % key_)
            del dict[key_]

        if self.debugging:
            debugf('DB.put(%s,%s,%s): %s' % (item, qual, data, out))
        return out

    def setdebug(self, T):          ## Set debugging state
        self.debugging = bool(T)


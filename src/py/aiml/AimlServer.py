#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2017-2023 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       AimlServer.py
##
## Purpose-
##       Source independent AIML input/output handler.
##
## Last change date-
##       2023/08/17
##
## Implementation notes-
##       Input:
##           UOW.sess (string)Session ID
##           UOW.work (string)Input string
##       Output:
##           UOW.work (string)Response string
##
##############################################################################
from __future__ import print_function

import aiml
import os
import pickle
import sys
import urllib
import time

from lib.Command import command
from lib.Control import control
from lib.Debug import *
from lib.Dispatch import TAB, UOW, WDW
import Common
import Profile

##############################################################################
## Define available imports
##############################################################################
__all__ = None                      ## Nothing exported

##############################################################################
## Control defaults
##############################################################################
_USE_ASYNCH_LOADER = False          ## Use asynchronous loader?
_VERBOSE = 0                        ## Verbosity, larger is noisier

_BRAIN_FILENAME  = "brain.freeze"   ## The saved brain's name
_DEFAULT_PROFILE = "Default"        ## The default profile
_DEFAULT_SESSION = "Default+Default"## The default session
_LOGGER_FILENAME = "log/aiml-server.log"

##############################################################################
## Internal variables
##############################################################################
_tab = None

##############################################################################
## Internal functions
##############################################################################
_debugf = Common._debugf
_tracef = Common._tracef

def fetch(kernel):
    kernel.learn("std-startup.xml")
    kernel.respond("LOAD AIML B")

def logger(session, inp, out):
    head = session + ": >>>> "

    log = open(_LOGGER_FILENAME, "a")
    log.write(head+inp+"\n")
    log.write(out+"\n")
    log.close()

def load_profile(kernel, profile):
    try:
        Profile.properties(kernel, profile)
    except FileNotFoundError:
        print("Failed to load profile({}), default used".format(profile))
        Profile.properties(kernel, None)
        profile = _DEFAULT_PROFILE
    except Exception as x:
        print("Failed to load profile({}), Exception: {}".format(profile,x))
        Profile.properties(kernel, None)
        profile = _DEFAULT_PROFILE

    return profile

def load_session(kernel, sessionID):
    name = "session/%s.session" % sessionID
    try:
        file = open(name, "rb")
        data = pickle.load(file)
        kernel._addSession(sessionID)
        kernel._sessions[sessionID] = data
        file.close()
    except FileNotFoundError:
        pass
    except Exception as x:
        print("Failed to load session(%s), Exception:" % name, x)

    return sessionID

def save_session(kernel, sessionID):
    name = "session/%s.session" % sessionID
    file = open(name, "wb")
    pickle.dump(kernel.getSessionData(sessionID), file)
    file.close()

def store(kernel):
    kernel.saveBrain(_BRAIN_FILENAME)

##############################################################################
## transmit: Send message to aiml server
##############################################################################
def transmit(sess, mess):
    try:
        tab = control['aiml-server']
        resp = "AIML inactive"
        if tab:
            uow = UOW(WDO=WDW())
            uow.sess = sess
            uow.work = mess
            Common.enqueue(tab, uow)
            uow.wdo.wait()
            resp = uow.work
    except Exception as x:
        resp = "*ERROR* Exception: " + str(x)

    return resp

##############################################################################
## _IgnoreTAB: The AIML server TAB, NOP replacement
##############################################################################
class _IgnoreTAB(TAB):
    def _loader(self):
        _LoaderTAB()                ## Asynchronously load the Server

    def work(self, uow):
        uow.work = "AIML server offline"
        uow.done(UOW.CC_ERROR)

##############################################################################
## _ServerTAB: The AIML server TAB
##############################################################################
class _ServerTAB(TAB):
    """The internal AIML server control TAB"""
    def _loader(self):
        ######################################################################
        ## Initialize fields
        self.avatars = {}           ## Avatar asynchronous web page dictionary

        ######################################################################
        ## Load the Kernel
        self._kernel = aiml.Kernel()

        # Load from saved brain, if available.
        if os.path.isfile(_BRAIN_FILENAME):
            self._kernel.bootstrap(brainFile = _BRAIN_FILENAME)
        else:
            fetch(self._kernel)     ## Not available, create new
            store(self._kernel)

        # Load default profile and session
        self._profile = load_profile(self._kernel, _DEFAULT_PROFILE)
        self._session = self._kernel._globalSessionID
        self._session = None
        _debugf(time.asctime(), "AIML ready")
        logger = Logger(_LOGGER_FILENAME, append=True)
        logger.tracef(time.asctime(), "AIML ready")
        del logger

    def work(self, uow):
        try:
            self._work(uow)
        except Exception as x:
            uow.work = "Internal error: %s" % x
            uow.done(cc=UOW.CC_ERROR)
            Handle.exception()

    def _work(self, uow):
        session = getattr(uow, 'sess', '*') ## format "Profile/Client"
        message = uow.work
        x = session.find("/")
        if x <= 0:
            session = self._kernel._globalSessionID
        else:
            avatar = session[0:x]
            client = session[x+1:]
            if client == '.IDENT': ## If .IDENT sequence
                uow.work = "OK"
                uow.done()

                self.avatars[avatar] = message
                _tracef("avatars[%s]='%s'" % (avatar, message))
                return

            if self._profile != avatar:
                self._profile = load_profile(self._kernel, avatar)

            session = avatar + "+" + client

        if self._session != session:
            if self._session:
                save_session(self._kernel, self._session)
            self._session = load_session(self._kernel, session)

        if len(message) > 0 and message[0] == ".":
            respond = "NG"
            if message.upper() == ".EXIT" or message.upper() == ".QUIT":
                save_session(self._kernel, self._session)
                self._session = None
                respond = "OK"
        else:
            respond = self._kernel.respond(message, session)
        uow.work = respond
        uow.done()

        logger(session, message, respond)

class _LoaderTAB(TAB):              ## Asynchronous AIML loader
    def __init__(self):
        super(_LoaderTAB, self).__init__()
        uow = UOW()
        uow.work = _ServerTAB()
        Common.enqueue(tab, uow)

    def work(self, uow):
        try:
            self._work(uow)
        except Exception as x:
            uow.done(cc=UOW.CC_ERROR)
            Handle.exception()

    def _work(self, uow):           ## Asynchronouly load the Kernel
        server = uow.work           ## The associated _ServerTAB
        server._loader()            ## Initialize
        control['aiml-server'] = server ## Activate the aiml-server

        uow.done() ## (Not really required here, does nothing)

if _USE_ASYNCH_LOADER:
    control['aiml-server'] = _IgnoreTAB() ## (Temporarily) disable AIML
##  control['aiml-server']._loader() ## Activate the aiml-server
##  _LoaderTAB()                    ## Asynchronously load the Server
else:
    control['aiml-server'] = _ServerTAB() ## Enable AIML
##  control['aiml-server']._loader() ## Activate the aiml-server

##############################################################################
## Class __Command
##
## The aiml-server command handler, display avatar status array.
##############################################################################
class __Command:
    @staticmethod
    def run(argv):
        tab = control['aiml-server']
        N = len(tab.avatars)
        debugf("{} avatar{} registered".format(N, "" if N == 1 else "s"))
        for avatar in tab.avatars:
            debugf("{}@{}".format(avatar, tab.avatars[avatar]))
        return 0

command['aiml-server'] = __Command

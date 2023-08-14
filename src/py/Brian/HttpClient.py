#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2016-2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       HttpClient.py
##
## Purpose-
##       Brian AI: HTTP client, command['curl']
##
## Last change date-
##       2018/01/01
##
##############################################################################
import configparser
import sys
import time
import urllib.request, urllib.error, urllib.parse

from lib.Command import command
from lib.Debug import Debug, debugf, tracef
import Config

##############################################################################
## Configuration defaults
##############################################################################
_WEBPAGE = 'http://bigblue:8080/'
_VERBOSE = '0'

##############################################################################
## __Command class (HttpClient command)
##############################################################################
class __Command:
    @staticmethod
    def run(argv):
        config = configparser.ConfigParser()
        config['http-client'] = {}
        config.read(Config.INI)
        client = config['http-client']
        WEBPAGE = client.get('web-page', _WEBPAGE)
        VERBOSE = client.getint('verbose', _VERBOSE)
        LOGFILE = client.get('log-file', 'log/http-client.log')
        logger = Debug(LOGFILE, append=True)

        url = WEBPAGE
        if len(argv) > 1:
            url = argv[1]

        if url == "shutdown": url = WEBPAGE + "shutdown.html"
        if url == "quit": url = WEBPAGE + "quit"
        if not url.startswith("http:"):
            url = "http://" + url

        request = urllib.request.Request(url)
        # request.add_header("accept", "text/plain; q=0.5, text/html")
        request.add_header("user-agent", "Python-Brian/0.0.1")
        request.add_header("X-something", "X-value")

        try:
            file = urllib.request.urlopen(request)
        except urllib.error.URLError as e:
            print(e, url)
            return
        except:
            Debug.handle_exception()
            raise

        logger.tracef(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        logger.tracef("%.3f URL: %s" % (time.time(), url))
        logger.tracef("\n".join((k +': '+ v) for k, v in file.getheaders()))
        logger.tracef()
        if VERBOSE:
            debugf("\n".join((k +': '+ v) for k, v in file.getheaders()))
            debugf()

        with Debug.lock():
            for line in file:
                line = str(line,encoding='iso-8859-1')

                suffix = ""
                while line.endswith(("\n", "\r")):
                    if line.endswith("\n"):
                        suffix = "\\n" + suffix
                    else:
                        suffix = "\\r" + suffix
                    line = line[:-1]

                if VERBOSE > 1:
                    line += suffix

                logger.tracef(line)
                debugf(line)

        return 0

command['curl'] = __Command

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
##       Reader.py
##
## Purpose-
##       Brian AI: Reader: reads files, handling content
##
## Last change date-
##       2018/01/01
##
## UOW.work stack-
##       Input:  UOW.work[0]: "source URL"
##       Output: UOW.work[0]: [byte] Content
##
##       Error:  UOW.work[0]: Error message string
##               UOW.work[1]: "source URL"
##
##############################################################################
import sys
import time
import urllib.request, urllib.error, urllib.parse

from lib.Debug import Debug, debugf, tracef
from lib.Dispatch import TAB, UOW
import Config
import Common

##############################################################################
## Define available imports
##############################################################################
__all__ = None

##############################################################################
## Configuration controls
##############################################################################
_MAXSIZE = 0x10000000               ## Maximum file size
_VERBOSE = Config.VERBOSE
if True:
    _VERBOSE = 1

##############################################################################
## _HttpReaderTAB: The HttpReader TAB
##############################################################################
def _readurl(url):
    if not url.startswith("http://"):
        url = "http://" + url

    request = urllib.request.Request(url)
    # request.add_header("accept", "text/plain; q=0.5, text/html")
    request.add_header("user-agent", "Python-Brian/0.0.1")

    with urllib.request.urlopen(request) as file:
        tracef(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        if _VERBOSE:
            debugf("URL: %s" % url)
            debugf("\n".join((k +': '+ v) for k, v in file.getheaders()))
            debugf()
        else:
            tracef("URL: %s" % url)
            tracef("\n".join((k +': '+ v) for k, v in file.getheaders()))
            tracef()

        buff = b''
        size = 0
        while True:
            chunk = file.read(8192)
            if len(chunk) == 0:
                break
            size += len(chunk)
            if size > _MAXSIZE:
                raise Exception("URL(%s) too large" % url)
            buff += chunk
        return buff

class _HttpReaderTAB(TAB):
    def __init__(self):
        super(_HttpReaderTAB,self).__init__()

    def work(self, uow):
        url = uow.work[0]
        try:
            uow.work[0] = _readurl(url)
        except Exception as e:
            uow.work = [e] + uow.work
            uow.done(cc=UOW.CC_ERROR)
        else:
            uow.done()

##############################################################################
## _FileReaderTAB: The FileReader TAB
##############################################################################
def _readfile(name):
    buff = b''
    size = 0
    with open(name, "rb", 8192) as file:
        while True:
            chunk = file.read(8192)
            if len(chunk) == 0:
                break
            size += len(chunk)
            if size > _MAXSIZE:
                raise Exception("File(%s) too large" % name)
            buff += chunk
    return buff

class _FileReaderTAB(TAB):
    def __init__(self):
        super(_FileReaderTAB,self).__init__()

    def work(self, uow):
        name = uow.work[0]
        try:
            uow.work[0] = _readfile(name)
        except Exception as e:
            uow.work = [e] + uow.work
            uow.done(cc=UOW.CC_ERROR)
        else:
            uow.done()

##############################################################################
## _ReaderTAB: The Reader TAB
##############################################################################
class _ReaderTAB(TAB):
    def __init__(self):
        super(_ReaderTAB,self).__init__()

    def work(self, uow):
        url = uow.work[0]
        if url.startswith("http://"):
            reader = _readurl
        else:
            reader = _readfile

        try:
            uow.work[0] = reader(url)
        except Exception as e:
            uow.work = [e] + uow.work
            uow.done(cc=UOW.CC_ERROR)
        else:
            uow.done()

##############################################################################
## Create the _ReaderTAB singletons and add them to the Common.service dict.
##############################################################################
Common.add_service('file-reader', _FileReaderTAB())
Common.add_service('http-reader', _HttpReaderTAB())
Common.add_service('reader', _ReaderTAB())

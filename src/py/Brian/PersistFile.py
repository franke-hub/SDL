##############################################################################
##
##       Copyright (C) 2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       PersistFile.py
##
## Purpose-
##       Brian AI: Persistent data service.
##
## Last change date-
##       2018/01/01
##
## Usage notes-
##       This Module does not create subdirectories. These must be created
##       in advance of any use. Use 'data/' only for permanent files.
##       Use 'temp/' for bringup.
##
##############################################################################
import os
import shutil
import stat
import sys
import time

from lib.Command import command
from lib.Debug import *
from lib.Dispatch import TAB
import Common

##############################################################################
## Internal constants
##############################################################################
_DATA = "data"

##############################################################################
## _logger: Log "ShouldNotOccur" conditions
##############################################################################
def _logger(*args, **kwargs):
    with Debug.lock():
        logger = Logger("log/exception.log", append=True)
        M = " ".join(str(arg) for arg in args)
        logger.debugf(time.asctime() + " >> PersistFile." + M, **kwargs)
        del logger

##############################################################################
## _get_stat: Get os.stat(name)
##############################################################################
def _get_stat_(name):
    try:
        st = os.stat(name)
        return st
    except FileNotFoundError:
        return None
    except Exception:
        raise

def _get_stat(name):
    st = _get_stat_(name)
    return st

##############################################################################
## replace: Replace a file
##############################################################################
def replace(name, content):         ## Replace a file
    try:
        _replace(name, content)
    except Exception as x:
        _logger("_replace(%s)\n>> Exception(%s)" % (name, x))
        raise

def _replace(name, content):        ## Replace a file
    if isinstance(content, str): content = content.encode()
    with open(name+"~", "wb") as f:
        f.write(content)

    try:
        os.remove(name)
    except FileNotFoundError:
        pass
    except:
        raise

    os.rename(name+"~", name)

##############################################################################
## restore: Restore a file, possibly from backup
##############################################################################
def restore(name):                  ## Restore a file
    try:
        return _restore(name)
    except Exception as x:
        if not isinstance(x, FileNotFoundError):
            _logger("_restore(%s)\n>> Exception(%s)" % (name, x))
        raise

def _restore(name):                 ## Restore a file
    if _get_stat(name + "~") :
        if _get_stat(name):
            _logger("restore(%s) removed ~" % name)
            os.remove(name+"~")
        else:
            _logger("restore(%s) from ~" % name)
            os.rename(name+"~", name)

    with open(name, "rb") as f: content = f.read()
    return content

##############################################################################
## _PersistTAB: Provides persistent file services
##    Inp:
##        uow.fc   = 'replace' or 'restore'
##        uow.work = The persistent file name string
##        uow.data = For 'replace' the file (bytes) content
##    Out:
##        uow.cc   = None if successful, Exception descriptor if not
##        uow.work = The persistent file name string
##        uow.data = For 'restore', the file (bytes) content
##############################################################################
class _PersistTAB(TAB):
    def work(self, uow):
        try:
            _work(self, uow)
        except Exception as x:
            uow.cc = x

    def _work(self, uow):
        cc = None
        if uow.fc == 'replace':
            replace(uow.work, uow.data)
        elif uow.fc == 'restore':
            uow.data = restore(uow.work)
        else:
            cc = RuntimeError("Invalid fc(%s)" % uow.fc)
        uow.done(cc)

Common.add_service('persist-file', _PersistTAB())

if False:
    ##########################################################################
    ## __Command class: Bringup test
    ##########################################################################
    _serial = 0
    from lib.Utility import int2bytes
    class __Command:
        @staticmethod
        def run(argv):
            global _serial
            try:
                bar = restore('foo')
                print("Foo:")
                print(bar.decode())
                _serial += 1
                bar = bar + b'Added line ' + int2bytes(_serial) + b'\n'
                replace('foo', bar)
            except:
                Debug.handle_exception()
                bar = b'exceptional data'

            return 0


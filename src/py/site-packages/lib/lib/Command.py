##############################################################################
##
##       Copyright (C) 2018-2021 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Command.py
##
## Purpose-
##       Define operator commands
##
## Last change date-
##       2021/04/01
##
##############################################################################
from __future__ import print_function

from lib.Debug import Debug, debugf
from lib.Utility import visify

##############################################################################
## Define available imports
##############################################################################
__all__ = ['Command', 'command']

##############################################################################
## Internal data areas
##############################################################################
command = {}                        ## The common command dictionary
driver  = None                      ## The last created Command

##############################################################################
## Class Command:
##
## The Command class provides a generic dictionary command parser.
## While it has the same work interface as a Dispatch.TAB, it is not one.
## The TAB work method signature is provided, but it must be called directly.
##############################################################################
class Command(object):
    def __init__(self, dict):
        global driver
        driver       = self         ## (Used by 'all' command)
        self.command = dict
        self.omit    = []

    @staticmethod
    def _get_list(command):
        return ', '.join(sorted([k for k in command.keys()]))

    def main(self, argv):
        errorCount = 0
        if not argv: argv = []

        if len(argv):
            for test in argv:
                if test[0] == '-': continue ## Ignore switch arguments

                try:
                    debugf("Running: '%s'" % test)
                    errorCount += self.command[test].run()
                except KeyError:
                    errorCount += 1
                    debugf('!!!!!!!! Test[%s] Not available:' % test)
                except Exception as X:
                    errorCount += 1
                    debugf('!!!!!!!! Test[%s] Exception:' % test)
                    Debug.handle_exception()
                debugf('')
        else:
            debugf('Default: running all tests')
            errorCount += self.runAll()

        if errorCount:
            debugf('!!!!!!!! %s Test%s failed'
                   % (errorCount, ('' if errorCount == 1 else 's')))
        else:
            debugf('All tests successful')

    @staticmethod
    def resetGlobals():             ## Reset the globals
        command = {}
        driver  = None

    def run(self, argv):
        name = argv[0]

        rc = -1
        try:
            Name = self.command[name]
            try:
                rc = Name.run(argv)
            except:
                Debug.handle_exception()
        except KeyError:
            debugf('Invalid command:', visify(name))
            debugf('Valid commands:', Command._get_list(self.command))

        return rc

    def runAll(self):
        errorCount = 0
        omit = self.omit + ['all']
        for test in sorted(command):
            if test in omit: continue
            debugf("Running: '%s'" % test)
            try:
                errorCount += command[test].run(('all', test))
            except Exception as X:
                errorCount += 1
                debugf('!!!!!!!! Test[%s] Exception: %s' % (test, X))
                Debug.handle_exception()
            debugf('')
        return errorCount

    def work(self, uow):
        work = uow.work
        if isinstance(work, str):
            if work == '':
                uow.done(0)
                return

            argv = tuple(work.split())
            uow.done(self.run(argv))
            return

        uow.done(-1)
        return

##############################################################################
## __All: Sample command processor. ('all' command)
##############################################################################
class __All:
    @staticmethod
    def run(*args):
        if driver is None:
            Command(command)
        driver.runAll()
        return 0

command['all'] = __All

##############################################################################
## __List: Sample command processor. ('list' command)
##
## Usage notes:
##       Commands processor classes only need to contain the run() method
##       with an appropriate signature, i.e.
##           @staticmethod
##           def run(*args):
##               Where args[0] is the argv tuple, or
##
##           @staticmethod
##           def run(argv):
##               Where the argv tuple parameter is explicitly specified.
##
##       The return value from the command is the UOW return value.
##
##############################################################################
class __List:
    @staticmethod
    def run(*args):
        print('Command list:', Command._get_list(command))
        return 0

command['list'] = __List

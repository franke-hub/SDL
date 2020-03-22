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
##       test/Reader.py
##
## Purpose-
##       Test Reader.py
##
## Last change date-
##       2018/01/01
##
##############################################################################
from lib.Command import command
from lib.Debug import *
from lib.Dispatch import TAB
import Common

##############################################################################
## Reader test command
##   Now testing: Basic Reader functionality
##############################################################################
class _TAB(TAB):
    def work(self, uow):
        print("...working...")
        uow.done()

class __ReaderTest:
    @staticmethod
    def run(argv):
        print("Running reader test...")
        from lib.Dispatch import TAB, UOW, WDW

        name = "http://bigblue:8888"
        if len(argv) > 1:
            name = argv[1]
        work = [name]

        ## WDS removed. This was it's only usage
        ## reader = Common.get_service('reader')
        ## wdw = WDW()
        ## wds = WDS(SEQ=[reader, _TAB(), wdw])
        ##
        ## uow = UOW(WDO=wds)
        ## uow.work = work
        ## uow.done()
        ## wdw.wait()
        ## if uow.cc:
        ##     print("Error: '%s' for(%s)" % (uow.work[0], uow.work[1]))
        ## else:
        ##     print(uow.work[0].decode('latin-1'))
        ##
        ## print("WDS %d, %d of %d" % (uow.cc, wds.index, len(wds.ops)))
        print("...Test complete")

command['test.reader'] = __ReaderTest


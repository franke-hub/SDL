#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2017 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Main.py
##
## Purpose-
##       Command processor.
##
## Last change date-
##       2017/01/01
##
##############################################################################
from __future__ import print_function

import os
import sys
import time

from lib.Command import *
from lib.Debug import *

##############################################################################
## Mainline code
##############################################################################
if __name__ == "__main__":
    if sys.version_info[0] < 2:
        print("Python version %s not supported" % sys.version_info[0])
        sys.exit(1)

    if sys.version_info[0] < 3:
        version = "Python2"
    else:
        version = "Python3"

    try:
        Debug(append=True)          ## Use append mode
        debugf("%.3f Running: %s %s" % (time.time(), version, time.asctime()))

        file_list = [f for f in os.listdir('S/.') if os.path.isfile("S/"+f)]
        for f in file_list:
            if len(f) > 3 and f[-3:] == ".py":
                name = f[:-3]
                if name == 'Main':
                    continue

                if name.startswith("Python"):
                    if not name.startswith(version):
                        continue

                exec("import " + name)

        argv = sys.argv[1:]
        name = 'sample'
        if len(argv) > 0:
            name = argv[0]
        Command(command).run(argv)

    except KeyError:
        debugf("No command:", name)
        debugf("Valid commands:", command['list'].run())

    except KeyboardInterrupt:
        debugf("\n%.3f Ctrl-C" % time.time())
        try:
            pass
        except KeyboardInterrupt:
            debugf("%.3f .... Quit ...." % time.time())
            sys.exit()

    except:
        Debug.handle_exception()
        ## os.kill(os.getpid(), 9)

    debugf("%.3f .... Done ...." % time.time())

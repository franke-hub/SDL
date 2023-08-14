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
##       AimlTester.py
##
## Purpose-
##       AIML tests.
##
## Last change date-
##       2017/01/01
##
##############################################################################
from __future__ import print_function

import aiml

from lib.Command import command
from lib.Debug import Debug, debugf, tracef

def fetch(kernel):
    debugf("Loading kernel...")
    kernel.learn("std-startup.xml")
    kernel.respond("LOAD AIML B")
    debugf("...Complete")

_NODE_KEYS = {0:"<_>", 1:"<*>", 2:"=", 3:"<THAT>", 4:"<TOPIC>", 5:"<BOT>"}
def pattern_tree(tree, prefix): ## Dump nodes in tree
    if( type(tree) is list ):
        tracef(prefix + str(tree))
    elif( type(tree) is dict ):
        for key in tree:
            if key in _NODE_KEYS:
                append = _NODE_KEYS[key]
            else:
                append = "[" + key + "]"

            pattern_tree(tree[key], prefix+append)
    else:
        tracef(prefix + str(tree))

def test_pattern_tree():
    _kernel = aiml.Kernel()

    # Load the Pattern Manager
    fetch(_kernel)

    ##########################################################################
    ## Print the node tree
    tracef("Kernel.brain:")
    pattern_tree(_kernel._brain._root, "")

def test_pattern_dump():
    _kernel = aiml.Kernel()

    # Load the Pattern Manager
    fetch(_kernel)

    ##########################################################################
    ## Dump the node tree
    tracef("Kernel.brain:")
    tracef(_kernel._brain._root)

##############################################################################
## __Brain_Command class (Pattern manager test)
##############################################################################
class __Brain_Command:
    @staticmethod
    def run(argv):
        if len(argv) > 1:
            if argv[1] == "tree":
                test_pattern_tree()
            else:
                test_pattern_dump()
        else:
            test_pattern_dump()
        return 0

command['brain'] = __Brain_Command ## Dispatch debugger

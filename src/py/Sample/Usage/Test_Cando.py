#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2017-2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Test_Cando.py
##
## Purpose-
##       "Can do" tests.
##
## Last change date-
##       2018/01/01
##
##############################################################################
from __future__ import print_function

from lib.Command import command
from lib.Debug import Debug, debugf, tracef

##############################################################################
## Local controls
##############################################################################
_cando = {}                         ## The "Can do" test list dictionary
error_count = 0                     ## Error counter

def format_count(error_count):
    if error_count:
        return "%d Error%s encountered" % (error_count, "" if error_count == 1 else "s")

    return "No errors encountered"

##############################################################################
## __MixedDict class: Mixed mode dictionary keys
##############################################################################
dictionary = { "alpha": [1, 2, 3]
             , b'beta': [4, 5, 6]   ## Note: python2: b'beta' is a string
             , "gamma": (7, 8, 9)
             ,      10: [11, 12, 13]
             ,      11: (11, 12, 13)
             #  [11, 12, 13]: 10    ## ERROR: unhashable type: 'list'
             ,  (11, 12, 13): 11    ## This DOES work!
             }

class __MixedDict:
    @staticmethod
    def run(argv):
        debugf(argv[0]+":", "Mixed mode dictionary, string % tuple formatting")
        error_count = 0
        try:
            for key in dictionary:
                if True:
                    debugf("Key({}) Value({})".format(key, dictionary[key]))
                else:
                    ##########################################################
                    ## The following statements fail at dictionary['gamma']
                    ## because it's a tuple containing more than one element
                    debugf("Key:", key, "Value(%s)" % dictionary[key])
                    debugf("Key:", key, ("Value(%s)" % dictionary[key]))
                    debugf("Key:", key, "Value(%s)" % (dictionary[key]))
                    debugf("Key:", key, "Value(%s)" % ((dictionary[key])))

                    ##########################################################
                    ## These altenatives handle the entire dictionary
                    debugf("Key:", key, "Value:", dictionary[key])
                    debugf("Key:", key, "Value(%s)" % str(dictionary[key]))
                    debugf("Key(%s) Value(%s)" % (key, dictionary[key]))
                    debugf("Key:", key, "Value({})".format(dictionary[key]))
                    pass
        except:
            Debug.handle_exception()
            error_count += 1

        debugf(argv[0]+":", format_count(error_count))
        return error_count

_cando['mixed-dict'] = __MixedDict

##############################################################################
## _MultiSignature class: The same method has different signatures.
##     ** FAILS ** The second foo overrides the first.
#############################################################################
class _MultiSignature:
    @staticmethod
    def foo(*args):
        debugf("Static foo:", *args)
        return 1

    def foo(self, *args):
        debugf("Instance foo:", *args)
        return 2

    ## Multiple signatures not directly supported, but you can do it yourself!
    def foo(*args):
        if len(args) > 0 and isinstance(args[0], _MultiSignature):
            self = args[0]
            args = args[1:]
            debugf("Instance foo:", *args)
            return 2

        debugf("Static foo:", *args)
        return 1

    @staticmethod
    def run(argv):
        debugf(argv[0]+":", "Function overloading")
        error_count = 0
        rc = _MultiSignature.foo("this is a test", "this is only a test")
        if rc != 1:
            error_count += 1
            debugf("Invoked wrong signature")

        bar = _MultiSignature()
        rc = bar.foo("This is", "another test")
        if rc != 2:
            error_count += 1
            debugf("Invoked wrong signature")

        debugf(argv[0]+":", format_count(error_count))
        return 0

_cando['multi-sigs'] = _MultiSignature
if False:                           ## We have a work-around, so keep test
    del _cando['multi-sigs']        ## Test demo fails
    del _MultiSignature             ## Test demo fails

##############################################################################
## __Command class
##############################################################################
class __Command:
    @staticmethod
    def run(argv):
        global error_count

        if len(argv) > 1:
            name = argv[1]
            argv = argv[1:]
            try:
                error_count += _cando[name].run(argv)
            except KeyError:
                error_count += 1
                debugf("No Can Do:" , name)
            except:
                error_count += 1
                Debug.handle_exception()
        else:
            for name in sorted(_cando):
                try:
                    error_count += _cando[name].run([name])
                    debugf("")
                except:
                    error_count += 1
                    Debug.handle_exception()

        debugf(format_count(error_count))
        return 0

command['cando'] = __Command

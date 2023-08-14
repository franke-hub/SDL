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
##       Timing.py
##
## Purpose-
##       Timing tests.
##
## Last change date-
##       2018/01/01
##
##############################################################################
from __future__ import print_function
import time

from lib.Command import command
from lib.Debug import Debug, debugf, tracef
from lib.Types import Bool, Integer, String

##############################################################################
## Local controls
##############################################################################
_tests = {}                         ## The timing test list dictionary
_error_count = 0                    ## Error counter
_ITERATIONS  = 100000000            ## Standard iteration counter
_var = "1234"                       ## Test variable
_dict = {"THIS": "alpha", "THAT": "beta", "OTHER": "delta"}

def format_count(error_count):
    if error_count:
        return "{} Error{} encountered".format(error_count, "" if error_count == 1 else "s")

    return "No errors encountered"

##############################################################################
## __Reader class: Simple variable read
##############################################################################
class __Reader:
    @staticmethod
    def run(argv):
        global _var
        _var = "abcd"
        debugf(argv[0] + ": Variable read test")
        start = time.time()
        for iter in range(_ITERATIONS):
            var = _var

        elapsed = time.time() - start
        debugf(argv[0] + ": %.3f" % elapsed)
        assert var == "abcd", "Should not occur"
        return 0

_tests['Reader'] = __Reader

##############################################################################
## __ReaderDict class: Simple dictionary read
##############################################################################
class __ReaderDict:
    @staticmethod
    def run(argv):
        global _dict
        debugf(argv[0] + ": Dictionary read test")
        index = "THAT"
        start = time.time()
        for iter in range(_ITERATIONS):
            var = _dict[index]

        elapsed = time.time() - start
        debugf(argv[0] + ": %.3f" % elapsed)
        assert var == "beta", "Should not occur"
        return 0

_tests['ReaderDict'] = __ReaderDict

##############################################################################
## __ReaderStrType class: lib.Types.String read
##############################################################################
class __ReaderStrType:
    @staticmethod
    def run(argv):
        from_ = String("abcd")      ## 'from' is a python keyword
        debugf(argv[0] + ": lib.Types.String read test")
        start = time.time()
        for iter in range(_ITERATIONS):
            into = from_._

        elapsed = time.time() - start
        debugf(argv[0] + ": %.3f" % elapsed)
        assert into == "abcd", "Should not occur"
        return 0

_tests['ReaderStrType'] = __ReaderStrType

##############################################################################
## __Writer class: Simple variable write
##############################################################################
class __Writer:
    @staticmethod
    def run(argv):
        global _var
        debugf(argv[0] + ": Variable write test")
        start = time.time()
        for iter in range(_ITERATIONS):
            _var = "4321"

        elapsed = time.time() - start
        debugf(argv[0] + ": %.3f" % elapsed)
        assert _var == "4321", "Should not occur"
        return 0

_tests['Writer'] = __Writer

##############################################################################
## __WriterPlus class: Simple variable write with type checking
##############################################################################
class __WriterPlus:
    @staticmethod
    def run(argv):
        global _var
        debugf(argv[0] + ": Variable write test with type checking")
        start = time.time()
        for iter in range(_ITERATIONS):
            assert isinstance("4321", str), "Just for timing"
            _var = "4321"

        elapsed = time.time() - start
        debugf(argv[0] + ": %.3f" % elapsed)
        assert _var == "4321", "Should not occur"
        return 0

_tests['Writer+'] = __WriterPlus

##############################################################################
## __WriterDict class: Simple dictionary write
##############################################################################
class __WriterDict:
    @staticmethod
    def run(argv):
        global _dict
        debugf(argv[0] + ": Dictionary write test")
        start = time.time()
        for iter in range(_ITERATIONS):
            _dict["THAT"] = "4321"

        elapsed = time.time() - start
        debugf(argv[0] + ": %.3f" % elapsed)
        assert _dict["THAT"] == "4321", "Should not occur"
        return 0

_tests['WriterDict'] = __WriterDict

##############################################################################
## __WriterStrType class: lib.Types.String writer
##############################################################################
class __WriterStrType:
    @staticmethod
    def run(argv):
        into = String("abcd")
        debugf(argv[0] + ": lib.Types.String write test")
        start = time.time()
        for iter in range(_ITERATIONS):
            into._ = "bcda"

        elapsed = time.time() - start
        debugf(argv[0] + ": %.3f" % elapsed)
        assert into._ == "bcda", "Should not occur"
        return 0

_tests['WriterStrType'] = __WriterStrType

##############################################################################
## __Command class
##############################################################################
class __Command:
    @staticmethod
    def run(argv):
        global _error_count

        if len(argv) > 1:
            name = argv[1]
            argv = argv[1:]
            try:
                _error_count += _tests[name].run(argv)
            except KeyError:
                _error_count += 1
                debugf("No test:" , name)
            except:
                _error_count += 1
                Debug.handle_exception()
        else:
            for _test in sorted(_tests):
                try:
                    _error_count += _tests[_test].run([_test])
                except:
                    _error_count += 1
                    Debug.handle_exception()
                debugf("")

        debugf(format_count(_error_count))
        return 0

command['timing'] = __Command

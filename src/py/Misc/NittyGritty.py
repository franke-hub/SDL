#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2019-2021 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       NittyGritty.py
##
## Purpose-
##       Demonstrate some of what python can do.
##
## Last change date-
##       2021/04/09
##
##############################################################################
import sys
import argparse
import calendar
import inspect
import keyword
import random

#### Compile-time controls ###################################################
USE_CT_TRACEBACK = False            ## Compile-time stack trace

#### lib #####################################################################
from lib.Command     import *
from lib.Debug       import *
from lib.Global      import *
from lib.Utility     import *

##############################################################################
import Main

##############################################################################
## Internal data areas
##############################################################################
TESTING  = None                     ## Testing, parameter controlled
VERBOSE  = None                     ## Verbosity, parameter controlled

demoDict = {}                       ## Demo function dictionary
wasname  = __name__                 ## Name during compilation

def initialize():                   ## Initialize attributes
    global TESTING, VERBOSE
    TESTING = Global.TESTING
    VERBOSE = Global.VERBOSE

##############################################################################
## Expected/missed exception handlers
##############################################################################
def handle(X):                      ## Handle expected exception
    S = type(X).__name__
    if VERBOSE: debugf("OK, expected %s:" % S, X)

def isabug(S):                      ## Handle error
    debugf("NG, %s" % S)
    return 1

def missed(X):                      ## Handle missed exception
    if X is None: X = 'Exception'
    debugf("NG, missed exception:", X)
    return 1

##############################################################################
## Trace utilities (NOTE: High overhead. Avoid use in production code.)
##############################################################################
def callername():                   ## TODO: Remove calls from production code
    stack = inspect.stack()
    si = stack[2]
    name = si[1] + ':' + str(si[2]) + ', in ' + si[3]
    del stack
    return name

def traceback(skip=2):              ## TODO: Remove calls from production code
    debugf('traceback, most recent call first')
    stack = inspect.stack()
    for i in range(skip, len(stack)): ## Skip traceback, call to traceback
        si = stack[i]
        file = si[1]
        func = si[3]
        line = str(si[2])
        text = si[4]
        if text is None: text = '** compiled **'
        else: text = text[0].strip()
        debugf('File "%s", line %s, in %s\n  %s' % (file, line, func, text))
    del stack
if USE_CT_TRACEBACK: traceback(0)

##############################################################################
## TEST: Sample (test and demo)
##############################################################################
def test_sample():                  ## Demo needs source code
##  if VERBOSE > -1: return         ## Sample skip
    if TESTING != 'sample': return  ## Sample skip

    debugf('\nThis sample is used to cut and paste more demos.')
    if True: return False           ## Sample test
    return 'DEMO'                   ## Sample demo
demoDict['test sample: (A sample test. If selected, it fails.)'] = test_sample

##############################################################################
## TEST: Fake True/False tests
##############################################################################
def test_False():                   ## Test False
    return 'False'

def test_True():                    ## Test True
    return 'True'

##############################################################################
## TEST: Fake ++/--/+=/-= operrator tests
##############################################################################
## X++ or X--: SyntaxError
## ++X or --X: X, but ---X == -X (as does +-X and -+X)

demoDict['Has ++ operator.'] = test_False
demoDict['Has += operator, so use X += 1 instead of X++.'] = test_True
demoDict['Has -- operator.'] = test_False
demoDict['Has -= operator, so use X -= 1 instead of X--.'] = test_True

##############################################################################
## TEST: Attributes (simply)
##############################################################################
def test_attributes():
    if VERBOSE > -1: return

    debugf('\nSee source code for detail')
    _dict = {'a': 'alpha', 'b': 'beta'}
    _int  = 732
    _list = ['a', 'b', _int, _dict]
    _str  = '=str='
    _tuple = (_dict, _int, _list, _str)

    ## print(_tuple) ## (Before modification) ################################
    _b = 'b'
    _dict[_b] = 'BETA'
    _dict['c'] = 'gamma'
    _int = 237
    _list += ['added']
    _str = '=STR='
    ##########################################################################
    ## The tuple holds the integer and string values, but for list and dict
    ## attribute holds references. Their content can change.
    ## print(_tuple) ## (After modification)
    assert _tuple[0]['c'] == 'gamma'   ## [_dict]['c'] (added)
    assert _tuple[1] == 732            ## << DOES NOT CHANGE >>
    assert _tuple[2][3]['b'] == 'BETA' ## [_list][_dict]['b'] (modified)
    assert _tuple[3] == '=str='        ## << DOES NOT CHANGE >>

    ## But, replacing the list or dictionary does not affect the tuple
    _dict = {}                         ## Replace the dictionary
    _list = []                         ## Replace the list
    assert _tuple[2][3]['b'] == 'BETA' ## [_list][_dict]['b'] (modified)

    ## But the tuple's original list and dictionary objects remain volatile
    _tuple[2][3]['b'] = 'DELTA'        ## [_list][_dict]['b'] (modified)
    assert _tuple[0]['b'] == 'DELTA'   ## [_dict]['b'] (modified)

    ## The tuple's list and dictionary references cannot be modified
    try:
        random.seed()
        if random.random() >= 0.5:
            _tuple[0] = _tuple[0]      ## This this might work someday but
        else:                          ## even with super optimization this
            _tuple[2] = _tuple[2]      ## should generate a warning, right?
        raise Exception('Huh? Do tuples allow item assignment now?')
    except TypeError:
        pass

    ##########################################################################
    ## You can't use dicts or lists as index values (they might change.)
    ## You also can't use tuples containing dicts or lists as index values,
    ## but tuples with strings and ints are OK.
    _test = [ _int,  _str, (_str, _int),   [],   {}, ([],), ({},)]
    _fail = [False, False,        False, True, True,  True,  True]
    _dict = {}
    for i in range(len(_test)):
        try:
            _dict[_test[i]] = 'foo'
            if _fail[i]:
                raise Exception('Unexpected success:', type(_test[i]))
        except TypeError:
            if _fail[i] == False:
                raise Exception('Unexpected failure:', type(_test[i]))

    ##########################################################################
    ## When int or string attribute values are replaced, copies do not change
    a = _str
    b = a
    a = 'mod'
    assert b == _str and a == 'mod'

    a = 732
    b = a
    a = 237
    assert b == 732 and a == 237

    return 'DEMO'

demoDict['demo -1: Attribute tests'] = test_attributes

##############################################################################
## TEST: Are declarations "class NAME" and "class NAME()" equivalent?
##############################################################################
def test_class_declaration_equivalence():
    ## See: https://docs.python.org/3/reference/compound_stmts.html#class
    return 'None'

text = 'Difference between "class X:", "class X():", and "class X(object):"'
demoDict[text] = test_class_declaration_equivalence

##############################################################################
## TEST: Deferred definition
##############################################################################
def test_deferred_definition():
    ## We can invoke classes defined later. This works because module
    ## compiliation only checks syntax. Name resolution occurs later,
    ## when the code actually runs.
    call_before_defined()
    assert Construct_before_defined.HAS_ATTRIBUTE == 'ja, you betcha'
    thing = Construct_before_defined()
    assert thing.has_attribute == 'ja, you betcha bootie it does'
    return True                     ## Exception if not True

def call_before_defined():
    return True

class Construct_before_defined():
    HAS_ATTRIBUTE = 'ja, you betcha'
    def __init__(self):
        self.has_attribute = 'ja, you betcha bootie it does'

demoDict["Can use classes and functions before they're defined"] = test_deferred_definition

##############################################################################
## TEST: Call by reference, Call by value
##############################################################################
class Thing:
    def __init__(self, init):
        self.data = init

def call_by_reference(T):
    T.data = 'ABC'
    return T

def call_by_value(S):
    S = 'ABC'
    return S

def test_call_by_reference():
    inp = Thing('abc')
    old = inp
    ret = call_by_reference(inp)
    if inp.data != 'ABC' or ret.data != 'ABC': return False
    assert inp is old and ret is old, '%s,%s,%s' % (old, inp, ret)
    return True

def test_call_by_value():
    inp = 'abc'
    old = inp
    ret = call_by_value(inp)
    if inp != 'abc' or ret != 'ABC': return False
    assert inp is old and ret is not old, '%s,%s,%s' % (old, inp, ret)
    return True

def test_call_by():
    return test_call_by_reference() and test_call_by_value()
demoDict['Can call by reference or value'] = test_call_by

##############################################################################
## DEMO: Compile-time traceback
##############################################################################
def demo_compile_time_traceback():  ## Demo needs source code
    if TESTING != 'compile-traceback': return

    debugf('\nTo run this test, modify "%s",' % __file__)
    debugf('changing "USE_CT_TRACEBACK" from "False" to "True"')
    return 'INFO'
demoDict['demo compile-traceback: Compile-time traceback'] = demo_compile_time_traceback

##############################################################################
## DEMO: Exception handling
##############################################################################
def demo_exception_handling():      ## Demo needs source code
    if VERBOSE > -3: return

    debugf('\nSee source code for detail')
    ng = NittyGritty(123, 456, dis='dat', und='so_weiter') ## Define a class
    try:
        debugf(ng.no_such_attribute)
        assert False, 'Expected AttributeError'
    except AttributeError as X:
        handle(X)
        Debug.handle_exception()
    return 'DEMO'
demoDict['demo -3: Exception handling'] = demo_exception_handling

##############################################################################
## DEMO: main-attributes: Getting variables directly from Main (+Misc)
##############################################################################
def demo_main_attributes():         ## Only runs on demand
    if TESTING != 'main-attributes': return

    Thing = Main.Thing
    debugf('\nin %s.demo_main_attributes' % __name__)
    debugf('__name__: When compiled(%s), Now(%s)' % (wasname, __name__))
    debugf('Global.TESTING(%s), Thing.TESTING(%s), Global.ADDED_BY(%s)' %
         (Global.TESTING, Thing.TESTING, Global.ADDED_BY))

    ## Why you can't access the compiled Main from outside of it:
    ##   When compiling Main, Thing's module name is '__main__'
    ##   When importing Main, Thing's module name is 'Main'
    ## While you could demangle '__main__', why bother?  You can simply put
    ##   attributes into lib.Global and be done with it. (e.g. Global.ADDED_BY)
    debugf('Thing: <%s (%s,%s,%s,%s)>' % \
           ( type(Thing), Thing.__module__, Thing.__name__
           , '*' ## 'dir("Thing"):\n' + str(dir("Thing"))
           , '*' ## 'dir("Main"):\n' + str(dir("Main"))
           )
          )
    return 'DEMO'
demoDict["demo main-attributes: How to get Main's attributes. (Don't try.)"] = demo_main_attributes

##############################################################################
## DEMO: traceback
##############################################################################
def demo_traceback():
    if VERBOSE > -2: return
    debugf('\nNORMAL: ', end=''); traceback()
    return 'DEMO'
demoDict['demo -2: Traceback'] = demo_traceback

##############################################################################
## DEMO: tuple vs int
##############################################################################
def demo_tuple_vs_int():            ## Only runs on demand
    if VERBOSE > -2: return

    ## The (456,) notation differentiates a TUPLE of length 1 from INT 456
    ## type((456)): <class 'int'>; type((456,)): <class 'tuple'>

    debugf('\ntype((456,)):', type((456,)), 'type((456)):', type((456)))
    debugf('print( (456,), (456) ) =>', (456,), (456))
    return 'DEMO'

demoDict['demo -2: Tuple vs int'] = demo_tuple_vs_int

##############################################################################
## DEMO: Usage
##############################################################################
def demo_usage():
    if VERBOSE < 0 or VERBOSE > 1 or TESTING: return

    debugf('\nUsage:')
    debugf('The --testing=test parameter runs '
           'a demo or test that\'s normally skipped.')
    debugf('  Example: use "--testing=sample" to run the sample test')
    debugf('The --verbose=N parameter controls output verbosity.')
    debugf('Positive values control verbosity in the normal manner:\n'
           '  0 selects the least detail, higher values select more.')
    debugf('Negative values select standard demos:\n'
           '  -1 selects the fewest demos, -999 selects all of them.')
##  debugf('When 0 < VERBOSE > 1 or TESTING: '
##         'usage information isn\'t displayed, 'but you already knew that.')

    return 'INFO'
demoDict['~Usage information'] = demo_usage

##############################################################################
## Command[keywords]: List python keywords
##############################################################################
class __Keywords:
    @staticmethod
    def run(*argv):
        initialize()
        if VERBOSE > 0: debugf(keyword.kwlist)
        else: debugf('Skipped: VERBOSE <= 0,', VERBOSE)
        return 0
command['keywords'] = __Keywords

##############################################################################
## Command[gritty]: Python demonstration
##############################################################################
class NittyGritty():
    ## Reserve space for attributes: optimization improvement
    ## If you use __slots__, you must name *ALL* the attributes,
    ## but not the methods
    __slots__ = '_this', '_that', '_other' \
              , 'number', 'args', 'kwargs'

    def __init__(self, number=732, *args, **kwargs): ## Constructor
        self._that = 'is that'
        self._this = 'is this'
        self._other = 'is other'

        self.number = number
        self.args   = args
        self.kwargs = kwargs

    def debug(self):
        debugf('%s.debug' % __class__.__name__)
        if VERBOSE: debugf('caller:', callername())
        if VERBOSE > 2: debugf('NORMAL ', end=''); traceback()

##      return ## TOO VERBOSE FOR NOW ##
        if VERBOSE > 1: debugf('self.__class__:', self.__class__)
        debugf('__class__:', __class__)
        debugf('__name__:', __class__.__name__)
        debugf('__slots__:', self.__slots__)

        debugf('_this:',  self._this)
        debugf('_that:',  self._that)
        debugf('_other:', self._other)
        debugf('number:', self.number)
        debugf('args:',   self.args)
        debugf('kwargs:', self.kwargs)

class __NittyGritty:
    @staticmethod
    def run(*argv):
        initialize()
        errors = 0

        for demo in sorted(demoDict):
            rc = demoDict[demo]()
            if rc is None: rc = 'SKIP'
            debugf('%8s = %s' % (rc, demo))
            if isinstance(rc, bool) and rc == False:
                errors += 1

        if errors:
            debugf('%8d ******** Test%s failed! ********'
                  % (errors, ('' if errors == 1 else 's')))
        return errors

command['gritty'] = __NittyGritty

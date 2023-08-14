#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2016-2019 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Samples.py
##
## Purpose-
##       Pythons sample program, demonstrating compiler features/user errors
##
## Last change date-
##       2019/08/13
##
## Implementation note-
##       python version 3 required: python2 disallows added keyword syntax
##
##############################################################################
from __future__ import print_function ## For print(*args)
import codecs                       ## For __UnicodeEncodeDecode
import json                         ## For __JSON
import pickle                       ## For pickle.dump
import sys                          ## For sys.stdout

import Config                       ## For Config.VERBOSE
from lib.Debug import Debug, debugf ## Debugging helpers
from Test import Test               ## Test base class

##############################################################################
## The Test Dictionary
##############################################################################
dict = {} ## dict['Test_name'] = Class_name
dict['base'] = Test

## This cannot be imported until the dictionary is defined
import Dirty                        ## A quick and dirty test

##############################################################################
## Controls
##############################################################################
VERBOSE = Config.VERBOSE
if False:
    VERBOSE = 1

##############################################################################
## Internal functions
##############################################################################
def assert_verified():
    print("Test internally verified by assertions")

def debugf(*args, **kwargs):
    if VERBOSE:
        print(*args, **kwargs)

##############################################################################
## The UnexpectedException
##############################################################################
class UnexpectedException(Exception):
    def __init__(self):
        Exception.__init__(self,"UnexpectedException")

##############################################################################
## Arguments
##
## Demonstrates-
##  1) The interpretation phase only checks syntax.
##     (Forward and backward references are allowed inside classes.)
##  2) The args and kwargs names are by convention only.
##  3) Note that for ArgBar, the 'added' parameter is never part of **kwargs.
##     It is always used or checked separately.
##
## Implementation notes-
##  Keyword dictionary names are not necessarily sequential. Our two keyword
##  arguments can be returned in either order.
##
##############################################################################
class __Arguments(Test):
    def run(self):
        assert_verified()
        expect = "('Test1', (), {})"
        assert self.ArgFoo("Test1") == expect
        expect = "('Test2', ('a', 'b', 'c'), {})"
        result = self.ArgFoo("Test2", "a", "b", "c")
        assert result == expect
        expect = "('Test3', ('a', 'b', 'c'), {'D': 'd', 'E': 'e'})"
        result = self.ArgFoo("Test3", "a", "b", "c", D="d", E="e")
        if result != expect:
            expect = "('Test3', ('a', 'b', 'c'), {'E': 'e', 'D': 'd'})"
            assert result == expect

        debugf("")
        self.ArgBar("Test1")
        self.ArgBar("Test2", "a", "b", "c")
        self.ArgBar("Test3", "a", "b", "c", D="d", E="e")

    def ArgFoo(self, parm, *args, **kwargs):
        result = str((parm, args, kwargs))
        debugf("ArgFoo:", parm, args, kwargs)
        self.ArgBar(parm, *args, **kwargs)
        if parm == "Test3":
            self.ArgBar("Oops3", args, kwargs) ## Example of improper subcall
            for arg in args:
                debugf("Arg:", "'"+arg+"'")
            for kwarg in kwargs:
                debugf("Kwarg:", kwarg, "=", "'"+kwargs[kwarg]+"'")

            expect = "('Test3', ('a', 'b', 'c'), {'D': 'd', 'E': 'e'})"
            output = self.ArgBar("Test3", *args, added='Value', **kwargs)
            if output != expect:
                expect = "('Test3', ('a', 'b', 'c'), {'E': 'e', 'D': 'd'})"
                assert output == expect
            assert self.added == 'Value'

            kwargs['added'] = 'Thing' ## Interesting case (no expected change)
            expect = "('Test3', ('a', 'b', 'c'), {'D': 'd', 'E': 'e'})"
            output = self.ArgBar("Test3", *args, **kwargs)
            if output != expect:
                expect = "('Test3', ('a', 'b', 'c'), {'E': 'e', 'D': 'd'})"
                assert output == expect
            assert self.added == 'Thing' ## But the keyword is recognised

        return result

    def ArgBar(self, parm, *sgrx, added='Nothing', **sgrxwk):
        result = str((parm, sgrx, sgrxwk))
        debugf("ArgBar:", parm, sgrx, sgrxwk)
        if parm == 'Test3': debugf("Argbar: added='%s'" % added)
        self.added = added

        return result

dict['args'] = __Arguments

##############################################################################
## Case (pseudo-statement)
##############################################################################
class CaseOBJ:
    def __init__(self, fc):
        self.cc = -1
        self.fc = fc

    def post(self, code):
        self.cc = code

    def stop(self, code):
        self.cc = -code

class CaseTest(Test):
    def switch(self, obj):
        def zip(obj, rc=0):
            debugf("case zip")
            obj.stop(obj.fc)

        def one(obj, rc=1):
            debugf("case one")
            obj.post(rc)

        def two(obj, rc=2):
            debugf("case two")
            obj.post(rc)

        dict = { 1: one
               , 2: two
               }

        ## These two versions are operationally identical
        if True:   ## Preferred version
            method = dict.get(obj.fc, zip)
        else:      ## Use this if you never heard of dict.get(value, default)
            try:
                method = dict[obj.fc]
            except KeyError: ## A.K.A. default:
                method = zip

        method(obj)

    def run(self):
        assert_verified()
        obj = CaseOBJ(1)
        self.switch(obj)
        assert obj.cc == 1, "Unexpected result %d" % obj.cc

        obj = CaseOBJ(2)
        self.switch(obj)
        assert obj.cc == 2, "Unexpected result %d" % obj.cc

        obj = CaseOBJ(3)
        self.switch(obj)
        assert obj.cc == -3, "Unexpected result %d" % obj.cc

dict['case'] = CaseTest

##############################################################################
## Classes
##############################################################################
class Classes(Test):
    class Base:
        is_base = True
        is_derived = False

        global_ = "Global data"     ## Note: "global" is a python keyword
        common_ = "Global common"

        def __init__(self,common="Local common"):
            self.local = "Local data"
            self.common_ = common
            ## self.update = "Overrides function!" ## Causes runtime error later

        def say(self):
            debugf("Global:", Classes.Base.global_)
            debugf("Local:", self.local)
            debugf("Global.common:", Classes.Base.common_)
            debugf("Local.common:", self.common_)
            debugf()

        def update(self,common):
            self.common_ = common

    class Derived(Base):
        is_base = False
        is_derived = True

        def __init__(self,common="Common local"):
            Classes.Base.__init__(self,common)

        def say(self):
            self.local = "I am derived"  ## Access Base data
            Classes.Base.say(self)       ## Access Base function

    def __init__(self):
        debugf("Classes.__init__")

    def run(self):
        assert_verified()
        obj = Classes.Base("My local common")
        assert isinstance(obj, Classes.Base)
        assert obj.is_base
        assert not obj.is_derived
        obj.local ## NOTE: THIS DOES NOTHING!
        obj.say   ## NOTE: THIS DOES NOTHING!
        obj.say()

        Classes.Base.common_ = "Changed Base"
        obj.common_ = "Changed object.common"
        obj.local = "Changed object.local"
        obj.say()

        obj.update("Changed object.common with update")
        obj.say()

        obj.addedVariable = "Well, howdy"
        obj.update("New variable added: " + obj.addedVariable)
        obj.say()

        der = Classes.Derived()
        assert isinstance(der, Classes.Base)
        assert isinstance(der, Classes.Derived)
        assert not der.is_base
        assert der.is_derived
        der.say()

dict['classes'] = Classes

##############################################################################
## Compile__: Accessing variables beginning with __
##############################################################################
class CompileUU:
    __du = None                     ## Trying to access this from outside

    def __init__(self, name):
        debugf("CompileUU.__init__")
        if CompileUU.__du == None:
            CompileUU.__du = self
            debugf("SET:", dir(CompileUU.__du))
        self.name = name

    @staticmethod
    def get():
        debugf("GET:", dir(CompileUU.__du))
        return CompileUU.__du

class Compile__(Test):
    def run(self):
        assert_verified()
        if True: # OK # Access local __ variables
            one = CompileUU('__du=one')
            two = CompileUU('two')

            debugf("ONE:", CompileUU.get().name)
            assert '__du=one' == CompileUU.get().name
            debugf("__du:", CompileUU._CompileUU__du.name) # Via name mangling
            assert '__du=one' == CompileUU._CompileUU__du.name

        if True: # OK # Usage of __XX__ functions
            obj = CompileOps(number=732)
            debugf("REP:", obj.__repr__())
            debugf("STR: %s" % obj)
            assert obj.initialized ## variable doesn't exist unless __init__ called
            assert int(obj) == 732
            assert len(obj) == 366
            assert str(obj) == "<CompileOps,732>"

def CompileRepr(obj):
    return Test.__repr__(obj)

class CompileOps(Test): ## This is just an object used by Compile__
    ##########################################################################
    ## The __XX__ functions
    def __init__(self, number=42): ## Called to initialize the object
        self.initialized = True
        self.number = number

    def __del__(self):       ## Called to finalize the object
        self.initialized = False

##  def __repr__(self):      ## The "official" self representation (for pickle?)
##      return Test.__repr__(self) ## Use the superclass version
##  __repr__ = Test.__repr__ ## Oooh, oooh. Define by setting! This works!
    __repr__ = CompileRepr   ## Oooh, oooh. Define by setting! This too!

    def __str__(self):       ## The result of str(self)
        return "<CompileOps," + str(self.number) + ">"

    def __len__(self):       ## The result of len(self)
        return self.number // 2

    ## Sample operators ######################################################
    ## See: https://docs.python.org/3/reference/datamodel.html#special-method-names
    def __radd__(self, L):    ## Called to reverse add the object
        return L + self.number

    def __add__(self, R):    ## Called to add the object
        return self.number + R

    def __int__(self):       ## Convert to integer
        return self.number

dict['__'] = Compile__

##############################################################################
## Conditional (Test conditionals)
##############################################################################
class Conditional(Test):
    @staticmethod
    def failed(R):
        raise Exception("Unexpected result(%s)" % R)

    def output(stmt, result):
        print("%s: %s" % (stmt, result))

    def run(self):
        output = Conditional.output
        F = "False"
        T = "True"

        assert_verified()
        print("But some results might surprise you")
        print('blank = " "')
        print('empty = ""')
        print('nada = None')
        print('text = "None"')
        print('')
        blank = " "
        empty = ""
        nada = None
        text = "None"

        output('if blank', T)
        if not blank: failed(F)

        output('if empty', F)
        if empty: failed(T)

        output('if "" == None', F)
        if "" == None: failed(F)

        output('if None == None', T)
        if not None == None: failed(F)

        output('if text == None', F)
        if text == None: failed(T)

        output('if nada == None', T)
        if not nada == None: failed(F)

dict['condition'] = Conditional

##############################################################################
## ConsoleInput
##############################################################################
class ConsoleInput(Test):
    def console(self, prompt):
        return input(prompt)

    def run(self):
        print('type "WHAT" when prompted...')
        cc = True
        while cc:
            cc = (self.console('> ').upper() != "WHAT")

dict['terminal'] = ConsoleInput

##############################################################################
## Decorators
##############################################################################
class _Decorators(Test):
    def __init__(self):
        self.bar = False
        self._p = None

    @classmethod
    def classy(cls):
        result = cls()
        result.p = "Bar"
        return result

    @staticmethod
    def staticy():
        return "ja, you betcha"

    @property
    def p(self):
        """Get the 'p' property"""
        debugf("'%s'=p.getter" % self._p)
        return self._p

    @p.setter
    def p(self, value):
        """Set the 'p' property"""
        debugf("p.setter(%s)" % value)
        self._p = value

    @p.deleter
    def p(self):
        """Delete the 'p' property"""
        debugf("p.deleter")
        self.bar = True
        del self._p

    def run(self):
        assert_verified()

        assert _Decorators.staticy() == "ja, you betcha"

        instance = _Decorators()
        assert instance.p is None
        instance.p = "Foo"
        assert instance.p == "Foo"
        del instance.p
        assert instance.bar

        instance = _Decorators.classy()
        assert instance.p == "Bar"
        assert instance.bar is False

        instance = _Classy.classy()
        assert instance.p == "Bar"  ## (classy run AFTER __init__)
        assert instance.bar is True ## (Set in _Classy.__init__)

class _Classy(_Decorators):
    def __init__(self):
        _Decorators.__init__(self)
        self.bar = True
        self._p = "Yo"

dict['decorators'] = _Decorators

##############################################################################
## Dictionary
##############################################################################
class Dictionary(Test):
    def printMe(self,book,name):
        try:
            result = book[name]
        except:
            result = "unknown"

        debugf("[%s] %s" % (name, result))
        return result

    def run(self):
        assert_verified()
        phonebook = {}
        phonebook["Jane"] = 1112223333
        phonebook["John"] = 1112224444
        phonebook["Jack"] = 1112225555
        phonebook["Jill"] = 1112226666

        assert self.printMe(phonebook, "Jack") == 1112225555
        assert self.printMe(phonebook, "Jane") == 1112223333
        assert self.printMe(phonebook, "Jill") == 1112226666
        assert self.printMe(phonebook, "John") == 1112224444
        assert self.printMe(phonebook, "Fred") == 'unknown'

dict['dict'] = Dictionary

##############################################################################
## ElseClause (Outside of if)
##############################################################################
class ElseClause(Test):
    def run(self):
        assert_verified()
        count = 0
        while count < 5:
            debugf(count)
            count += 1
        else:
            debugf("count:", count)
            assert count == 5

        count = 0
        while count < 5:
            debugf(count)
            count += 1
            if count > 2:
                count += 1
                continue

        else:
            debugf("count:", count)
            assert count == 6

        count = 0
        while count < 5:
            debugf(count)
            count += 1
            if count > 3:
                break

        else:
            debugf("count:", count)
            assert False, "LOGIC ERROR: else clause invoked"

        try:
            raise Exception("expected")
        except:
            debugf("Expected Exception raised")
        else:
            assert False, "Else after raised exception"

        try:
            pass
        except:
            assert False, "pass statement raised exception"
        else:
            debugf("No exception raised, none expected")

dict['else'] = ElseClause

##############################################################################
## Exceptional (Throw an exception or two)
##############################################################################
class Exceptional(Test):
    def run(self):
       raise Exception("Just Kidding")

dict['exception'] = Exceptional

##############################################################################
## Idiom (Test python idioms)
##############################################################################
class __Idiom(Test):
    def run(self):
        assert_verified()
        ## The c++ "message = control ? 'foo' : 'bar'" equivalent
        control = True
        message = 'foo' if control else 'bar'
        assert(message == 'foo')

        control = False
        message = 'foo' if control else 'bar'
        assert(message == 'bar')

dict['idiom'] = __Idiom

##############################################################################
## Init (Is __init__ called from a derived class with or without __init__?
##   If derived class does not have __init__, base __init__ implicitly called
##   If derived class has __init__, base __init__ must be explicitly called
##############################################################################
class _Init_X1(object):
    def __init__(self):
        debugf("_Init_X1.__init__")
        self.init_called = True

class _Init_X2(_Init_X1):
    def verify(self):
        assert self.init_called == True

class _Init_X3(_Init_X2):
    pass

class _Init_X4(_Init_X1):
    def __init__(self):
        debugf("_Init_X4.__init__")

class __InitTest(Test):
    def run(self):
        if False:
            global VERBOSE
            VERBOSE = 1
        assert_verified()
        x2 = _Init_X2()
        x2.verify()
        x3 = _Init_X3()
        assert hasattr(x3, 'init_called')
        x3.verify()
        x4 = _Init_X4()
        assert not hasattr(x4, 'init_called')

dict['init'] = __InitTest

##############################################################################
## JSON
##
## This demonstrates that I haven't figured out JSON yet.
##
##############################################################################
def quote(string):
    return "'" + str(string) + "'"

class __JSON(Test):
    def run(self):
        stuff = [1, 2, 3, "a", "b", "c"]
        print("STUFF:", quote(stuff))
        print("json.dumps:", quote(json.dumps(stuff)))
        print("json.loads:", quote(json.loads(json.dumps(stuff))))
        print("pickle:", quote(pickle.loads(pickle.dumps(stuff))))

dict['json'] = __JSON

##############################################################################
## List handling
## Can lists contain lists? (You betcha. Floats and ints can print as strings)
##############################################################################
class __Lists(Test):
    def run(self):
        inner1 = [1, 3, 3]
        inner1[2] = inner1[0] + inner1[1]
        inner2 = ["a", "b", "c"]
        inner2[2] = inner2[0] + inner2[1]
        inner3 = [5.0,6.0,7.0]
        inner3[2] = inner3[0] + inner3[1]

        ## This works too
        inner3 = [5.0,6.0]
        inner3 += [inner3[0] + inner3[1]]

        outer1 = [inner1, inner2, inner3]
        for i in range(0,3):
            for j in range(0,3):
                print("[%s][%s] '%s'" % (i, j, outer1[i][j]))

        print(inner1 * 3)
        print(inner2 * 3)
        print(inner3 * 3)

        inner4 = []
        for i in inner1:
            inner4 += [i * 3]
        print(inner4)

dict['lists'] = __Lists

##############################################################################
## Mistakes (Things you shouldn't do)
##############################################################################
class __Mistakes(Test):
    def run(self):
        assert_verified()
        ## Use a really large range (only a problem in python2)
        ## This is only a problem in python2
        ## In python3, range becomes xrange and xrange is deprecated
        if sys.version_info.major < 3:
            try:
                for i in range(0x7fffffffffff): ## Use xrange in python2
                    if i > 1000:
                        raise UnexpectedException()
            except UnexpectedException:
                print("UnexpectedException raised!")
                raise
            except Exception as x:
                print("Exception(%s) raised as expected" % x)
                Debug.handle_exception()
            else:
                raise Exception("Exception expected, not raised")

dict['mistakes'] = __Mistakes

##############################################################################
## Module (Examine Module variables, functions, and classes)
##############################################################################
class __Module(Test):
    def run(self):
        import Module
        assert_verified()

        ######################################################################
        ## This verifies module access to inner classes
        assert Module.Outer.local_get() == "__Local.data OK"
        assert getattr(Module, '__Local')().get() == "__Local.data OK"

        ######################################################################
        ## Access module variables and functions from outside the module
        assert Module.CONTROL == 1
        assert Module._CONTROL == 2
        assert Module.get_control() == 2

        Module.set_control(3)
        assert Module._CONTROL == 3
        assert Module.get_control() == 3

        Module.CONTROL = 3
        Module._CONTROL = 4
        assert Module.CONTROL == 3
        assert Module._CONTROL == 4
        assert Module.get_control() == 4

        assert Module.get() == "password"
        assert Module.set("wordpass") == "password"
        assert Module.get() == "wordpass"

        ######################################################################
        ## Access mangled name (before it's changed)
        assert Module.Other.get_outer__classvar() == "Mangled class variable"
        assert Module.Outer._Outer__classvar == "Mangled class variable"

        ######################################################################
        ## Access module class information from outside the module
        assert Module.Outer.static_get() == "Mangled class variable"
        assert Module.Outer.static_set("CHANGED") == "Mangled class variable"
        assert Module.Outer.static_get() == "CHANGED"

        one = Module.Outer()
        assert Module.Outer.outervar == "Outer class variable"
        Module.Outer.outervar = "OUTER class variable"
        assert Module.Outer.outervar == "OUTER class variable"

        two = Module.Outer(data="datum")
        assert two.data == "datum"
        assert two.outervar == "OUTER class variable"
        two.outervar = "two"
        assert Module.Outer.outervar == "OUTER class variable"
        assert one.outervar == "OUTER class variable" ## Access class
        assert two.outervar == "two" ## Access instance (because it changed)

        assert two.get() == "PASSWORD"
        assert two.set("WORDPASS") == "PASSWORD"
        assert two.get() == "WORDPASS"

        ## @staticmethod can be called using an instance variable
        assert one.static_get() == "CHANGED"
        assert two.static_get() == "CHANGED"

        ######################################################################
        ## Access mangled name (after it's changed)
        assert Module.Other.get_outer__classvar() == "CHANGED"
        assert Module.Outer._Outer__classvar == "CHANGED"

        ######################################################################
        ## Attributes can be dynamically added to classes and instances
        Module.Outer.Inner.added = "Added to Inner"
        assert Module.Outer.Inner.added == "Added to Inner"

        one = Module.Outer.Inner()
        Module.Outer.Inner.added = "Now changed"
        two = Module.Outer.Inner()
        two.added = "Two changed"
        assert one.added == "Now changed"
        assert two.added == "Two changed"

        ######################################################################
        ## Note that the global thingy declared in Module.Outer.Inner
        ## Applies to the module, not to Module.Outer.Inner
        assert getattr(Module.Outer.Inner, "thingy", None) == None
        assert Module.thingy == "Module thingy"

        assert getattr(one, "thingy", None) == None
        one.thingy = "Added one thingy"
        assert getattr(Module.Outer.Inner, "thingy", None) == None
        assert getattr(Module, "thingy", None) == "Module thingy"
        assert getattr(one, "thingy", None) == "Added one thingy"
        assert getattr(two, "thingy", None) == None

dict['module'] = __Module

##############################################################################
## NumberSize
## How big a number can we have (No obvious limit found)
##############################################################################
class NumberSize(Test):
    def run(self):
        n= 1
        p= 1
        while p < 66:
            print("power(%2d) %s" % (p, n))
            n *= 10
            p += 1

dict['numbersize'] = NumberSize

##############################################################################
## StringFormat
##############################################################################
class StringFormat(Test):
    def run(self):
        string = "Hello World!"
        print("str:", string)
        print("len:", len(string))
        print(".index('o'):", string.index("o"))
        print(".count('o'):", string.count("o"))
        print(".split(' '):", string.split(" "))
        ## print ".split('lo'):", string.split(["l","o"]) ## Runtime error
        ## print ".split(''):", string.split("") ## Runtime error
        print(".lower():", string.lower())
        print(".upper():", string.upper())
        start = 3
        stop  = 7
        skip  = 2
        print("[3:7]:", string[3:7])
        print("[3:7]:", string[start:stop])
        print("[3:7:2]: '%s'" % string[start:stop:skip])

        print("01234567890123456789012345")
        string = "abcdefghijklmnopqrstuvwxyz"
        ###### =  01234567890123456789012345
        print("%s[%s:%s:%s]: '%s'" % (string, 3,10, 2,string[ 3:10: 2]))
        print("%s[%s:%s:%s]: '%s'" % (string,"","",-1,string[  :  :-1]))
        print("%s[%s:%s:%s]: '%s'" % (string,"","",-2,string[  :  :-2]))
        print("%s[%s:%s:%s]: '%s'" % (string,16,"","",string[16:  :  ]))
        print("%s[%s:%s:%s]: '%s'" % (string,"",16,"",string[  :16:  ]))
        print("%s[%s:%s:%s]: '%s'" % (string, 1,"","",string[ 1:  :  ]))
        print("%s[%s:%s:%s]: '%s'" % (string,"",-1,"",string[  :-1:  ]))

        ## Howto: Sort format a dictionary...
        dict_ = {}
        dict_['W'] = 'WWWW'
        dict_['X'] = 'xxxx'
        dict_['Y'] = 'yYYy'
        dict_['Z'] = 'ZzzZ'

        print("one")
        print(dict_)
        print("two")
        print("{%s}" % ", ".join(["'%s': '%s'" % (name, dict_[name]) for name in sorted(dict_)]))

dict['string'] = StringFormat

##############################################################################
## Adding a Unicode encoder/decoder (which duplicates the 'latin1' encoding)
##############################################################################
class Identity:
    _identity_decoding_map = codecs.make_identity_dict(range(256))
    _identity_encoding_map = codecs.make_encoding_map(_identity_decoding_map)

    # Stateless encoder/decoder
    class IdentityCodec(codecs.Codec):
        def encode(self, input, errors='strict'):
            return codecs.charmap_encode(input, errors, Identity._identity_encoding_map)

        def decode(self, input, errors='strict'):
            return codecs.charmap_decode(input, errors, Identity._identity_decoding_map)

    # Incremental forms
    class IdentityIncrementalEncoder(codecs.IncrementalEncoder):
        def encode(self, input, final=False):
            return codecs.charmap_encode(input, self.errors, _identity_encoding_map)[0]

    class IdentityIncrementalDecoder(codecs.IncrementalDecoder):
        def decode(self, input, final=False):
            return codecs.charmap_decode(input, self.errors, _identity_decoding_map)[0]

    # Stream reader and writer
    class IdentityStreamReader(IdentityCodec, codecs.StreamReader):
        pass

    class IdentityStreamWriter(IdentityCodec, codecs.StreamWriter):
        pass

    def identify(encoding):
        """Return the codec for 'identity'."""
        if encoding == 'identity':
            return codecs.CodecInfo(
                name='identity',
                encode=Identity.IdentityCodec().encode,
                decode=Identity.IdentityCodec().decode,
                incrementalencoder=Identity.IdentityIncrementalEncoder,
                incrementaldecoder=Identity.IdentityIncrementalDecoder,
                streamreader=Identity.IdentityStreamReader,
                streamwriter=Identity.IdentityStreamWriter,
                )

        return None

    def register():
        codecs.register(Identity.identify)

class __UnicodeEncodeDecode(Test):
    def sample_test(self):
        try:
            codex = codecs.lookup('identity')
        except LookupError:
            pass
        else:
            raise Exception("identity codex already defined")

        if True and not VERBOSE: return
        text = u'pi: \u03c0'
        for error in [ 'ignore', 'replace', 'strict' ]:
            try:
                encoded = codecs.charmap_encode(text, error, Identity._identity_encoding_map)
            except UnicodeEncodeError as err:
                encoded = str(err)
            print('{:7}: {}'.format(error, encoded))

    def sample_utf8(self):
        """Examine utf-8 encoding"""
        if True and not VERBOSE: return
        print("\nsample_utf8")
        for i in range(256):
            s = chr(i)
            b = s.encode('utf-8')
            print("0x%.2x %3d s(%s) b(%s)" % (i, i, s, b))
            assert s == b.decode('utf-8')

        s = "\u03c0"
        b = s.encode('utf-8')
        print("0x%.4x %4d s(%s) b(%s)" % (0x03c0, 0x03c0, s, b))
        assert s == b.decode('utf-8')

    def run(self):
        assert_verified()
        self.sample_test()
        self.sample_utf8()

        Identity.register()
        codecs.lookup('identity')

        s = ""
        for i in range(256):
            s += chr(i)
        b = s.encode('identity')
        t = b.decode('identity')
        for i in range(256):
            assert ord(s[i]) == i
            assert b[i] == i
            assert ord(t[i]) == i
        assert len(b) == 256
        assert len(s) == 256
        assert len(t) == 256

        b = s.encode('latin1')
        t = b.decode('latin1')
        for i in range(256):
            assert ord(s[i]) == i
            assert b[i] == i
            assert ord(t[i]) == i
        assert len(b) == 256
        assert len(s) == 256
        assert len(t) == 256

dict['unicode'] = __UnicodeEncodeDecode

##############################################################################
## Variables
##############################################################################
class Variables(Test):
    def run(self):
        assert_verified()

        if True: ## OK ## Demonstrate arguments are passed by reference
            obj = VarObject()
            obj.name = 'original'
            assert obj.name == 'original'
            orig = obj.name
            assert orig is obj.name
            self.change(obj)
            assert obj.name == 'changed'
            assert obj.name != orig

    def change(self, arg):
        arg.name = 'changed'

class VarObject:
    pass

dict['vars'] = Variables

##############################################################################
## Compile-only tests:
##############################################################################
class CompileOnly(Test):
    def run(self):
        ## Test yesno with try/except
        print()
        print("Test yesno using try/except")
        try:
            while yesno('Hey, what? ', 2) == False:
                pass
        except IOError as e:                ## IOError exception
            print("IOError({0}): {1}".format(e.errno, e.strerror))
        #   print pickle.dump(e, sys.stdout)
            print(e)
            print("xxyyz")
        #     print "xxyyz1" ## Unexpected indent, won't compile
        # print "xxyyz2"     ## Unexpected indent, won't compile
            print("xxyyz3")
        except:                             ## Any other exception
            raise                           ## Raise it to next level
        else:                               ## No exception
            pass
        finally:
            print("Whew!")

        ## Test complete
            print("zzxxy1")  ## This works, even with a blank line
        print()
        # print "zzxxy2"     ## No indent match, won't compile
        print("zzxxy3")

def yesno(prompt, retries=4, complaint='Yes or no, please!'): # Optional parms
    while True:
        ok = input(prompt)
        if ok in ('y', 'ye', 'yes'):
            return True
        if ok in ('n', 'no', 'nop', 'nope'):
            return False
        retries = retries - 1
        if retries < 0:
            raise IOError(-1, 'refusenik user')
        print(complaint)

dict['compile-only'] = CompileOnly

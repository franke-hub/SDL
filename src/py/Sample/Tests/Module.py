#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2016 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Module.py
##
## Purpose-
##       Examine imported module and class attributes
##
## Last change date-
##       2016/06/07
##
##############################################################################
import sys

##############################################################################
## Module variables
##############################################################################
CONTROL = 1
_CONTROL = 2
__CONTROL = 3          ## Mangled name

##############################################################################
## Module functions
##############################################################################
__secret = "password"
def get():             ## Does not need global, variable only referenced
    return __secret

def set(v):            ## Demonstrate why global is needed
##  was = __secret     ## RUNTIME error: reference before assignment
    was = get()        ## This fixes the runtime error, but now
    __secret = v       ## Here __secret is a FUNCTION LOCAL variable
    return was         ## (And the global __secret is unchanged)

def set(v): ############# This REPLACES the broken implementation above
    global __secret
    was = __secret
    __secret = v
    return was

######################### Variable scope has nothing to do with the name
def get_control():     ## Similar logic to above with a different name
    return _CONTROL

def set_control(v):    ## This fails exactly like set(v) above
    _CONTROL = v       ## Here _CONTROL is a FUNCTION LOCAL variable

def set_control(v):    ## Again we replace the broken implementation
    global _CONTROL
    _CONTROL = v

##############################################################################
## Module.__Local class
##############################################################################
class __Local:
    def __init__(self):
        self.data = "__Local.data OK"

    def get(self):
        return self.data

_Local = __Local

##############################################################################
## Module.Outer class
##############################################################################
class Outer:
    __classvar = "Mangled class variable"
    outervar = "Outer class variable"

    class Inner:
        innervar = "Inner class variable"
        global thingy
        thingy = "Module thingy"

    def __init__(self, data=None):
        self.__secret = "PASSWORD"
        self.data = data

    ##########################################################################
    ## Double underscore module classes and functions cannot be easily
    ## accessed from within another class or function, so don't do it.
    ## Use a single underscore to indicate a protected class or function.
    @staticmethod
    def local_get():
        if False:                   ## Runtime failure
            global __Local          ## (Has no apparent effect)
            local = __Local()       ## name '_Outer__Local' is not defined
        if False:                   ## Runtime failure
            local = Module.__Local()  ## name 'Module' is not defined
        if False:                   ## Runtime failure
            local = getattr(Module, '__Local')()  ## name 'Module' is not defined
        if True:                    ## It can be done like this
            local = getattr(sys.modules['Module'], '__Local')()
        if True:                    ## or like this
            Module = sys.modules['Module']        ## Define Module
            local = getattr(Module, '__Local')()  ## Now this works

        local = _Local()            ## This is preferred
        return local.get()

    ##########################################################################
    ## Classes do not use global names to access class-level attributes.
    ## The attribute name is simply class qualified.
    @staticmethod
    def static_get():
        return Outer.__classvar

    @staticmethod
    def static_set(v):
        was = Outer.__classvar
        Outer.__classvar = v
        return was

    def get(self):
        return self.__secret

    def set(self, v):
        was = self.__secret
        self.__secret = v
        return was

##############################################################################
## Module.Other class
##############################################################################
class Other:
    @staticmethod
    def get_outer__classvar():
        return Outer._Outer__classvar

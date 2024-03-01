##############################################################################
##
##       Copyright (C) 2016-2023 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Utility.py
##
## Purpose-
##       Utility (or reminder) functions.
##
## Last change date-
##       2023/08/13
##
##############################################################################
import sys
import string                       ## For string.whitespace

from lib.Global import *            ## For Global.TESTING

##############################################################################
## Define available imports
##############################################################################
__all__ = [ 'bytes2str', 'int2bytes', 'len2bytes', 'quotify', 'str2bytes'
          , 'str2value' , 'Tokenizer', 'tokenize', 'visify' ]

##############################################################################
## Constants
##############################################################################
ESC    = 27                         ## The escape character ordinal
QUOTES = ["'", '"']                 ## The quotation characters

##############################################################################
## HowTo reminder functions
##############################################################################
class ReminderFunctions():          ## Reminder functions (Class deleted)
    def _echo(args):                ## (Argument echo, for debugging)
        count= 0
        for arg in args:
            print("[%2d] '%s'" % (count, arg))
            count= count + 1

    def _findWhite(S, x):           ## Find next blank in string S_
        L = len(S)
        while x < L and not S[x] in string.whitespace:
            x += 1
        if x >= L: x = -1
        return x

    def _isWhite(C):                ## Is character white space?
        assert isinstance(C, str), 'isWhiteSpace(%s) type(%s)' % (c, type(C))
        assert len(C) == 1, 'isWhiteSpace(%s) len(%s)' % (C, len(C))
        if C in string.whitespace:  ## (Preferred alternative)
            return True
        return False

    def _join(L):                   ## Join list L into a string
        return ' '.join(str(i) for i in L)

    def _plurals(N):                ## '' if N == 1 else 's'
        return '' if N == 1 else 's' ## Wrap in () in print statement

    def _split(S):                  ## Split string S into a list
        return S.split()

    def _skipWhite(S, x):           ## Skip white space in string_
        L = len(S)
        while x < L and S[x] in string.whitespace:
            x += 1
        if x >= L: x = -1
        return x

if Global.TESTING != 'lib.Utility.ReminderFunctions':
    del ReminderFunctions

##############################################################################
## ...For those of us who can't remember whether we should encode or decode...
##############################################################################
def bytes2str(bytes_):
    """Convert bytes into a string"""
    return bytes_.decode()

def int2bytes(int_):
    """Convert an integer into bytes representation"""
    return str(int_).encode()

def len2bytes(seq_):
    """A shortcut for int2bytes(len(seq_))"""
    return str(len(seq_)).encode()

def str2bytes(str_):
    """Convert a string into bytes"""
    return str_.encode()

##############################################################################
## Function quotify: Change ' to \', " to \" in string
##############################################################################
def quotify(inp):
    out = ''
    for c in inp:
        x = ord(c)
        if c == "'" or c == '"':
            out += '\\'
        out += c

    return out

##############################################################################
## Convert string to a typed value
##############################################################################
_VALUES = dict()                    ## Typed value dictionary
_VALUES['FALSE'] = False
_VALUES['TRUE']  = True
_VALUES['NONE']  = None

def str2value(S):
    """(Preliminary) Convert a string into a typed value."""
    S_upper = S.upper()
    if S_upper in _VALUES:
        return _VALUES[S_upper]

    try:
        V = int(S)
        return V
    except ValueError:
        pass

    try:
        V = float(S)
        return V
    except ValueError:
        pass

    return S

##############################################################################
## Tokenize string S, handling QUOTE characters
## If QUOTE characters are not an issue, use S.split()
##############################################################################
def tokenize(S):                    ## Tokenize string S into an array
    if S == None: return None       ## Handle no string

    if not isinstance(S, str):
        raise TypeError('Got %s, not str in: %s' % (type(S).__name__, S))
    L = len(S)                      ## The string length
    r = []                          ## Result, default empty array
    x = 0                           ## Current string index

    while x < L:                    ## Tokeninze the string
        ## x = skipBlank(S, x)
        while x < L and S[x] in string.whitespace:
            x += 1
        if x >= L: break            ## If line ends in whitespace

        T = ''                      ## Begin token
        while x < L:                ## Extract token
            C = S[x]                ## The next character
            if C in string.whitespace: break
            if C in QUOTES:         ## If begin quoted string
                Q = C               ## Save quote delimiter
                T += C              ## Add quote to token
                while x < L:
                    x += 1
                    if x >= L:
                        raise ValueError("Missing close quote in: '%s'"
                                        % quotify(S))
                    C = S[x]
                    T += C
                    ## print('Q(%s) X(%s) C(%s) T(%s)' % (Q, x, C, T))
                    if C == Q: break
                x += 1
                continue

            T += C
            x += 1

        if T[0] in QUOTES:
            if T[0] == T[-1]:
                T = T[1:-1]

        r.append(T)
        x += 1

    return r

##############################################################################
## Function _typeof: Display type and representation of an object
##  Note: Not part of __all__. Requires 'from lib.Utility import _typeof'
##############################################################################
def _typeof(obj):
    return '%s(%s)' % (type(obj), obj)

##############################################################################
## Remove backspaces from string, deleting the prior character
##############################################################################
def unbs(inp):
    out = ''
    for c in inp:
        if c == '\b':
            out = out[:-1]
        else:
            out += c

    return out

##############################################################################
## Function visify: Make string readable in message
##############################################################################
def visify(inp):
    out = ''
    for c in inp:
        x = ord(c)
        if x < 32 or x >= 127:
            if c == '\a':
                out += '\\a'
            elif c == '\b':
                out += '\\b'
            elif c == '\n':
                out += '\\n'
            elif c == '\r':
                out += '\\r'
            elif c == '\t':
                out += '\\t'
            elif ord(c) == ESC:     ## ESC is not a Python \\ character
                out += '\\E'
            else:
                out += '\\X%.2X' % x
        elif c == '\\':
            out += '\\\\'
        else:
            out += c

    return out

##############################################################################
## class Tokenizer: Tokenizer with quote handling
##
## Implementation note:
##  The Tokenizer class differs from the tokenizer function in that
##    1) a blank character must complete a Tokenizer token and
##    2) Tokenizer tokens cannot contain quotes
##
##############################################################################
class Tokenizer():
    def __init__(self, S):
        S = str(S)                  ## Force type 'string'
        L = len(S)
        while L > 0 and S[L-1] in string.whitespace:
            L -= 1                  ## Strip trailing white space

        self.S = S                  ## The string to tokenize
        self.L = L                  ## The string length
        self.X = 0                  ## The current string index

        self.skipWhite()            ## Skip initial white space

    def hasNextToken(self):         ## Does the string contain more tokens?
        return (self.X < self.L)

    def nextToken(self):            ## Return the next token
        L = self.L
        S = self.S
        X = self.X
        if X >= L: raise IndexError('nextToken non-existent')

        delim = string.whitespace  ## Default, white space delimiter
        C = S[X]
        if C in QUOTES:
            delim = C

        first = X                   ## First index in token
        X += 1
        while X < L and not S[X] in delim:
            X += 1

        if delim in QUOTES:
            X += 1
            if X > L: raise IndexError('missing close quote(%s)' % delim)
        final = X

        self.X = X
        self.skipWhite()
        return S[first:final]

    def remainder(self):            ## Return the remaining string
        return self.S[self.X:self.L]

    def skipWhite(self):            ## Skip over blanks
        L = self.L
        S = self.S
        X = self.X
        while X < L and S[X] in string.whitespace: X += 1
        self.X = X

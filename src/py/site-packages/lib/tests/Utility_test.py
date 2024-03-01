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
##       Utility_test.py
##
## Purpose-
##       Utility.py bringup test
##
## Last change date-
##       2019/09/04
##
##############################################################################
import string
import sys
import threading
import time

##############################################################################
## Required in Main.py. Has no effect here. (Modules are only imported once.)
## Global.TESTING = 'lib.Utility.ReminderFunctions'
##############################################################################

from lib.Command import command
from lib.Debug   import *
from lib.Utility import *

##############################################################################
## __Command class (The utility test command)
##############################################################################
def test_reminder_functions(test):
    _findWhite = test._findWhite
    _isWhite   = test._isWhite
    _join      = test._join
    _plurals   = test._plurals
    _split     = test._split
    _skipWhite = test._skipWhite

    print('_findWhite(" 1234567", 2): %s' % _findWhite(' 1234567', 2))
    assert _findWhite(" 1234567", 2) == -1
    print('_findWhite(" 123 567", 2): %s' % _findWhite(' 123 567', 2))
    assert _findWhite(" 123 567", 2) == 4
    print('_findWhite(" 1234567", 8): %s' % _findWhite(' 1234567', 8))
    assert _findWhite(" 1234567", 8) == -1
    print('_findWhite(" 123 567", 8): %s' % _findWhite(' 123 567', 8))
    assert _findWhite(" 123 567", 8) == -1
    print('_isWhite("N"): %s' % _isWhite("N"))
    assert _isWhite("N") == False
    print('_isWhite(" "): %s' % _isWhite(" "))
    assert _isWhite(" ") == True
    def test_plurals(N, expected):
        print('%s _plural%s' % (N, _plurals(N)))
        assert _plurals(N) == expected
    test_plurals(-1, 's')
    test_plurals( 0, 's')
    test_plurals( 1, '')
    test_plurals( 2, 's')
    S = 'alpha beta gamma'
    L = _split(S)
    print('_join(%s): "%s"' % (L, _join(L)))
    assert _join(L) == S            ## (Verifies _join/_split inversion)
    S = 'one two three'
    print('_split(%s): %s' % (S, _split(S))) ## (Verified above)
    print('_skipWhite("01   567", 2): %s' % _skipWhite("01   567", 2))
    assert _skipWhite("01   567", 2) == 5
    print('_skipWhite("01   567", 8): %s' % _skipWhite("01   567", 8))
    assert _skipWhite("01   567", 8) == -1

def test_ReminderFunctions():
    import lib.Utility as Utility
    if 'ReminderFunctions' in Utility.__dict__:
        test = Utility.ReminderFunctions
        test_reminder_functions(test)
        return True

    debugf('**** Unable to test lib.Utility.ReminderFunctions ****')
    debugf("Hack to avoid lib.Utility 'del ReminderFunctions' failed")

def test_tokenize(S):
    debugf('%s = test_tokenize(%s)' % (tokenize(S), S))

def test_tokenize_typeerror(S, text):
    try:
        debugf('%s = test_tokenize(%s)' % (tokenize(S), S))
        assert False, 'Expected TypeError not raised'
    except TypeError as e:
        debugf('Expected TypeError(%s) raised:' % text, e)

def test_tokenize_valueerror(S, text):
    try:
        debugf('%s = test_tokenize(%s)' % (tokenize(S), S))
        assert False, 'Expected ValueError not raised'
    except ValueError as e:
        debugf('Expected ValueError(%s) raised:' % text, e)

class __Command:
    @staticmethod
    def run(*args):
        debugf('lib.Utility self-test')

        debugf('Display tests:')
        debugf('To bytes:({})'.format(str2bytes('To bytes:')))
        debugf('To string:({})'.format(bytes2str(str2bytes('To bytes:'))))
        debugf('To length:({})'.format(len2bytes(str2bytes('To bytes:'))))
        debugf('22 bytes:({})'.format(int2bytes(22)))
        debugf('visify:({})'.format(visify("text '\\ \a\b\c\e\n\r\t'")))

        test_tokenize(' a  b c\t \t  d  ')
        test_tokenize("a b' 'c d\" \"e ")
        test_tokenize("a '' c \"\" e")
        test_tokenize("a \"\'\'\" c \'\"\"\' e")

        ############################## Error tests
        debugf('\nError tests:')
        test_tokenize_typeerror(4, 'int, not str')
        test_tokenize_valueerror("a b c 'd e", 'close quote')
        test_tokenize_valueerror('a b c "d e', 'close quote')
        test_tokenize_valueerror("U don't xx", 'close quote')

        ############################## Verification tests
        debugf('\nVerification tests:')
        tokenizer = Tokenizer(4)
        token = tokenizer.nextToken()
        assert token == '4' and not tokenizer.hasNextToken(), 'Tokenizer(4)'

        tokenizer = Tokenizer('  a  b c\t \t  d  ')
        assert tokenizer.hasNextToken(), 'Tokenizer("  a  b c\t \t  d  ") 1st'
        assert tokenizer.nextToken() == 'a', 'Tokenizer("  a  b c  d  ")'
        assert tokenizer.nextToken() == 'b', 'Tokenizer("  a  b c  d  ")'
        assert tokenizer.nextToken() == 'c', 'Tokenizer("  a  b c  d  ")'
        assert tokenizer.nextToken() == 'd', 'Tokenizer("  a  b c  d  ")'
        assert not tokenizer.hasNextToken(), 'Tokenizer("  a  b c  d  ")'

        tokenizer = Tokenizer(' "a b" \'c d\' e ')
        assert tokenizer.nextToken() == '"a b"', 'Tokenizer(" "a b" \'c d\' e ")'
        assert tokenizer.nextToken() == "'c d'", 'Tokenizer(" "a b" \'c d\' e ")'
        assert tokenizer.nextToken() == 'e', 'Tokenizer(" "a b" \'c d\' e ")'
        assert not tokenizer.hasNextToken(), 'Tokenizer(" "a b" \'c d\' e ")'

        if sys.version_info[0] < 3:
            debugf('python version >= 3 required for additional verification')
        else:
            assert bytes2str(b'these were bytes') == 'these were bytes'
            assert int2bytes(732) == b'732'
            assert len2bytes(b'sample bytes') == b'12'
            assert len2bytes('sample string') == b'13'
            assert str2bytes('these were stringy') == b'these were stringy'
            assert visify('\\ \a\b\c\e\n\r\t') == '\\\\ \\a\\b\\\\c\\\\e\\n\\r\\t'

        assert test_ReminderFunctions(), 'Cannot test ReminderFunctions'

        debugf('lib.Utility self-test completed')
        return 0

command['utility'] = __Command


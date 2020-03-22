##############################################################################
##
##       Copyright (C) 2019 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Collections_test.py
##
## Purpose-
##       Collection regression tests.
##
## Last change date-
##       2019/08/27
##
##############################################################################
import sys
import traceback

#### lib #####################################################################
from lib.Collections import *
from lib.Command     import *
from lib.Debug       import *
from lib.Utility     import *

##############################################################################
## Expected/missed exception handlers
##############################################################################
def handle(X):                      ## Handle expected exception
    S = type(X).__name__
    debugf('OK, expected %s:' % S, X)

def isabug(S):                      ## Handle error
    debugf('NG, %s' % S)
    return 1

def missed(X):                      ## Handle missed exception
    debugf('NG, missed exception:', X)
    return 1

##############################################################################
##
## Class-
##       Test_Node
##
## Purpose-
##       Implement command['Node]; Test lib.Collections.Node
##       Implement command['Misuse:Node]; Test lib.Collections.Node
##
## Implementation notes-
##       Misuse tests are implementation dependent.
##       These tests should be updated to match the current implementation.
##
##############################################################################
class MyNode(Node):                 ## Node subclass (Easy to misuse!)
    _serial = 0

    def __init__(self, parent):
        self.serial = '%.4d' % MyNode._serial
        MyNode._serial += 1
        super().__init__(parent)

    def __repr__(self):
        return '<%s>' % self.serial

    def __eq__(self, that):
        if isinstance(that, MyNode):
            return self.serial == that.serial

        if isinstance(that, str):
            return self.serial == that
        return False

    def __hash__(self):
        return self.serial.__hash__()

class Test_Node():
    @staticmethod
    def run(*args):
        debugf('Test_Node.run')
        debugf('** All tests self-verified **')
        error_count = 0

        ## Construction tests ################################################
        debugf('\nTest construction')
        root = MyNode(None)
        n001 = MyNode(root)
        n002 = MyNode(root)
        n003 = MyNode(root)
        n004 = MyNode(root)
        n005 = MyNode(n001)
        n006 = MyNode(n001)
        n007 = MyNode(n001)

        assert len(root) == 4
        assert root not in root; assert root not in n001

        assert n001 in root; assert n001 not in n001
        assert n002 in root; assert n002 not in n001
        assert n003 in root; assert n003 not in n001
        assert n004 in root; assert n004 not in n001

        assert len(n001) == 3
        assert n005 in n001; assert n005 not in root
        assert n006 in n001; assert n006 not in root
        assert n007 in n001; assert n007 not in root

        debugf('check:'); root.check()
        debugf('debug:'); root.debug()

        ## Method tests ######################################################
        debugf('\nTest Methods (permanantly removing n003)')
        assert len(root) == 4       ## (Internal state verification)
        assert len(n001) == 3       ## (Internal state verification)

        root.remove(n003)           ## (permanently removed)
        assert n003.parent is None
        assert n003 not in root
        assert len(root) == 3

        n001.remove(n006)
        assert n006.parent is None
        assert n006 not in n001
        assert len(n001) == 2

        try:
            n001.remove(n006)
            error_count += missed('KeyError') ## (Not present)
        except KeyError as X:
            handle(X)

        n001.add(n006)              ## Restore n006 in n001
        try:
            del n001[n006]
            error_count += missed('TypeError') ## Not subscriptable
        except TypeError as X:
            handle(X)

        ## Insert exception test #############################################
        try:
            debugf(root[n003])      ## Set is not subscriptable (__getitem__)
            error_count += missed('TypeError')
        except TypeError as X:
            handle(X)

        try:
            n001[n007] = n007       ## Set is not subscriptable (__setitem__)
            error_count += missed('TypeError')
        except TypeError as X:
            handle(X)

        try:
            n001.add(n004)          ## RuntimeError (On some list)
            error_count += missed('RuntimeError')
        except RuntimeError as X:
            handle(X)

        try:
            n001.add(n006)          ## RuntimeError (On n001 list)
            error_count += missed('RuntimeError')
        except RuntimeError as X:
            handle(X)

        try:
            debugf(root[n003.serial]) ## Set is not subscriptable
            error_count += missed('TypeError')
        except TypeError as X:
            handle(X)

        try:
            root.add('text')       ## Can't add strings to Nodes
            error_count += missed('TypeError')
        except TypeError as X:
            handle(X)

        ## Verify expected final state #######################################
        assert len(root) == 3
        assert root not in root; assert root not in n001

        assert n001 in root; assert n001 not in n001
        assert n002 in root; assert n002 not in n001
        assert n003 not in root; assert n003 not in n001 ## (Removed)
        assert n004 in root; assert n004 not in n001

        assert len(n001) == 3
        assert n005 in n001; assert n005 not in root
        assert n006 in n001; assert n006 not in root
        assert n007 in n001; assert n007 not in root

        for child in root:
            assert child in [n001, n002, n004]
            assert child.parent is root

        for child in n001:
            assert child in [n005, n006, n007]
            assert child.parent is n001
        debugf('check:'); root.check()
        debugf('debug:'); root.debug()
        debugf('lib.Collections.Node self-test completed')

        return error_count
command['Node'] = Test_Node

class Test_Misused_Node(): ########### DOES NOT duplicate Test_Node tests
    @staticmethod
    def run(*args):
        debugf('Test_Misused_Node.run')
        debugf('** All tests self-verified **')
        error_count = 0

        ## Construction tests ################################################
        root = MyNode(None)
        n001 = MyNode(root)
        n002 = MyNode(root)
        n003 = MyNode(None)         ## (Not used here)
        n004 = MyNode(root)
        n005 = MyNode(n001)
        n006 = MyNode(n001)
        n007 = MyNode(n001)

        d006 = MyNode(None)         ## Duplicate key
        d006.serial = n006.serial

        p006 = MyNode(None)         ## Has n001 parent, but isn't on list
        p006.parent = n001

        x006 = MyNode(None)         ## Looks like n006 but isn't
        x006.parent = n001
        x006.serial = n006.serial

        ## Test the imposter nodes
        assert d006 == n006         ## (Only has the same serial)
        assert p006 != n006         ## (Only has the same parent)
        assert x006 == n006         ## (Has both serial and parent)

        assert d006 is not n006     ## (These are not the same objects)
        assert p006 is not n006
        assert x006 is not n006

        if d006 in n001: debugf('OK, because d006.parent is not n001')
        if p006 in n001: debugf('NG, d006.parent is n001'); error_count += 1
        if x006 in n001: debugf('NG, x006.parent is n001'); error_count += 1

        ## Method tests (improper usage) #####################################
        try:
            debugf(d006.serial in n001)
            error_count += missed('TypeError')
        except AssertionError as X:
            error_count += isabug('Must use Node in Node, not str in Node')

        try:                        ## This fails more or less properly
            n001.remove(x006.serial)
            errorCount += isabug('UNCHECKED: Node.remove(str)')
        except AttributeError as X:
            handle(X)
            error_count += isabug('Unexpected exception type')
        if n006.parent is n001: debugf('OK, sort of, no Node damage')

        ## USER ERROR test ###################################################
        n005.serial = 'OOPS';       ## Mess with n005.__hash__()
        debugf('n005 DAMAGE INSERTED')
        debugf('n001.parent:', n001.parent, 'n001._child:', n001._child)
        assert n005 in n001         ## True because n005.parent is still n001

        try:
            debugf('check:'); n001.check() ## This DOES fail, as it should
            error_count += isabug('IMPLEMENTATION CHANGED')
        except AssertionError:
            error_count += isabug('n001.check() (properly) fails')

        ## n005 is in a bad state ############################################
        debugf('problem demo:')
        debugf('n005.parent:', n005.parent, 'n001._child:', n001._child)

        try:
            n001.remove(n005)           ## We try to remove it from the list
            error_count += isabug('IMPLEMENTATION CHANGED')
        except KeyError as X:
            handle(X)
            error_count += isabug('n001.remove(n005) fails')
        if n005.parent is None: isabug('n001 list is inconsistent')
        debugf('n005.parent:', n005.parent, 'n001._child:', n001._child)
        n005.parent = n001 ## (Repair)

        debugf('n005.parent:', n005.parent, 'n001._child:', n001._child)
        n001._child.discard(n005)   ## Try direct removal from _child
        n001._child.discard('0005')
        n001._child.discard('OOPS')

        assert n005 not in n001._child ## Where is it?
        assert '0005' not in n001._child
        assert 'OOPS' not in n001._child
        debugf('n005.parent:', n005.parent, 'n001._child:', n001._child)

        ############################## It can be fixed!
        n005.serial = '0005'        ## (Restores the __hash__ result)
        debugf('n005 REPAIRED')
        debugf('n001.remove(n005)'); n001.remove(n005)
        debugf('n005.parent:', n005.parent, 'n001._child:', n001._child)

        debugf('check:'); root.check()
        debugf('debug:'); root.debug()

        debugf('**ERRORS EXPECTED**\nMisuse counter:', error_count)
        debugf('lib.Collections.Node misuse self-test completed')
        return error_count
command['Misuse:Node'] = Test_Misused_Node

##############################################################################
##
## Class-
##       Test_UserSet
##
## Purpose-
##       Implement command['UserSet]; Minimally test lib.Collections.UserSet
##
##############################################################################
class TestSet(UserSet):             ## UserSet subclass
    def __init__(self, name=None):
        super().__init__()
        self.name = name

    def debug(self):
        debugf('%s %s' % (self.name, self.data))
        for elem in self:
            if isinstance(elem, TestSet): elem.debug()

class Test_UserSet():
    @staticmethod
    def run(*args):
        print('Test_UserSet.run')
        error_count = 0

        ## Construction test
        root = TestSet(name='root')
        n001 = TestSet(name='n001')
        n002 = 'n002'
        n003 = 'n003'
        n004 = 'n004'
        n005 = 'n005'
        n006 = 'n006'
        n007 = 'n007'
        root.add(n001)
        root.add(n002)
        root.add(n003)
        root.add(n004)

        n001.add(n005)
        n001.add(n006)
        n001.add(n007)
        root.debug()
        if True:
            print('toor', root)
            print('n001', n001)

        assert n001 in root
        assert n005 not in root
        assert n005 in n001
        debugf('lib.Collections.UserSet self-test completed')
        return error_count

command['UserSet'] = Test_UserSet

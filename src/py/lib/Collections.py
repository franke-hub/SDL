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
##       Collections.py
##
## Purpose-
##       Library collection classes.
##
## Last change date-
##       2019/08/27
##
## Implemented classes-
##       Inspect
##       Node
##       UserSet
##
##############################################################################
import sys
import inspect

#### lib #####################################################################
from lib.Debug       import *
from lib.Utility     import *

##############################################################################
## Exports
##############################################################################
__all__  = []                       ## (Classes add themselves as defined)

##############################################################################
## Controls
##############################################################################
_HCDM = False                       ## Hard Core Debug Mode?

##############################################################################
##
## Class-
##       Inspect
##
## Purpose-
##       Object inspection utilities.
##
##############################################################################
class Inspect:
    @staticmethod
    def to_tree(name, obj, parent=None, oids=None): ## Return Node tree
        if oids is None: oids = set()
        if parent is None:
            parent = Node(None)
            parent.parent = parent
            parent.name = None
            parent.obj  = None
            parent.oid  = None

        node = Node(parent)         ## Add a child Node
        node.name = name
        node.obj  = obj
        oid = id(obj)
        node.oid  = oid
        if oid in oids:
            if _HCDM: writef('<%10s %s:%s>' % ('__DUP__', name, obj))
            node.obj = None
            return None
        oids.add(oid)

        to_tree = Inspect.to_tree
        _name = name
        _attr = obj
        _type = type(obj).__name__
        if _HCDM: writef('<%10s %s:%s>' % (_type, name, obj))

        ######################################################################
        ## Traverse collection types
        if _type == 'dict':
            if _HCDM: writef('<<dict %s>>' % name)
            for key in obj:
                _name = '%s[%s]' % (name, key)
                _attr = obj[key]
                to_tree(_name, _attr, node, oids)
            assert not hasattr(obj, '__dict__'), _type
            assert not hasattr(obj, '__slots__'), _type

        if _type == 'list':
            if _HCDM: writef('<<list %s>>' % name)
            for i in range(len(obj)):
                _name = '%s[%3d]' % (name, i)
                _attr = obj[i]
                to_tree(_name, _attr, node, oids)
            assert not hasattr(obj, '__dict__'), _type
            assert not hasattr(obj, '__slots__'), _type

        if _type == 'set':
            if _HCDM: writef('<<set %s>>' % name)
            for item in obj:
                _name = '%s(%s)' % (name, item)
                _attr = item
                to_tree(_name, _attr, node, oids)
            assert not hasattr(obj, '__dict__'), _type
            assert not hasattr(obj, '__slots__'), _type

        ######################################################################
        ## Traverse sub-objects
        if hasattr(obj, '__dict__'):
            if _HCDM: writef('<__dict__ <%s %s>>' % (_type, name))
            for attr in obj.__dict__:
                _name = name + '.' + attr
                _attr = obj.__dict__[attr]
                to_tree(_name, _attr, node, oids)

        if hasattr(obj, '__slots__'):
            if _HCDM: writef('<__slots__ <%s %s>>' % (_type, name))
            for attr in obj.__slots__:
                _name = name + '.' + attr
                _attr = getattr(obj, attr)
                to_tree(_name, _attr, node, oids)

        ######################################################################
        ## Diagnostics
        if _HCDM:                   ## Display leaf objects?
            if _type in [ 'dict', 'list', 'set' ]:
                pass
            elif hasattr(obj, '__dict__') or hasattr(obj, '__slots__'):
                pass
            else:
                if _HCDM: writef('<%10s %s:%s> LEAF OBJECT' % (_type, name, obj))
##              members = getmembers(obj)
##              for member in members:
##                  writef(".%16s = '%s'" % member)

        ######################################################################
        ## Return the current Node
        return node

    ##########################################################################
    ## size_visitor, adds:
    ##     ._size = sys.getsizeof(node.obj)
    ##     ._kids = 0 + (child._size + child._kids) for child in node
    def size_visitor(root, node):
        _size = 0
        if node.obj is not None:
            _size = sys.getsizeof(node.obj)
        node._size = _size

        _kids = 0
        for child in node:          ## (All children already visited)
            _kids += child._size
            _kids += child._kids
        node._kids = _kids
__all__ += ['Inspect']

##############################################################################
##
## Class-
##       Node
##
## Purpose-
##       Node in a Tree
##
## Implementation notes-
##       Node only interoperate with other Nodes.
##
##       It is easy to misuse Nodes that:
##           override the __eq__ and/or __hash__ methods
##           modify the parent or _child attributes
##
##       discard() NOT IMPLEMENTED, NOT WANTED
##           remove()'s key checking is a feature, not a bug.
##
##############################################################################
class Node(object):
    def __init__(self, parent):
    ##  print('Node(%s).__init__(%s)' % (self, parent))
        assert isinstance(parent, Node) or parent is None
        self.parent = parent
        self._child = set()
        if parent is not None:
            parent._child.add(self)

    def __contains__(self, child):  ## Uses shortcut
        assert isinstance(child, Node)
        return child.parent is self

    def __delitem__(self, child):
        raise TypeError('Disallowed: use remove')

    def __iter__(self):
        return self._child.__iter__()

    def __getitem__(self, key):
        return self._child[key]     ## (Always raises TypeError)

    def __len__(self):
        return len(self._child)

    def __setitem__(self, child, value): ## Only valid use: self[child] = child
        raise TypeError('Disallowed: use add')

    def check(self):                ## Check parent and subtree
        assert self.parent is None or self.parent._child.__contains__(self)
        for child in self:
            assert child.parent is self
            child.check()

    def debug(self):                ## Diagnostic subtree display
        debugf('[%s] %s.debug %s' % (self.parent, self, self._child))
        for child in self:
            child.debug()

    def add(self, child):
        if not isinstance(child, Node):
            raise TypeError('<%s %s> not Node' % (type(child), child))

        if child.parent is not None:
            raise RuntimeError('has parent')

        self._child.add(child)
        child.parent = self

    def remove(self, child):
        if child.parent is not self: raise KeyError(child)
        self._child.remove(child)
        child.parent = None

    def visit(self, visitor, root=None): ## Depth first visitor
        for child in self:
            child.visit(visitor, root)
        return visitor(root, self)

    @staticmethod
    ## Usage: node.visit(Node.visitor [, root=object])
    def visitor(root, node):        ## SAMPLE visitor
        pass                        ## (Examine node, depth first)


__all__ += ['Node']

##############################################################################
##
## Class-
##       UserSet
##
## Purpose-
##       UserSet, like UserList except for set() instead of list()
##
## This is a preliminary and incomplete implementation-
##       Many set methods are not exposed.
##       This class is *UNUSED*, and only minimally tested.
##
##############################################################################
class UserSet(object):
    def __init__(self):
        self.data = set()

    def __contains__(self, elem):
        return self.data.__contains__(elem)

    def __delitem__(self, elem):
        return self.data.__delitem__(elem)

    def __iter__(self):
        return self.data.__iter__()

    def __getitem__(self, elem):
        return self.data.__getitem__(elem)

    def __len__(self):
        return self.data.__len__()

    def __repr__(self):
        return self.data.__repr__()

    def __setitem__(self, elem, value):
        return self.data.__setitem__(elem, value)

    def add(self, elem):
        return self.data.add(elem)

    def clear(self):
        return self.data.clear()

    def discard(self, elem):
        return self.data.discard(elem)

    def pop(self):
        return self.data.pop()

    def remove(self, elem):
        return self.data.remove(elem)
__all__ += ['UserSet']

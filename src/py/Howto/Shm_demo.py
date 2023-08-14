#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2021 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Shm_demo.py
##
## Purpose-
##       Shared memory demo
##
## Last change date-
##       2021/04/01
##
## Usage-
##       ./Shm_demo.py
##
##############################################################################
import atexit
from multiprocessing import shared_memory
import random as Random
import time

from lib.Command import command
from lib.Debug import *

_SEED = 3141927
_SIZE = 32768
_TEST = 'shm'

def _debugf(*args, **kwargs):
    printf(*args, **kwargs)
    writef(*args, **kwargs)

class _Demo:
    global _TEST

    def __init__(self):
        self._shm = {}
        self._cmd = {}
        self._cmd['del'] =  self._old
        self._cmd['new'] =  self._new
        self._cmd['ver'] =  self._ver
        self._cmd['list'] = self._list
        atexit.register(self.__term__)

    def __term__(self):             ## (atexit handler)
        old = dict(self._shm)       ## (Avoid changing loop object)
        for name in old:
            _debugf('shm.atexit auto-delete(%s)' % (name))
            self._old((_TEST, 'del', name))

    def run(self, argv):
        ## _debugf('%s%s %s' % (_TEST, argv, len(argv)))

        try:
            op = self.help
            if len(argv) < 2:
                op = self.main
            elif argv[1] == 'list' or len(argv) > 2:
                op = self._cmd[argv[1]]
        except Exception as X:
            _debugf('Exception:', X)
            pass

        op(argv)

    def _old(self, argv):
        name = argv[2]
        try:
            shm = self._shm[name]
            shm.close()
            shm.unlink()
            del self._shm[name]
        except KeyError:
            _debugf('shm[%s] nonexistent' % (name))

    def _new(self, argv):
        name = argv[2]
        try:
            shm = self._shm[name]
            _debugf('shm[%s] already exists' % (name))
        except KeyError:
            shm = shared_memory.SharedMemory(name=name, create=True \
                                            , size= _SIZE)
            self._shm[name] = shm
            self._load(shm)

    def _ver(self, argv):
        name = argv[2]
        try:
            shm = self._shm[name]
            _debugf('ver:', self._test(shm))
        except KeyError:
            _debugf('shm[%s] nonexistent' % (name))

    def _list(self, argv):
        _debugf(len(self._shm), 'Shared memory tables')
        count = 0
        for name in self._shm:
            _debugf('..[%3s] "%s"' % (count, name))
            count += 1

    def _load(self, shm):
        Random.seed(a=_SEED)
        buf = shm.buf
        for x in range(len(buf)):
            v = Random.randint(0, 255)
            buf[x] = v

    def _test(self, shm):
        Random.seed(a=_SEED)
        buf = shm.buf
        for x in range(len(buf)):
            want = Random.randint(0, 255)
            if buf[x] != want:
                _debugf('[%3s](%3s), not(%3s)' % (x, buf[x], want))
                return 'NG'
        return('OK');

    def help(self, argv):
        _debugf(_TEST, '(command, arguments)')
        _debugf('  help\t\tThis help message')
        _debugf('  list\t\tList shm tables')
        _debugf("  new 'name'\tCreate and initialize 'name'")
        _debugf("  del 'name'\tDelete 'name'")
        _debugf("  ver 'name'\tVerify 'name' content")

    def main(self, argv):
        _debugf(__file__)
        self.run((_TEST, 'new', 'blob'))
        self.run((_TEST, 'ver', 'blob'))
        self.run((_TEST, 'ver', 'blob')) ## (Duplicate verify OK)
        self.run((_TEST, 'new', 'this')) ## (For atexit test)
        self.run((_TEST, 'new', 'that')) ## (For atexit test)
        self.run((_TEST, 'list'))        ## (blob, this, that)
        self.run((_TEST, 'del', 'blob'))
        _debugf('\nError tests')
        self.run((_TEST, 'ver', 'blob')) ## non-existent
        self.run((_TEST, 'ver', 'blot')) ## non-existent (never existed)
        self.run((_TEST, 'new', 'that')) ## duplicate

command[_TEST] = _Demo()

##############################################################################
## Standalone test
##############################################################################
if __name__ == '__main__':
    command[_TEST].run((_TEST,))

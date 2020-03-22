#!/usr/bin/python
##############################################################################
##
##       Copyright (C) 2016 Frank Eskesen.
##
##       This file is free content, distributed under the "un-license,"
##       explicitly released into the Public Domain.
##       (See accompanying file LICENSE.UNLICENSE or the original
##       contained within http://unlicense.org)
##
##############################################################################
##
## Title-
##       hello.py
##
## Purpose-
##       Python demonstration program.
##
## Last change date-
##       2016/01/01
##
##############################################################################
from distutils import sysconfig

print('Hello, python world!')
print('Python version: %s' % sysconfig.get_config_var('VERSION'))

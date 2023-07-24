#!/usr/bin/python
##############################################################################
##
##       Copyright (C) 2016 Frank Eskesen.
##
##       This file is free content, distributed under creative commons CC0,
##       explicitly released into the Public Domain.
##       (See accompanying html file LICENSE.ZERO or the original contained
##       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
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

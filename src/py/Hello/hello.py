#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2016-2023 Frank Eskesen.
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
##       2023/08/13
##
##############################################################################
from distutils import sysconfig

##############################################################################
## Mainline code
##############################################################################
if __name__ == "__main__":
    print('Hello, python world!')
    print('Python version: %s' % sysconfig.get_config_var('VERSION'))

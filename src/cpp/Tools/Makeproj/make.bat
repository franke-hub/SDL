@REM #########################################################################
@REM
@REM     Copyright (c) 2006-2017 Frank Eskesen.
@REM
@REM     This file is free content, distributed under the "un-license,"
@REM     explicitly released into the Public Domain.
@REM     (See accompanying file LICENSE.UNLICENSE or the original
@REM     contained within http://unlicense.org)
@REM
@REM #########################################################################
@REM
@REM Title-
@REM     make
@REM
@REM Function-
@REM     Local alias for make
@REM
@REM Last change date-
@REM     2017/01/01
@REM
@REM #########################################################################
@echo on
cl -D_OS_WIN -I\data\src\cpp\inc Makeproj.cpp

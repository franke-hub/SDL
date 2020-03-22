//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       unistd.h
//
// Purpose-
//       Simulation of BSD unistd.h file for Windows.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef UNISTD_H_INCLUDED
#define UNISTD_H_INCLUDED

#ifdef _OS_WIN
#include <direct.h>
#include <io.h>
#include <process.h>

#else
#include </usr/include/unistd.h>
#endif

#endif

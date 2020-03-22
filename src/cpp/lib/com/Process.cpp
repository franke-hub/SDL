//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Process.cpp
//
// Purpose-
//       Process object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#if   defined(_OS_WIN)
#include "OS/WIN/Process.cpp"

#elif defined(_OS_BSD)
#include "OS/BSD/Process.cpp"

#else
#error "Invalid OS"
#endif


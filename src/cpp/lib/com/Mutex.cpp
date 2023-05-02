//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Mutex.cpp
//
// Purpose-
//       Mutex object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#if   defined(_OS_WIN)
#include "OS/WIN/Mutex.cpp"

#elif defined(_OS_BSD)
#include "OS/BSD/Mutex.cpp"

#else
#error "Invalid OS"
#endif


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
//       nativeio.h
//
// Purpose-
//       Version includes for Native I/O functions: open, close, read, write
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NATIVEIO_H_INCLUDED
#define NATIVEIO_H_INCLUDED

#if defined(_OS_WIN)
  #include <direct.h>
  #include <io.h>
#endif

#include <fcntl.h>
#include <unistd.h>

#ifndef O_BINARY
  #define O_BINARY 0
#endif

#ifndef O_RSHARE
  #define O_RSHARE 0
#endif

#endif // NATIVEIO_H_INCLUDED

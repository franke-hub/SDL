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
//       ocrw.h
//
// Purpose-
//       Includes for Native I/O functions: open, close, read, write
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef OCRW_H_INCLUDED
#define OCRW_H_INCLUDED

#include <stdint.h>

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

#if defined(_OS_WIN)
  typedef int64_t off64_t;          // Windows supports 64 bit offsets
  #define open64  open              // Windows uses open, not open64

  #define S_IRUSR S_IREAD
  #define S_IWUSR S_IWRITE
  #define S_IXUSR S_IEXEC
#else
  #if defined(_OS_CYGWIN)
    #define off64_t off_t           // Cygwin does not support stat64
    #define open64  open            // Cygwin does not support open64
  #endif
#endif

#endif // OCRW_H_INCLUDED

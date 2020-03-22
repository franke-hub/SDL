//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       debugging.cpp
//
// Purpose-
//       Implement debugging namespace methods not in Debug.cpp.
//
// Last change date-
//       2020/01/24
//
//----------------------------------------------------------------------------
#include <stdarg.h>                 // For va_list, ...
#include <stdio.h>                  // For perror, vsnprintf

#include "pub/Debug.h"              // (Defines debugging::errorp)

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Namespace-
//       debugging
//
// Purpose-
//       Implement Debug.h debugging functions not in Debug.cpp.
//
//----------------------------------------------------------------------------
namespace debugging {
//----------------------------------------------------------------------------
//
// Subroutine-
//       debugging::errorp
//
// Purpose-
//       Wrap perror message.
//
//----------------------------------------------------------------------------
void
   errorp(                          // Write message to stderr
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   fflush(NULL);                    // Flush everything first

   char buffer[4096];               // Format buffer
   va_start(argptr, fmt);           // Initialize va_ functions
   vsnprintf(buffer, sizeof(buffer), fmt, argptr); // Format the message
   va_end(argptr);                  // Close va_ functions

   perror(buffer);                  // Write the message in one line
   fflush(stderr);                  // Flush stderr
}
}  // namespace debugging
}  // namespace _PUB_NAMESPACE

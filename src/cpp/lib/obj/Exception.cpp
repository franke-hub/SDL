//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Exception.cpp
//
// Purpose-
//       Exception static attributes.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <unistd.h>
#ifdef _OS_WIN
  #define isatty _isatty
#endif

#include "obj/Object.h"

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
// The default Exception::string. Only used if Exception::string fails.
std::string            Exception::default_exception_string("Exception");

//----------------------------------------------------------------------------
//
// Method-
//       Exception::abort
//
// Purpose-
//       Abort with error message.
//
//----------------------------------------------------------------------------
int                                 // Normally no return, otherwise true
   Exception::abort(                // Write to trace and stderr, throw exception
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
{
   va_list             argptr;      // Argument list pointer

   if( stdout != stderr && (!isatty(fileno(stdout)) || !isatty(fileno(stderr))))
     printf("\nException::abort\n");

   fprintf(stderr, "\nException::abort\n");
   va_start(argptr, fmt);           // Initialize va_ functions
     vfprintf(stderr, fmt, argptr); // Write to stderr
   va_end(argptr);                  // Close va_ functions
   fprintf(stderr, "\n");

   fflush(stdout);
   fflush(stderr);

   throw std::runtime_error("Exception::abort"); // Should not occur
   return true;
}
} // namespace _OBJ_NAMESPACE


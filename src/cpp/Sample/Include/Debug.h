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
//       Debug.h
//
// Purpose-
//       STANDALONE MINIMAL VERSION of com/Debug.h
//
// Last change date-
//       2018/01/01
//
// Implemntation notes-
//       Trace file "debug.log" is append-only.
//
//----------------------------------------------------------------------------
#ifndef DEBUG_H_INCLUDED            // Conflicts with com/Debug.h
#define DEBUG_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>

#ifdef __GNUC__
static inline void
   vtracef(                         // Write to trace
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   __attribute__ ((format (printf, 1, 0)));

static inline void
   vdebugf(                         // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   __attribute__ ((format (printf, 1, 0)));

static inline void
   tracef(                          // Write to trace
     const char*       fmt,         // The PRINTF format string
                       ...)         // Additional arguments
   __attribute__ ((format (printf, 1, 2)));

static inline void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // Additional arguments
   __attribute__ ((format (printf, 1, 2)));
#endif

static inline void
   vtracef(                         // Write to trace
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   FILE* file= fopen("debug.log", "a+");
   vfprintf(file, fmt, argptr);
   fclose(file);
}

static inline void
   vdebugf(                         // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   va_list outptr;
   va_copy(outptr, argptr);
   vtracef(fmt, outptr);
   va_end(outptr);

   vprintf(fmt, outptr);
   fflush(stdout);
}

static inline void
   tracef(                          // Write to trace
     const char*       fmt,         // The PRINTF format string
                       ...)         // Additional arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vtracef(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

static inline void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // Additional arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vdebugf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

#endif // DEBUG_H_INCLUDED

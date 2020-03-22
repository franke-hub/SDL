//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       logger.h
//
// Purpose-
//       Logging facility.
//
// Last change date-
//       2020/01/29
//
// Implementation notes-
//       syslog() needs a syslog server in order to handle messages.
//
//----------------------------------------------------------------------------
#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#if !defined(_OS_WIN) // && false   // Uncomment "&& false" to print messages
#include <syslog.h>

#else
#include <stdarg.h>
#include <stdio.h>
#define LOG_CONS 0
#define LOG_ERR  0
#define LOG_PID  0
#define LOG_INFO 0
#define LOG_USER 0

inline void
   syslog(int opt, const char* fmt, ...)
{
   va_list           argptr;        // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vprintf(fmt, argptr);            // Perform error trace
   va_end(argptr);                  // Close va_ functions
}

inline void openlog(const char* fmt, int options, int source) {}
inline void closelog(void)       {}
inline int  setlogmask(int mask) {return mask;}
#endif

#endif // LOGGER_H_INCLUDED

//----------------------------------------------------------------------------
//
//       Copyright (C) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       System.h
//
// Purpose-
//       Define the System namespace
//
// Last change date-
//       2026/03/08
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_SYSTEM_H_INCLUDED
#define _LIBPUB_SYSTEM_H_INCLUDED

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Namespace-
//       pub::System
//
// Purpose-
//       System PUB library controls
//
// Implementation notes-
//       The system log file name is "$USER/.local/log/system.log".
//       All log messages are recorded there, regardless of level.
//
//----------------------------------------------------------------------------
namespace System {                  // The System namespace
//----------------------------------------------------------------------------
// System::Typedefs and enumerations
//----------------------------------------------------------------------------
typedef unsigned       log_level_t; // The log_level type

enum LOG_LEVEL                      // Console logging level
{  LL_NONE= 0                       // No console logging.
,  LL_INFO                          // Log Informational events
,  LL_ERROR                         // Log Error events
,  LL_HCDM                          // Log Hard Core Debug Mode events
,  LL_ALL                           // Log all events
,  LL_DEFAULT= LL_ERROR             // The default stderr logging level
};

//----------------------------------------------------------------------------
// System::log_level
//
// The log_level value is the minimum value required to cause a log message
// to be written to the console as well as the system log.
//
// Note: set_log_level(LL_NONE) is converted to set_log_level(LL_INFO).
// System::log(LL_NONE, <message>) never writes to the console.
//
//----------------------------------------------------------------------------
// log_level_t         log_level;   // The current log level

//----------------------------------------------------------------------------
// System::Accessor methods
//----------------------------------------------------------------------------
extern log_level_t                  // The current log_level
   get_log_level( void );           // Get current log_level

extern void
   set_log_level(                   // Set current log_level
     log_level_t      level);       // To this level

//----------------------------------------------------------------------------
//
// Method-
//       debug
//
// Purpose-
//       Debugging display
//
// Implementation notes-
//       Use this method to display pub library static debugging information.
//       This method writes the message to stdout and the current Debug trace
//       file, but does not write to the system log file.
//
//----------------------------------------------------------------------------
extern void
   debug(                           // System debugging display
     const char*      info="",      // Caller information
     bool             detail= false); // Add detailed information?

//----------------------------------------------------------------------------
//
// Method-
//       System::Hardware accessor methods
//
// Purpose-
//       Get hardware information
//
//       System::get_LR   Get Link Register
//       System::get_SP   Get Stack Pointer
//       System::get_TSC  Get Time Stamp Counter
//
// Implementation notes-
//       These methods are simulated unless using the GNU compiler and
//       (for get_TSC) X86 hardware. If unsupported:
//           System::get_LR   returns nullptr
//           System::get_SP   returns nullptr
//           System::get_TSC  (atomically) returns its invocation count
//
//       get_TSC returns a 64 bit value even on 32 bit hardware.
//
//----------------------------------------------------------------------------
extern void*                        // The Link Register
   get_LR( void );                  // Get Link Register

extern void*                        // The Stack Pointer
   get_SP( void );                  // Get Stack Pointer

extern uint64_t                     // The Time Stamp Counter
   get_TSC( void );                 // Get Time Stamp Counter

//----------------------------------------------------------------------------
//
// Method-
//       System::log
//
// Purpose-
//       Record an event in the system log.
//
// Implementation notes-
//       If the current log_level is >= `level`, the message is also recorded
//       in stdout.
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 3)
extern void
   log(                             // Record a system event
     log_level_t       level,       // The message event level
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list
}; // namespace System
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SYSTEM_H_INCLUDED

//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       RdCommon.h
//
// Purpose-
//       Common objects and subroutines.
//
// Last change date-
//       2026/07/05
//
//----------------------------------------------------------------------------
#ifndef RDCOMMON_H_INCLUDED
#define RDCOMMON_H_INCLUDED

#include <string>                   // For std::string

#include <cstdio>                   // For FILE*
#include <cstdlib>                  // For size_t

#include <limits.h>                 // For NAME_MAX, PATH_MAX

#include "pub/config.h"             // For ATTRIB_PRINTF macros
#include <pub/Data.h>               // For namespace pub::data
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/IO.h>                 // For namespace pub::io
#include <pub/Signals.h>            // For pub::signals::Signal
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For (pub::utility::) methods
#include <pub/utility.i>            // For (pub::) utility functions

//----------------------------------------------------------------------------
// Global compile-time controls
//----------------------------------------------------------------------------
enum                                // Compile-time controls
{  BRINGUP_MODE= false              // Compile in BRINGUP mode?
}; // Compile-time controls

//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef uint16_t       HOST16_t;    // Host format, 16 bit integer
typedef uint32_t       HOST32_t;    // Host format, 32 bit integer
typedef uint64_t       HOST64_t;    // Host format, 64 bit integer

typedef uint16_t       PEER16_t;    // Network format, 16 bit integer
typedef uint32_t       PEER32_t;    // Network format, 32 bit integer
typedef uint64_t       PEER64_t;    // Network format, 64 bit integer

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
extern FILE*           stdlog;      // Log file handle

extern int             env_hcdm;    // Hard Core Debug Mode
extern unsigned        env_iodm;    // In/Output Debug maximum trace size
extern int             env_scdm;    // Soft Core Debug Mode

extern int             opt_help;    // --help option specified
extern int             opt_hcdm;    // --hcdm (Hard Core Debug Mode)
extern int             opt_verbose; // Verbosity, higher is more verbose

extern int             opt_erase;   // Erase local target if it does
                                    // not exist remotely
extern int             opt_older;   // Update local target even if remote
                                    // source is older
extern int             opt_quiet;   // Quiet mode
extern int             opt_unsafe;  // Unsafe mode (allow path mismatch)
extern int             opt_verify;  // Verify (checksum) mode

extern int             port;        // Connection port number

//----------------------------------------------------------------------------
// Expose PUB types and methods
#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;                   // For convenience
using PUB::Trace;                   // For convenience
using PUB::utility::dump;           // For convenience

using namespace PUB::debugging;     // For debugging subroutines
using PUB::s2c;                     // String to const char* (in pub/utility.i)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Expose STD types and methods
using std::lock_guard;              // For convenience
using std::string;                  // For convenience

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_check_signal
//       handle_check_signal
//       raise_check_signal
//
// Purpose-
//       Access the debugging check Signal
//       Define a check Signal handler
//       Raise check Signal
//
//----------------------------------------------------------------------------
struct CheckEvent : public pub::signals::Event_t {
const char*            info= nullptr; // Signal information
}; // CheckEvent

extern pub::signals::Signal*        // The (singleton) debugging signal
   get_check_signal( void );        // Get (singleton) debugging signal

extern pub::signals::Connector      // The check Signal Connector
   handle_check_signal(             // Get check Signal Connector
     const pub::signals::Signal::Function&
                       function);   // The check Signal handler function

extern void
   raise_check_signal(              // Raise the check Signal
     const char*       info= "");   // Signal source information

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_final_signal
//       handle_final_signal
//       raise_final_signal
//
// Purpose-
//       Access the terminaiton Signal
//       Define a termination Signal handler
//       Raise termination Signal
//
//----------------------------------------------------------------------------
struct FinalEvent : public pub::signals::Event_t {
const char*            info= nullptr; // Signal information
}; // FinalEvent

extern pub::signals::Signal*        // The (singleton) debugging signal
   get_final_signal( void );        // Get (singleton) debugging signal

extern pub::signals::Connector      // The check Signal Connector
   handle_final_signal(             // Get check Signal Connector
     const pub::signals::Signal::Function&
                       function);   // The check Signal handler function

extern void
   raise_final_signal(              // Raise the check Signal
     const char*       info= "");   // Signal source information

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_file_name
//
// Purpose-
//       Extract file name from fully qualified name
//
//----------------------------------------------------------------------------
static inline string                // The file name
   get_file_name(                   // Get file name from
     const string&     full_name)   // This fully qualified name
{  return pub::data::Name::get_file_name(full_name); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_full_name
//
// Purpose-
//       Convert path_name and file_name into fully qualified name
//
//----------------------------------------------------------------------------
static inline string                // The fully qualified name
   get_full_name(                   // Get fully qualified name from
     const string&     path_name,   // This path name and
     const string&     file_name)   // This file name
{  return pub::data::Name::get_full_name(path_name, file_name); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_path_name
//
// Purpose-
//       Extract path name from fully qualified name
//
//----------------------------------------------------------------------------
static inline string                // The path name
   get_path_name(                   // Get path name from
     const string&     full_name)   // This fully qualified name
{  return pub::data::Name::get_path_name(full_name); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       host_to_peer
//       peer_to_host
//
// Purpose-
//       Convert HOST (any endian) to PEER (big endian) format.
//       Convert PEER (big endian) to HOST (any endian) format.
//
//----------------------------------------------------------------------------
extern PEER16_t                     // PEER format
   host_to_peer(                    // Convert HOST format to PEER format
     HOST16_t          host16);     // HOST format short value

extern PEER32_t                     // PEER format
   host_to_peer(                    // Convert HOST format to PEER format
     HOST32_t          host32);     // HOST format int value

extern PEER64_t                     // PEER format
   host_to_peer(                    // Convert HOST format to PEER format
     HOST64_t          host64);     // HOST format long value

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern HOST16_t                     // HOST format
   peer_to_host(                    // Convert PEER format to HOST format
     PEER16_t          peer16);     // PEER format short value

extern HOST32_t                     // HOST format
   peer_to_host(                    // Convert PEER format to HOST format
     PEER32_t          peer32);     // PEER format int value

extern HOST64_t                     // HOST format
   peer_to_host(                    // Convert PEER format to HOST format
     PEER64_t          peer64);     // PEER format long value

//----------------------------------------------------------------------------
//
// Subroutine-
//       min
//
// Purpose-
//       Return minimum value
//
//----------------------------------------------------------------------------
inline size_t                       // The minimum value
   min(                             // Get minimum value
     size_t            lhs,         // Left Hand Side
     size_t            rhs)         // Right Hand Side
{  if( lhs < rhs ) return lhs; else return rhs; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgdump
//
// Purpose-
//       Debugging dump to msglog.
//
//----------------------------------------------------------------------------
extern void
   msgdump(                         // Debugging dump to log
     const void*       addr,        // Dump address
     unsigned long     size);       // Dump length

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgerr
//
// Purpose-
//       Write an error message to stderr and stdlog.
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
extern void
   msgerr(                          // Write an error message
     const char*       fmt,         // The PRINTF format string
                       ...);        // The remaining arguments

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgioerr
//
// Purpose-
//       Write an I/O error message to stderr and stdlog.
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
extern void
   msgioerr(                        // Write an I/O error message
     const char*       fmt,         // The PRINTF format string
                       ...);        // The remaining arguments

//----------------------------------------------------------------------------
//
// Subroutine-
//       msglog
//
// Purpose-
//       Write a log message to stdlog.
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
extern void
   msglog(                          // Write a log message
     const char*       fmt,         // The PRINTF format string
                       ...);        // The remaining arguments

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgout
//
// Purpose-
//       Write a message to stdout and stdlog.
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
extern void
   msgout(                          // Write a message
     const char*       fmt,         // The PRINTF format string
                       ...);        // The remaining arguments

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdinit
//
// Purpose-
//       Initialize.
//
//----------------------------------------------------------------------------
extern void
   rdinit( void );                  // Initialize

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdterm
//
// Purpose-
//       Terminate.
//
//----------------------------------------------------------------------------
extern void
   rdterm( void );                  // Terminate

//----------------------------------------------------------------------------
//
// Subroutine-
//       set_log_name
//
// Purpose-
//       Set the default log filename
//
//----------------------------------------------------------------------------
extern void
   set_log_name(const char*);       // Set the default log filename

//----------------------------------------------------------------------------
// IoCommon requires RdCommon
#include "IoCommon.h"               // For I/O types and subroutines
#endif // RDCOMMON_H_INCLUDED

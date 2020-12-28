//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Config.h
//
// Purpose-
//       Editor: Configuration controls
//
// Last change date-
//       2020/12/25
//
//----------------------------------------------------------------------------
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <string>                   // For std::string
#include <pub/config.h>             // For _ATTRIBUTE_PRINTF macros
#include <pub/Signals.h>            // For pub::signals

//----------------------------------------------------------------------------
//
// Class-
//       Config
//
// Purpose-
//       Constructor/destructor (for namespace config)
//
//----------------------------------------------------------------------------
class Config {                      // Config constuctor/destructor
public:
   Config(                          // Constructor
     int               argc,        // Argument count
     char*             argv[]);     // Argument array

   ~Config( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       Config::backtrace
//
// Purpose-
//       Debugging backtrace
//
//----------------------------------------------------------------------------
static void
   backtrace( void );               // Debugging backtrace

//----------------------------------------------------------------------------
//
// Method-
//       Config::check
//
// Purpose-
//       Debugging consistency check
//
//----------------------------------------------------------------------------
static void
   check(                           // Debugging consistency check
     const char*       info= nullptr); // Associated info

//----------------------------------------------------------------------------
//
// Method-
//       Config::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
static void
   debug(                           // Debugging display
     const char*       info= nullptr); // Associated info

//----------------------------------------------------------------------------
//
// Method-
//       Config::alertf
//
// Purpose-
//       Extend errorf to write screen alert.
//
// Implementation note-
//       Do not include trailing '\n' in string.
//
//----------------------------------------------------------------------------
static void
   alertf(                          // Write to stderr, trace iff opt_hcdm
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

//----------------------------------------------------------------------------
//
// Method-
//       Config::errorf
//
// Purpose-
//       Write to stderr, write to trace iff opt_hcdm
//
//----------------------------------------------------------------------------
static void
   errorf(                          // Write to stderr, trace iff opt_hcdm
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

//----------------------------------------------------------------------------
//
// Method-
//       Config::failure
//
// Purpose-
//       Write error message and exit
//
//----------------------------------------------------------------------------
static void
   failure(                         // Write error message and exit
     std::string       mess);       // (The error message)

//----------------------------------------------------------------------------
//
// Subroutine-
//       Config::trace
//
// Purpose-
//       Simple trace event
//
//----------------------------------------------------------------------------
static void*                        // The trace record (uninitialized)
   trace(                           // Get trace record
     unsigned          size= 0);    // Of this extra size

static void
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     uint32_t          code= 0,     // Trace code
     const char*       info= nullptr); // Trace info (15 characters max)

static void
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     const char*       code,        // Trace sub-identifier
     void*             _one= nullptr,  // Word one
     void*             _two= nullptr); // Word two
}; // class Config

//----------------------------------------------------------------------------
//
// Namespace-
//       config
//
// Purpose-
//       Editor: Configuration controls
//
//----------------------------------------------------------------------------
namespace config {                  // The Config namespace
//----------------------------------------------------------------------------
// config::Global attributes
//----------------------------------------------------------------------------
// Debugging controls
extern int             opt_hcdm;    // Hard Core Debug Mode?
extern const char*     opt_test;    // Bringup test?
extern int             opt_verbose; // Debugging verbosity

// Editor controls
extern int             autowrap;    // Autowrap locate (= false)
extern int             ignore_case; // Ignore case when searching (= true)
extern int             search_mode; // (Positive= forward, else reverse) (= 0)

// Initialized controls
extern std::string     AUTO;        // The AUTOSAVE directory
extern std::string     HOME;        // The HOME directory

//----------------------------------------------------------------------------
// config::Signals
//----------------------------------------------------------------------------
extern pub::signals::Signal<const char*> // The CheckEvent signal
                       checkSignal;
extern pub::signals::Signal<const char*> // The DebugEvent signal
                       debugSignal;
extern pub::signals::Signal<const int>   // The SignalEvent signal
                       signalSignal;

//----------------------------------------------------------------------------
// Static string constants
//----------------------------------------------------------------------------
static constexpr const char* const AUTOFILE= "*AUTOSAVE*."; // Name prefix
static constexpr const char* const UUID= "e743e3ac-6816-4878-81a2-b47c9bbc2d37";
}  // namespace config
#endif // CONFIG_H_INCLUDED

//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
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
//       2024/04/12
//
//----------------------------------------------------------------------------
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <string>                   // For std::string
#include <xcb/xproto.h>             // For xcb_rectangle_t

#include <pub/config.h>             // For ATTRIB_PRINTF macro
#include <pub/Signals.h>            // For pub::signals

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class EdFile;
class EdLine;
class EdRedo;

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
//       Config::errorf
//
// Purpose-
//       Write to stderr, write to trace iff opt_hcdm
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2);
static void
   errorf(                          // Write to stderr, trace iff opt_hcdm
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

//----------------------------------------------------------------------------
//
// Method-
//       Config::failure
//
// Purpose-
//       Write error message and exit
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2);
static void
   failure(                         // Write error message and exit
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

//----------------------------------------------------------------------------
//
// Method-
//       Config::join
//       Config::start
//
// Purpose-
//       Virtual thread implementation
//
//----------------------------------------------------------------------------
static void
   join( void );                    // Wait for "Thread"

static void
   start( void );                   // Start "Thread"
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
extern int             opt_verbose; // Debugging verbosity

// Color controls ------------------------------------------------------------
extern uint32_t        mark_bg;     // mark.bg: Marked text BG (background)
extern uint32_t        mark_fg;     // mark.bg: Marked text FG (foreground)

extern uint32_t        text_bg;     // text.bg: Normal Text BG
extern uint32_t        text_fg;     // text.bg: Normal Text FG

extern uint32_t        change_bg;   // change.bg: Status BG, modified file
extern uint32_t        change_fg;   // change.fg: Status FG, modified file

extern uint32_t        status_bg;   // status.bg: Status BG, pristine file
extern uint32_t        status_fg;   // status.fg: Status FG, pristine file

extern uint32_t        message_bg;  // message.bg: Message line BG
extern uint32_t        message_fg;  // message.fg: Message line FG

// Screen controls -----------------------------------------------------------
extern xcb_rectangle_t geom;        // The screen geometry

// (Operational controls) ----------------------------------------------------
extern uint32_t        USE_MOUSE_HIDE; // Use mouse hide logic?

// Initialized controls
extern std::string     AUTO;        // The AUTOSAVE directory
extern std::string     HOME;        // The HOME directory

//----------------------------------------------------------------------------
// config::Signals
//----------------------------------------------------------------------------
extern pub::signals::Signal<const char*>* // The RAII Check signal
   check_signal();                  // Run consistency checks

//----------------------------------------------------------------------------
// Static string constants
//----------------------------------------------------------------------------
static constexpr const char* const AUTOFILE= "*AUTOSAVE*."; // Name prefix
}  // namespace config
#endif // CONFIG_H_INCLUDED

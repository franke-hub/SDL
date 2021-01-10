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
//       2021/01/10
//
//----------------------------------------------------------------------------
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <string>                   // For std::string
#include <pub/config.h>             // For _ATTRIBUTE_PRINTF macros
#include <pub/Signals.h>            // For pub::signals

#include "Xcb/Device.h"             // For xcb::Device
#include "Xcb/Font.h"               // For xcb::Font
#include "Xcb/Window.h"             // For xcb::Window

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
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

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

static void
   trace(                           // Trace undo/redo operation
     const char*       ident,       // Trace identifier (.UDO, .RDO)
     const char*       code,        // Trace sub-identifer (file,init,mark)
     EdRedo*           redo,        // The UNDO/REDO
     EdFile*           file,        // The UNDO/REDO file
     EdLine*           line= nullptr); // The UNDO/REDO cursor line
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

// XCB objects --------------------------------------------------------------
extern xcb::Device*    device;      // The root Device
extern xcb::Window*    window;      // The test Window
extern xcb::Font*      font;        // The Font object

// Color controls ------------------------------------------------------------
extern uint32_t        mark_bg;     // mark.bg: Marked text BG (background)
extern uint32_t        mark_fg;     // mark.bg: Marked text FG (foreground)

extern uint32_t        text_bg;     // text.bg: Normal Text BG
extern uint32_t        text_fg;     // text.bg: Normal Text FG

extern uint32_t        change_bg;   // change.bg: Status BG, modified file
extern uint32_t        change_fg;   // change.fg: Status FG, modified file

extern uint32_t        status_bg;   // status.bg: Status BG, pristine file
extern uint32_t        status_fg;   // status.fg: Status FG, pristine file

extern uint32_t        command_bg;  // command.bg: Command line BG
extern uint32_t        command_fg;  // command.fg: Command line FG

extern uint32_t        message_bg;  // message.bg: Message line BG
extern uint32_t        message_fg;  // message.fg: Message line FG

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

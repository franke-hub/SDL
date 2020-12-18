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
//       2020/12/17
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
//       Config::check
//
// Purpose-
//       Debugging consistency check
//
//----------------------------------------------------------------------------
static void
   check(                           // Debugging consistency check
     const char*       info= nullptr); // Informational text

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
     const char*       info= nullptr); // Informational text

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

//----------------------------------------------------------------------------
// Immutable constants
//----------------------------------------------------------------------------
enum Colors
{  CLR_Black=          0x00000000
,  CLR_DarkRed=        0x00900000   // A.K.A red4
,  CLR_FireBrick=      0x00B22222
,  CLR_LightBlue=      0x00C0F0FF
,  CLR_LightSkyBlue=   0x00B0E0FF
,  CLR_PaleBlue=       0x00F8F8FF
,  CLR_PaleMagenta=    0x00FFC0FF   // A.K.A plum1
,  CLR_PaleYellow=     0x00FFFFF0   // A.K.A ivory
,  CLR_PowderBlue=     0x00B0E0E0
,  CLR_White=          0x00FFFFFF
,  CLR_Yellow=         0x00FFFF00

// Color selectors
,  CHG_FG= CLR_DarkRed              // FG: Status line, file changed
,  CHG_BG= CLR_LightBlue            // BG: Status line, file changed

,  CMD_FG= CLR_Black                // FG: Command line
,  CMD_BG= CLR_PaleMagenta          // BG: Command line

,  MSG_FG= CLR_DarkRed              // FG: Message line
,  MSG_BG= CLR_Yellow               // BG: Message line

,  SEL_FG= CLR_Black                // FG: Selected line
,  SEL_BG= CLR_LightBlue            // BG: Selected line

,  STS_FG= CLR_Black                // FG: Status line, file unchanged
,  STS_BG= CLR_LightBlue            // BG: Status line, file unchanged

,  TXT_FG= CLR_Black                // FG: Text
,  TXT_BG= CLR_PaleYellow           // BG: Text
}; // generic enum
}  // namespace config
#endif // CONFIG_H_INCLUDED

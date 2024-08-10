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
//       EdOpts.h
//
// Purpose-
//       TERM/XCM Editor: Configuration options
//
// Last change date-
//       2024/07/27
//
//----------------------------------------------------------------------------
#ifndef EDOPTS_H_INCLUDED
#define EDOPTS_H_INCLUDED

#include <string>                   // For std::string

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class EdUnit;

//----------------------------------------------------------------------------
//
// Class-
//       EdOpts
//
// Purpose-
//       Xcb/Term control interface.
//
//----------------------------------------------------------------------------
class EdOpts {                      // Editor text Window viewport
public:
//----------------------------------------------------------------------------
// EdOpts::Enumerations and typedefs
//----------------------------------------------------------------------------
enum VERSION                        // Version information
{  MAJOR= 3                         // Version 3.0.PATCH
,  MINOR= 0
}; // VERSION

//----------------------------------------------------------------------------
// EdOpts::Attributes
//----------------------------------------------------------------------------
struct alignas(16)
{  char                _[64];
}                      local;       // Implementation-defined local area

//----------------------------------------------------------------------------
// EdOpts::Initialization/termination
//----------------------------------------------------------------------------
static EdUnit*                      // The EdUnit
   initialize( void );              // Initialize

static void
   terminate(EdUnit*);              // Terminate (the EdUnit)

static void
   at_exit( void );                 // Idempotent termination error handler

//----------------------------------------------------------------------------
// EdOpts::Suspend/resume (Used by EdBifs.cpp)
//----------------------------------------------------------------------------
static void
   resume();                        // Resume NCURSES

static void
   suspend();                       // Suspend NCURSES

//----------------------------------------------------------------------------
// EdOpts::Control attributes
//----------------------------------------------------------------------------
// TRUE iff opt_bg is implemented
static bool
   is_bg_enabled();                 // Is opt_bg implemented?

// TRUE iff UTF combining characters are supported
static bool
   has_unicode_combining();         // Is UTF combining supported?

// TRUE iff Unicode character display is supported
static bool
   has_unicode_support();           // Is Unicode display supported?

//----------------------------------------------------------------------------
// EdOpts::Static attributes
//----------------------------------------------------------------------------
static std::string
   DEFAULT_CONFIG();                // The default configuration file
static std::string
   EDITOR();                        // The editor's name
static std::string
   PATCH();                         // Version patch level
}; // class EdOpts
#endif // EDOPTS_H_INCLUDED

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
//       Editor: Xcb vs Term version controls
//
// Last change date-
//       2024/05/15
//
//----------------------------------------------------------------------------
#ifndef EDOPTS_H_INCLUDED
#define EDOPTS_H_INCLUDED

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
//       Xcb vs Term controls.
//
//----------------------------------------------------------------------------
class EdOpts {                      // Editor text Window viewport
public:
//----------------------------------------------------------------------------
// EdOpts::Enumerations and typedefs
//----------------------------------------------------------------------------
enum                                // Version information
{  MAJOR= 3                         // Version 3.0
,  MINOR= 0
}; // Version

//----------------------------------------------------------------------------
// EdOpts::PATCH version information
//----------------------------------------------------------------------------
static const char*     PATCH;       // Version patch level

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
// EdOpts::Control attributes
//----------------------------------------------------------------------------
static int             bg_enabled;  // opt_bg allowed?
static int             utf8_enabled;  // Is UTF-8 supported?

//----------------------------------------------------------------------------
// EdOpts::Static attributes
//----------------------------------------------------------------------------
static const char*     EDITOR;      // The editor's name
static const char*     DEFAULT_CONFIG; // The default configuration file
}; // class EdOpts
#endif // EDOPTS_H_INCLUDED

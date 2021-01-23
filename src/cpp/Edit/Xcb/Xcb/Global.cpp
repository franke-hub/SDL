//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Xcb/Global.cpp
//
// Purpose-
//       Instantiate externals.
//
// Last change date-
//       2021/01/22
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For Debug object
#include <pub/Trace.h>              // For Trace object

#include "Xcb/Global.h"             // Implementation class

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
int                    xcb::opt_hcdm= false; // Hard Core Debug Mode?
const char*            xcb::opt_test= nullptr; // Run bringup test?
int                    xcb::opt_verbose= -1; // Verbosity, default NONE

uint32_t               xcb::keystate= KS_INS; // Keyboard state (insert)

namespace xcb {
//----------------------------------------------------------------------------
//
// Subroutine-
//       xcb::user_debug
//
// Purpose-
//       Write to stderr. If opt_hcdm, also write to debug trace file.
//
//----------------------------------------------------------------------------
void
   user_debug(                      // User error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);   // Write to stderr
   va_end(argptr);                  // Close va_ functions

   if( opt_hcdm ) {                 // If Hard Core Debug Mode
     va_start(argptr, fmt);
     ::pub::debugging::vtraceh(fmt, argptr);
     va_end(argptr);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       xcb::checkstop
//
// Purpose-
//       Handle checkstop condition.
//
//----------------------------------------------------------------------------
void
   checkstop(                       // Check stop
     int               line,        // Line number
     const char*       name)        // Function name
{
   debugh("%4d CHECKSTOP(%s)\n", line, name);
   debug_flush();
   exit(2);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       xcbcheck
//
// Purpose-
//       Validate an XCB result.
//
//----------------------------------------------------------------------------
void
   xcbcheck(                        // Verify XCB function result
     int               line,        // Line number
     const char*       name,        // Function name
     int               xc)          // Assertion (must be TRUE)
{
   if( opt_hcdm || opt_verbose > 1 ) {
     xcbdebug(line, name, xc);
     if( xc == false )
       checkstop(line, "xcbcheck");
   } else if( xc == false ) {
     xcbdebug(line, name, xc);
     checkstop(line, "xcbcheck");
   }
}

void
   xcbcheck(                        // Verify XCB function result
     int               line,        // Line number
     const char*       name,        // Function name
     xcb_generic_error_t* xc)       // Generic error
{
   if( xc ) {
     debugf("%4d EC(%d)= %s()\n", line, xc->error_code, name);
     xcberror(xc);
     checkstop(line, "xcbcheck");
   } else if( opt_hcdm || opt_verbose > 1 ) {
     xcbdebug(line, name, 0);
   }
}

void
   xcbcheck(                        // Verify XCB function result
     int               line,        // Line number
     const char*       name,        // Function name
     void*             xc)          // Object pointer (Must not be nullptr)
{
   if( opt_hcdm || opt_verbose > 1 ) {
     xcbdebug(line, name, xc);
     if( xc == nullptr )
       checkstop(line, "xcbcheck");
   } else if( xc == nullptr ) {
     xcbdebug(line, name, xc);
     checkstop(line, "xcbcheck");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       xcb::xcbdebug
//
// Purpose-
//       Display an XCB result.
//
//----------------------------------------------------------------------------
void
   xcbdebug(                        // Log XCB function result
     int               line,        // Line number
     const char*       name,        // Function name
     int               xc)          // Return code
{  debugh("%4d 0x%x= %s()\n", line, xc, name); }

void
   xcbdebug(                        // Log XCB function result
     int               line,        // Line number
     const char*       name,        // Function name
     void*             xc)          // Pointer
{  debugh("%4d %p= %s()\n", line, xc, name); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       xcb::xcberror
//
// Purpose-
//       XCB error diagnostic display.
//
//----------------------------------------------------------------------------
void
   xcberror(                        // XCB error disagnostic display
     xcb_generic_error_t* E)        // The error response
{
   user_debug("XCB error(%u) id(%u) op[%u,%u]\n", E->error_code
             , E->resource_id, E->major_code, E->minor_code);
}
}  // namespace xcb

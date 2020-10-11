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
//       Xcb/Global.cpp
//
// Purpose-
//       Instantiate externals.
//
// Last change date-
//       2020/10/08
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For Debug object
#include <pub/Trace.h>              // For Trace object

#include "Xcb/Global.h"             // Implementation class

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging subroutines

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
int                    xcb::opt_hcdm= false; // Hard Core Debug Mode?
const char*            xcb::opt_test= nullptr; // Run bringup test?
int                    xcb::opt_verbose= -1; // Verbosity, default NONE

uint32_t               xcb::keystate= 0; // Keyboard state

namespace xcb {
//----------------------------------------------------------------------------
//
// Subroutine-
//       xcb::debug_flush
//       xcb::debugf
//       xcb::debugh
//       xcb::tracef
//       xcb::traceh
//
// Purpose-
//       Debugging interfaces, exactly the same as ::pub::debugging
//
//----------------------------------------------------------------------------
void
   debug_flush( void )              // Flush write the trace file
{  ::pub::debugging::debug_flush(); }

void
   debugf(                          // Debug debug printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   ::pub::debugging::vdebugf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   debugh(                          // Debug debug printf facility with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   ::pub::debugging::vdebugh(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   tracef(                          // Debug trace printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   ::pub::debugging::vtracef(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   traceh(                          // Debug trace printf facility, with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   ::pub::debugging::vtraceh(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

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
//       xcb::trace
//
// Purpose-
//       Simple trace
//
//----------------------------------------------------------------------------
void
   trace(                           // Simple trace event
     const char*       ident,       // Trace identifier
     uint32_t          code,        // Trace code
     const char*       text)        // Trace text (15 characters max)
{
  typedef ::pub::Trace::Record Record;
  Record* record= (Record*)::pub::Trace::storage_if(sizeof(Record));
  if( record ) {                    // Trace event
    char* unit= (char*)&record->unit;
    unit[3]= char(code >>  0);
    unit[2]= char(code >>  8);
    unit[1]= char(code >> 16);
    unit[0]= char(code >> 24);

    memset(record->value, 0, sizeof(record->value));
    if( text )
      strcpy(record->value, text);
    record->trace(ident);
  }
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
{  debugf("%4d 0x%x= %s()\n", line, xc, name); }

void
   xcbdebug(                        // Log XCB function result
     int               line,        // Line number
     const char*       name,        // Function name
     void*             xc)          // Pointer
{  debugf("%4d %p= %s()\n", line, xc, name); }

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

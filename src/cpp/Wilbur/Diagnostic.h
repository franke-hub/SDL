//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Diagnostic.h
//
// Purpose-
//       Diagnostic utility methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef DIAGNOSTIC_H_INCLUDED
#define DIAGNOSTIC_H_INCLUDED

#include <com/Debug.h>              // This include is guaranteed
#include <com/Socket.h>

//----------------------------------------------------------------------------
//
// Class-
//       Diagnostic
//
// Purpose-
//       Diagnostic utilities.
//
//----------------------------------------------------------------------------
class Diagnostic                    // Diagnostic utilities
{
//----------------------------------------------------------------------------
// Diagnostic::Constructors
//----------------------------------------------------------------------------
protected:                          // Only static methods defined
   ~Diagnostic( void );             // Destructor
   Diagnostic( void );              // Default constructor

//----------------------------------------------------------------------------
// Diagnostic::Methods
//----------------------------------------------------------------------------
public:
static void
   httpTrace(                       // Trace HTTP request or response
     const char*       prefix,      // Request/response identifier prefix
     const char*       buffer,      // The request/response data
     unsigned          length);     // The request/response length

static int                          // Socket.recv() return code
   recv(                            // Receive Socket message
     Socket&           socket,      // The socket
     char*             addr,        // The message address
     int               size,        // The message length
     Socket::SocketMO  opts= Socket::MO_UNSPEC); // Options

static int                          // Return code, 0 OK
   recvLine(                        // Receive Socket message line
     Socket&           socket,      // The socket
     char*             buffer,      // Result buffer
     unsigned          size,        // Result buffer size
     unsigned          timeout= 125); // Timeout (milliseconds)

static int                          // Socket.send() return code
   send(                            // Send Socket message
     Socket&           socket,      // The socket
     const char*       addr,        // The message address
     int               size,        // The message length
     Socket::SocketMO  opts= Socket::MO_UNSPEC); // Options
}; // class Diagnostic

#endif // DIAGNOSTIC_H_INCLUDED

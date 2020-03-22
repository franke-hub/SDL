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
//       HttpServerThread.h
//
// Purpose-
//       Define the HTTP server thread.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef HTTPSERVERTHREAD_H_INCLUDED
#define HTTPSERVERTHREAD_H_INCLUDED

#include <com/Socket.h>
#include <com/Thread.h>

//----------------------------------------------------------------------------
//
// Class-
//       HttpServerThread
//
// Purpose-
//       The HTTP server Thread.
//
//----------------------------------------------------------------------------
class HttpServerThread : public NamedThread {
//----------------------------------------------------------------------------
// HttpServerThread::Attributes
//----------------------------------------------------------------------------
public:
int                    fsm;         // Finite State Machine
Socket                 listen;      // The listener Socket

//----------------------------------------------------------------------------
// HttpServerThread::Constructors
//----------------------------------------------------------------------------
public:
   ~HttpServerThread( void );       // Destructor
   HttpServerThread( void );        // Constructor

//----------------------------------------------------------------------------
// HttpServerThread::Methods
//----------------------------------------------------------------------------
public:
unsigned                            // The Listen network address
   getAddr( void ) const            // Get Listen network address
{  return listen.getHostAddr();
}

unsigned                            // The Listen port number
   getPort( void ) const            // Get Listen port number
{  return listen.getHostPort();
}

virtual int                         // Return code (0 OK)
   notify(                          // Notify the HttpServerThread
     int               code);       // Notification code

virtual long
   run();
}; // class HttpServerThread

#endif // HTTPSERVERTHREAD_H_INCLUDED

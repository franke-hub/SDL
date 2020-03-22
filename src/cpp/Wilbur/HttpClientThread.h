//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpClientThread.h
//
// Purpose-
//       Define the HTTP client thread.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef HTTPCLIENTTHREAD_H_INCLUDED
#define HTTPCLIENTTHREAD_H_INCLUDED

#include "com/Thread.h"

//----------------------------------------------------------------------------
//
// Class-
//       HttpClientThread
//
// Purpose-
//       Drive the HTTP client.
//
// Implementation notes-
//       The HttpClientThread uses static objects.
//       Only one HttpClientThread instance can exist.
//
//----------------------------------------------------------------------------
class HttpClientThread : public NamedThread {
//----------------------------------------------------------------------------
// HttpClientThread::Attributes
//----------------------------------------------------------------------------
public:
enum FSM                            // Finite State Machine
{  FSM_RESET                        // (Default) RESET
,  FSM_READY                        // READY (Operational)
,  FSM_CLOSE                        // CLOSE (terminated)
}; // enum FSM

int                    fsm;         // Finite State Machine

//----------------------------------------------------------------------------
// HttpClientThread::Constructors
//----------------------------------------------------------------------------
public:
   ~HttpClientThread( void );       // Destructor
   HttpClientThread( void );        // Constructor

//----------------------------------------------------------------------------
// HttpClientThread::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   notify(                          // Notify the HttpClientThread
     int               code);       // Notification code

virtual long
   run();
}; // class HttpClientThread

#endif // HTTPCLIENTTHREAD_H_INCLUDED

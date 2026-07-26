//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       ListenThread.h
//
// Purpose-
//       The listener thread
//
// Last change date-
//       2026/07/23
//
//----------------------------------------------------------------------------
#ifndef LISTENTHREAD_H_INCLUDED
#define LISTENTHREAD_H_INCLUDED

#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread, base class

#include "IoCommon.h"               // For I/O common objects and subroutines

//----------------------------------------------------------------------------
//
// Class-
//       ListenThread
//
// Purpose-
//       ListenThread descriptor.
//
//----------------------------------------------------------------------------
class ListenThread : public pub::Thread { // ListenThread descriptor
//----------------------------------------------------------------------------
// ListenThread::Attributes
//----------------------------------------------------------------------------
protected:
pub::Socket*           socket= nullptr; // Listener Socket
char*                  init_path= nullptr; // Initial path
int                    port;        // Server port

//----------------------------------------------------------------------------
// ListenThread::Constructors/destructor
//----------------------------------------------------------------------------
public:
   ListenThread(                    // Constructor
     int               port);       // The connection port

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual
   ~ListenThread( void );           // Destructor

//----------------------------------------------------------------------------
// ListenThread::Accessors
//----------------------------------------------------------------------------
public:
virtual int                         // TRUE iff ListenThread
   isListenThread( void ) const     // Is this the ListenThread?
{  return true; }

//----------------------------------------------------------------------------
//
// Method-
//       ListenThread::run()
//
// Purpose-
//       Listen for new connections.
//
//----------------------------------------------------------------------------
protected:
virtual void
   run( void );                     // Operate the Thread
}; // class ListenThread
#endif // LISTENTHREAD_H_INCLUDED

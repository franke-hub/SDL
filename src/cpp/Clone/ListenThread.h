//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ListenThread.h
//
// Purpose-
//       Base object for ListenThread and ServerThread
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef LISTENTHREAD_H_INCLUDED
#define LISTENTHREAD_H_INCLUDED

#ifndef COMMONTHREAD_H_INCLUDED
#include "CommonThread.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       ListenThread
//
// Purpose-
//       ListenThread descriptor.
//
//----------------------------------------------------------------------------
class ListenThread : public CommonThread { // ListenThread descriptor
//----------------------------------------------------------------------------
// ListenThread::Attributes
//----------------------------------------------------------------------------
protected:
char*                  path;        // Starting path
int                    port;        // Server port

//----------------------------------------------------------------------------
// ListenThread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ListenThread( void );           // Destructor
   ListenThread(                    // Constructor
     int               port);       // The connection port

//----------------------------------------------------------------------------
// ListenThread::Accessors
//----------------------------------------------------------------------------
public:
virtual int                         // TRUE iff ListenThread
   isListenThread( void ) const     // Is this the ListenThread?
{  return TRUE; }

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
virtual long                        // Return code
   run( void );                     // Operate the Thread
}; // class ListenThread

#endif // LISTENTHREAD_H_INCLUDED

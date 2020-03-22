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
//       ServerThread.h
//
// Purpose-
//       The server thread
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef SERVERTHREAD_H_INCLUDED
#define SERVERTHREAD_H_INCLUDED

#ifndef COMMONTHREAD_H_INCLUDED
#include "CommonThread.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class DirEntry;

//----------------------------------------------------------------------------
//
// Class-
//       ServerThread
//
// Purpose-
//       ServerThread descriptor.
//
//----------------------------------------------------------------------------
class ServerThread : public CommonThread { // ServerThread descriptor
//----------------------------------------------------------------------------
// ServerThread::Attributes
//----------------------------------------------------------------------------
protected:
const char*            path;        // The starting directory

//----------------------------------------------------------------------------
// ServerThread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ServerThread( void );           // Destructor
   ServerThread(                    // Constructor
     Socket*           socket,      // Our working Socket
     const char*       path);       // Our starting path

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::exchangeVersionID
//
// Function-
//       Exchange version identifiers.
//
//----------------------------------------------------------------------------
int                                 // TRUE if version identifiers match
   exchangeVersionID( void );       // Exchange version identifiers

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::run()
//
// Purpose-
//       Operate the server for one client.
//
//----------------------------------------------------------------------------
public:
virtual long                        // Return code
   run( void );                     // Operate the Thread

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serve
//
// Purpose-
//       Process server requests (Initial directory level)
//
//----------------------------------------------------------------------------
void
   serve( void );                   // Process server requests

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serveDirectory
//
// Function-
//       Install a directory subtree.
//
//----------------------------------------------------------------------------
void
   serveDirectory(                  // Serve directory subtree
     const char*       path,        // Path to current directory
     DirEntry*         ptrE);       // -> DirEntry (DirEntry.list != NULL)

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serveFile
//
// Function-
//       Return a file to the client.
//
//----------------------------------------------------------------------------
void
   serveFile(                       // Install a file
     const char*       path,        // Current Path
     DirEntry*         ptrE);       // -> DirEntry

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::term
//
// Purpose-
//       Terminate this ServerThread.
//
//----------------------------------------------------------------------------
virtual void
   term( void );                    // Terminate this ServerThread

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::verifyType
//
// Function-
//       Verify that an item is of the appropriate type
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   verifyType(                      // Verify item type
     DirEntry*         ptrE,        // -> DirEntry
     int               type);       // Expected type
}; // class ServerThread

#endif // SERVERTHREAD_H_INCLUDED

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
//       ClientThread.h
//
// Purpose-
//       The client Thread
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef CLIENTTHREAD_H_INCLUDED
#define CLIENTTHREAD_H_INCLUDED

#ifndef COMMONTHREAD_H_INCLUDED
#include "CommonThread.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class DirEntry;
class Socket;

//----------------------------------------------------------------------------
//
// Class-
//       ClientThread
//
// Purpose-
//       ClientThread descriptor.
//
//----------------------------------------------------------------------------
class ClientThread : public CommonThread { // ClientThread descriptor
//----------------------------------------------------------------------------
// ClientThread::Attributes
//----------------------------------------------------------------------------
protected:
const char*            path;        // The starting directory

//----------------------------------------------------------------------------
// ClientThread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ClientThread( void );           // Destructor
   ClientThread(                    // Constructor
     Socket*           socket,      // Associated Socket
     const char*       path);       // Initial directory

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::exchangeVersionID
//
// Function-
//       Exchange version identifiers.
//
//----------------------------------------------------------------------------
public:
int                                 // TRUE if version identifiers match
   exchangeVersionID( void );       // Exchange version identifiers

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::installItem
//
// Function-
//       Install one file, link or directory.
//
//----------------------------------------------------------------------------
int
   installItem(                     // Install something
     const char*       path,        // Current Path
     DirEntry*         serverE,     // -> Server DirEntry
     DirEntry*         clientE);    // -> Target file descriptor

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::removeDirectory
//
// Function-
//       Remove all files and directories from a subtree.
//
//----------------------------------------------------------------------------
int                                 // Return code
   removeDirectory(                 // Remove a directory
     const char*       path,        // The current path
     DirEntry*         clientE);    // -> Client item descriptor

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::removeItem
//
// Function-
//       Delete a file, link or directory.
//
// Implementation notes-
//       For a directory, call removeDirectory first.
//       (This mechanism avoids an extra level of recursion per subdirectory.)
//
//----------------------------------------------------------------------------
int                                 // Return code
   removeItem(                      // Remove something
     const char*       path,        // Current Path
     DirEntry*         clientE);    // -> Client item descriptor

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::term
//
// Purpose-
//       Terminate this ClientThread.
//
//----------------------------------------------------------------------------
virtual void
   term( void );                    // Terminate this ClientThread

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::updateAttr
//
// Function-
//       Update item attributes.
//
//----------------------------------------------------------------------------
void
   updateAttr(                      // Update attributes
     const char*       path,        // Current Path
     DirEntry*         serverE,     // -> Source file descriptor
     DirEntry*         clientE);    // -> Target file descriptor

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::updateDirectory
//
// Function-
//       Update new and changed files, links and directories within a
//       a directory subtree.
//
//----------------------------------------------------------------------------
void
   updateDirectory(                 // Update directory subtree
     const char*       path,        // Base path
     DirList*          clientL,     // -> Client list containing clientE
     DirEntry*         clientE);    // -> Client file descriptor

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::updateItem
//
// Function-
//       Update a file, link or directory.
//
//----------------------------------------------------------------------------
int                                 // Return code
   updateItem(                      // Update something
     const char*       path,        // Current Path
     DirEntry*         serverE,     // -> Source file descriptor
     DirEntry*         clientE);    // -> Target file descriptor

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::run()
//
// Purpose-
//       Operate the client.
//
//----------------------------------------------------------------------------
protected:
virtual long                        // Return code
   run( void );                     // Operate the Thread
}; // class ClientThread

#endif // CLIENTTHREAD_H_INCLUDED

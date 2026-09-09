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
//       ServerThread.h
//
// Purpose-
//       The server thread
//
// Last change date-
//       2026/08/01
//
//----------------------------------------------------------------------------
#ifndef SERVERTHREAD_H_INCLUDED
#define SERVERTHREAD_H_INCLUDED

#include "CommonThread.h"           // For CommonThread, base class
#include "IoCommon.h"               // For I/O common objects and subroutines

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class RdFile;

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
string                 init_path;   // The initial path name
string                 real_path;   // The initial path name, fully resolved
                                    // (canonical, symlink-free). All GOTO
                                    // requests are validated against this.

//----------------------------------------------------------------------------
// ServerThread::Constructors
//----------------------------------------------------------------------------
public:
   ServerThread(                    // Constructor
     Socket*           socket,      // Our I/O Socket
     string            path);       // Our initial path

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual
   ~ServerThread( void );           // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::exchange_versionID
//
// Function-
//       Exchange version identifiers, setting rVersionInfo.
//
//----------------------------------------------------------------------------
int                                 // TRUE if version identifiers match
   exchange_versionID( void );      // Exchange version identifiers

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::run
//
// Purpose-
//       Operate the server for one client.
//
//----------------------------------------------------------------------------
virtual void
   run( void );                     // Operate the Thread

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::say_no
//
// Purpose-
//       Send negative reponse
//
//----------------------------------------------------------------------------
void
   say_no( void );                  // Send negative response

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::say_yo
//
// Purpose-
//       Send positive response
//
//----------------------------------------------------------------------------
void
   say_yo( void );                  // Send positive response

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serve_file
//
// Function-
//       Return a file to the client.
//
//----------------------------------------------------------------------------
void
   serve_file(                      // Install a file
     string            path,        // Current Path
     RdFile*           file);       // -> RdFile

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::serve_path
//
// Function-
//       Install a subdirectory tree.
//
//----------------------------------------------------------------------------
void
   serve_path(                      // Serve subdirectory tree
     RdFile*           file);       // The subdirectory's RdFile

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::path_is_valid
//
// Function-
//       Verify that a (client-influenced) path name resolves to somewhere
//       within the served directory tree (real_path), rejecting attempts
//       to escape it using "..", a leading "..", or a symbolic link.
//
//----------------------------------------------------------------------------
protected:
bool                                // TRUE iff path_name is within real_path
   path_is_valid(                   // Is this path within the served tree?
     const string&     path_name);  // The (possibly relative) path name
}; // class ServerThread
#endif // SERVERTHREAD_H_INCLUDED

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
//       ClientThread.h
//
// Purpose-
//       The client Thread
//
// Last change date-
//       2026/07/05
//
// Implementation notes-
//       The ClientThread does not run under control of a Thread, but it does
//       use CommonThread services.
//
//----------------------------------------------------------------------------
#ifndef CLIENTTHREAD_H_INCLUDED
#define CLIENTTHREAD_H_INCLUDED

#include "IoCommon.h"               // For I/O common objects and subroutines
#include "CommonThread.h"           // For CommonThread, base class

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class RdFile;

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
string                 init_path;   // The initial path name

//----------------------------------------------------------------------------
// ClientThread::Constructors/destructor
//----------------------------------------------------------------------------
public:
   ClientThread(                    // Constructor
     Socket*           socket,      // Associated Socket
     const string&     path);       // Initial path name

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual
   ~ClientThread( void );           // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::exchange_versionID
//
// Function-
//       Exchange version identifiers.
//
//----------------------------------------------------------------------------
public:
int                                 // TRUE if version identifiers match
   exchange_versionID( void );      // Exchange version identifiers

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::install_item
//
// Function-
//       Install one file, link or directory.
//
//----------------------------------------------------------------------------
int
   install_item(                    // Install something
     RdFile*           client,      // -> Client RdFile
     RdFile*           server);     // -> Server RdFile

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::remove_item
//
// Function-
//       Delete a file, link or directory.
//
//----------------------------------------------------------------------------
int                                 // Return code
   remove_item(                     // Remove something
     RdFile*           file);       // -> Item RdFile

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::remove_path
//
// Function-
//       Remove all subtree content
//
//----------------------------------------------------------------------------
void
   remove_path(                     // Remove all subtree content
     RdFile*           file);       // -> Path RdFile

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::run
//
// Purpose-
//       Operate the client.
//
//----------------------------------------------------------------------------
virtual void
   run( void ) override;            // Operate the Client

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::update_attr
//
// Function-
//       Update item attributes.
//
//----------------------------------------------------------------------------
void
   update_attr(                     // Update attributes
     RdFile*           client,      // -> Client RdFile
     RdFile*           server);     // -> Server RdFile

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::update_item
//
// Function-
//       Update a file, link or directory.
//
//----------------------------------------------------------------------------
int                                 // Return code
   update_item(                     // Update something
     RdFile*           client,      // -> Client RdFile
     RdFile*           server);     // -> Server RdFile

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::update_path
//
// Function-
//       Update a path subtree
//
//----------------------------------------------------------------------------
void
   update_path(                     // Update path subtree
     const RdFile*     path_file);  // The directory RdFile
}; // class ClientThread
#endif // CLIENTTHREAD_H_INCLUDED

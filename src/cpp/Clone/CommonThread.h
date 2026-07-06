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
//       CommonThread.h
//
// Purpose-
//       Base object for ListenThread, ClientThread, and ServerThread.
//
// Last change date-
//       2026/07/05
//
//----------------------------------------------------------------------------
#ifndef COMMONTHREAD_H_INCLUDED
#define COMMONTHREAD_H_INCLUDED

#include <pub/List.h>               // For pub::List
#include <pub/Thread.h>             // For pub::Thread, base class

#include "IoCommon.h"               // For I/O common objects and subroutines

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct RdFile;

//----------------------------------------------------------------------------
//
// Class-
//       CommonThread
//
// Purpose-
//       CommonThread descriptor.
//
//----------------------------------------------------------------------------
class CommonThread : public pub::Thread {
//----------------------------------------------------------------------------
// CommonThread::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef pub::List<RdPath> Path_stack; // The RdPath list (stack)

enum FSM                            // Finite State Machine
{  FSM_RESET                        // Reset, not started. Set in: constructor
,  FSM_READY                        // Ready, active       Set in: init
,  FSM_CLOSE                        // Terminating         Set in: ----
,  FSM_FINAL                        // Terminated          Set in: term
};

enum MODE                           // Buffer mode
{  MODE_RESET                       // Reset, idle
,  MODE_WR                          // WRITE mode
,  MODE_RD                          // READ  mode
};

enum                                // Compile-time control: RdClient/RdServer
{  USE_RCVBUF_SIZE= 8192            // SO_RCVBUF buffer size, 0 if unused
};

//----------------------------------------------------------------------------
// CommonThread::Attributes
//----------------------------------------------------------------------------
int                    fsm= FSM_RESET;  // Finite State Machine
int                    mode= MODE_RESET; // Buffer mode

char*                  buffer= nullptr; // Our working input/output bufferInput Ioda
size_t                 buff_size= 0; // Number of bytes read or written
size_t                 buff_used= 0; // Number of used (processed) bytes

Socket*                socket= nullptr; // Our working Socket
Path_stack             stack;       // The RdPath Stack

VersionInfo            gVersionInfo; // Global version information
VersionInfo            lVersionInfo; // Local  version information
VersionInfo            rVersionInfo; // Remote version information

//----------------------------------------------------------------------------
// CommonThread::Constructors/destructor
//----------------------------------------------------------------------------
   CommonThread(                    // Constructor
     Socket*           socket);     // Our working Socket

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual
   ~CommonThread( void );           // Destructor

//----------------------------------------------------------------------------
// CommonThread::Accessors
//----------------------------------------------------------------------------
int                                 // The current state
   getFSM( void ) const             // Get current state
{  return fsm; }

const VersionInfo&                  // The global version info
   getGVersionInfo( void ) const    // Get global version info
{  return gVersionInfo; }

const VersionInfo&                  // The local  version info
   getLVersionInfo( void ) const    // Get local  version info
{  return lVersionInfo; }

const VersionInfo&                  // The remote version info
   getRVersionInfo( void ) const    // Get remote version info
{  return rVersionInfo; }

virtual int                         // TRUE iff ListenThread
   isListenThread( void ) const     // Is this the ListenThread?
{  return false; }

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::compare
//
// Purpose-
//       Compare file name strings, accounting for case
//
//----------------------------------------------------------------------------
int                                 // Result: <0, =0, >0
   compare(                         // Compare strings
     const string&     lhs,         // Left hand side
     const string&     rhs);        // Right hand side

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::pop
//
// Purpose-
//       Remove path from the Path_stack
//
// Implementation notes-
//       TODO: Decide whether this method should delete all files
//
//----------------------------------------------------------------------------
RdPath*                             // The removed Path
   pop( void )                      // Remove newest RdPath from the Path_stack
{  return stack.remq(); }

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::push
//
// Purpose-
//       Add path to the Path_stack
//
//----------------------------------------------------------------------------
void
   push(                            // Add to the Path_stack
     RdPath*           path)        // This path
{  stack.lifo(path); }

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::set_globalVersionInformation
//
// Purpose-
//       Set the global capability vector.
//
//----------------------------------------------------------------------------
void
   set_globalVersionInformation( void ); // Initialize global version info

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::set_localVersionInformation
//
// Purpose-
//       Set the local capability vector.
//
//----------------------------------------------------------------------------
void
   set_localVersionInformation( void ); // Initialize local version info

//----------------------------------------------------------------------------
//
// Method-
//       rd_buff
//       rd_data
//       rd_mode
//       rd_path
//       rd_recv
//
// Purpose-
//       Read from buffer
//       Read data
//       Go into read mode
//       Read entire RdPath
//       Read from Socket
//
//----------------------------------------------------------------------------
void io_debug(int line, const char* info= ""); // I/O Debugging

void
   rd_buff(size_t size);            // Fill the read buffer (to minimum length)

void
   rd_buff(void*, size_t);          // Receive this data from the buffer

HOST16_t                            // The converted PEER16_t
   rd_buff(HOST16_t&);              // Read and convert a PEER16_t size

string                              // The received string
   rd_buff(string&);                // Receive this string from the buffer

void
   rd_data(void*, size_t);          // Unconditionally read this data

string                              // The received string
   rd_data(string&);                // Unconditionally read this string

void
   rd_mode( void );                 // Go into read mode

RdPath*                             // The (new) RdPath
   rd_path(const RdFile* file);     // Read path information (Path RdFile)

// Socket receive
size_t                              // Number of bytes read
   rd_recv(void*, size_t);          // Receive this data

//----------------------------------------------------------------------------
//
// Method-
//       wr_buff
//       wr_data
//       wr_mode
//       wr_path
//       wr_send
//
// Purpose-
//       Write into buffer
//       Write data, emptying buffer first
//       Go into write mode
//       Write entire RdPath
//       Write into Socket
//
//----------------------------------------------------------------------------
void
   wr_buff( void );                 // Empty the write buffer

void
   wr_buff(const void*, size_t);    // Write into the write buffer

void
   wr_buff(const HOST16_t&);        // Write this size into write buffer

void
   wr_buff(const string&);          // Write string into write buffer

void
   wr_data(const void*, size_t);    // Unconditionally write this data

void
   wr_data(const string&);          // Unconditionally write this string

void
   wr_mode( void );                 // Go into write mode

void
   wr_path(const RdPath* path);     // Write path information

// Socket write
size_t                              // The number of bytes written
   wr_send(const void*, size_t);    // Send data
}; // class CommonThread
#endif // COMMONTHREAD_H_INCLUDED

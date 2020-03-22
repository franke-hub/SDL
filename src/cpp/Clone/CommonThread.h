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
//       CommonThread.h
//
// Purpose-
//       Base object for ListenThread, ClientThread, and ServerThread.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef COMMONTHREAD_H_INCLUDED
#define COMMONTHREAD_H_INCLUDED

#include <com/Semaphore.h>
#include <com/Socket.h>
#include <com/Thread.h>

#ifndef RDCOMMON_H_INCLUDED
#include "RdCommon.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       CommonThread
//
// Purpose-
//       CommonThread descriptor.
//
//----------------------------------------------------------------------------
class CommonThread : public Thread { // CommonThread descriptor
//----------------------------------------------------------------------------
// CommonThread::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum NFC                            // Notify Function Code
{  NFC_CLOSE                        // Terminate the CommonThread
,  NFC_FINAL                        // Program termination
,  NFC_COUNT                        // Number of functions
};

enum FSM                            // Finite State Machine
{  FSM_RESET                        // Reset, not started. Set in: constructor
,  FSM_READY                        // Ready, active       Set in: init
,  FSM_CLOSE                        // Terminating         Set in: notify
,  FSM_FINAL                        // Terminated          Set in: term
};

//----------------------------------------------------------------------------
// CommonThread::Global attributes
//----------------------------------------------------------------------------
public:
static Semaphore       semaphore;   // CommonThread completion semaphore
static int             threadCount; // The number of threadArray elements
static CommonThread**  threadArray; // The CommonThread array

//----------------------------------------------------------------------------
// CommonThread::Attributes
//----------------------------------------------------------------------------
protected:
int                    fsm;         // Finite State
Socket*                socket;      // Our working Socket
char*                  buffer;      // Socket I/O buffer, size MAX_TRANSFER

VersionInfo            gVersionInfo; // Global version information
VersionInfo            lVersionInfo; // Local  version information
VersionInfo            rVersionInfo; // Remote version information

//----------------------------------------------------------------------------
// CommonThread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~CommonThread( void );           // Destructor
   CommonThread(                    // Constructor
     Socket*           socket);     // Our working Socket

//----------------------------------------------------------------------------
// CommonThread::Accessors
//----------------------------------------------------------------------------
public:
inline char*                        // The buffer
   getBuffer( void ) const          // Get buffer
{  return buffer; }

inline int                          // The current state
   getFSM( void ) const             // Get current state
{  return fsm; }

inline const VersionInfo&           // The global version info
   getGVersionInfo( void ) const    // Get global version info
{  return gVersionInfo; }

inline const VersionInfo&           // The local  version info
   getLVersionInfo( void ) const    // Get local  version info
{  return lVersionInfo; }

inline const VersionInfo&           // The remote version info
   getRVersionInfo( void ) const    // Get remote version info
{  return rVersionInfo; }

virtual int                         // TRUE iff ListenThread
   isListenThread( void ) const     // Is this the ListenThread?
{  return FALSE; }

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::init
//
// Purpose-
//       Initialize the CommonThread.
//
//----------------------------------------------------------------------------
virtual void
   init( void );                    // Initialize the CommonThread

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::term
//
// Purpose-
//       Terminate the CommonThread.
//
//----------------------------------------------------------------------------
virtual void
   term( void );                    // Terminate the CommonThread

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::globallVersionInformation
//
// Purpose-
//       Set the global capability vector.
//
//----------------------------------------------------------------------------
virtual void
   globalVersionInformation( void ); // Initialize local version information

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::localVersionInformation
//
// Purpose-
//       Set the local capability vector.
//
//----------------------------------------------------------------------------
virtual void
   localVersionInformation( void ); // Initialize local version information

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::notify()
//
// Purpose-
//       CommonThread event notification.
//
//----------------------------------------------------------------------------
virtual int                         // Return code (unused)
   notify(                          // Notify this Thread
     int               code);       // Using this enum NFC

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::notifyAll()
//
// Purpose-
//       Notify/terminate/wait for all CommonThreads.
//
//----------------------------------------------------------------------------
static void
   notifyAll(                       // Notify all CommonThreads
     int               code);       // Using this enum NFC

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nRecv
//
// Purpose-
//       Receive from network.
//
//----------------------------------------------------------------------------
virtual unsigned                    // Number of bytes read
   nRecv(                           // Read from network
     void*             addr,        // Data address
     unsigned          size);       // Data length

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nRecvDirectory
//
// Purpose-
//       Receive a sorted directory.
//
//----------------------------------------------------------------------------
virtual DirList*                    // -> DirList
   nRecvDirectory(                  // Receive a sorted directory
     const char*       path);       // With this relative path

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nRecvString
//
// Purpose-
//       Receive string from network.
//
//----------------------------------------------------------------------------
virtual int                         // Number of bytes read
   nRecvString(                     // Read string from network
     void*             addr,        // Data address
     unsigned          size);       // Data length

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nRecvStruct
//
// Purpose-
//       Receive structure from network.
//
//----------------------------------------------------------------------------
virtual void
   nRecvStruct(                     // Read structure from network
     void*             addr,        // Data address
     unsigned          size);       // Data length

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nSend
//
// Purpose-
//       Send to network.
//
//----------------------------------------------------------------------------
virtual unsigned                    // Number of bytes sent
   nSend(                           // Send to network
     const void*       addr,        // Data address
     unsigned          size);       // Data length

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nSendDirectory
//
// Purpose-
//       Send a sorted directory.
//
//----------------------------------------------------------------------------
virtual void
   nSendDirectory(                  // Send a sorted directory
     DirList*          ptrA);       // -> DirList

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nSendString
//
// Purpose-
//       Send string to network
//
//----------------------------------------------------------------------------
virtual void
   nSendString(                     // Send string to network
     const void*       addr,        // Data address
     unsigned          size);       // Data length

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::nSendStruct
//
// Purpose-
//       Send structure to network.
//
//----------------------------------------------------------------------------
virtual void
   nSendStruct(                     // Send structure to network
     const void*       addr,        // Data address
     unsigned          size);       // Data length

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::status
//
// Purpose-
//       Display the status of all active CommonThread objects.
//
//----------------------------------------------------------------------------
static void
   status( void );                  // Display CommonThread status

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::wait
//
// Purpose-
//       Thread::wait, possibly compiled to add a debugging message.
//
//----------------------------------------------------------------------------
virtual long
   wait( void );                    // Wait for termination

//----------------------------------------------------------------------------
//
// Method-
//       CommonThread::waiter
//
// Purpose-
//       Wait for interuption (called from control thread.)
//
//----------------------------------------------------------------------------
virtual void
   waiter( void );                  // Wait for interruption
}; // class CommonThread

#endif // COMMONTHREAD_H_INCLUDED

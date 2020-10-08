//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_posix.cpp
//
// Purpose-
//       Posix library tests.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
// POSIX (gcc) library includes
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>

#include "built_in.h"
#include "Command.h"
#include "Exception.h"

//----------------------------------------------------------------------------
// Class: PosixSocket, using sys/socket.h implemenation
//----------------------------------------------------------------------------
class PosixSocket {
// Attributes
public:
static const int       CLOSED= -1;  // Closed socket handle
typedef int            Port;        // A port type

int                    sock;        // The socket handle
Port                   host_port;   // This socket's port number
Port                   peer_port;   // Peer socket's port number

// Destructor
public:
virtual
   ~PosixSocket( void )
{
   close();                         // Close the Socket
}

// Constructors
   PosixSocket( void )
:  sock(CLOSED)
{
}

// Methods
virtual void
   close( void )                    // Close the socket
{
   if( sock >= 0 ) {
     int rc= ::close(sock);
     if( rc != 0 )
       throw Exception(built_in::to_string("PosixSocket.close %d", rc));
   }

   sock= CLOSED;
}

size_t                              // The number of bytes received
   read(                            // Read from the socket
     char*             addr,        // Data address
     size_t            length)      // Maximum data length
     // May need socket options
{
   (void)addr; (void)length;        // Unused (Not implemented)
   throw NotImplementedException("PosixSocket::read");
   return 0;
}

void
   write(                           // Write to the socket
     const char*       addr,        // Data address
     size_t            length)      // Data length
     // May need socket options
{
   (void)addr; (void)length;        // Unused (Not implemented)
   throw NotImplementedException("PosixSocket::write");
}

// Internal methods (to be defined)
}; // class PosixSocket

//----------------------------------------------------------------------------
// Class: PosixThread, using pthread.h implemenation
//----------------------------------------------------------------------------
class PosixThread {
// Attributes
public:
bool                   daemon;      // Run the thread as a daemon
unsigned long          stack_size;  // The stack size
pthread_t              thread_id;   // The thread identifier

// Destructor
public:
virtual
   ~PosixThread( void )
{  // NO CHECKING: We do not have or check "running" state.
}  // We do not keep the thread attribute after start, so state cannot change.

// Constructors
   PosixThread( void )
:  daemon(false)
,  stack_size(0)
,  thread_id(0)
{
}

// Methods
virtual void
   join( void )                     // Wait for thread completion
{
   int rc= pthread_join(thread_id, NULL);
   assert( rc == 0 );
   thread_id= 0;
}

virtual void*                       // Resultant
   run( void ) = 0;                 // Operate the thread

virtual void
   start( void )                    // Start the thread
{
   pthread_attr_t      attr;
   int rc= pthread_attr_init(&attr);
   assert( rc == 0);
   int detach_state= (daemon ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE);
   pthread_attr_setdetachstate(&attr, detach_state);
   pthread_attr_setstacksize(&attr, stack_size);
   rc= pthread_create(&thread_id, &attr, driver, (void*)this);
   assert(rc == 0);
   pthread_attr_destroy(&attr);
}

// Internal methods
protected:
static inline void*                 // Resultant
   driver(                          // Operate
     void*             thread)      // This thread
{
   try {
     return ((PosixThread*)thread)->run();
   } catch(Exception& X) {
     printf("PosixThread.run %s\n", X.get_class_what().c_str());
   } catch(std::exception& X) {
     printf("PosixThread.run std::exception.what(%s)\n", X.what());
   } catch(...) {
     printf("PosixThread.run catch(...)\n");
   }

   return nullptr;
}
}; // class PosixThread

//----------------------------------------------------------------------------
// Class: ListenerThread
//----------------------------------------------------------------------------
class ListenerThread : public PosixThread {
   using PosixThread::PosixThread;

virtual void*                       // Resultant
   run( void )
{
   PosixSocket&        listener= *(new PosixSocket());
   printf("ListenerThread.run\n");
   listener.write("testing", strlen("testing"));
   listener.close();
   return NULL;
}
};

//----------------------------------------------------------------------------
// The 'dirty' command: For quick and dirty testing
//----------------------------------------------------------------------------
static inline int                   // Return code
   dirty(                           // The 'dirty' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   (void)argc; (void)argv;          // (Currently unused)
   return 1;
}

// INSTALL_COMMAND(dirty)

//----------------------------------------------------------------------------
// The 'bringup' command: An example bringup test using the posix library.
//----------------------------------------------------------------------------
static inline int                   // Return code
   bringup(                         // The 'bringup' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   (void)argc; (void)argv;          // (Currently unused)
   printf("bringup...\n");

   PosixThread& listener= *(new ListenerThread());
   listener.start();
   listener.join();

   printf("...bringup\n");
   return 0;
}

INSTALL_COMMAND(bringup)

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainline entry
     int             argc,          // Parameter count
     const char*     argv[])        // Parameter vector
{
   int result= 0;

   if( argc < 2 )
     result= bringup(0, NULL);
   else
     result= INSTALL_COMMAND_AT.main(argc, argv);

   return result;
}


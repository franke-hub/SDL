//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpServerThread.cpp
//
// Purpose-
//       HttpServerThread implementation methods.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <string>

#include <com/Barrier.h>
#include <com/Dispatch.h>           // For base class
#include <com/Reader.h>

#include "Common.h"
#include "Diagnostic.h"
#include "HttpSocketServer.h"

#include "HttpServerThread.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define SERVER_PORT 8080            // Our Server Port

enum FSM                            // Finite State Machine states
{  FSM_RESET                        // RESET
,  FSM_START                        // START, ready to run
,  FSM_READY                        // READY, operational
,  FSM_CLOSE                        // CLOSE, terminating
}; // enum FSM

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Notify Barrier

//----------------------------------------------------------------------------
//
// Class-
//       HttpServerTask
//
// Purpose-
//       The HttpServer Task Action Block
//
//----------------------------------------------------------------------------
class HttpServerTask : public DispatchTask { // HttpServer Task
//----------------------------------------------------------------------------
// HttpServerTask::Attributes
//----------------------------------------------------------------------------
protected:
Socket*                socket;      // Connection Socket

//----------------------------------------------------------------------------
// HttpServerTask::Constructor
//----------------------------------------------------------------------------
public:
virtual
   ~HttpServerTask( void );         // Destructor
   HttpServerTask(                  // Constructor
     Socket*           socket);     // Associated Socket

//----------------------------------------------------------------------------
// HttpServerTask::Methods
//----------------------------------------------------------------------------
public:
virtual void
   work(                            // Process work Item
     DispatchItem*     item);       // The work Item
}; // class HttpServerTask

//----------------------------------------------------------------------------
//
// Class-
//       HttpServerItem
//
// Purpose-
//       The HttpServer work Item, container for HttpServerTask
//
//----------------------------------------------------------------------------
class HttpServerItem : public DispatchItem { // HttpServer work Item
//----------------------------------------------------------------------------
// HttpServerItem::Attributes
//----------------------------------------------------------------------------
protected:
HttpServerTask         task;         // Our HttpServerTask

//----------------------------------------------------------------------------
// HttpServerItem::Constructor
//----------------------------------------------------------------------------
public:
   ~HttpServerItem( void );         // Destructor
   HttpServerItem(                  // Constructor
     Socket*           socket);     // Associated Socket

//----------------------------------------------------------------------------
// HttpServerItem::Accessors
//----------------------------------------------------------------------------
public:
inline DispatchTask*                // Our HttpServerTask
   getTask( void )                  // Get HttpServerTask
{  return &task;
}
}; // class HttpServerItem

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerItem::~HttpServerItem
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   HttpServerItem::~HttpServerItem( void ) // Destructor
{
   IFHCDM( logf("HttpServerItem(%p)::~HttpServerItem()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerItem::HttpServerItem
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   HttpServerItem::HttpServerItem(  // Constructor
     Socket*           socket)      // Associated Socket
:  DispatchItem(DispatchItem::FC_VALID, NULL)
,  task(socket)
{
   IFHCDM( logf("HttpServerItem(%p)::HttpServerItem(%p)\n", this, socket); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerTask::~HttpServerTask
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   HttpServerTask::~HttpServerTask( void ) // Destructor
{
   IFHCDM( logf("HttpServerTask(%p)::~HttpServerTask()\n", this); )

   socket->close();
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerTask::HttpServerTask
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   HttpServerTask::HttpServerTask(  // Constructor
     Socket*           socket)      // Associated Socket
:  DispatchTask()
,  socket(socket)
{
   IFHCDM( logf("HttpServerItem(%p)::HttpServerItem()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerTask::work
//
// Purpose-
//       Process work Item
//
//----------------------------------------------------------------------------
void
   HttpServerTask::work(            // Process Work
     DispatchItem*     item)        // The work Item
{
   IFSCDM( logf("HttpServerTask(%p)::work(%p)...\n", this, item); )

   HttpServerItem* serverItem= dynamic_cast<HttpServerItem*>(item);
   if( serverItem == NULL )
   {
     logf("%d %s SHOULD NOT OCCUR\n", __LINE__, __FILE__);
     item->post(item->CC_ERROR);
     return;
   }

   try {
     HttpSocketServer server(socket); // Our HttpSocketServer

     for(;;)
     {
       int cc= server.work();
       if( cc )
         break;
     }
   } catch(const char* X) {
     logf("HttpServerTask::work catch(const char(%s))\n", X);
   } catch(std::exception& X) {
     logf("HttpServerTask::work catch(exception.what(%s))\n", X.what());
   } catch(...) {
     logf("HttpServerTask::work catch(...)\n");
   }

   item->setFC(DispatchItem::FC_RESET);
   Common::get()->dispatcher.enqueue(this, item);

   #ifdef HCDM
     logf("...HttpServerTask(%p)::work(%p)\n", this, item);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerThread::~HttpServerThread
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpServerThread::~HttpServerThread( void )
{
   logf("HttpServerThread(%p)::~HttpServerThread()\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerThread::HttpServerThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HttpServerThread::HttpServerThread( void )
:  NamedThread("HTTP::Server")
,  fsm(FSM_RESET)
,  listen(Socket::ST_STREAM)
{
   logf("HttpServerThread(%p)::HttpServerThread()\n", this);

   fsm= FSM_START;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerThread::notify
//
// Purpose-
//       Termination notification.
//
//----------------------------------------------------------------------------
int                                 // Return code
   HttpServerThread::notify(        // Notify HttpServerThread
     int               code)        // Notification code
{
   logf("HttpServerThread(%p)::notify(%d)\n", this, code);

   AutoBarrier lock(barrier);       // Single thread mode
   if( fsm != FSM_READY )
     fsm= FSM_RESET;
   else
   {
     fsm= FSM_CLOSE;
     Socket socket;
     socket.connect(getAddr(), getPort());
     sleep(0.125);
     socket.close();
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerThread::run
//
// Purpose-
//       HttpServer driver.
//
//----------------------------------------------------------------------------
long                                // Return code
   HttpServerThread::run( void )    // HttpServer driver
{
   logf("HttpServerThread(%p)::run\n", this);
   Common& common= *Common::get();

   {{{{
     AutoBarrier lock(barrier);     // Single thread mode

     if( fsm == FSM_START )
     {
       if( listen.setHost(0, SERVER_PORT) == 0 )
         fsm= FSM_READY;
       else
       {
         debugf("%4d %s: Unable to setHost, EI(%s)\n", __LINE__, __FILE__,
                listen.getSocketEI());
         fsm= FSM_RESET;
       }
     }

     if( fsm != FSM_READY )
       return 1;
   }}}}

   char buffer[32];
   strcpy(buffer, Socket::addrToChar(listen.getHostAddr()));
   printf("Server: http://%s:%d\n", buffer, listen.getHostPort());
     logf("Server: http://%s:%d\n", buffer, listen.getHostPort());

   for(;;)
   {
     Socket* socket= listen.listen();
     if( fsm != FSM_READY )
     {
       if( socket != NULL )
         socket->close();

       break;
     }

     //-----------------------------------------------------------------------
     // Check for NULL socket. This can occur when an alarm timer expires.
     // As long as we think we're active, we ignore the condition.
     if( socket == NULL )
     {
       if( fsm != FSM_READY )
         break;

       logf("%4d %s SOCKET == NULL\n", __LINE__, __FILE__);
       continue;
     }

     HttpServerItem* item= new HttpServerItem(socket);
     common.dispatcher.enqueue(item->getTask(), item);
   }

   {{{{
     AutoBarrier lock(barrier);     // Single thread mode

     listen.close();
     fsm= FSM_RESET;
   }}}}

   logf("HttpServerThread(%p)::terminated\n", this);
   return 0;
}


//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Bringup.cpp
//
// Purpose-
//       Wilbur bringup component tests.
//
// Last change date-
//       2014/01/01
//
// Controls-
//       If the first parameter is not a switch parameter, it specifies the
//       log file name (and sets intensive debug mode.)
//
//       --test            (Sleep 60 seconds)
//       --testDispatcher  (Test com/Dispatch.h)
//       --testHttpCached  (Test HttpCached.h)
//       --testHttpSource  (Test HttpSource.h)
//       --testNetClient   (Test NetClient.h)
//       --testTimers      (Test Dispatch::DispatchTimers)
//
//----------------------------------------------------------------------------
#include <exception>

#include <string.h>

#include <com/Clock.h>
#include <com/DataSource.h>
#include <com/Debug.h>
#include <com/Exception.h>
#include <com/Thread.h>             // For Thread::sleep

#include "Common.h"
#include "DbMeta.h"
#include "HttpCached.h"
#include "HttpSource.h"
#include "NetClient.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DEL_COUNT 5                 // Number of Delay loops
#define TASK_COUNT 5                // Number of MyTask blocks
#define USE_WAIT_FOR_NETCLIENT_TIMEOUT 0 // Verify NetClientTask timeout?

#define HTTP_ADDR "localhost:8080/"

//----------------------------------------------------------------------------
//
// Subroutine-
//       listDataSource
//
// Purpose-
//       Display DataSource content.
//
//----------------------------------------------------------------------------
static void
   listDataSource(                  // List DataSource content
     DataSource&       source)      // For this DataSource
{
   int cc= Debug::obtain();
   tracef("listDataSource(%s)\n", source.getName().c_str());
   for(;;)
   {
     int C= source.get();
     if( C < 0 )
       break;

     if( C != '\r' )
       tracef("%c", C);
   }

   if( cc == 0 )
     Debug::release();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testDispatcher
//
// Purpose-
//       Dispatcher bringup test
//
//----------------------------------------------------------------------------
class MyTask : public DispatchTask { // Work processor
public:
virtual
   ~MyTask( void )
{
   logf("MyTask(%p)::~MyTask()\n", this);
}

   MyTask( void )
:  DispatchTask()
{
   logf("MyTask(%p)::MyTask()\n", this);
}

virtual void
   work(
     DispatchItem*     item)
{
   Common&             common= *Common::get();

   logf("MyTask(%p).work(%p) fc(%d)\n", this, item, item->getFC());
   for(int i= 0; i<DEL_COUNT; i++)
   {
     int delay= common.random.modulus(2500);
     Thread::sleep((double)delay/1000.0);
     logf("MyTask(%p).work(%p) delay(%6d)\n", this, item, delay);
   }

   logf("MyTask(%p).work(%p) done\n", this, item);
   item->post(0);
}
}; // class MyTask

static inline void
   testDispatcher( void )
{
   Common&             common= *Common::get();

   MyTask              myTask[TASK_COUNT];
   DispatchItem        item[TASK_COUNT];
   DispatchWait        wait[TASK_COUNT];

   for(int i= 0; i<TASK_COUNT; i++)
   {
     logf("%d %s TIME\n", __LINE__, __FILE__);
     item[i].setFC(i+1);
     item[i].setDone(&wait[i]);
     common.dispatcher.enqueue(&myTask[i], &item[i]);
   }

   for(int i= 0; i<TASK_COUNT; i++)
   {
     logf("%d %s TIME\n", __LINE__, __FILE__);
     wait[i].wait();
   }

   for(int i= 0; i<TASK_COUNT; i++)
   {
     logf("%d %s TIME\n", __LINE__, __FILE__);
     wait[0].reset();
     DispatchItem reset(DispatchItem::FC_RESET, &wait[0]);

     common.dispatcher.enqueue(&myTask[i], &reset);
     wait[0].wait();
   }

   logf("%d %s TIME\n", __LINE__, __FILE__);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testHttpCached
//
// Purpose-
//       Test HttpCached.h
//
//----------------------------------------------------------------------------
static void
   testHttpCached( void )           // Test HttpCached.h
{
   HttpCached          httpCached;  // Test object

   httpCached.setNullTimeout(7*24*60*60);

   int rc= httpCached.open(HTTP_ADDR "robots.txt");
   tracef("\n\n%d= httpCached.open(~/robots.txt)\n", rc);
   listDataSource(httpCached);
   httpCached.close();

   rc= httpCached.open(HTTP_ADDR "index.html");
   tracef("\n\n%d= httpCached.open(~/index.html)\n", rc);
   listDataSource(httpCached);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testHttpSource
//
// Purpose-
//       Test HttpSource object.
//
//----------------------------------------------------------------------------
static void
   testHttpSource( void )           // Test HttpSource object
{
   HttpSource          httpSource;  // Test object

   int rc= httpSource.open(HTTP_ADDR "index.html");
   tracef("\n\n%d= httpSource.open(~/index.html)\n", rc);
   listDataSource(httpSource);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testNetClient
//
// Purpose-
//       Test NetClient.h
//
//----------------------------------------------------------------------------
static void
   testNetClient( void )            // Test NetClient.h
{
   Common&             common= *Common::get();
   DispatchTask*       netClient= common.netClient;
   NetClientItem       netClientItem; // Test object
   DispatchWait        wait;        // Wait object

   netClientItem.setDone(&wait);

   //-------------------------------------------------------------------------
   // Read/display index.html
   wait.reset();
   netClientItem.url.setURI(HTTP_ADDR "index.html");
   common.dispatcher.enqueue(netClient, &netClientItem);
   wait.wait();

   tracef("\n\n%d= netClientItem(~/index.html)\n", netClientItem.rc);
   listDataSource(netClientItem.data);

   //-------------------------------------------------------------------------
   // (OPTIONAL) Wait for timeout
   if( USE_WAIT_FOR_NETCLIENT_TIMEOUT )
   {
     debugf("Waiting for NETCLIENT timeout...\n");
     Thread::sleep(15.0);
     debugf("...Done\n");
   }

   //-------------------------------------------------------------------------
   // Read/display input.html
   wait.reset();
   netClientItem.url.setURI(HTTP_ADDR "input.html");
   common.dispatcher.enqueue(netClient, &netClientItem);
   wait.wait();

   tracef("\n\n%d= netClientItem(~/input.html)\n", netClientItem.rc);
   listDataSource(netClientItem.data);

   //-------------------------------------------------------------------------
   // Read/display non-existent.html
   wait.reset();
   netClientItem.url.setURI(HTTP_ADDR "non-existent.html");
   common.dispatcher.enqueue(netClient, &netClientItem);
   wait.wait();

   tracef("\n\n%d= netClientItem(~/non-existent.html)\n", netClientItem.rc);
   listDataSource(netClientItem.data);

   //-------------------------------------------------------------------------
   // Read/display the forbidden forbidden.html
   wait.reset();
   netClientItem.url.setURI(HTTP_ADDR "forbidden.html");
   common.dispatcher.enqueue(netClient, &netClientItem);
   wait.wait();

   tracef("\n\n%d= netClientItem(~/forbidden.html)\n", netClientItem.rc);
   listDataSource(netClientItem.data);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testSleep
//
// Purpose-
//       Test sleep in main thread.
//
//----------------------------------------------------------------------------
static inline void
   testSleep(                       // Test main thread sleep
     int               delay)       // Sleep time
{
   Thread::sleep(delay);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testTimers
//
// Purpose-
//       DispatchTimers bringup test
//
//----------------------------------------------------------------------------
static inline void
   testTimers( void )
{
   Common&             common= *Common::get();

   enum
   {  SIZE= 6                        // Number of items
   ,  INVERTED= 1                    // TRUE for inverted times
   }; // enum

   logf("%d %s testTimers...\n", __LINE__, __FILE__);
   DispatchItem        item[SIZE];
   DispatchWait        wait[SIZE];

   for(int i= 0; i<SIZE; i++)
     item[i].setDone(&wait[i]);

   for(int i= 0; i<SIZE; i++)
   {
     logf("%d %s [%2d] testTimers delay(%p)\n", __LINE__, __FILE__,
          i, wait + i);
     if( INVERTED )                 // TRUE for reverse times
       common.dispatcher.delay(SIZE - i + 0.5, &item[i]);
     else
       common.dispatcher.delay(i + 0.5, &item[i]);

     Thread::yield();               // Allow DispatchTimers to run
   }

   for(int i= 0; i<SIZE; i++)
   {
     if( INVERTED )
     {
       logf("%d %s [%2d] testTimers wait(%p)\n", __LINE__, __FILE__,
            SIZE - i - 1, wait + SIZE - i - 1);
       wait[SIZE - i - 1].wait();
     }
     else
     {
       logf("%d %s [%2d] testTimers wait(%p)\n", __LINE__, __FILE__,
            i, wait + i );
       wait[i].wait();
     }
   }

   // Test timer cancel (CANCELLED)
   debugf("%d %s testTimers cancel...\n", __LINE__, __FILE__);
   wait[0].reset();
   logf("%d %s testTimers TIME\n", __LINE__, __FILE__);
   double start= Clock::current();

   void* token= common.dispatcher.delay(10.0, &item[0]);
   Thread::sleep(1.001);
   common.dispatcher.cancel(token);
   wait[0].wait();
   double elapsed= Clock::current() - start;
   logf("%d %s testTimers TIME\n", __LINE__, __FILE__);

   if( item[0].getCC() != DispatchItem::CC_ERROR )
     debugf("%d %s ERROR, completion code(%d)\n", __LINE__, __FILE__,
            item[0].getCC());

   if( elapsed < 1.0 )
     debugf("%d %s INTERNAL ERROR(%e)\n", __LINE__, __FILE__, elapsed);
   if( elapsed > 2.0 )
     debugf("%d %s Cancel delay too long(%8.3f)\n", __LINE__, __FILE__, elapsed);

   // Test timer cancel (No effect)
   wait[0].reset();
   logf("%d %s testTimers TIME\n", __LINE__, __FILE__);
   start= Clock::current();

   token= common.dispatcher.delay(1.0, &item[0]);
   Thread::sleep(2.001);
   common.dispatcher.cancel(token);
   wait[0].wait();
   elapsed= Clock::current() - start;
   logf("%d %s testTimers TIME\n", __LINE__, __FILE__);

   if( item[0].getCC() != DispatchItem::CC_NORMAL )
     debugf("%d %s ERROR, completion code(%d)\n", __LINE__, __FILE__,
            item[0].getCC());

   if( elapsed < 2.0 )
     debugf("%d %s INTERNAL ERROR(%e)\n", __LINE__, __FILE__, elapsed);
   if( elapsed > 3.0 )
     debugf("%d %s Cancel delay too long(%8.3f)\n", __LINE__, __FILE__, elapsed);

   debugf("%d %s ...testTimers cancel\n", __LINE__, __FILE__);
   logf("%d %s ...testTimers\n", __LINE__, __FILE__);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       usage
//
// Purpose-
//       Write usage information
//
//----------------------------------------------------------------------------
static void
   usage( void )                    // Write usage information
{
   printf("Bringup <option>\n"
          "--test            (Test for 60 seconds)\n"
          "--testDispatcher  (Test com/Dispatch.h)\n"
          "--testHttpCached  (Test HttpCached.h)\n"
          "--testHttpSource  (Test HttpSource.h)\n"
          "--testNetClient   (Test NetClient.h)\n"
          "--testTimers      (Test DispatchTimers.h)\n"
          );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Common*             common= NULL;// Wilbur common area

   //-------------------------------------------------------------------------
   // Set logFile
   //-------------------------------------------------------------------------
   remove("debug.out");             // Logfile appends
   const char* logFile= NULL;       // Default log file name
   int arg1= 1;                     // Default first argument
   if( argc > arg1 && *argv[arg1] != '-' ) // If filename parameter
     logFile= argv[arg1++];         // Set the filename parameter

// logFile= "2>";                   // Extreme HCDM, logFile= stderr

   //-------------------------------------------------------------------------
   // Operate Bringup
   //-------------------------------------------------------------------------
   try {
     printf("Starting Bringup...\n");
     common= Common::activate(logFile); // Access Common area
     if( logFile != NULL )
       debugSetIntensiveMode();       // (Allow tail -f debug.out)

     logf("... Bringup READY ...\n");

     for(int argx= arg1; argx<argc; argx++)
     {
       if( strcmp(argv[argx], "--help") != 0 )
         logf("Bringup %s\n", argv[argx]);

       if( strcmp(argv[argx], "--help") == 0 )
         usage();
       else if( strcmp(argv[argx], "--test") == 0 )
         testSleep(60);
       else if( strcmp(argv[argx], "--testDispatcher") == 0 )
         testDispatcher();
       else if( strcmp(argv[argx], "--testHttpCached") == 0 )
         testHttpCached();
       else if( strcmp(argv[argx], "--testHttpSource") == 0 )
         testHttpSource();
       else if( strcmp(argv[argx], "--testNetClient") == 0 )
         testNetClient();
       else if( strcmp(argv[argx], "--testTimers") == 0 )
         testTimers();
       else
       {
         usage();
         break;
       }
     }

     common->shutdown();            // Initiate shutdown
     common->finalize();            // Wait for shutdown completion
     printf("...Bringup Complete\n");
   } catch(const char* X) {
     logf("catch(const char*(%s))\n", X);
   } catch(Exception& X) {
     logf("catch(Exception.what(%s))\n", X.what());
   } catch(std::exception& X) {
     logf("catch(exception.what(%s))\n", X.what());
   } catch(...) {
     logf("catch(...)\n");
   }

   return 0;
}


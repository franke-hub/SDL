//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_MP.cpp
//
// Purpose-
//       Skeleton multi-processor, multi-thread testcase.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <new>
#include <assert.h>
#include <limits.h>                 // For INT_MAX
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>                 // For atoi, malloc
#include <string.h>
#include <unistd.h>                 // For fork, sleep

#ifdef _OS_WIN
  #include <direct.h>
  #define getpid _getpid
#endif

#include <com/Barrier.h>
#include <com/Clock.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/FileName.h>
#include <com/Hardware.h>
#include <com/Latch.h>
#include <com/Process.h>
#include <com/Service.h>
#include <com/SharedMem.h>
#include <com/Signal.h>
//nclude <com/Socket.h>
#include <com/Thread.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEST_MP " // Source file name, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#undef  SERVICE_HCDM
#define SERVICE_HCDM(word) SERVICE_INFO(word)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define TEST_SECONDS           (10) // Number of seconds to test

#define MAX_PROCESSES         (100) // Maximum number of processes
#define MAX_THREADS           (100) // Maximum number of threads per process
//efine MAX_SOCKETS           (100) // Maximum number of sockets per thread

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define FTOK_ID           487987345 // Random identifier

//----------------------------------------------------------------------------
//
// Class-
//       MySignal
//
// Purpose-
//       Define a Signal object.
//
//----------------------------------------------------------------------------
class MySignal : public Signal
{
//----------------------------------------------------------------------------
// MySignal::Constructors/Destructors
//----------------------------------------------------------------------------
public:
virtual
   ~MySignal( void ) {}
   MySignal(
     int             pid,
     int             tid)
:  pid(pid)
,  tid(tid)
{
}

//----------------------------------------------------------------------------
// MySignal::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 iff COMPLETELY handled)
   handle(                          // Handle a signal
     SignalCode      signal);       // The signal to handle

//----------------------------------------------------------------------------
// MySignal::Attributes
//----------------------------------------------------------------------------
private:
   int               pid;
   int               tid;
}; // class MySignal

//----------------------------------------------------------------------------
//
// Class-
//       MyThread
//
// Purpose-
//       Define a Thread object.
//
//----------------------------------------------------------------------------
class MyThread : public Thread
{
//----------------------------------------------------------------------------
// MyThread::Constructors/Destructors
//----------------------------------------------------------------------------
public:
virtual
   ~MyThread( void ) {}
   MyThread(
     int             pid,
     int             tid)
:  pid(pid)
,  tid(tid)
{
}

//----------------------------------------------------------------------------
// MyThread::Methods
//----------------------------------------------------------------------------
public:
void
   debug(                           // Debug Thread
     int            line,           // Caller's line number
     int            size);          // Sizeof stack dump

virtual long
   run( void );

//----------------------------------------------------------------------------
// MyThread::Attributes
//----------------------------------------------------------------------------
private:
   int               pid;
   int               tid;
}; // class MyThread

//----------------------------------------------------------------------------
//
// Struct-
//       PerThread
//
// Purpose-
//       Define the per thread storage region.
//
//----------------------------------------------------------------------------
struct PerThread
{
// Latch             latch;         // Thread Latch

   unsigned          operations;    // Number of operations
   MyThread*         thread;        // -> Associated Thread
// Socket*           socket[MAX_SOCKETS]; // -> Socket array
};

//----------------------------------------------------------------------------
//
// Struct-
//       PerProcess
//
// Purpose-
//       Define the per process common storage region.
//
//----------------------------------------------------------------------------
struct PerProcess
{
// Latch             latch;         // Process latch

enum State
{
   PS_INITIAL,                      // Initial
   PS_OPERATIONAL                   // Operational
};
   State             fsm;           // Thread state

   Clock             after;         // Time last operation completed
   unsigned long     operations;    // Number of operations
   int               status;        // Completion status

   Process*          process;       // -> Associated Process
   PerThread         thread[MAX_THREADS]; // Thread storage region
};

//----------------------------------------------------------------------------
//
// Struct-
//       Common
//
// Purpose-
//       Define common storage region (testcase communication)
//
//----------------------------------------------------------------------------
struct Common
{
   char              ident[8];      // Identifier
// Latch             latch;         // Common latch

   Clock             initialTime;   // Initial time
   Clock             startTime;     // Startup time
   Clock             finisTime;     // Completion time
   Clock             finisWindow;   // Completion time window

   PerProcess        process[MAX_PROCESSES]; // Process information area
};

//----------------------------------------------------------------------------
//
// Struct-
//       Global
//
// Purpose-
//       Define global storage region (independent of testcase name)
//
//----------------------------------------------------------------------------
struct Global
{
   char              ident[8];      // Identifier
   Latch             latch;         // Global latch
};

//----------------------------------------------------------------------------
// Local data areas -- available to each process
//----------------------------------------------------------------------------
static Common*       common;        // Common storage region
static Global*       global;        // Common storage region

static char*         buffer;        // Local  storage region
static unsigned      buffSize;      // Local  storage region size
static char          parmString[1024]; // Parameter string save area

static Barrier       barrier= BARRIER_INIT; // Thread latch
static int           startupDelay;  // Startup time delay, in seconds
static int           testTime;      // Test time, in seconds
static int           verbose;       // Verbosity

static int           parmPid;       // Process identifier (parameter)
static int           parmTid;       // Thread identifier (parameter)

static int           pCount;        // Number of processes to create
static int           tCount;        // Number of threads per process
//atic int           sCount;        // Number of sockets per thread

//----------------------------------------------------------------------------
//
// Subroutine-
//       prepend
//
// Purpose-
//       Print from process to stderr, prepending the process identifier.
//
// Notes-
//       The string is printed with one operation to avoid inter-process and
//       inter-thread mixing.
//
//----------------------------------------------------------------------------
static void
   prepend(                         // Process debugf(...)
     int             pid,           // Process identifier
     const char*     fmt,           // The format string
                     ...)           // Variable arguments
{
   Clock             nowTime;       // Current time
   char              fmtString[256];// Message formatting area
   char              msgString[512];// Message formatting area
   va_list           argptr;        // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vsprintf(fmtString, fmt, argptr);// Format the string
   va_end(argptr);                  // Close va_ functions

   nowTime= Clock::current();
   sprintf(msgString, "[%3d] %12.2f %s",
                      pid, (double)nowTime, fmtString);
   debugf("%s", msgString);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       stackOffset
//
// Purpose-
//       Get the stack offset.
//
//----------------------------------------------------------------------------
static inline int                   // The stack offset (Currently unused)
   stackOffset( void )              // Get the stack offset
{
   void*             stack;         // Stack pointer

   stack= Hardware::getSP();        // Get the Stack pointer
   return (int)((char*)&stack-(char*)stack);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       doThread
//
// Purpose-
//       Run test under control of a Thread.
//
//----------------------------------------------------------------------------
static int                          // Completion Code
   doThread(                        // Thread function
     int             pid,           // Process identifier
     int             tid)           // Thread identifier
//   MyThread*       thread)        // -> Thread object
{
   Global*           G= global;
   Common*           C= common;
   PerThread*        T= &C->process[pid].thread[tid];
   double            const finTime= (double)C->finisTime;

   Clock             after;         // Time last operation completed
   int               operations;    // Number of operations

   #ifdef HCDM
     debugf("%s %4d: doThread(%d,%d) started\n", __SOURCE__, __LINE__,
            pid, tid);
   #endif

   SERVICE_INFO((tid<<16) + pid);

   if( strcmp(C->ident, "COMMON") != 0 )
   {
     debugf("COMMON not initialized\n");
     exit(EXIT_FAILURE);
   }

   if( strcmp(G->ident, "GLOBAL") != 0 )
   {
     debugf("GLOBAL not initialized\n");
     exit(EXIT_FAILURE);
   }

   if( verbose > 2 )
     prepend(pid, "[%3d] Thread started\n", tid);

   operations= 0;
   #if 0
     if( pid == 0 && tid == 0 )
     {
       #if 1
         G.latch.obtainXCL();
         prepend(pid, "Tid(%d) XCL obtained, sleep\n", tid);
         Thread::sleep(TEST_SECONDS);
         G.latch.releaseXCL();
         prepend(pid, "Tid(%d) XCL released\n", tid);
       #else
         G.latch.obtainSHR();
         prepend(pid, "Tid(%d) SHR obtained, sleep\n", tid);
         Thread::sleep(TEST_SECONDS);
         G.latch.releaseSHR();
         prepend(pid, "Tid(%d) SHR released\n", tid);
       #endif
     }
   #endif

   #if 1
     for (operations=0; ;operations++)
     {
       after= Clock::current();
       if( (double)after >= finTime )
         break;

       #if 1
         G->latch.obtainXCL();
         G->latch.releaseXCL();
       #endif

       #if 1
         G->latch.obtainSHR();
         G->latch.releaseSHR();
       #endif

       #if 1
         Thread::yield();
       #endif
     }
   #endif

   #if 0                            // CYGWIN fault: test_mp 1 3
     if( tid == 1 )
       Thread::sleep(1);
   #endif

   if( verbose > 2 )
     prepend(pid, "[%3d] Thread complete(%d)\n", tid, operations);

   T->operations= operations;

   #ifdef HCDM
     debugf("%s %4d: doThread(%d,%d) complete\n", __SOURCE__, __LINE__,
            pid, tid);
   #endif

   SERVICE_INFO((tid<<16) + pid);
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       MySignal::handle
//
// Purpose-
//       Handle a Signal.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 iff COMPLETELY handled)
   MySignal::handle(                // Handle a signal
     SignalCode      signal)        // The signal to handle
{
   debugf("[%3d] Signal(%d) '%s' received\n", pid,
          signal, Signal::getSignalName(signal));
   return 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       MyThread::debug
//
// Purpose-
//       Thread debugging.
//
// Notes-
//       The stack will also contain this routine's frame.
//
//----------------------------------------------------------------------------
void
   MyThread::debug(                 // Debugging
     int             line,          // Caller's line number
     int             size)          // Sizeof stack dump
{
   void*             stack= Hardware::getSP();

   AutoBarrier lock(barrier);
   {
     tracef("%4d: MyThread(%p)::debug() P(%d) T(%d) Stack(%p)\n", line,
            this, pid, tid, stack);
     tracef("Thread\n"); dump(this, sizeof(MyThread));
     tracef("Stack\n");  dump(stack, size);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       MyThread::run
//
// Purpose-
//       Run my Thread.
//
// Notes-
//       CYGWIN certification failure:
//
//       The compiler generates a call to ___get_eh_context in doThread
//       if any constructors are called.
//       This routine is not thread-safe, and causes faults on exit.
//
//----------------------------------------------------------------------------
long
   MyThread::run( void )
{
   int               returncd;

   SERVICE_INFO((tid<<16) + pid);
   returncd= doThread(pid, tid);
   SERVICE_INFO((tid<<16) + pid);

   return returncd;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       synchro
//
// Purpose-
//       Insure that all processes start pretty much at the same time.
//
//----------------------------------------------------------------------------
static int                          // TRUE if valid
   synchro( void )                  // Synchronize
{
   Clock             newTime;
   double            delTime;

   newTime= Clock::current();
   delTime= (double)common->startTime - (double)newTime;
   if( delTime < 0.0 )
     return FALSE;

   #ifdef HCDM
     debugf("%s %4d: synchro(%11.9f)\n", __SOURCE__, __LINE__, delTime);
   #endif
   Thread::sleep(delTime);
   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Diagnostic exit.
//
//----------------------------------------------------------------------------
static void                         // (DOES NOT RETURN)
   info( void )                     // Informational exit
{
   fprintf(stderr, "Usage: " __SOURCE__
                   " [-D:seconds] [-R:seconds] [-V:verbosity]\n"
                   " processCount (3)\n"
                   " [threadsPerProcess (3)"
////               " [socketsPerThread (3)"
////               " [messageLength (1000)"
                   "]]]\n"
          );
   fprintf(stderr, " -D (Startup delay)\n");
   fprintf(stderr, " -R (Test Run time)\n");
   fprintf(stderr, " -V (Diagnostic verbosity)\n");

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,
     char*           argv[])
{
   int               error= FALSE;  // Error encountered indicator
   int               argi;          // Argument index
   char*             argp;          // Argument pointer
   int               parmError;     // Positional error
   int               pindex;        // Positional index

   //-------------------------------------------------------------------------
   // Parameter defaults
   //-------------------------------------------------------------------------
   pCount=       3;
   tCount=       3;
// sCount=       3;
   buffSize=     1000;
   parmPid=      (-1);
   parmTid=      (-1);
   parmString[0]= '\0';
   startupDelay= (-1);
   testTime=     (-1);
   verbose=      1;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   pindex= 1;
   for(argi=1; argi<argc; argi++)
   {
     argp= argv[argi];              // Address the parameter
     parmError= 0;                  // No error encountered

     if( *argp == '-' )             // If switch parameter
     {
       argp++;                      // Skip the switch parameter

       if( *argp == 'D' )           // If startup delay
       {
         argp++;
         if( *argp != ':' )
           parmError= 2;
         else
         {
           argp++;
           startupDelay= atoi(argp);
         }
       }

       else if( *argp == 'R' )      // If test time
       {
         argp++;
         if( *argp != ':' )
           parmError= 2;
         else
         {
           argp++;
           testTime= atoi(argp);
         }
       }

       else if( *argp == 'V' )      // If verbosity
       {
         argp++;
         if( *argp != ':' )
           parmError= 2;
         else
         {
           argp++;
           verbose= atoi(argp);
         }
       }

       else if( *argp == ':' )      // If process identifier
       {
         argp++;
         parmPid= atoi(argp);
       }

       else                         // Undefined parameter
       {
         fprintf(stderr, "Undefined parameter(-%s)\n", argp);
         error= TRUE;
       }
     }
     else                           // If positional parameter
     {
       switch(pindex)               // Process positional parameter
       {
         case 1:
            pCount= atoi(argp);
            if( pCount > MAX_PROCESSES )
            {
              fprintf(stderr,"processCount(%d) bigger than limit(%d)\n",
                             pCount, MAX_PROCESSES);
              error= TRUE;
            }
            break;

         case 2:
            tCount= atoi(argp);
            if( tCount > MAX_THREADS )
            {
              fprintf(stderr,"threadCount(%d) bigger than limit(%d)\n",
                             tCount, MAX_THREADS);
              error= TRUE;
            }
            break;

//       case 3:
//          sCount= atoi(argp);
//          if( sCount > MAX_SOCKETS )
//          {
//            fprintf(stderr,"socketCount(%d) bigger than limit(%d)\n",
//                           sCount, MAX_SOCKETS);
//            error= TRUE;
//          }
//          break;

//       case 4:
//          buffSize= atoi(argp);
//          if( buffSize < 1 )
//          {
//            fprintf(stderr, "Invalid buffer size(%ld)\n", (long)buffSize);
//            error= TRUE;
//          }
//          break;

         default:
           parmError= 1;
       }

       pindex++;
     }

     switch(parmError)              // Process positional error
     {
       case 1:                      // Too many positional parameters
         fprintf(stderr, "Too many positional parmameters(%s)\n", argv[argi]);
         error= TRUE;
         break;

       case 2:                      // Invalid parameters
         fprintf(stderr, "Invalid parameter(%s)\n", argv[argi]);
         error= TRUE;
         break;

       default:
         strcat(parmString, " ");
         strcat(parmString, argv[argi]);
         break;
     }
   }

   //-------------------------------------------------------------------------
   // Consistency checks
   //-------------------------------------------------------------------------
   buffer= (char*)malloc(buffSize+8);
   {
     if( buffer == NULL )
     {
       fprintf(stderr, "Buffer size(%ld) too big\n", (long)buffSize);
       error= TRUE;
     }
   }
   *(short*)buffer= buffSize;
   memset(buffer+2, 'B', buffSize);

   if( error )
     info();

   if( startupDelay < 0 )           // If invalid or not initialized
   {
     startupDelay= pCount / 4;      // Default startup delay
     if( startupDelay < 5 )         // If too small
       startupDelay= 5;             // Minimum startup delay
   }

   if( testTime < 0 )               // If invalid or not initialized
   {
     testTime= TEST_SECONDS;        // Minimum test time
     if( testTime < (pCount / 2) )
       testTime= pCount / 2;
   }

   //-------------------------------------------------------------------------
   // Debugging
   //-------------------------------------------------------------------------
   #if defined(HCDM)
   {{{{
     char            string[128];   // Working string

     strcpy(string, "debug.out");
     if( parmPid >= 0 )
       sprintf(string, "debug.%.3d", parmPid);
     debugName(string);
     debugSetIntensiveMode();
   }}}}
   #endif

   if( parmPid >= 0 )
     return;

   fprintf(stderr, "%10s = %s %s\n", "Version", __DATE__, __TIME__);
   fprintf(stderr, "%10d = -D Startup Delay\n", startupDelay);
   fprintf(stderr, "%10d = -R Run Time\n", testTime);
   fprintf(stderr, "%10d = -V Verbosity\n", verbose);
   fprintf(stderr, "\n");
   fprintf(stderr, "%10d = Number of processes\n", pCount);
   fprintf(stderr, "%10d = Number of threads per process\n", tCount);
// fprintf(stderr, "%10d = Number of sockets per thread\n", sCount);
// fprintf(stderr, "%10d = Buffer size\n", buffSize);
   fprintf(stderr, "\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       doProcess
//
// Purpose-
//       Run test under control of a Process.
//
//----------------------------------------------------------------------------
static int                          // Completion code
   doProcess(                       // Process function
     int             pid)           // Process identifier
{
   Common&           C= *common;    // Address the Common
   PerProcess&       P= C.process[pid]; // Address the PerProcess

   MySignal          sigObject(pid,0); // Signal object

// int               sid;           // Socket identifier
   int               tid;           // Thread identifier

   unsigned          operations= 0; // Number of operations

   Clock             first;         // Time first operation started
   Clock             after;         // Time last  operation completed
   Clock             diff;          // Time difference

// int               L;             // Transfer length
   int               returncd;      // This routine's return code

   #ifdef HCDM
     debugf("%s %4d: doProcess(%d) started\n", __SOURCE__, __LINE__, pid);
   #endif
   SERVICE_INFO(0xffff0000 | pid);

   //-------------------------------------------------------------------------
   // Match local process id to system process id.
   //-------------------------------------------------------------------------
   if( verbose > 2 )
     prepend(pid, "Process id(%d)\n", getpid());

   P.fsm= PerProcess::PS_OPERATIONAL;

   //-------------------------------------------------------------------------
   // On a multi-processor, the clocks may be skewed.
   // If they are off by too much, there is no point in running.
   //-------------------------------------------------------------------------
   #ifndef HCDM                     // This also fails if debugging
     double          deltaT;        // Elapsed time in seconds

     first= Clock::current();       // The current time
     diff= first - C.initialTime;
     deltaT= (double)diff;

     if( deltaT < 0.0 || deltaT > ((double)startupDelay/2.0) )
     {
       prepend(pid, "Start time(%.3f)\n", (double)C.startTime);
       prepend(pid, "Time skew(%.3f) invalid\n", deltaT);
       return 1;
     }
   #endif

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: working\n", __SOURCE__, __LINE__);
   #endif
   for(tid= 0; tid<tCount; tid++)
   {
     SERVICE_INFO((tid<<16) + pid);
     P.thread[tid].thread= new MyThread(pid,tid);
     P.thread[tid].operations= 0;

//   for(sid=0; sid<sCount; sid++)
//   {
//     P.thread[tid].socket[sid]= NULL;
//   }
   }

   //-------------------------------------------------------------------------
   //
   // Synchronize with other processes by waiting
   //
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: working\n", __SOURCE__, __LINE__);
   #endif
   if( !synchro() )
   {
     prepend(pid, "START after T(%12.2f)\n", (double)C.startTime);
     returncd= 3;
     goto cleanup;
   }

   //-------------------------------------------------------------------------
   // Timed loop
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: starting Threads\n", __SOURCE__, __LINE__);
   #endif
   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
     prepend(pid, "Before LOOP\n");

   first= Clock::current();
   for(tid= 0; tid<tCount; tid++)
   {
     P.thread[tid].thread->start();
   }

   #ifdef HCDM
     debugf("%s %4d: in doProcess(%d), Thread wait\n",
            __SOURCE__, __LINE__, pid);
   #endif
   operations= 0;

   for(tid= 0; tid<tCount; tid++)
   {
     P.thread[tid].thread->wait();
     operations += P.thread[tid].operations;
     #ifdef HCDM
       prepend(pid, "%s %4d: in doProcess(%d), Thread[%d] complete\n",
                    __SOURCE__, __LINE__, pid, tid);
     #endif
   }

   after= Clock::current();

   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
     prepend(pid, "After  LOOP(%ld)\n", operations);
   Thread::sleep(C.finisWindow.getTime()); // Measurement complete

   //-------------------------------------------------------------------------
   // Report the completion time and operation count
   //-------------------------------------------------------------------------
   SERVICE_INFO(0xffff0000 | pid);
   #ifdef HCDM
     debugf("%s %4d: working\n", __SOURCE__, __LINE__);
   #endif
   P.after= after;
   P.operations= operations;
   returncd= 0;

   //-------------------------------------------------------------------------
   // Cleanup exit
   //-------------------------------------------------------------------------
cleanup:
   SERVICE_INFO(0xffff0000 | pid);
   for(tid= 0; tid<tCount; tid++)
   {
   }

   if( verbose > 2 )
     prepend(pid, "Before EXIT\n");

   #ifdef HCDM
     debugf("%s %4d: doProcess(%d) complete\n", __SOURCE__, __LINE__, pid);
   #endif
   SERVICE_INFO(0xffff0000 | pid);
   return returncd;
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
int
   main(
     int             argc,
     char*           argv[])
{
   char              fileName[FILENAME_MAX]; // File name
   SharedMem*        sharedCommon;  // Shared Common control
   SharedMem*        sharedGlobal;  // Shared Global control

   Clock             after;         // Last completion time
   Clock             diff;          // Last completion interval

   double            total;         // Total operations per second
   double            rating;        // Operations per second

   char              string[512];   // Working string
   int               operations;    // Operation count
   int               pid;           // Local process id
   int               tid;           // Local thread id
   int               returncd;      // This routine's return code
   int               success;       // Number of successful children
   int               status;        // Completion status

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis
   #ifdef HCDM
     debugSetIntensiveMode();
     debugf("%s %4d: main started\n", __SOURCE__, __LINE__);
     {{
       for(int i=0; i<argc; i++)
         debugf("[%d] %s\n", i, argv[i]);
     }}
   #endif

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: global Storage\n", __SOURCE__, __LINE__);
   #endif
   const char* T;
   if( (T= FileName::resolve(fileName, argv[0])) != 0 )
   {
     fprintf(stderr, "%s= FileName::resolve(%s)\n", T, argv[0]);
     return 2;
   }

   sharedCommon= new SharedMem(sizeof(Common),
                            SharedMem::getToken(fileName, FTOK_ID),
                            SharedMem::Create | // Create it if non-existent
                            SharedMem::Write  );// Allow us to write it

   sharedGlobal= new SharedMem(sizeof(Global),
                            SharedMem::Token(FTOK_ID + 1),
                            SharedMem::Create | // Create it if non-existent
                            SharedMem::Write  );// Allow us to write it

   common= (Common*)sharedCommon->getAddress();
   global= (Global*)sharedGlobal->getAddress();

   #if defined(HCDM)
     debugf("Common:\n");
     snap(common, sizeof(Common));
     debugf("\n");
     debugf("Global:\n");
     snap(global, sizeof(Global));
   #endif

   //-------------------------------------------------------------------------
   // If this is a started process, run it
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: child check\n", __SOURCE__, __LINE__);
   #endif
   if( parmPid >= 0 )
   {
     returncd= doProcess(parmPid);  // Run the child process

     #ifdef HCDM
       debugf("%s %4d: child complete\n", __SOURCE__, __LINE__);
     #endif

     delete sharedGlobal;
     delete sharedCommon;

     #ifdef HCDM
       debugf("%s %4d: child storage deleted\n", __SOURCE__, __LINE__);
     #endif
     return returncd;               // And return
   }

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: mainline\n", __SOURCE__, __LINE__);
   #endif
   memset((char*)common, 0, sizeof(Common));
   memset((char*)global, 0, sizeof(Global));
   new (common) Common();
   new (global) Global();

   for (pid=0; pid<pCount; pid++)
   {
     common->process[pid].fsm= PerProcess::PS_INITIAL;
   }

   strcpy(common->ident, "COMMON");
   strcpy(global->ident, "GLOBAL");

// Global&           G= *global;    // (Unused)
   Common&           C= *common;

   #if defined(HCDM)
     debugf("Common:\n");
     snap(common, sizeof(Common));
     debugf("\n");
     debugf("Global:\n");
     snap(global, sizeof(Global));
   #endif

   //-------------------------------------------------------------------------
   // Set the synchronization times.
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: working\n", __SOURCE__, __LINE__);
   #endif
   C.initialTime= Clock::current();
   C.startTime= (double)C.initialTime + (double)startupDelay;
   C.finisTime= C.startTime;
   C.finisTime= (double)C.finisTime + (double)testTime;
   C.finisWindow= 0.2 * (double)testTime;

   //-------------------------------------------------------------------------
   // Start all the child processes
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: creating child processes\n", __SOURCE__, __LINE__);
   #endif
   for (pid=0; pid<pCount; pid++)
   {
     sprintf(string, "-:%d", pid);
     strcat(string, parmString);

     C.process[pid].process= new Process();
     C.process[pid].process->start(argv[0], string);
   }

   //-------------------------------------------------------------------------
   // Wait for each child to start
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: main, waiting for startup\n", __SOURCE__, __LINE__);
   #endif
   for(pid=0; pid<pCount; pid++)
   {
     while(C.process[pid].fsm == PerProcess::PS_INITIAL )
       Thread::sleep(1);
   }

   //-------------------------------------------------------------------------
   // Wait for each child to complete
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: in main, Process wait\n", __SOURCE__, __LINE__);
   #endif
   success= pCount;
   for(pid=0; pid<pCount; pid++)
   {
     status= C.process[pid].process->wait();
     if( status != 0 )
     {
       success--;
       debugf("[%3d] Failed, status(0x%.8X)\n", pid, status);
     }
     C.process[pid].status= status;
     #ifdef HCDM
       debugf("%s %4d: [%3d] Process complete\n", __SOURCE__, __LINE__, pid);
     #endif
   }
   debugf("%d of %d successful\n", success, pCount);

   //-------------------------------------------------------------------------
   // Summary
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: working\n", __SOURCE__, __LINE__);
   #endif
   if( success == 0 )
     success= (-1);

   debugf("\n");
   debugf("Operation distribution by thread:\n");
   for(pid=0; pid<pCount; pid++)
   {
     if( C.process[pid].status != 0 )
       debugf("[%3d] FAILED\n", pid);
     else
     {
       debugf("[%3d]:", pid);
       for(tid=0; tid<tCount; tid++)
         debugf(" %8d", C.process[pid].thread[tid].operations);
       debugf("\n");
     }
   }

   debugf("\n");
   debugf("Operation counts:\n");
   after= C.process[0].after;
   operations= 0;
   total= 0.0;
   for(pid=0; pid<pCount; pid++)
   {
     operations += C.process[pid].operations;
     if( (double)C.process[pid].after > (double)after )
       after= C.process[pid].after;

     if( C.process[pid].status != 0 )
       debugf("[%3d] FAILED\n", pid);
     else
     {
       diff= C.process[pid].after - C.startTime;
       rating= (double)C.process[pid].operations / (double)diff;
       debugf("[%3d] %12.3f Ops/sec\n", pid, rating);

       total += rating;
     }
   }
   debugf("----- ------------\n");
   debugf("Total %12.3f Ops/sec, %12.3f per process\n",
          total, total/(double)success);

   debugf("\n");
   debugf("Operation ratings:\n");
   diff= after - C.startTime;
   operations= 0;
   total= 0.0;
   for(pid=0; pid<pCount; pid++)
   {
     operations += C.process[pid].operations;
     if( C.process[pid].status != 0 )
       debugf("[%3d] FAILED\n", pid);
     else
     {
       rating= (double)C.process[pid].operations / (double)diff;
       debugf("[%3d] %12.3f Ops/sec\n", pid, rating);

       total += rating;
     }
   }
   debugf("----- ------------\n");
   debugf("Rated %12.3f Ops/sec, %12.3f per process\n",
          total, total/(double)success);
   returncd= 0;

   //-------------------------------------------------------------------------
   // Cleanup
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: main cleanup\n", __SOURCE__, __LINE__);
   #endif

   delete sharedGlobal;
   delete sharedCommon;

   return(returncd);
}


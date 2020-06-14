//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       RMsend.cpp
//
// Purpose-
//       Examine request management send.
//
// Last change date-
//       2020/06/13
//
//----------------------------------------------------------------------------
#include <new>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "RMconn.h"
#include <com/define.h>
#include <com/Clock.h>
#include <com/Debug.h>
#include <com/FileName.h>
#include <com/Latch.h>
#include <com/Network.h>
#include <com/Process.h>
#include <com/Socket.h>
#include <com/Software.h>
#include <com/SharedMem.h>
#include <com/Thread.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "RMSEND  " // Source file name, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define FTOK_ID            20070311 // FTOK identifier

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define TEST_SECONDS            (5) // Number of seconds to test

#define MAX_PROCESSES         (100) // Maximum number of processes
#define MAX_THREADS           (100) // Maximum number of threads per process
#define MAX_SOCKETS            (50) // Maximum number of sockets per thread

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
long                                // Completion Code
   doThread(                        // Thread function
     int               pid,         // Process identifier
     int               tid);        // Thread identifier

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
     int               pid,
     int               tid)
:  pid(pid)
,  tid(tid)
{
}

//----------------------------------------------------------------------------
// MyThread::Methods
//----------------------------------------------------------------------------
public:
virtual long
   run( void )
{
   return doThread(pid, tid);
}

//----------------------------------------------------------------------------
// MyThread::Attributes
//----------------------------------------------------------------------------
private:
   int                 pid;
   int                 tid;
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
// Latch               latch;       // Thread Latch

   unsigned            operations;  // Number of operations
   MyThread*           thread;      // -> Associated Thread
   Socket*             socket[MAX_SOCKETS]; // -> StreamSocket array
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
enum State
{
   PS_INITIAL,                      // Initial
   PS_OPERATIONAL                   // Operational
};
   State               fsm;         // Thread state

// Latch               latch;       // Process latch

   Clock               after;       // Time last operation completed
   unsigned long       operations;  // Number of operations
   int                 status;      // Completion status

   Process*            process;     // -> Associated Process
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
   char                ident[8];    // Identifier

// Latch               latch;       // Common latch

   Clock               initialTime; // Initial time
   Clock               startTime;   // Startup time
   Clock               finisTime;   // Completion time
   Clock               finisWindow; // Completion time window

   PerProcess          process[MAX_PROCESSES]; // Process information area
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
   char                ident[8];    // Identifier

// Latch               latch;       // Global latch
};

//----------------------------------------------------------------------------
// Local data areas -- available to all processes
//----------------------------------------------------------------------------
static Common*         common;      // Common storage region
static Global*         global;      // Global storage region

static char*           buffer;      // Local  storage region
static unsigned        buffSize;    // Local  storage region size

static PerThread       thread[MAX_THREADS]; // Thread storage region

static Socket::Addr    peerAddr;    // Partner network address
static Socket::Port    peerPort;    // Partner port

static int             startupDelay;// Startup time delay, in seconds
static int             testTime;    // Test time, in seconds
static int             verbose;     // Verbosity

static int             parmPid;     // Process identifier (parameter)
static char            parmString[1024]; // Parameter string save area

static int             pCount;      // Number of processes to create
static int             tCount;      // Number of threads per process
static int             sCount;      // Number of sockets per thread

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugThread
//
// Purpose-
//       Debug the thread.
//
//----------------------------------------------------------------------------
static void debugThread(int tid)
{
#if 0
   debugf("Thread[%d].debug()\n", tid);
   thread[tid].thread->debug();
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       prepend
//
// Purpose-
//       Print from process to stderr, prepending the process identifier.
//
//----------------------------------------------------------------------------
static void
   prepend(                         // Process debugf(...)
     int               pid,         // Process identifier
     const char*       fmt,         // The format string
                       ...)         // Variable arguments
{
   char                fmtString[256]; // Message formatting area
   char                msgString[512]; // Message formatting area
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vsprintf(fmtString, fmt, argptr);// Format the message
   va_end(argptr);                  // Close va_ functions

   sprintf(msgString, "[%3d] %12.2f %s",
                      pid, Clock::current(), fmtString);
   debugf("%s", msgString);         // Print the string all at once
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
   Common&           C= *common;

   Clock             newTime;
   Clock             delTime;

   newTime= Clock::current();
   delTime= C.startTime - newTime;
   if( (long)delTime.getTime() < 0 )
   {
     return FALSE;
   }

   #ifdef HCDM
     debugf("%s %d: synchro(%12.9f)\n",
            __SOURCE__, __LINE__, (double)delTime);
   #endif
   Thread::sleep((double)delTime);
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
                   "\tnetworkAddr networkPort\n"
                   "\t[processCount (1)"
                   " [threadsPerProcess (1)"
                   " [socketsPerThread (1)"
                   " [messageLength (1000)"
                   "]]]]\n"
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
     int               argc,
     char*             argv[])
{
   int                 error= FALSE;// Error encountered indicator
   int                 argi;        // Argument index
   char*               argp;        // Argument pointer
   int                 pindex;      // Positional index

   char                string[128]; // Working string
   unsigned            connects;    // Number of connections

   //-------------------------------------------------------------------------
   // Parameter defaults
   //-------------------------------------------------------------------------
   pCount=       1;
   tCount=       1;
   sCount=       1;
   buffSize=     1000;
   parmPid=      (-1);
   parmString[0]= '\0';
   startupDelay= (-1);
   testTime=     (-1);
   verbose=      1;
   #ifdef HCDM
     verbose=    9;
   #endif

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   if( argc < 2 )
     info();

   pindex= 1;
   for(argi=1; argi<argc; argi++)
   {
     argp= argv[argi];              // Address the parameter

     strcat(parmString, " ");       // Save the parameter
     strcat(parmString, argp);

     if( *argp == '-' )             // If switch parameter
     {
       argp++;                      // Skip the switch parameter

       if( *argp == 'D' )           // If startup delay
       {
         argp++;
         if( *argp != ':' )
         {
           fprintf(stderr, "Invalid parameter(-%s)\n", argp);
           error= TRUE;
         }
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
         {
           fprintf(stderr, "Invalid parameter(-%s)\n", argp);
           error= TRUE;
         }
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
         {
           fprintf(stderr, "Invalid parameter(-%s)\n", argp);
           error= TRUE;
         }
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
            peerAddr= Socket::nameToAddr(argp);
            break;

         case 2:
            peerPort= atoi(argp);
            break;

         case 3:
            pCount= atoi(argp);
            if( pCount > MAX_PROCESSES )
            {
               fprintf(stderr,"processCount(%d) bigger than limit(%d)\n",
                              pCount, MAX_PROCESSES);
               error= TRUE;
            }
            break;

         case 4:
            tCount= atoi(argp);
            if( tCount > MAX_THREADS )
            {
               fprintf(stderr,"threadCount(%d) bigger than limit(%d)\n",
                              tCount, MAX_THREADS);
               error= TRUE;
            }
            break;

         case 5:
            sCount= atoi(argp);
            if( sCount > MAX_SOCKETS )
            {
               fprintf(stderr,"socketCount(%d) bigger than limit(%d)\n",
                              sCount, MAX_SOCKETS);
               error= TRUE;
            }
            break;

         case 6:
            buffSize= atoi(argp);
            if( buffSize < 1 )
            {
              fprintf(stderr, "Invalid buffer size(%ld)\n", (long)buffSize);
              error= TRUE;
            }
            break;

         default:
           fprintf(stderr, "Too many positional parmameters(%s)\n", argp);
           error= TRUE;
       }

       pindex++;
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

   if( error || pindex < 3 )
     info();

   connects= pCount * tCount * sCount;
   if( startupDelay < 0 )           // If invalid or not initialized
   {
     startupDelay= connects / 50;   // Default startup delay
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
   strcpy(string, "debugS.out");
   if( parmPid >= 0 )
     sprintf(string, "debugS.%.3d", parmPid);
   debugSetName(string);
   debugSetIntensiveMode();

   //-------------------------------------------------------------------------
   // Information display
   //-------------------------------------------------------------------------
   if( parmPid >= 0 )
     return;

   debugf("%10s = %s %s %s\n", "Version", __DATE__, __TIME__, __SOURCE__);
   debugf("%10d = -D Startup Delay\n", startupDelay);
   debugf("%10d = -R Run Time\n", testTime);
   debugf("%10d = -V Verbosity\n", verbose);
   debugf("\n");
   debugf("%10d = Connections\n", connects);
   debugf("%10d = Number of processes\n", pCount);
   debugf("%10d = Number of threads per process\n", tCount);
   debugf("%10d = Number of sockets per thread\n", sCount);
   debugf("%10d = Buffer size\n", buffSize);
   debugf("\n");
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
long                                // Completion Code
   doThread(                        // Thread function
     int               pid,         // Process identifier
     int               tid)         // Thread identifier
{
   Common&             C= *common;
   PerThread&          T= thread[tid];
   double              const finTime= (double)C.finisTime;
   unsigned            const xmitSize= buffSize+sizeof(Network::Net16);

   Socket*             S;           // -> Socket

   Clock               after;       // Time last operation completed
   int                 operations;  // Number of operations
   int                 sid;         // Socket identifier

   int                 l;           // Transmission size (required)
   int                 L;           // Transmission size (actual)
   int                 O;           // Transmission offset

   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
   {
     #ifdef HCDM
       prepend(pid, "[%3d] started\n", tid);

       for(sid=0; sid<sCount; sid++)
       {
         prepend(pid, "[%3d] S(%2d) %p\n", tid, sid, T.socket[sid]);
       }
     #endif
   }

   sid= 0;
   for (operations=0; ;operations++)
   {
     S= T.socket[sid];

     O= 0;
     while( O < xmitSize )
     {
       l= xmitSize - O;
       L= S->send(buffer+O, l);
       #ifdef HCDM
         debugf("%s %d: T(%d) S(%d) %d= %p->send(...)\n",
                __SOURCE__, __LINE__, tid, sid, L, S);
       #endif
       if( L <= 0 )
       {
         prepend(pid, "S(%p) P(%d): I/O Error(%s)\n", S, S->getHostPort(),
                      S->getSocketEI());
         return 2;
       }

       O += L;
     }

     after= Clock::current();
     if( (double)after > finTime )
       break;

     sid++;
     if( sid >= sCount )
       sid= 0;
   }

   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
     prepend(pid, "[%3d] complete(%d)\n", tid, operations);

   T.operations= operations;
   return 0;
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
int                                 // Completion code
   doProcess(                       // Process function
     int               pid)         // Process identifier
{
   Common&             C= *common;  // Address the Common
   PerProcess&         P= C.process[pid]; // Address the PerProcess

   Socket*             dgSock;      // Datagram Socket
   RMconnQ             connQ;       // Connection request
   RMconnS             connS;       // Connection response

   int                 sid;         // Socket identifier
   int                 tid;         // Thread identifier

   unsigned            errorCount= 0; // Number of errors encountered
   unsigned            operations;  // Number of operations

   Clock               first;       // Time first operation started
   Clock               after;       // Time last  operation completed
   Clock               diff;        // Time difference

   int                 L;           // Transfer length
   int                 returncd;    // This routine's return code
   double              deltaT;      // Elapsed time in seconds

   int                 rc;

   #ifdef HCDM
     debugf("%s %d: doProcess(%d) started\n", __SOURCE__, __LINE__, pid);
   #endif

   //-------------------------------------------------------------------------
   // Match local process id to system process id.
   //-------------------------------------------------------------------------
   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
     prepend(pid, "PID(%d)\n", getpid());

   P.fsm= PerProcess::PS_OPERATIONAL;

   //-------------------------------------------------------------------------
   // On a multi-processor, the clocks may be skewed.
   // If they are off by too much, there is no point in running.
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: clock check\n", __SOURCE__, __LINE__);
   #endif
   returncd= 1;                     // Default, error completion
   first= Clock::current();         // The current time
   diff= first - C.initialTime;
   deltaT= (double)diff;

   if( deltaT < 0.0 || deltaT > ((double)startupDelay/2.0) )
   {
     prepend(pid, "started(%.4f)\n", (double)C.startTime);

     prepend(pid, "time skew(%.3f) invalid (try sudo setclock)\n", deltaT);
     return 2;
   }

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: doProcess(%d) creating threads\n",
            __SOURCE__, __LINE__, pid);
   #endif
   for(tid= 0; tid<tCount; tid++)
   {
     thread[tid].thread= new MyThread(pid,tid);
     debugThread(tid);
     thread[tid].operations= 0;

     for(sid=0; sid<sCount; sid++)
     {
       thread[tid].socket[sid]= NULL;
     }
   }

   //-------------------------------------------------------------------------
   // Get connection port
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: doProcess(%d)\n", __SOURCE__, __LINE__, pid);
   #endif
   dgSock= new Socket(Socket::ST_DGRAM);
   dgSock->setHost();
   dgSock->setPeer(peerAddr, peerPort);
   connQ.fc= Network::hton32(RMconnQ::FC_Connect);

   //-------------------------------------------------------------------------
   // Connect
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: doProcess(%d) connecting\n", __SOURCE__, __LINE__, pid);
   #endif
   for(tid= 0; tid<tCount; tid++)
   {
     for(sid=0; sid<sCount; sid++)
     {
       for(operations= 0;;operations++)
       {
         // Get a port to try
         for(;;)
         {
           L= dgSock->send((char*)&connQ, sizeof(connQ));
           if( L == (-1) )
           {
             debugf("dgSock->send() EC(%d) EI(%s)\n",
                    dgSock->getSocketEC(), dgSock->getSocketEI());
             exit(EXIT_FAILURE);
           }

           if( L == sizeof(connQ) )
           {
             L= dgSock->recv((char*)&connS, sizeof(connS));
             if( L == sizeof(connS) )
               break;

             prepend(pid, "T[%d] S[%d] %d=DGsock.recv()\n", tid, sid, L);
             errorCount++;
             if( errorCount > 5 )
               goto cleanup;
           }
         }

         // Try to connect using the connection port just returned
         if( operations != 0 )
           prepend(pid, "T[%d] S[%d] Retry  IO connect(%.12llX::%d)\n",
                        tid, sid,
                        Network::ntoh64(connS.host),
                        Network::ntoh32(connS.port));

         if( verbose > 3 )
           prepend(pid, "T[%d] S[%d] Before IO connect(%.8lX::%d)\n",
                        tid, sid,
                        Network::ntoh64(connS.host),
                        Network::ntoh32(connS.port));

         thread[tid].socket[sid]= new Socket(Socket::ST_STREAM);
         rc= thread[tid].socket[sid]->connect(
             Socket::Addr(Network::ntoh64(connS.host)),
             Socket::Port(Network::ntoh32(connS.port)));
         if( rc == 0 )
         {
           if( verbose > 3 )
             prepend(pid, "T[%d] S[%d] Connected(%.8lX::%ld)\n",
                          tid, sid,
                          (long)thread[tid].socket[sid]->getPeerAddr(),
                          (long)thread[tid].socket[sid]->getPeerPort());
           break;
         }

         prepend(pid, "T[%d] S[%d] FAILED IO connect(%.8lX::%d) %s\n",
                      tid, sid,
                      Network::ntoh64(connS.host),
                      Network::ntoh32(connS.port),
                      thread[tid].socket[sid]->getSocketEI());

         delete thread[tid].socket[sid];
         first= Clock::current(); // The current time
         if( first > C.startTime || FALSE )
         {
           prepend(pid, "Giving up\n");
           delete dgSock;
           returncd= 2;
           goto cleanup;
         }
       }
     }
   }

   delete dgSock;

   //-------------------------------------------------------------------------
   //
   // Synchronize with other processes by waiting
   //
   //-------------------------------------------------------------------------
   if( !synchro() )
   {
     prepend(pid, "START after T(%12.2f)\n", (double)C.startTime);
     returncd= 3;
     goto cleanup;
   }

   //-------------------------------------------------------------------------
   //
   //   Timed loop
   //
   //-------------------------------------------------------------------------
   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
     prepend(pid, "Before LOOP\n");

   first= Clock::current();
   for(tid= 0; tid<tCount; tid++)
   {
     thread[tid].thread->start();
   }

   operations= 0;
   for(tid= 0; tid<tCount; tid++)
   {
     thread[tid].thread->wait();
     operations += thread[tid].operations;
   }
   after= Clock::current();

   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
     prepend(pid, "After  LOOP(%ld)\n", operations);

   Thread::sleep((double)C.finisWindow);

   P.after= after;
   P.operations= operations;
   returncd= 0;

   //-------------------------------------------------------------------------
   //
   //   Cleanup exit
   //
   //-------------------------------------------------------------------------
cleanup:
   #ifdef HCDM
     debugf("%s %d: doProcess(%d) cleanup\n", __SOURCE__, __LINE__, pid);
   #endif
   for(tid= 0; tid<tCount; tid++)
   {
     for(sid=0; sid<sCount; sid++)
     {
       delete thread[tid].socket[sid];
     }
   }

   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
     prepend(pid, "Before EXIT\n");

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
     int               argc,
     char*             argv[])
{
   SharedMem::Segment  fileSegment; // File segment
   SharedMem::Token    fileToken;   // File token

   SharedMem::Segment  userSegment; // User segment
   SharedMem::Token    userToken;   // User token

   Clock               after;       // Last completion time
   Clock               diff;        // Last completion interval

   double              total;       // Total operations per second
   double              rating;      // Operations per second

   int                 operations;  // Operation count
   int                 pid;         // Fork loop iterator
   int                 returncd;    // This routine's return code
   int                 status;      // How child completed
   char                string[1024];// Working string
   int                 success;     // Number of successful children

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis
   #ifdef HCDM
     debugSetIntensiveMode();
     debugf("%s %d: mainline\n", __SOURCE__, __LINE__);
   #endif

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   *(Network::Net16*)buffer= Network::hton16(buffSize);
   memset(buffer+sizeof(Network::Net16), 'B', buffSize);

   FileName fileName(argv[0]);
   fileName.resolve();
   fileToken= SharedMem::getToken(fileName.getFileName(), FTOK_ID);
   fileSegment= SharedMem::access(sizeof(Common), fileToken,
                    SharedMem::Create | // Create it if non-existent
                    SharedMem::Write); // Allow us to write it
   common= (Common*)SharedMem::attach(fileSegment);
   if( common == NULL )
   {
     fprintf(stderr, "No common storage\n");
     exit(EXIT_FAILURE);
   }

   userToken= 0x20000107;
   userSegment= SharedMem::access(sizeof(Global), userToken,
                    SharedMem::Create | // Create it if non-existent
                    SharedMem::Write  ); // Allow us to write it
   global= (Global*)SharedMem::attach(userSegment);
   if( global == NULL )
   {
     fprintf(stderr, "No global storage\n");
     exit(EXIT_FAILURE);
   }

   Common&             C= *common;
   Global&             G= *global;

   //-------------------------------------------------------------------------
   // If this is a started process, run it
   //-------------------------------------------------------------------------
   if( parmPid >= 0 )
   {
     returncd= doProcess(parmPid);  // Run the child process
     C.process[parmPid].status= returncd;
     SharedMem::detach(global);     // Detach the global region
     SharedMem::remove(userSegment);// Destroy the global region
     SharedMem::detach(common);     // Detach the common region
     SharedMem::remove(fileSegment);// Destroy the common region
     return returncd;               // And return
   }

   //-------------------------------------------------------------------------
   // Initialize main Thread
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: mainline\n", __SOURCE__, __LINE__);

     debugf("%s %d: Common(%d) Global(%d)\n", __SOURCE__, __LINE__,
                    sizeof(Common), sizeof(Global));
   #endif
   memset(common, 0, sizeof(Common));
   memset(global, 0, sizeof(Global));
   new (common) Common();
   new (global) Global();

   strcpy(C.ident, "COMMON");
   strcpy(G.ident, "GLOBAL");

   for (pid=0; pid<pCount; pid++)
   {
     C.process[pid].fsm= PerProcess::PS_INITIAL;
   }

   C.initialTime= Clock::current();
   C.startTime= (double)C.initialTime + (double)startupDelay;
   C.finisTime= (double)C.startTime + (double)testTime;
   C.finisWindow= 0.2 * (double)testTime;

// Latch::resetStatistics();        // Reset the latch statistics

   //-------------------------------------------------------------------------
   // Start all the child processes
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: starting processes\n", __SOURCE__, __LINE__);
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
     debugf("%s %d: waiting for process start\n", __SOURCE__, __LINE__);
   #endif
   for(pid=0; pid<pCount; pid++)
   {
     while(C.process[pid].fsm == PerProcess::PS_INITIAL )
       Thread::sleep(1.0);
   }

   //-------------------------------------------------------------------------
   // Wait for each child to complete
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: waiting for process completion\n", __SOURCE__, __LINE__);
   #endif
   success= pCount;
   for(pid=0; pid<pCount; pid++)
   {
     C.process[pid].process->wait();
     status= C.process[pid].status;
     if( status != 0 )
     {
       success--;
       debugf("[%3d] Failed, status(0x%.8X)\n", pid, status);
     }
   }
   debugf("%d of %d successful\n", success, pCount);

   //-------------------------------------------------------------------------
   // Summary
   //-------------------------------------------------------------------------
   if( success == 0 )
     success= (-1);

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
     prepend(-1, "%s %d: mainline cleanup\n", __SOURCE__, __LINE__);
   #endif
   SharedMem::detach(global);       // Detach the global region
   SharedMem::remove(userSegment);  // Destroy the global region
   SharedMem::detach(common);       // Detach the common region
   SharedMem::remove(fileSegment);  // Destroy the common region

// Latch::displayStatistics();

   return(0);
}


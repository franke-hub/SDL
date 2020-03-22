//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       RMrecv.cpp
//
// Purpose-
//       Examine request management receive.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <new>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

#include "RMconn.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "RMRECV  " // Source file name, for debugging

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
#define SIZEOF_BUFFER       (10000) // Maximum buffer size
#define MAX_PROCESSES        (1000) // Maximum number of processes
#define MAX_THREADS          (1000) // Maximum number of threads per process
#define MAX_SOCKETS           (100) // Maximum number of sockets per thread

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
enum State
{
   PS_INITIAL,                      // Initial
   PS_OPERATIONAL                   // Operational
};

   State               fsm;         // Process state

   Latch               latch;       // Thread Latch
   MyThread*           thread;      // -> Associated Thread

   unsigned            waits;       // The number of waiting sockets
   unsigned            inUse;       // The number of sockets in use
   Socket*             socket[MAX_SOCKETS]; // The associated Sockets
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
   PS_OPERATIONAL,                  // Listener operational
   PS_LISTENING                     // Listener enabled
};

   State               fsm;         // Process state

   Socket::Addr        addr;        // Listener Host number
   Socket::Port        port;        // Listener Port number

   Process*            process;     // -> Associated Process
   int                 status;      // Completion status
};

//----------------------------------------------------------------------------
//
// Struct-
//       Common
//
// Purpose-
//       Define common storage region.
//
//----------------------------------------------------------------------------
struct Common
{
   PerProcess          process[MAX_PROCESSES]; // Process information area
};

//----------------------------------------------------------------------------
// Local data areas -- available to each process
//----------------------------------------------------------------------------
static Common*         common;      // Common storage region
static char*           buffer;      // Local  storage region

static PerThread       thread[MAX_THREADS]; // Thread storage region

static int             parmPid;     // Process identifier (parameter)
static char            parmString[1024]; // Parameter string save area

static int             pCount;      // Number of processes to create
static int             tCount;      // Number of threads per process
static int             sCount;      // Number of sockets per thread
static int             verbose;     // Verbosity

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
     int               id,          // Process identifier
     const char*       fmt,         // The format string
                       ...)         // Variable arguments
{
   Clock               nowTime;     // Current time
   char                fmtString[256]; // Message formatting area
   char                msgString[256]; // Message formatting area
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vsprintf(fmtString, fmt, argptr);// Format the message
   va_end(argptr);                  // Close va_ functions

   nowTime= Clock::current();
   sprintf(msgString, "[%3d] %12.2f %s",
                      id, (double)nowTime, fmtString);
   debugf("%s", msgString);         // Print the string all at once
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
                   " [-V:verbosity]\n"
                   "\tprocessCount"
                   " [threadsPerProcess (1)"
                   " [socketsPerThread (1)"
                   "]]\n"
          );
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
   parmPid=      (-1);
   parmString[0]= '\0';
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
       if( *argp == 'V' )           // If verbosity
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
            pCount=atoi(argp);
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

         case 3:
            sCount= atoi(argp);
            if( sCount > MAX_SOCKETS )
            {
               fprintf(stderr,"socketCount(%d) bigger than limit(%d)\n",
                              sCount, MAX_SOCKETS);
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
   buffer= (char*)malloc(SIZEOF_BUFFER+8);
   if( buffer == NULL )
   {
     fprintf(stderr, "Storage shortage\n");
     exit(EXIT_FAILURE);
   }

   if( error )
     info();

   //-------------------------------------------------------------------------
   // Debugging
   //-------------------------------------------------------------------------
   strcpy(string, "debugR.out");
   if( parmPid >= 0 )
     sprintf(string, "debugR.%.3d", parmPid);
   debugSetName(string);
   debugSetIntensiveMode();

   //-------------------------------------------------------------------------
   // Information display
   //-------------------------------------------------------------------------
   if( parmPid >= 0 )
     return;

   connects= pCount * tCount * sCount;
   debugf("%10s = %s %s %s\n", "Version", __DATE__, __TIME__, __SOURCE__);
   debugf("%10d = -V Verbosity\n", verbose);
   debugf("\n");
   debugf("%10d = Possible connections\n", connects);
   debugf("%10d = Number of processes\n", pCount);
   debugf("%10d = Number of threads per process\n", tCount);
   debugf("%10d = Number of sockets per thread\n", sCount);
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
   char                dropReason[128]; // Reason for drop
   PerThread&          T= thread[tid]; // Address the PerThread object
   Socket*             ioSock;      // -> I/O Socket
   SockSelect          select;      // Socket selector

   int                 sid;         // Socket identifier

   char                buffChar[sizeof(Network::Net16)]; // Buffer
   short               buffSize;    // Buffer size
   int                 L;           // Transmission size
   int                 O;           // Transmission offset

   int                 i;

   T.latch.obtainXCL();
   T.fsm= PerThread::PS_OPERATIONAL;
   T.latch.releaseXCL();

   #ifdef HCDM
     debugf("%s %d: doThread(%d,%d) started\n", __SOURCE__, __LINE__, pid, tid);
   #endif

waitForWork:
   if( verbose > 3 || (verbose > 1 && pid == (pCount-1)) )
     prepend(pid, "T(%d) waiting\n", tid);

   T.latch.obtainXCL();
   T.waits= 0;
   while( T.inUse == 0 )
   {
     T.latch.releaseXCL();
     Thread::sleep(1.0);
     T.latch.obtainXCL();
   }

   // Transition, 0 to some sockets
   if( verbose > 3 || (verbose > 1 && pid == (pCount-1)) )
     prepend(pid, "T(%d) working\n", tid);

   T.latch.releaseXCL();
   goto waitForData;

dropSocket:
   if( verbose > 3 || (verbose > 1 && pid == (pCount-1)) )
     prepend(pid, "T(%d) dropped S(%p) P(%d) %s\n",
                  tid, ioSock, ioSock->getHostPort(), dropReason);

   T.latch.obtainXCL();
   #ifdef HCDM
     debugf("%s %d: T(%d) status\n", __SOURCE__, __LINE__, tid);
     debugf(">>T.inUse(%d) T.waits(%d)\n",
            T.inUse, T.waits);
     for(sid= 0; sid<sCount; sid++)
     {
       debugf(">>[%2d] %p\n", sid, T.socket[sid]);
     }
   #endif

   for(i= 0; i<T.inUse; i++)
   {
     if( ioSock == T.socket[i] )
       break;
   }

   if( i < T.inUse )
   {
     for(++i; i<T.inUse; i++)
       T.socket[i-1]= T.socket[i];
   }

   select.remove(ioSock);
   delete ioSock;

   T.inUse--;
   T.waits--;
   T.latch.releaseXCL();
   if( T.waits == 0 )
     goto waitForWork;

waitForData:
   #ifdef HCDM
     tracef("%s %d: T(%d) Dwait\n", __SOURCE__, __LINE__, tid);
   #endif
   if( T.inUse != T.waits )
   {
     T.latch.obtainXCL();
     for(sid= T.waits; sid<T.inUse; sid++)
     {
       if( verbose > 3 || (verbose > 1 && pid == (pCount-1)) )
         prepend(pid, "T(%d)   added S(%p) P(%d)\n",
                      tid, T.socket[sid], T.socket[sid]->getHostPort());

       select.insert(T.socket[sid]);
     }

     T.waits= T.inUse;
     T.latch.releaseXCL();

     if( T.waits == 0 )
       goto waitForWork;
   }

   #ifdef HCDM
     tracef("%s %d: T(%d) Swait\n", __SOURCE__, __LINE__, tid);
   #endif
   ioSock= select.selectInp(30000);
   if( ioSock == NULL )
     goto waitForData;

   //-------------------------------------------------------------------------
   // Data available
   //-------------------------------------------------------------------------
   strcpy(dropReason, "Code Bug - reason not set");

   L= ioSock->recv(buffChar, sizeof(buffChar));
   #ifdef HCDM
     tracef("%s %d: T(%d) %d= %p->recv()\n",
            __SOURCE__, __LINE__, tid, L, ioSock);
   #endif
   if( L != sizeof(buffChar) )
   {
     if( L <= 0 )
     {
       sprintf(dropReason, "%4d: Receive Length(%d)", __LINE__, L);
       goto dropSocket;
     }

     O= L;
     for(;;)
     {
       L= ioSock->recv(&buffChar[O], sizeof(buffChar)-O);
       #ifdef HCDM
         tracef("%s %d: T(%d) %d= %p->recv()\n",
                __SOURCE__, __LINE__, tid, L, ioSock);
       #endif
       if( L <= 0 )
       {
         sprintf(dropReason, "%4d: Receive Length(%d)", __LINE__, L);
         goto dropSocket;
       }

       O += L;
       if( O == sizeof(buffChar) )
         break;
     }
   }

   buffSize= Network::ntoh16(*(Network::Net16*)buffChar);
   if( buffSize >= SIZEOF_BUFFER )
   {
     sprintf(dropReason, "%4d: buffSize(%d)", __LINE__, buffSize);
     goto dropSocket;
   }

   while( buffSize > 0 )
   {
     L= ioSock->recv(buffer, buffSize);
     #ifdef HCDM
       tracef("%s %d: T(%d) %d= %p->recv()\n",
              __SOURCE__, __LINE__, tid, L, ioSock);
     #endif
     if( L == 0 )
     {
       sprintf(dropReason, "%4d: Receive Length(%d)", __LINE__, L);
       goto dropSocket;
     }

     if( L < 0 )
     {
       sprintf(dropReason, "%4d: Receive Length(%d)", __LINE__, L);
       debugf("%s %d: T(%d) I/O error(%s)\n", __SOURCE__, __LINE__,
              tid, ioSock->getSocketEI());
       goto dropSocket;
     }

     buffSize -= L;
   }
   goto waitForData;

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
void
   doProcess(                       // Process function
     int               pid)         // Process identifier
{
   PerProcess&         P= common->process[pid]; // Address the PerProcess

   Socket*             listen;
   Socket*             ioSock;

   unsigned            attempts;    // Connection attempts
   unsigned            failures;    // Connection attempt failures
   int                 sid;         // Socket identifier
   int                 tid;         // Thread identifier

   //-------------------------------------------------------------------------
   //
   //   Match local process id to system process id.
   //
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: doProcess(%d) started\n", __SOURCE__, __LINE__, pid);
   #endif
   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
     prepend(pid, "PID(%d)\n", getpid());

   P.fsm= PerProcess::PS_OPERATIONAL;

   //-------------------------------------------------------------------------
   //
   //   Initialize
   //
   //-------------------------------------------------------------------------
   for(tid= 0; tid<tCount; tid++)
   {
     thread[tid].thread= new MyThread(pid,tid);
     thread[tid].inUse= 0;

     for(sid=0; sid<sCount; sid++)
     {
       thread[tid].socket[sid]= NULL;
     }
   }

   for(tid= 0; tid<tCount; tid++)   // Start all the Threads
   {
     thread[tid].fsm= PerThread::PS_INITIAL;
     thread[tid].thread->start();
   }

   for(tid=0; tid<tCount; tid++)    // Wait for all the Threads to start
   {
     while(thread[tid].fsm == PerThread::PS_INITIAL )
       Thread::sleep(1.0);
   }

   listen= new Socket(Socket::ST_STREAM);
   if( listen->setHost() )
   {
     debugf("%s %d: Unable to setHost EI(%s)\n", __SOURCE__, __LINE__,
            listen->getSocketEI());
     exit(EXIT_FAILURE);
   }

   P.addr= listen->getHostAddr();
   P.port= listen->getHostPort();
   P.fsm= PerProcess::PS_LISTENING;

   //-------------------------------------------------------------------------
   // Wait for connections
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: doProcess(%d) waiting\n", __SOURCE__, __LINE__, pid);
   #endif
   if( verbose > 2 || (verbose > 0 && pid == (pCount-1)) )
     prepend(pid, "Host(%s,%s) Port(%ld): Listening\n",
                  listen->getHostName(), listen->addrToChar(P.addr),
                  (long)listen->getHostPort());

   failures= 0;
   tid= tCount;
   for(;;)
   {
     ioSock= listen->listen();
     if( ioSock == NULL )
     {
       debugf("Unable to create connection(%s)\n",
              Software::getSystemEI(Software::getSystemEC()));
       Thread::sleep(15.0);
       continue;
     }

     for(attempts= 0;;attempts++)
     {
       tid++;
       if( tid >= tCount )
         tid= 0;

       thread[tid].latch.obtainXCL();

       sid= thread[tid].inUse;
       if( sid < sCount )
         break;

       thread[tid].latch.releaseXCL();

       if( attempts > (tCount*2) )
       {
         if( failures == 0 )
           prepend(pid, "Unable to assign connection(%.8X::%d)\n",
                   ioSock->getPeerAddr(), ioSock->getPeerPort());
         failures++;

         delete ioSock;
         continue;
       }
     }

     #ifdef HCDM
       prepend(pid, "tid(%d) sid(%d)= socket(%p)\n", tid, sid, ioSock);
     #endif
     thread[tid].socket[sid]= ioSock;
     thread[tid].inUse++;
     thread[tid].latch.releaseXCL();
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       doConnect
//
// Purpose-
//       Handle connection requests.
//
//----------------------------------------------------------------------------
void
   doConnect( void )                // Make connection requests
{
   Common&             C= *common;
   Socket*             master= new Socket(Socket::ST_DGRAM);

   int                 L;           // Transfer length
   int                 pid;         // Process iterator

   RMconnQ             request;     // Request  data
   RMconnS             response;    // Response data
   Socket::Addr        sockAddr;    // Socket address

   Clock               oldTime;     // Last statistic time
   Clock               nowTime;     // Current time

   //-------------------------------------------------------------------------
   // Wait for connections
   //-------------------------------------------------------------------------
   master->setHost();
   sockAddr= master->getHostAddr();
   prepend(-1, "Host(%s,%s) Port(%d): Master socket\n",
               master->getHostName(),
               master->addrToChar(sockAddr),
               master->getHostPort());
   prepend(-1, "Use RMsend %s %d\n",
               master->addrToChar(sockAddr), master->getHostPort());

   pid= 0;
   oldTime= Clock::current();
   for(;;)
   {
     #ifdef HCDM
       tracef("%s %d: doConnect master->recv\n", __SOURCE__, __LINE__);
     #endif
     L= master->recv((char*)&request, sizeof(request));
     if( L != sizeof(request) )
     {
       debugf("%s %d: recv length(%ld) EC(%d) EI(%s)\n",
              __SOURCE__, __LINE__, (long)L,
              master->getSocketEC(), master->getSocketEI());
       break;
     }

     if( Network::ntoh32(request.fc) == RMconnQ::FC_Final )
       break;
     if( Network::ntoh32(request.fc) != RMconnQ::FC_Connect )
     {
       debugf("%s %d: recv fc(%ld)\n", __SOURCE__, __LINE__,
              (long)Network::ntoh32(request.fc));
       continue;
     }

     response.host= Network::hton64(C.process[pid].addr);
     response.port= Network::hton32(C.process[pid].port);
     L= master->send((char*)&response, sizeof(response));
     if( L != sizeof(response) )
     {
       debugf("%s %d: send length(%ld)\n", __SOURCE__, __LINE__, (long)L);

       debugf("%s %d: %.8lX::%d  failure(%.8lX::%d)\n", __SOURCE__, __LINE__,
              (long)master->getPeerAddr(), master->getPeerPort(),
              (long)response.host, response.port);
       continue;
     }
     #ifdef HCDM
       debugf("%s %d: %.8lX::%d assigned(%.8lX::%d)\n", __SOURCE__, __LINE__,
              (long)master->getPeerAddr(), master->getPeerPort(),
              (long)C.process[pid].addr, C.process[pid].port);
     #endif

     // Next process
     pid++;
     if( pid >= pCount )
       pid= 0;

     // Time analysis
     nowTime= Clock::current();
     if( ((double)nowTime - (double)oldTime) > 60.0 )
     {
       oldTime= nowTime;
       printf("\n");
//     Latch::displayStatistics();
     }
   }

   delete master;
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

   int                 pid;         // Process identifier
   int                 status;      // How child completed
   char                string[1024];// Working string
   int                 success;     // Number of successful children

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
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

   Common&             C= *common;

   //-------------------------------------------------------------------------
   // If this is a started process, run it
   //-------------------------------------------------------------------------
   if( parmPid >= 0 )
   {
     #ifdef HCDM
       debugf("%s %d: pid(%d) started\n", __SOURCE__, __LINE__, parmPid);
     #endif
     doProcess(parmPid);            // Run the child process
     SharedMem::detach(common);     // Detach the common region
     SharedMem::remove(fileSegment);// Destroy the common region
     return 0;                      // And return
   }

   //-------------------------------------------------------------------------
   // Initialize main Thread
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: mainline\n", __SOURCE__, __LINE__);
   #endif
   memset(common, 0, sizeof(Common));
   new (common) Common();

   for (pid=0; pid<pCount; pid++)
   {
     C.process[pid].fsm= PerProcess::PS_INITIAL;
   }

// Latch::resetStatistics();        // Reset the latch statistics

   //-------------------------------------------------------------------------
   // Start all the child processes
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: starting child processes\n", __SOURCE__, __LINE__);
   #endif
   for (pid=0; pid<pCount; pid++)
   {
     sprintf(string, "-:%d", pid);
     strcat(string, parmString);

     C.process[pid].process= new Process();
     C.process[pid].process->start(argv[0], string);
   }

   //-------------------------------------------------------------------------
   // Wait for each child to be ready
   //-------------------------------------------------------------------------
   for(pid=0; pid<pCount; pid++)
   {
     while(common->process[pid].fsm != PerProcess::PS_LISTENING )
       Thread::sleep(1.0);
   }

   //-------------------------------------------------------------------------
   // Wait for connections
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("Listener ports:\n");
     for(pid=0; pid<pCount; pid++)
       debugf("[%3d] 0x%.8x::%d\n",
              pid, C.process[pid].addr, C.process[pid].port);
   #endif
   doConnect();

   //-------------------------------------------------------------------------
   // Wait for each child to complete
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %d: Waiting for completion\n", __SOURCE__, __LINE__);
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
   }
   debugf("%d of %d successful\n", success, pCount);

   //-------------------------------------------------------------------------
   // Cleanup
   //-------------------------------------------------------------------------
   SharedMem::detach(common);       // Detach the common region
   SharedMem::remove(fileSegment);  // Destroy the common region

   return(0);
}


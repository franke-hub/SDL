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
//       Test_Pro.CPP
//
// Purpose-
//       Test Process function.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>              // (For conditional test)

#include <com/Clock.h>              // (For conditional test)
#include <com/Debug.h>
#include <com/define.h>
#include "com/Thread.h"
#include "com/Process.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEST_PRO" // Source file, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_PROCESS               5 // Number of processes to start
#define MAX_THREADS               5 // Number of threads per process

//----------------------------------------------------------------------------
// Local data areas -- available to each process
//----------------------------------------------------------------------------
static int           parmPid;       // Process identifier (parameter)
static int           verbose;       // Verbosity

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::doThread
//
// Purpose-
//       Verify the correct operation of a Thread inside a Process.
//
//----------------------------------------------------------------------------
static int                          // Return code
   doThread(                        // Run a Thread
     int             pid,           // Process identifier
     int             tid)           // Thread identifier
{
   debugf("[%3d] doThread(%d)\n", pid, tid);

   #if 0
     Clock           after;         // Time last operation completed
     double          finTime;       // Expiration Time
     int             operations;    // Number of operations

     after= Clock::current()();
     finTime= (double)after + 5.0;
     for (operations=0; ;operations++)
     {
       after= Clock::current()();
       if( (double)after >= finTime )
         break;
     }
   #endif

   #if 0
     struct timeb    ticker;        // UTC time base
     int             operations;    // Number of operations

     ftime(&ticker);
     for (operations=0; operations<8000; operations++)
     {
       ftime(&ticker);
     }
   #endif

   #if 0
     Clock           after;         // Time last operation completed
     int             operations;    // Number of operations

     for (operations=0; operations<4000; operations++)
     {
       after= Clock::current()();
     }
   #endif

   return 0;
}

//----------------------------------------------------------------------------
//
// Class
//       QuietThread
//
// Purpose-
//       Define a simple, but quiet, Thread
//
//----------------------------------------------------------------------------
class QuietThread : public Thread { // Quiet Thread class
public:
   QuietThread(
     int             pid,           // Process identifier
     int             tid);          // Thread identifier

private:
virtual long
   run(void);

   int               pid;           // Process identifier
   int               tid;           // Thread identifier
}; // class QuietThread

//----------------------------------------------------------------------------
//
// Subroutine-
//       QuietThread::QuietThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   QuietThread::QuietThread(        // Constructor
     int             pid,           // Process identifier
     int             tid)           // Thread identifier
:  pid(pid)
,  tid(tid)
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       QuietThread::run
//
// Purpose-
//       Drive a thread.
//
//----------------------------------------------------------------------------
long
   QuietThread::run(void)           // Drive a thread
{
   return doThread(pid, tid);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::doProcess
//
// Purpose-
//       Verify the correct operation of a standard Process.
//
//----------------------------------------------------------------------------
static int                          // Return code
   doProcess(                       // Run a Process
     int             pid)           // Process identifier
{
   QuietThread*      thread[MAX_THREADS];

   int               tid;

   debugf("[%3d] doProcess()\n", pid);
   for(tid=0; tid<MAX_THREADS; tid++)
     thread[tid]= new QuietThread(pid, tid);

   for(tid=0; tid<MAX_THREADS; tid++)
     thread[tid]->start();

   for(tid=0; tid<MAX_THREADS; tid++)
     thread[tid]->wait();

   debugf("[%3d] doProcess() complete\n", pid);
   return 0;
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

   //-------------------------------------------------------------------------
   // Parameter defaults
   //-------------------------------------------------------------------------
   parmPid=      (-1);
   verbose=      1;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   for(argi=1; argi<argc; argi++)
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If switch parameter
     {
       argp++;                      // Skip the switch parameter

       if( *argp == 'V' )           // If verbosity
       {
         argp++;
         if( *argp == ':' )
         {
           argp++;
           verbose= atoi(argp);
         }
         else
         {
           error= TRUE;
           fprintf(stderr, "Missing ':' in -V option\n");
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
       fprintf(stderr, "Undefined parameter(%s)\n", argp);
       error= TRUE;
     }
   }

   if( error )
     info();
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
extern int
   main(                            // Mainline code
     int             argc,          // Argument count
     char           *argv[])        // Argument array
{
   Process*          process[MAX_PROCESS]; // Process array
   int               status[MAX_PROCESS];  // Completion status

   char              string[512];   // Working string
   int               pid;           // Local process id
   int               returncd;      // This routine's return code
   int               success;       // Number of successful children

   //-------------------------------------------------------------------------
   // Hard Core Debug Mode
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugSetIntensiveMode();
   #endif

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis
   debugf("%s %4d: started\n", __SOURCE__, __LINE__);

   //-------------------------------------------------------------------------
   // If this is a started process, run it
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: is this a child?\n", __SOURCE__, __LINE__);
   #endif
   if( parmPid >= 0 )
   {
     #ifdef HCDM
       debugf("%s %4d: Yes, running it\n", __SOURCE__, __LINE__);
     #endif
     returncd= doProcess(parmPid);  // Run the child process
     return returncd;               // And return
   }

   //-------------------------------------------------------------------------
   // Start all the child processes
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: No, starting children\n", __SOURCE__, __LINE__);
   #endif
   for (pid=0; pid<MAX_PROCESS; pid++)
   {
      sprintf(string, "-:%d", pid);

      process[pid]= new Process();
      process[pid]->start(argv[0], string);
   }

   //-------------------------------------------------------------------------
   // Wait for each child to complete
   //-------------------------------------------------------------------------
   #ifdef HCDM
     debugf("%s %4d: Wait for child processes\n", __SOURCE__, __LINE__);
   #endif
   success= MAX_PROCESS;
   for(pid=0; pid<MAX_PROCESS; pid++)
   {
     status[pid]= process[pid]->wait();
     if( status[pid] != 0 )
     {
       success--;
       fprintf(stderr, "[%3d] Failed, status(0x%.8X)\n", pid, status[pid]);
     }
     #ifdef HCDM
       debugf("%s %4d: [%3d] Process complete\n", __SOURCE__, __LINE__, pid);
     #endif
   }
   debugf("%d of %d successful\n", success, MAX_PROCESS);

   return(0);
}


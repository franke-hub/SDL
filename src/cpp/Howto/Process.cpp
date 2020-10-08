//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Process.cpp
//
// Purpose-
//       Sample program: How to create and use processes.
//
// Last change date-
//       2020/10/04
//
// Implementation notes-
//       mqueue.h:     Defines inter-process message queue functions.
//       spawn.h:      Defines process generation and control functions.
//
//----------------------------------------------------------------------------
#include <chrono>                   // For durations
#include <mutex>                    // For std::lock_guard
#include <thread>                   // For std::thread

#include <ctype.h>                  // For isprint()
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For O_* constants
#include <getopt.h>                 // For getopt_long()
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <mqueue.h>                 // For mq_* functions
#include <semaphore.h>              // For semaphore
#include <signal.h>                 // For sigevent
#include <spawn.h>                  // For posix_spawn functions
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <time.h>                   // For timespec
#include <unistd.h>                 // For getpid, ...
#include <sys/stat.h>               // For S_* constants and macros
#include <sys/wait.h>               // For waitpid

#include <pub/Debug.h>              // For debugging functions
#include <pub/Exception.h>          // For pub::Exception, std::exception
#include <pub/Latch.h>              // For pub::NullLatch
#include <pub/utility.h>            // For pub::utility::dump (debugging)
using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging functions

#include "Process.h"                // Helper classes

//----------------------------------------------------------------------------
// Compile-time options
//----------------------------------------------------------------------------
enum // Compile time-options
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_DELAY= 100                   // Default delay, milliseconds
,  MQ_SIGNO= SIGIO                  // Our default signal number
}; // Compile time-options

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
static const int       MAX_SENDS= 4; // Maxium queue length
static const int       MAX_QUEUE= FILENAME_MAX + 8; // Maxium message length
static const char*     MSG_QUEUE= "/Process.cpp"; // Our message queue name

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_hcdm= false; // Hard Core Debug Mode
static int             opt_index;   // Option index

static int             opt_delay_rd= 0; // Read delay
static int             opt_delay_wr= 0; // Send delay
static int             opt_delay_ex= 0; // Notify_call exit delay
static int             opt_per_pid= false; // Use a debug file per process?
static int             opt_reset= false; // Reset everything
static int             opt_signal= 0; // Use signal number (0 if none)
static int             opt_verbose= -1; // --verbose=n (Verbosity)

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",     no_argument,       &opt_help,    true}
,  {"hcdm",     no_argument,       &opt_hcdm,    true}

,  {"delay-rd", optional_argument, &opt_delay_rd, USE_DELAY}
,  {"delay-wr", optional_argument, &opt_delay_wr, USE_DELAY}
,  {"delay-ex", optional_argument, &opt_delay_ex, USE_DELAY}
,  {"per-pid",  no_argument,       &opt_per_pid,  true}
,  {"reset",    no_argument,       &opt_reset,    true}
,  {"signal",   optional_argument, &opt_signal,   MQ_SIGNO}
,  {"verbose",  optional_argument, &opt_verbose,  0}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM

,  OPT_DELAY_RD
,  OPT_DELAY_WR
,  OPT_DELAY_EX
,  OPT_PER_PID
,  OPT_RESET
,  OPT_SIGNAL
,  OPT_VERBOSE
,  OPT_SIZE
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Debug*          debug= nullptr; // Our Debug object
static int             operational= false; // TRUE while operational

//============================================================================
// Class: Main (Mainline code)
//----------------------------------------------------------------------------
class Main : public Catcher {       // Mainline code
public: // Attributes --------------------------------------------------------
int                    argc;        // Argument count
char**                 argv;        // Argument array

mqd_t                  fd;          // The Message Queue Descriptor

// Constructor ---------------------------------------------------------------
   Main(int argc, char* argv[])
:  Catcher(), argc(argc), argv(argv), fd(-1)
{
// if( opt_verbose > 0 )            // (For reference)
//   debugf("[%6d] Main(%p)::Main()\n", getpid(), this);
// debugf("[%6d] %4d HCDM\n", getpid(), __LINE__); // (For reference)
   if( opt_verbose > 0 ) {
     debugf("[%6d] Main(%p)::Main\n", getpid(), this);

     if( opt_verbose > 1 ) {
       for(int i= 0; i<=argc; i++)
         debugf("[%2d] '%s'\n", i, argv[i]);
     }
   }
}

// Signal handling methods ---------------------------------------------------
        void notify_attr( void );   // Set O_NONBLOCK attribute
static  void notify_call(int, siginfo_t*, void*); // The signal handler
        void notify_init( void );   // Set up signal handler

// Methods -------------------------------------------------------------------
        int  process( void );       // Spawned process operation
static  void thread(void*);         // Thread driver, creates and runs a Task

        int  reset( void );         // Reset message queues
virtual int  run( void );           // The mainline code
}; // class Main =============================================================

//============================================================================
// Class: Task (Message reader thread)
//----------------------------------------------------------------------------
class Task : public Catcher {       // Message reader Thread
public: // Attributes --------------------------------------------------------
mqd_t                  fd;          // The Message Queue Descriptor

// Constructor ---------------------------------------------------------------
   Task(mqd_t fd)
:  Catcher(), fd(fd) {}

// Methods -------------------------------------------------------------------
virtual int  run( void );           // Operate the reader Thread
}; // class Task =============================================================

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
typedef struct sigaction sigaction_t;
static sigaction_t     sys1_action; // System MQ_SIGNO signal handler

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init(int, char**)                // Initialize
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   //-------------------------------------------------------------------------
   // Initialize signal handling
   if( opt_signal ) {
     sigaction_t sa;
     memset(&sa, 0, sizeof(sa));
     sa.sa_sigaction= Main::notify_call;
     sa.sa_flags= SA_SIGINFO;

     int rc= sigaction(opt_signal, &sa, &sys1_action);
     if( rc ) {
       errorf("%4d sigaction: %s\n", __LINE__, error());
       return 1;
     }
   }

   //-------------------------------------------------------------------------
   // Initialize/activate debugging trace (with options)
   if( opt_per_pid ) {              // Multiple debug files?
     char buffer[128];              // Working buffer
     sprintf(buffer, "debug.%.6d", getpid()); // Debug output file
     printf("Buffer:%s:\n", buffer);
     debug= new Debug(buffer);      // Create our Debug object
     Debug::set(debug);             // Set it as the default

     if( HCDM ) opt_hcdm= true;     // If HCDM compile-time, force opt_hcdm
     if( opt_hcdm ) {               // If --hcdm option specified
       debug->set_mode(Debug::MODE_INTENSIVE); // Hard Core Debug Mode
     }
   } else {
     Debug* debug= Debug::get();    // (To set file mode before use)
     debug->set_file_mode("ab");    // (Append so second PID doesn't truncate)

     if( HCDM ) opt_hcdm= true;     // If HCDM compile-time, force opt_hcdm
     if( opt_hcdm ) {               // If --hcdm option specified
       debug->set_mode(Debug::MODE_INTENSIVE); // Hard Core Debug Mode
     }
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       terminate
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
   //-------------------------------------------------------------------------
   // Restore system signal handlers
   if( opt_signal ) {
     sigaction(opt_signal, &sys1_action, nullptr);
   }

   //-------------------------------------------------------------------------
   // Terminate debugging
   if( debug ) {
     delete debug;
     debug= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       to_integer
//
// Purpose-
//       Convert string to integer, handling error cases
//
// Implementation note-
//       Leading or trailing blanks are NOT allowed.
//
//----------------------------------------------------------------------------
static int                          // The integer value
   to_integer(                      // Extract and verify integer value
     const char*       inp)         // From this string
{
   errno= 0;
   char* strend;                    // Ending character
   long value= strtol(inp, &strend, 0);
   if( strend == inp || *inp == ' ' || *strend != '\0' )
     errno= EINVAL;
   else if( value < INT_MIN || value > INT_MAX )
     errno= ERANGE;

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm_int
//
// Purpose-
//       Convert parameter to integer, handling error cases
//
// Implementation note-
//       optarg: The argument string
//       opt_index: The argument index
//
//----------------------------------------------------------------------------
static int                          // The integer value
   parm_int( void )                 // Extract and verify integer value
{
   int value= to_integer(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static int                          // Return code (Always 1)
   info( void)                      // Parameter description
{
   fprintf(stderr, "%s <options>\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"

                   "  --delay-rd\tDelay after receive (ms)\n"
                   "  --delay-wr\tDelay after send (ms)\n"
                   "  --delay-ex\tDelay after signal complete (ms)\n"
                   "  --per-pid\tUse process unique debug files\n"
                   "  --reset\tReset (clean up)\n"
                   "  --signal\tUse signal mq_notify\n"
                   "  --verbose{=n}\tVerbosity, default 0\n"
                   , __FILE__
          );

   return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 if OK)
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   int C;                           // The option character
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 )
   {
     switch( C )
     {
       case 0:
       {{{{
         switch( opt_index )
         {
           case OPT_HELP:           // These options handled by getopt
           case OPT_HCDM:
           case OPT_PER_PID:
           case OPT_RESET:
             break;

           case OPT_SIGNAL:
             if( optarg ) {
               opt_signal= parm_int();
               if( errno == 0 ) {
                 if( opt_signal <= 0 || opt_signal >= 64 ) {
                   opt_help= true;
                   fprintf(stderr, "%4d '%s' Invalid, range 1..63\n", __LINE__
                                 , argv[optind-1]);
                 }
               }
             }
             break;

           case OPT_DELAY_RD:
             if( optarg )
               opt_delay_rd= parm_int();
             break;

           case OPT_DELAY_WR:
             if( optarg )
               opt_delay_wr= parm_int();
             break;

           case OPT_DELAY_EX:
             if( optarg )
               opt_delay_ex= parm_int();
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           default:
             fprintf(stderr, "%4d Unexpected opt_index(%d)\n", __LINE__,
                             opt_index);
             break;
         }
         break;
       }}}}

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Option requires an argument '%s'.\n", __LINE__,
                           argv[optind-1]);
         else
           fprintf(stderr, "%4d Option requires an argument '-%c'.\n", __LINE__,
                           optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Unknown option '%s'.\n", __LINE__,
                           argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "%4d Unknown option '-%c'.\n", __LINE__, optopt);
         else
           fprintf(stderr, "%4d Unknown option character '0x%x'.\n", __LINE__,
                           (optopt & 0x00ff));
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).\n", __LINE__,
                         C, (C & 0x00ff));
         break;
     }
   }

   // Disallow positional parameters
   for(int i= optind; i < argc; i++ ) {
     opt_help= true;
     fprintf(stderr, "'%s' Positional parameter not supported\n", argv[i]);
   }

   // Return sequence
   int rc= 0;
   if( opt_help )
     rc= info();
   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::notify_attr
//
// Purpose-
//       Update attributes for signal-driven operation
//
// Implementation notes-
//       The O_NONBLOCK attribute is needed so that notify_call can exit,
//       although mq_timedreceive could probably be used also. (Not tested.)
//
//----------------------------------------------------------------------------
void
   Main::notify_attr( void )        // Update signal-driven attributes
{
   mq_attr attr;                    // Attribute work area
   mq_getattr(fd, &attr);           // Get the attributes
   attr.mq_flags |= O_NONBLOCK;     // Set non-blocking attribute
   mq_setattr(fd, &attr, nullptr);  // Set the attributes
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::notify_call
//
// Purpose-
//       Handle signal event
//
//----------------------------------------------------------------------------
void
   Main::notify_call(               // Handle signal event
     int               signo,       // Signal number
     siginfo_t*        siginfo,     // Signal information
     void*             ignore)      // Context
{
   (void)ignore;                    // Unused parameter

   Main* main= (Main*)siginfo->si_value.sival_ptr;
   mqd_t fd= main->fd;

   if( opt_verbose > 0 )
     debugf("[%6d] Main(%p)::notify_call(%d) 0x%lx\n", getpid()
            , main, signo, (long)fd);

   //-------------------------------------------------------------------------
   // The signal handler MUST be reinitialized, or it won't be invoked again
   // (This MUST be done before draining the queue)
   main->notify_init();

   //-------------------------------------------------------------------------
   // The message queue MUST be drained, or it won't be invoked again
   char buffer[MAX_QUEUE];
   for(;;) {                        // Drain the message queue
     ssize_t L= mq_receive(fd, buffer, sizeof(buffer), nullptr);
     if( L >= 0 ) {
       buffer[L]= '\0';
       debugf("[%6d] %4d <<<<(%s,%zd)\n", getpid(), __LINE__, buffer, L);
       if( opt_delay_rd > 0 )
         std::this_thread::sleep_for(std::chrono::milliseconds(opt_delay_rd));
     } else {
       if( errno == EAGAIN )        // If the queue was empty
         break;
       if( errno == EINTR )         // If interrupted
         continue;

       errorf("%4d mq_receive(0x%lx): %s\n", __LINE__, (long)fd, error());
       break;
     }
   }

   //-------------------------------------------------------------------------
   // Exit delay
   if( opt_delay_ex > 0 )
     std::this_thread::sleep_for(std::chrono::milliseconds(opt_delay_ex));
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::notify_init
//
// Purpose-
//       Initialize signal handler
//
//----------------------------------------------------------------------------
void
   Main::notify_init( void )        // Initialize signal event handling
{
   if( opt_verbose > 0 )
     debugf("[%6d] Main(%p)::notify_init()\n", getpid(), this);

   struct sigevent sev;
   memset(&sev, 0, sizeof(sev));

   sev.sigev_notify= SIGEV_SIGNAL;  // Using SIGNAL notification method
   sev.sigev_signo= opt_signal;     // The signal number
   sev.sigev_value.sival_ptr= this; // (Indirect pointer to fd)

   int rc= mq_notify(fd, &sev);
   if( rc )
     errorf("%4d mq_notify(0x%lx): %s\n", __LINE__, (long)fd, error());
   else if( opt_verbose > 1 )
     debugf("[%6d] %4d %d=mq_notify(0x%lx)\n", getpid(), __LINE__
            , rc, (long)fd);
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::process
//
// Purpose-
//       Run the spawned process
//
//----------------------------------------------------------------------------
int                                 // Error count
   Main::process( void )            // Run the spawned process
{
   if( opt_verbose > 0 )
     debugf("[%6d] Main(%p)::process()\n", getpid(), this);

   mq_attr attr;                    // Attributes
   memset(&attr, 0, sizeof(attr));  // (Initialize all zeros)
   attr.mq_maxmsg= MAX_SENDS;       // Maximum number of messages in queue
   attr.mq_msgsize= MAX_QUEUE;      // Maximum message length
   mqd_t fd= mq_open(MSG_QUEUE, O_RDWR, S_IRWXU, &attr);
   if( fd < 0 ) {
     errorf("%4d mq_open(%s): %s", __LINE__, MSG_QUEUE, error());
     return 1;
   }
   if( opt_verbose > 1 )
     debugf("[%6d] %4d 0x%lx=mq_open(%s)\n", getpid(), __LINE__
            , (long)fd, MSG_QUEUE);

   //-------------------------------------------------------------------------
   // Send some messages
   char buffer[512];
   for(int i= 0; i<12; i++) {
     int L= snprintf(buffer, sizeof(buffer), "[%2d] Message", i);
     int rc= mq_send(fd, buffer, L, 0);
     if( rc ) {
       errorf("%4d mq_send(0x%lx,%s,%d): %s\n", __LINE__
               , (long)fd, buffer, L, error());
     } else {
       debugf("[%6d] %4d >>>>(%s,%d)\n", getpid(), __LINE__, buffer, L);
       if( opt_delay_wr > 0 )
         std::this_thread::sleep_for(std::chrono::milliseconds(opt_delay_wr));
     }
   }

   //-------------------------------------------------------------------------
   // Close the message queue
   int rc= mq_close(fd);
   if( rc )
     errorf("%4d mq_close(0x%lx): %s\n", __LINE__, (long)fd, error());

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::reset
//
// Purpose-
//       Reset the message queues
//
//----------------------------------------------------------------------------
int                                 // Error count
   Main::reset( void )              // Reset the message queues
{
   int error_count= 0;

   int
   rc= mq_unlink(MSG_QUEUE);        // Delete the message queue
   if( rc ) {
     errorf("%4d mq_unlink(%s): %s\n", __LINE__, MSG_QUEUE, error());
     error_count++;
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::run
//
// Purpose-
//       The mainline code, with try/catch wrapper
//
//----------------------------------------------------------------------------
int                                 // Error count
   Main::run( void )                // Mainline code
{
   if( opt_verbose > 0 )
     debugf("[%6d] Main(%p)::run()\n", getpid(), this);

   //-------------------------------------------------------------------------
   // If required, delete the message queue
   if( opt_reset )
     return reset();

   //-------------------------------------------------------------------------
   // Create the message queue
   mq_attr attr;                    // Attributes
   memset(&attr, 0, sizeof(attr));  // (Initialize all zeros)
   attr.mq_maxmsg= MAX_SENDS;       // Maximum number of messages in queue
   attr.mq_msgsize= MAX_QUEUE;      // Maximum message length
   errno= 0;
   mqd_t fd= mq_open(MSG_QUEUE, O_RDWR | O_CREAT | O_EXCL, S_IRWXU, &attr);
   if( fd < 0 ) {
     if( errno == EEXIST ) {        // If already open
       exit( process() );           // We must be the child process
     }

     errorf("%4d mq_open(%s): %s\n", __LINE__, MSG_QUEUE, error());
     return 1;
   }
   if( opt_verbose > 1 )
     debugf("[%6d] %4d 0x%lx=mq_open(%s)\n", getpid(), __LINE__
            , (long)fd, MSG_QUEUE);

   this->fd= fd;

   //-------------------------------------------------------------------------
   // Create the child thread or signal handler
   operational= true;               // We are operational
   std::thread reader;              // (Keep in scope)
   if( opt_signal ) {
     notify_attr();
     notify_init();
   } else {
     std::thread t(thread, this);   // Create and start the child thread
     reader= std::move(t);          // (Back in scope)
   }

   //-------------------------------------------------------------------------
   // Create the child process
   int pid;
   int
   rc= posix_spawn(&pid, argv[0], nullptr, nullptr, argv, environ);
   if( rc ) {
     errorf("%4d posix_spawn(%s): %s\n", __LINE__, argv[0], error());
     return 1;
   }

   if( opt_verbose > 1 )
     debugf("[%6d] spawned(%d)\n", getpid(), pid);
   for(;;) {
     errno= 0;
     pid_t wait= waitpid(pid, &rc, 0);
     if( wait == pid )
       break;

     if( errno == EINTR )
       continue;

     errorf("%4d waitpid(%d): %s\n", __LINE__, pid, error());
     break;
   }
   operational= false;              // The thread can complete

   if( opt_signal == false )        // If using thread reader
     reader.join();                 // Wait for it to complete

   //-------------------------------------------------------------------------
   // Close the message queue
   rc= mq_close(fd);
   if( rc )
     errorf("%4d mq_close(0x%lx): %s\n", __LINE__, (long)fd, error());

   //-------------------------------------------------------------------------
   // And we're done
   return reset();                  // Clean up and exit
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::thread
//
// Purpose-
//       Drive the mq_receive thread
//
//----------------------------------------------------------------------------
void
   Main::thread(                    // Drive the mq_receive thread
     void*             self)        // (Main*)
{
   Main* main= (Main*)self;         // Our Main*
   if( opt_verbose > 0 )
     debugf("[%6d] Main(%p)::thread()\n", getpid(), self);

   Task task(main->fd);             // Add try/catch wrapper
   task.start();                    // Drive the mq_receive thread
}

//----------------------------------------------------------------------------
//
// Method-
//       Task::run
//
// Purpose-
//       Operate the mq_receive thread
//
//----------------------------------------------------------------------------
int                                 // Error count
   Task::run( void )                // Operate the mq_receive thread
{
   if( opt_verbose > 0 )
     debugf("[%6d] Task(%p)::run(0x%lx)\n", getpid(), this, (long)this->fd);

   mqd_t fd= this->fd;

   //-------------------------------------------------------------------------
   // Receive queued messages
   char buffer[MAX_QUEUE];
   struct timespec timeout;
   while( operational ) {
     clock_gettime(CLOCK_REALTIME, &timeout);
     timeout.tv_sec += 3;           // Three second timeout
     ssize_t L= mq_timedreceive(fd, buffer, sizeof(buffer), nullptr, &timeout);
     if( L >= 0 ) {
       buffer[L]= '\0';
       debugf("[%6d] %4d <<<<(%s,%zd)\n", getpid(), __LINE__, buffer, L);
       if( opt_delay_rd > 0 )
         std::this_thread::sleep_for(std::chrono::milliseconds(opt_delay_rd));
     } else {
       if( errno == EINTR || errno == ETIMEDOUT )
         continue;

       errorf("%4d mq_timedreceive: %s\n", __LINE__, error());
       operational= false;
     }
   }

   if( opt_verbose > 1 )
     debugf("[%6d] %4d Task::run() complete\n", getpid(), __LINE__);
   return 0;
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
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   int rc= parm(argc, argv);        // Argument analysis
   if( rc ) return rc;              // Return if invalid

   rc= init(argc, argv);            // Initialize
   if( rc ) return rc;              // Return if invalid

   if( opt_verbose >= 0 ) {
     debugf("[%6d] %s: %s %s\n", getpid(), __FILE__, __DATE__, __TIME__);
     debugf("[%6d] --hcdm(%d) --reset(%d) --signal(%d) --verbose(%d)\n"
            , getpid(), opt_hcdm, opt_reset, opt_signal, opt_verbose);
     debugf("[%6d] --delay_rd(%d) --delay_wr(%d) --delay_ex(%d)\n", getpid()
            , opt_delay_rd, opt_delay_wr, opt_delay_ex);
   }

   //-------------------------------------------------------------------------
   // Operate
   //-------------------------------------------------------------------------
   Main main(argc, argv);
   rc= main.run();

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   debugf("%d Error%s\n", rc, rc == 1 ? "" : "s");
   term();                          // Termination cleanup

   if( rc )
     rc= 1;
   return rc;
}

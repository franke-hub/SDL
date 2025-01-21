//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Brian.cpp
//
// Purpose-
//       Brian mainline.
//
// Last change date-
//       2025/01/21
//
//----------------------------------------------------------------------------
#include <cstdlib>                  // For getenv
#include <ctime>                    // For time subroutines

#include <fcntl.h>                  // For open, O_*, ...
#include <getopt.h>                 // For getopt()
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, munmap, ...
#include <sys/signal.h>             // For signal, ...

#include <pub/Debug.h>              // For namespace debugging
#include <pub/Exception.h>          // For catch(pub::Exception)
#include <pub/Thread.h>             // For pub::Thread::sleep
#include <pub/Trace.h>              // For pub::Trace
#include <pub/Worker.h>             // For pub::WorkerPool

#include "Command.h"                // For Command
#include "Common.h"                 // For Common, StaticCommon
#include "Loader.h"                 // For Loader

#include "HttpMapper.h"             // For HttpMapper [sizeof]
#include "HttpListen.h"             // For HttpListen [sizeof]
#include "HttpServer.h"             // For HttpServer [sizeof]

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;                   // For Debug object
using PUB::Exception;               // For Exception handling
using PUB::Thread;                  // For Thread::sleep
using PUB::Trace;                   // For Debug object
using namespace PUB::debugging;     // For debugging subroutines

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_hcdm= HCDM; // --hcdm
static int             opt_verbose= VERBOSE; // --verbose{=verbosity}
static int             opt_help= false; // --help or error
static unsigned        opt_trace= 0x0010'0000; // Trace table size

static int             opt_index;   // Option index

static struct option   OPTS[]=      // Options
{  {"hcdm",    no_argument,       &opt_hcdm,    true}
,  {"verbose", optional_argument, nullptr,      0}
,  {"help",    no_argument,       &opt_help,    true}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HCDM
,  OPT_VERBOSE
,  OPT_HELP
};

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static void sig_handler(int);       // The signal handler
static inline void term( void );    // Terminate

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
Loader                 loader;      // Include built-in objects

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
Common*                common= nullptr; // Brian's Common area
void*                  trace_table= nullptr; // Internal trace table

// Signal handlers
typedef void           (*sig_handler_t)(int);
static sig_handler_t   sys1_handler= nullptr; // System SIGINT  signal handler
static sig_handler_t   sys2_handler= nullptr; // System SIGSEGV signal handler
static sig_handler_t   usr1_handler= nullptr; // System SIGUSR1 signal handler
static sig_handler_t   usr2_handler= nullptr; // System SIGUSR2 signal handler

//----------------------------------------------------------------------------
//
// Class-
//       Command_trap
//
// Purpose-
//       Allow for breakpoints here
//
//----------------------------------------------------------------------------
static class Command_trap : public Command {
public:
   Command_trap() : Command("trap")
{  }

virtual Command::resultant          // Resultant
   main(int, char**)                // Handle Command
{  debugh("trap\n");                // Line 0092
   return nullptr;
}
} command_trap; // static class Command_trap

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static void
   info( void)                      // Parameter description
{
   fprintf(stderr, "Brian [options]\n"
                   "Options:\n"
                   "  --hcdm\n"
                   "  --verbose{=value}\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Method-
//       init_trace
//
// Purpose-
//       Initialize memory mapped trace file
//
//----------------------------------------------------------------------------
static void*                        // The (initialized) trace file
   init_trace(                      // Initialize memory mapped trace file
     const char*       file,        // The trace file name
     unsigned          size)        // The trace file size
{
   if( size_t(size) > size_t(Trace::TABLE_SIZE_MAX) )
     size= Trace::TABLE_SIZE_MAX;
   else if( size_t(size) < size_t(Trace::TABLE_SIZE_MIN) )
     size= Trace::TABLE_SIZE_MIN;

   int mode= O_RDWR | O_CREAT;
   int perm= S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
   int fd= open(file, mode, perm);
   if( fd < 0 ) {
     debugf("%4d open(%s) %s\n", __LINE__, file, strerror(errno));
     return nullptr;
   }

   int rc= ftruncate(fd, size);     // (Truncate/expand to size)
   if( rc ) {
     debugf("%4d ftruncate(%s,%.8x) %s\n", __LINE__
           , file, size, strerror(errno));
     close(fd);
     return nullptr;
   }

   mode= PROT_READ | PROT_WRITE;
   void* table= mmap(nullptr, size, mode, MAP_SHARED, fd, 0);
   if( table == MAP_FAILED ) {    // If no can do
     debugf("%4d mmap(%s,%.8x) %s\n", __LINE__, file, size, strerror(errno));
     close(fd);
     return nullptr;
   }

   Trace::table= Trace::make(table, size);
   close(fd);                     // Descriptor not needed once mapped

   Trace::trace(".INI", 0, "TRACE STARTED") ;
   return table;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static inline void
   init( void )                     // Initialize
{
   // Set intensive debug mode
   debug_set_mode(Debug::MODE_INTENSIVE);

   // Initialize Common areas
   common= Common::make();          // Create the Common area
   StaticCommon::make();            // Initialize the StaticCommon area

   // Startup message
   char buffer[64];
   time_t now= time(nullptr);       // The current time
   size_t L= strftime(buffer, sizeof(buffer), "%b %d %Y %H:%M:%S"
                     , localtime(&now));
   if( L == 0 )                     // If ridiculous resultant
     strcpy(buffer, "Today");       // Set ridiculous time

   debugh("==============================================\n");
   debugh("======== Starting Brian\n");
   debugh("======== Compiled %s %s\n", __DATE__, __TIME__);
   debugh("========  Started %s\n", buffer);
   debugh("==============================================\n");
   if( opt_verbose ) {              // Display environment
     debugh("OPTIMIZE:   %s\n", getenv("OPTIMIZE"));
     debugh("USE_STATIC: %s\n", getenv("USE_STATIC"));
   }

   // Initialize trace table
   trace_table= init_trace("./trace.mem", opt_trace);

   //-------------------------------------------------------------------------
   // Initialize signal handling
   sys1_handler= signal(SIGINT,  sig_handler);
   sys2_handler= signal(SIGSEGV, sig_handler);
   usr1_handler= signal(SIGUSR1, sig_handler);
   usr2_handler= signal(SIGUSR2, sig_handler);

   // Startup complete event
   struct Startup_complete : public pub::signals::Event {
   } startup_complete;

   static_common->startup_complete.signal(startup_complete); // Raise startup_complete
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sig_handler
//
// Purpose-
//       Handle system signals.
//
//----------------------------------------------------------------------------
static void
   sig_handler(                     // Handle system signals
     int               id)          // The signal identifier
{
   static int recursion= 0;         // Signal recursion depth
   if( recursion ) {                // If signal recursion
     fprintf(stderr, "sig_handler(%d) recursion\n", id);
     fflush(stderr);
     exit(EXIT_FAILURE);
   }

   // Handle signal
   recursion++;                     // Disallow recursion
   const char* text= "<<Unexpected>>";
   if( id == SIGINT ) text= "SIGINT";
   else if( id == SIGSEGV ) text= "SIGSEGV";
   else if( id == SIGUSR1 ) text= "SIGUSR1";
   else if( id == SIGUSR2 ) text= "SIGUSR2";
   fprintf(stderr, "sig_handler(%d) %s\n", id, text);

   switch(id) {                     // Handle the signal
     case SIGINT:                   // (Console CTRL-C)
       Trace::trace(".BUG", __LINE__, text);
       debug_set_mode(Debug::MODE_INTENSIVE);
       term();                      // Termination cleanup, then
       exit(EXIT_FAILURE);          // Unconditional immediate exit
       break;

     case SIGSEGV:                  // (Program fault)
       Trace::trace(".BUG", __LINE__, text);
       debug_set_mode(Debug::MODE_INTENSIVE);
       debug_backtrace();           // Attempt diagnosis (recursion aborts)
       debugf("SIGSEGV: terminated\n");
       exit(EXIT_FAILURE);
       break;

     case SIGUSR1:                  // Diagnostic signal
     case SIGUSR2: {
       Trace::trace(".SIG", __LINE__, text);
       StaticCommon::DiagnosticEvent event(id);
       static_common->run_diagnostics.signal(event);
       break;
     }
     default:                       // (Unexpected)
       Trace::trace(".BUG", __LINE__, text);
       break;                       // (No configured action)
   }

   recursion--;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sizes
//
// Purpose-
//       Display object sizes
//
//----------------------------------------------------------------------------
#define SIZEOF(x) debugf("  " #x ": 0x%.4zx, %4zd\n", sizeof(x), sizeof(x));

static inline void
   sizes( void )                    // Display object sizes
{
   debugf("\nObject sizes:\n");
   SIZEOF(HttpMapper);
   SIZEOF(HttpListen);
   SIZEOF(HttpServer);
   debugf("\n\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       term_trace
//
// Purpose-
//       Terminate internal trace
//
//----------------------------------------------------------------------------
static void
   term_trace(                       // Terminate internal trace
     void*             table,        // Output from init_trace
     int               size)         // The trace table size
{
   if( table ) {
     if( size_t(size) > size_t(Trace::TABLE_SIZE_MAX) )
       size= Trace::TABLE_SIZE_MAX;
     else if( size_t(size) < size_t(Trace::TABLE_SIZE_MIN) )
       size= Trace::TABLE_SIZE_MIN;

     Trace::table= nullptr;
     munmap(table, size);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Teminate
//
//----------------------------------------------------------------------------
static inline void
   term( void )                     // Terminate
{
   term_trace(trace_table, opt_trace); // Terminate internal trace

   delete common;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis example.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 C;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   opterr= 0;                       // Do not write error messages

   while( (C= getopt_long(argc, (char**)argv, ":", OPTS, &opt_index)) != -1 )
     switch( C )
     {
       case 0:
         switch( opt_index )
         {
           case OPT_HCDM:           // Flags
           case OPT_HELP:
             break;

           case OPT_VERBOSE:
             opt_verbose= VERBOSE + 1; // Default "extra" verbosity
             if( optarg )
               opt_verbose= atoi(optarg);
             break;

           default:
             break;
         }
         break;

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Option requires an argument '%s'.\n",
                           argv[optind-1]);
         else
           fprintf(stderr, "Option requires an argument '-%c'.\n", optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Unknown option '%s'.\n", argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "Unknown option '-%c'.\n", optopt);
         else
           fprintf(stderr, "Unknown option character '0x%x'.\n", optopt);
         break;

       default:
         fprintf(stderr, "%4d SNO ('%c',0x%x).\n", __LINE__, C, C);
         exit( EXIT_FAILURE );
     }

   if( opt_help )
     info();

   if( opt_verbose < VERBOSE )
     opt_verbose= VERBOSE;
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
   // Initialiize
   //-------------------------------------------------------------------------
   parm(argc, argv);

   debug_set_head(Debug::HEAD_TIME | Debug::HEAD_THREAD);
   if( opt_hcdm || opt_verbose > 1 ) {
     debug_set_mode(Debug::MODE_INTENSIVE);
   }

   //-------------------------------------------------------------------------
   // Operate Brian
   //-------------------------------------------------------------------------
   try {
     init();                        // Initialize

     // Bringup tests, currently unused
     if( false ) {                  // (Handled properly)
       debugh("Should raise SIGSEGV\n");
       Common* common= nullptr;
       common->shutdown();
       debugh("ShouldNotOccur\n");
     }

     if( false ) {                  // (Handled properly)
       debugh("Should raise SIGABRT\n");
       abort();
       debugh("ShouldNotOccur\n");
     }

     if( false ) {                  // (Handled properly)
       debugh("Should throw(const char*)\n");
       throw "That's all, Folks";
       debugh("ShouldNotOccur\n");
     }

     //-----------------------------------------------------------------------
     // Wait for quit command
     common->wait();
     pub::WorkerPool::reset();

     // Termination delay longer than Server read timeout
     // printf("(Termination delay)\n");
     // Thread::sleep(3.125);       // Termination cleanup delay
   } catch(const char* X) {
     debugh("Exception(const char* %s)\n", X);
     debug_backtrace();
   } catch(Exception& X) {
     debugh("%4d %s\n", __LINE__, std::string(X).c_str());
     debug_backtrace();
   } catch(std::exception& X) {
     debugh("catch(std::exception.what(%s))\n", X.what());
     debug_backtrace();
   } catch(...) {
     debugh("Exception(...)\n");
     debug_backtrace();
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   term();

   traceh("...Brian complete\n");
   printf("...Brian complete\n");

   if( true ) {
     sizes();                       // Display interesting object sizes
   }

   return 0;
}

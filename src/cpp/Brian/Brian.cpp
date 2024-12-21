//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2024 Frank Eskesen.
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
//       2024/12/20
//
//----------------------------------------------------------------------------
#include <fcntl.h>                  // For open, O_*, ...
#include <getopt.h>                 // For getopt()
#include <stdlib.h>                 // For getenv
#include <time.h>                   // For time subroutines
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, munmap, ...

#include <pub/Debug.h>              // For namespace debugging
#include <pub/Exception.h>          // For catch(pub::Exception)
#include <pub/Thread.h>             // For pub::Thread::sleep
#include <pub/Trace.h>              // For pub::Trace

#include "_STATUS.H"                // For _STATUS
#include "Command.h"                // For Command
#include "Common.h"                 // For Common
#include "Loader.h"                 // For Loader

#define PUB _PUB_NAMESPACE
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
// External references
//----------------------------------------------------------------------------
Loader                 loader;      // Include built-in objects

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
Common*                common= nullptr; // Brian's Common area
void*                  trace_table= nullptr; // Internal trace table

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
                   "  --debug=file_name\n"
                   "  --verbosity{=value}\n"
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
   // Startup message
   char buffer[64];
   time_t now= time(nullptr);       // The current time
   size_t L= strftime(buffer, sizeof(buffer), "%b %d %Y %H:%M:%S"
                     , localtime(&now));
   if( L == 0 )                     // If ridiculous resultant
     strcpy(buffer, "Today");       // Set ridiculous time

   common= Common::make();          // Create the Common area
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

   // Startup complete event
   StaticCommon::Sevent_t& event= static_common->event;
   static_common->startup_complete.signal(event); // Raise startup_complete
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
             opt_verbose= 2;         // Default "extra" verbosity
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
// Signal  signal;                  // Signal handler

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

     // Termination delay longer than Server read timeout
     // printf("(Termination delay)\n");
     // Thread::sleep(3.125);          // Termination cleanup delay

   } catch(const char* X) {
     debugh("Exception(const char* %s)\n", X);
   } catch(Exception& X) {
     debugh("%4d %s\n", __LINE__, std::string(X).c_str());
   } catch(std::exception& X) {
     debugh("catch(std::exception.what(%s))\n", X.what());
   } catch(...) {
     debugh("Exception(...)\n");
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   term();

   traceh("...Brian complete\n");
   printf("...Brian complete\n");

   return 0;
}

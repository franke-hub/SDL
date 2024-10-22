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
//       2024/10/07
//
//----------------------------------------------------------------------------
#include <getopt.h>                 // For getopt()

#include <pub/Debug.h>              // For namespace debugging
#include <pub/Exception.h>          // For catch(pub::Exception)

#include "Command.h"                // For Command
#include "Common.h"                 // For Common

#define PUB _PUB_NAMESPACE
using PUB::Debug;
using namespace PUB::debugging;
using PUB::Exception;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_hcdm= HCDM; // --hcdm
static int             opt_verbose= VERBOSE; // --verbose{=verbosity}
static int             opt_help= false; // --help or error

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
// Internal data areas
//----------------------------------------------------------------------------
static Debug*          debug= nullptr; // Our Debug object

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
   work(int, char**)                // Handle Command
{  debugf("trap\n");                // Line 0092
   return nullptr;
}
} command_trap; // static class Command_trap

//----------------------------------------------------------------------------
//
// Subroutine-
//       exit_handler
//
// Purpose-
//       Run exit handler.
//
// Implementation notes-
//       The TraceLogger has been deleted at this point. Use printf.
//
//----------------------------------------------------------------------------
static inline void
   exit_handler( void )             // Exit handler
{  if( HCDM ) printf("Brian: exit_handler\n"); }

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
// Subroutine-
//       init
//
// Purpose-
//       Set up termination handlers.
//
//----------------------------------------------------------------------------
static inline void
   init( void )                      // Initialize
{
   if( atexit(exit_handler) != 0 )  // If cannot register handler
     throw "atexit failure";
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
   Common* common= nullptr;         // Brian Common area
// Signal  signal;                  // Signal handler

   //-------------------------------------------------------------------------
   // Initialiize
   //-------------------------------------------------------------------------
   parm(argc, argv);
   init();

   debug= new Debug();
   Debug::set(debug);
   debug_set_head(Debug::HEAD_TIME | Debug::HEAD_THREAD);
   if( true || opt_hcdm || opt_verbose > 1 )
     debug_set_mode(Debug::MODE_INTENSIVE);

   //-------------------------------------------------------------------------
   // Operate Brian
   //-------------------------------------------------------------------------
   try {
     common= Common::make();        // Create the Common area
     if( opt_hcdm || opt_verbose > 1 ) {
       traceh("==========================================================\n");
       traceh("======== Starting %s\n", common->get_name().c_str());
       traceh("==========================================================\n");
     }

     // Initialization complete
     printf("Brian started...\n");
     traceh("Brian started...\n");

     if( false ) {                  // (Handled properly)
       debugf("Should raise SIGSEGV\n");
       Common* common= nullptr;
       common->shutdown();
       debugf("ShouldNotOccur\n");
     }

     if( false ) {                  // (Handled properly)
       debugf("Should raise SIGABRT\n");
       abort();
       debugf("ShouldNotOccur\n");
     }

     if( false ) {                  // (Handled properly)
       debugf("Should throw(const char*)\n");
       throw "That's all, Folks";
       debugf("ShouldNotOccur\n");
     }

     common->wait();                // (Wait for quit command)
   } catch(const char* X) {
     debugf("Exception(const char* %s)\n", X);
   } catch(Exception& X) {
     debugf("%4d %s\n", __LINE__, std::string(X).c_str());
   } catch(std::exception& X) {
     debugf("catch(std::exception.what(%s))\n", X.what());
   } catch(...) {
     debugf("Exception(...)\n");
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   delete common;
   traceh("...Brian complete\n");
   printf("...Brian complete\n");
   Debug::set(nullptr);

   return 0;
}

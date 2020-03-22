//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2020 Frank Eskesen.
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
//       2020/01/10
//
// Controls-
//       If the first parameter is not a switch parameter, it specifies the
//       log file name (and sets intensive debug mode.)
//
//       (No other parameters are available.)
//
//----------------------------------------------------------------------------
#include <getopt.h>                 // For getopt()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Signal.h>
#include <pub/Debug.h>
#include <pub/Exception.h>

#include "Command.h"
#include "Common.h"
#include "Install.h"
#include "Service.h"
#include "DirtyInstall.h"           // TODO: REMOVE: BRINGUP TEST

using _PUB_NAMESPACE::Debug;
using namespace _PUB_NAMESPACE::debugging;
using _PUB_NAMESPACE::Exception;

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static const char*     opt_debug= nullptr; // --debug{=filename}
static int             opt_help= false; // --help or error
static int             opt_index;   // Option index
static int             opt_verbose= 0; // --verbose{=verbosity}

static struct option   OPTS[]=      // Options
{  {"help",    no_argument,       &opt_help,    true}

,  {"debug",   required_argument, nullptr,      0}
,  {"verbose", optional_argument, nullptr,      0}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HELP
,  OPT_DEBUG
,  OPT_VERBOSE
};

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
{
   // printf("Brian: exit_handler\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setup
//
// Purpose-
//       Set up termination handlers.
//
//----------------------------------------------------------------------------
static inline void
   setup( void )                    // Set up termination handlers
{
   if( atexit(exit_handler) != 0 )  // If cannot register handler
     throw "atexit failure";
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
           case OPT_DEBUG:
             opt_debug= optarg;
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

   if( opt_debug && opt_verbose == 0 )
     opt_verbose= 1;
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

   Debug debug(opt_debug);
   Debug::set(&debug);
   debug_set_head(Debug::HeadThread);
   if( opt_verbose > 5 )
     debug_set_mode(Debug::ModeIntensive);

   //-------------------------------------------------------------------------
   // Operate Brian
   //-------------------------------------------------------------------------
   try {
     common= Common::make();        // Initialize makefile
     traceh("============================================================\n");
     traceh("======== Starting %s\n", common->get_name().c_str());
     traceh("============================================================\n");

     // Install Commands and Services
     Install install;               // Install Commands and Services

     // Bringup testing
     DirtyInstall dirty;

     // Initialization complete
     debugf("Brian started...\n");

//   sleep(30);                     // 30 second runtime
//   common->shutdown();            // TODO: REMOVE: BRINGUP
     common->wait();

     if( false ) {                  // (Handled properly)
       debugf("Should raise SIGSEGV\n");
       Common* common= NULL;
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
   debugf("...Brian complete\n");
   Debug::set(nullptr);

   return 0;
}


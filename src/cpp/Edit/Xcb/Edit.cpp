//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Edit.cpp
//
// Purpose-
//       Editor: Command line processor
//
// Last change date-
//       2024/05/13
//
//----------------------------------------------------------------------------
#include <exception>                // For std::exception
#include <string>                   // For std::string

#include <ctype.h>                  // For isprint, toupper
#include <errno.h>                  // For errno
#include <getopt.h>                 // For getopt_long
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <unistd.h>                 // For close, ftruncate
#include <sys/types.h>              // For type definitions

#include <pub/Debug.h>              // For Debug object
#include <pub/Exception.h>          // For Exception object

#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For namespace editor
#include "EdOpts.h"                 // For EdOpts::bg_enabled
#include "EdUnit.h"                 // For EdUnit (start/join)

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, larger is more verbose
}; // Compilation controls

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_index;   // Option index

static int             opt_help= false; // --help (or error)
static int             opt_hcdm= HCDM;  // Hard Core Debug Mode?
static int             opt_verbose= VERBOSE; // Verbosity

static int             opt_bg= true; // Run editor in background?

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true} // --help
,  {"hcdm",    no_argument,       &opt_hcdm,    true} // --hcdm
,  {"verbose", optional_argument, &opt_verbose, 1} // --verbose {optional}

,  {"fg",      no_argument,       &opt_bg,      false} // --fg
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM
,  OPT_VERBOSE

,  OPT_FG
};

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
   fprintf(stderr, "%s <options> filename ...\n"
                   "File editor\n\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"
                   "  --verbose\t{=n} Verbosity, default 1\n"

                   "  --fg\t\tRun editor in foreground\n"
                   , __FILE__
          );

   return 1;
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

   return int(value);
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
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
   }

   return value;
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
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 ) {
     switch( C ) {
       case 0:
       {{{{
         switch( opt_index ) {
           case OPT_HELP:           // These options handled by getopt
           case OPT_HCDM:
           case OPT_FG:
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           default:
             fprintf(stderr, "Unexpected opt_index(%d)\n", opt_index);
             break;
         }
         break;
       }}}}

       case ':':
         opt_help= true;
         if( optopt == 0 ) {
           if( strchr(argv[optind-1], '=') )
             fprintf(stderr, "Option '%s' argument disallowed.\n"
                          , argv[optind-1]);
           else
             fprintf(stderr, "Option '%s' requires an argument.\n"
                           , argv[optind-1]);
         } else {
           fprintf(stderr, "Option '-%c' requires an argument.\n", optopt);
         }
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Unknown option '%s'.\n", argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "Unknown option '-%c'.\n", optopt);
         else
           fprintf(stderr, "Unknown option character '0x%.2x'.\n"
                         , (optopt & 0x00ff));
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%.2x).\n", __LINE__
                       , C, (C & 0x00ff));
         break;
     }
   }

   // Return sequence
   int rc= 0;
   if( opt_help )
     rc= info();
   return rc;
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

   if( EdOpts::bg_enabled && opt_bg ) { // If run in background
     if( fork() )                   // If parent (foreground)
       return 0;
   }

   //-------------------------------------------------------------------------
   // Operate the Editor
   //-------------------------------------------------------------------------
   try {
     // Initialize environment
     setlocale(LC_ALL, "");         // Allows printf("%'d\n", 123456789);
     std::string lang= getenv("LANG");
     if( lang.find(".utf8") == std::string::npos
         && lang.find(".UTF-8") == std::string::npos ) {
       lang += ".utf8";
     }
     setlocale(LC_CTYPE, lang.c_str());

     config::opt_hcdm= opt_hcdm;    // Expose config:: options
     config::opt_verbose= opt_verbose;

     // Configure and operate the Editor
     Config config(argc, argv);     // Configure
     if( opt_hcdm || opt_verbose > 0 ) {
       Config::errorf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       Config::errorf("--hcdm(%d) --verbose(%d) --fg(%d)\n"
                     , opt_hcdm, opt_verbose, !opt_bg);
     }

     Editor editor(optind, argc, argv); // Load the initial file set
     editor::unit->start();         // Initial screen draw, polling loop
     // :                           // : Wait for completion
     editor::unit->join();          // Polling loop complete
   } catch(pub::Exception& X) {
     debugf("%s\n", std::string(X).c_str());
   } catch(std::exception& X) {
     printf("std::exception.what(%s))\n", X.what());
   } catch(const char* X) {
     printf("catch(const char* '%s')\n", X);
   } catch(...) {
     printf("catch(...)\n");
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   if( opt_hcdm && opt_verbose > 0 )
     Config::errorf("Edit completed\n");

   return rc;
}

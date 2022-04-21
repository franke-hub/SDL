//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2022 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Wrapper.cpp
//
// Purpose-
//       Implement Wrapper.h generic program wrapper.
//
// Last change date-
//       2022/04/19
//
// Implementation notes-
//       Orignally created as a test case wrapper, now repurposed as a generic
//       program wrapper.
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isprint()
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For open, O_*, ...
#include <getopt.h>                 // For getopt_long()
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For fprintf, vfprintf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strerror
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, ...

#include <pub/Debug.h>              // For namespace `debugging`
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Trace.h>              // For pub::Trace

#include "pub/Wrapper.h"             // For class Wrapper, implemented

using namespace pub;                // For pub:: classes

_LIBPUB_BEGIN_NAMESPACE
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, greater is more verbose

,  USE_DEBUG_APPEND= false          // Append to Debug file?
}; // enum

static constexpr const char* TRACE_FILE= "./trace.mem"; // (Trace file name)

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
int                    Wrapper::opt_hcdm= HCDM; // Hard Core Debug Mode
int                    Wrapper::opt_verbose= VERBOSE; // Verbosity

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Debug*          debug= nullptr; // The debug object
static void*           table= nullptr; // The trace data area

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_index;   // Option index

static const char*     opt_debug= nullptr; // --debug, default none
static int             opt_trace= 0; // --trace, default none

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"help",    no_argument,       &opt_help,             true}
,  {"hcdm",    no_argument,       &Wrapper::opt_hcdm,    true}
,  {"verbose", optional_argument, &Wrapper::opt_verbose,    1}

,  {"debug",   optional_argument, nullptr,                  0} // Debug filename
,  {"trace",   optional_argument, &opt_trace,      0x00040000} // Trace length
,  {0, 0, 0, 0}                     // (End of Wrapper internal option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM
,  OPT_VERBOSE

,  OPT_DEBUG
,  OPT_TRACE
,  OPT_SIZE
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugf
//
// Purpose-
//       Write error message
//
//----------------------------------------------------------------------------
static void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _LIBPUB_PRINTF(1, 2);
static void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
{
   va_list argptr;
   va_start(argptr, fmt);

   if( debug )
     debug->vdebugf(fmt, argptr);
   else
     vfprintf(stdout, fmt, argptr);

   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::~Wrapper
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Wrapper::~Wrapper()
{
   if( OPTS && OPTS != opts ) {
     free(OPTS);
     OPTS= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::Wrapper
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Wrapper::Wrapper(                // Default/option list constructor
     option*           O)           // The option list
:  info_f([]() {})
,  init_f([](int, char**) { return 0; })
,  main_f([](int, char**) { return 0; })
,  parm_f([](std::string, const char*) { return 0; })
,  term_f([]() {})
{
   OPTS= opts;                      // Default, internal option table
   OPNO= OPT_SIZE;                  // Default, internal option table size

   if( O ) {                        // If an option list was specified
     size_t N= 0;
     for(; O[N].name; ++N)          // Count the options
       ;

     OPNO= N + OPT_SIZE + 1;        // The total option count
     OPTS= (struct option*)malloc(OPNO * sizeof(struct option));
     if( OPTS == nullptr )
       throw std::bad_alloc();

     for(size_t i= 0; i<OPT_SIZE; ++i)
       OPTS[i]= opts[i];

     for(size_t i= OPT_SIZE; i<OPNO; ++i)
       OPTS[i]= O[i - OPT_SIZE];
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
void
   Wrapper::info( void ) const
{
   fprintf(stderr, "%s <options> ...\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"

                   "  --trace\t{=size} Enable trace, default size= 1M\n"
                   "  --verbose\t{=n} Verbosity, default 1\n"
                   , __FILE__
          );

   info_f();
   exit(0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Wrapper::init(                   // Initialize
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Create memory-mapped trace file
   if( opt_trace ) {                // If --trace specified
     int mode= O_RDWR | O_CREAT;
     int perm= S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
     int fd= open(TRACE_FILE, mode, perm);
     if( fd < 0 ) {
       fprintf(stderr, "%4d open(%s) %s\n", __LINE__
                     , TRACE_FILE, strerror(errno));
       return 1;
     }

     int rc= ftruncate(fd, opt_trace); // (Expand to opt_trace length)
     if( rc ) {
       fprintf(stderr, "%4d ftruncate(%s,%.8x) %s\n", __LINE__
                     , TRACE_FILE, opt_trace, strerror(errno));
       return 1;
     }

     mode= PROT_READ | PROT_WRITE;
     table= mmap(nullptr, opt_trace, mode, MAP_SHARED, fd, 0);
     if( table == MAP_FAILED ) {    // If no can do
       fprintf(stderr, "%4d mmap(%s,%.8x) %s\n", __LINE__
                     , TRACE_FILE, opt_trace, strerror(errno));
       table= nullptr;
       return 1;
     }

     Trace::table= pub::Trace::make(table, opt_trace);
     close(fd);                     // Descriptor not needed once mapped

     Trace::trace(".INI", 0, "TRACE STARTED") ;
   }

   //-------------------------------------------------------------------------
   // Create debugging output file
   if( opt_debug ) {
     debug= new Debug("debug.out");
     Debug::set(debug);

     debug->set_head(Debug::HEAD_THREAD);
     if( USE_DEBUG_APPEND )
       debug->set_file_mode("ab");
     if( opt_hcdm )
       debug->set_mode(Debug::MODE_INTENSIVE);
   }

   //-------------------------------------------------------------------------
   // User extension
   int rc= init_f(argc, argv);
   if( rc )
     term();

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::atoi
//
// Purpose-
//       Convert string to integer, setting errno.
//
// Implementation note-
//       Leading or trailing blanks are NOT allowed.
//
//----------------------------------------------------------------------------
int                                 // The integer value
   Wrapper::atoi(                   // Extract and verify integer value
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
// Method-
//       Wrapper::ptoi
//
// Purpose-
//       Convert parameter to integer, handling error cases
//
// Implementation note-
//       optarg: The argument string
//       opt_index: The argument index
//
//----------------------------------------------------------------------------
int                                 // The integer value
   Wrapper::ptoi(const char* V, const char* N) // Extract/verify parameter
{
   int value= atoi(V);
   if( errno ) {
     opt_help= true;
     if( N == nullptr )
       N= "parameter";

     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", N, V);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", N);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", N, V);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Method-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   Wrapper::parm(                   // Parameter analysis
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
             break;

           case OPT_DEBUG:
             opt_debug= optarg;
             if( optarg == nullptr )
               opt_debug= "debug.out";
             break;

           case OPT_TRACE:
             if( optarg )
               opt_trace= ptoi(optarg, OPTS[opt_index].name);

             if( opt_trace < int(Trace::TABLE_SIZE_MIN) )
               opt_trace= Trace::TABLE_SIZE_MIN;
             else if( opt_trace > int(Trace::TABLE_SIZE_MAX) )
               opt_trace= Trace::TABLE_SIZE_MAX;
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= ptoi(optarg, OPTS[opt_index].name);
             break;

           default: {
             if( size_t(opt_index) < OPNO ) {
               std::string S= (argv[optind-1] + 2); // Strip out leading "--"
               size_t X= S.find('='); // Strip out trailing "=value"
               if( X != std::string::npos )
                 S= S.substr(0, X);
               if( parm_f(S, optarg) )
                 opt_help= true;
             } else {
               opt_help= true;
               fprintf(stderr, "%4d Unexpected opt_index(%d)\n", __LINE__,
                               opt_index);
             }
             break;
           }
         }
         break;
       }}}}

       case ':':
         opt_help= true;
         if( optopt == 0 ) {
           if( strchr(argv[optind-1], '=') )
             fprintf(stderr, "Option has no argument '%s'.\n"
                           , argv[optind-1]);
           else
             fprintf(stderr, "Option requires an argument '%s'.\n"
                           , argv[optind-1]);
         } else {
           fprintf(stderr, "%4d Option requires an argument '-%c'.\n", __LINE__,
                           optopt);
         }
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Unknown option '%s'.\n", __LINE__
                         , argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "%4d Unknown option '-%c'.\n", __LINE__, optopt);
         else
           fprintf(stderr, "%4d Unknown option character '0x.2%x'.\n"
                         , __LINE__, (optopt & 0x00ff));
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).\n", __LINE__
         ,
                         C, (C & 0x00ff));
         opt_help= true;
         break;
     }
   }

   if( opt_help ) {
     info();
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::report_errors
//
// Purpose-
//       Display the error count
//
//----------------------------------------------------------------------------
void
   Wrapper::report_errors(int error_count) // Display the error count
{
   if( error_count == 0 ) {
     if( opt_verbose )
       debugf("NO errors detected\n");
   } else if( error_count == 1 )
     debugf(" 1 error detected\n");
   else
     debugf("%2d errors detected\n", error_count);
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::run
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   Wrapper::run(                    // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   int rc= parm(argc, argv);
   if( rc ) return rc;

   rc= init(argc, argv);
   if( rc )
     return rc;

   //-------------------------------------------------------------------------
   // Run the tests
   //-------------------------------------------------------------------------
   try {
     rc= main_f(argc, argv);
   } catch(pub::Exception& x) {
     debugf("pub::exception(%s)\n", x.what());
     rc= 2;
   } catch(std::exception& x) {
     debugf("std::exception(%s)\n", x.what());
     rc= 2;
   } catch(const char* x) {
     debugf("const char*(%s) exception\n", x);
     rc= 2;
   } catch(...) {
     debugf("Exception ...\n");
     rc= 2;
   }

   //-------------------------------------------------------------------------
   // Termination cleanup and exit
   //-------------------------------------------------------------------------
   term();
   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wrapper::term
//
// Purpose-
//       Termination cleanup
//
//----------------------------------------------------------------------------
void
   Wrapper::term( void )                      // Terminate
{
   //-------------------------------------------------------------------------
   // User termination extension
   term_f();

   //-------------------------------------------------------------------------
   // Terminate internal trace
   if( table == Trace::table ) {
     Trace::table= nullptr;
     munmap(table, opt_trace);
   }

   //-------------------------------------------------------------------------
   // Delete (close) our debugging trace file
   Debug::lock();
   if( debug == Debug::show() )
     Debug::set(nullptr);
   Debug::unlock();

   delete debug;
}
_LIBPUB_END_NAMESPACE

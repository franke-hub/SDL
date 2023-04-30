//----------------------------------------------------------------------------
//
//       Copyright (c) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestWrap.cpp
//
// Purpose-
//       Test pub/Wrapper.h
//
// Last change date-
//       2023/04/29
//
//----------------------------------------------------------------------------
#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string

#include <assert.h>
#include <string.h>

#include <pub/Debug.h>

// The tested includes
#include <pub/TEST.H>               // For VERIFY, ...
#include <pub/Debug.h>              // For debugging

#include "pub/Wrapper.h"            // For class Wrapper, verified

// Namespace accessors
#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub objects
using namespace PUB::debugging;     // For debugging subroutines
using PUB::Wrapper;                 // For pub::Wrapper class

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             error_count= 0; // Error counter
static Debug*          debug= nullptr; // The Debug object
static void*           table= nullptr; // The Trace table

//----------------------------------------------------------------------------
// Extended options
//----------------------------------------------------------------------------
static int             opt_args=     0; // --args
static int             opt_debug=    0; // --debug
static const char*     opt_feedme=   nullptr; // --feedme
static int             opt_that=     0; // --that
static int             opt_this=     0; // --this
static int             opt_throw=    0; // --throw
static int             opt_trace=    0; // --trace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"args",            no_argument,       &opt_args,        true}
,  {"debug",           no_argument,       &opt_debug,       true}
,  {"feedme",          required_argument, nullptr,             0}
,  {"that",            no_argument,       &opt_that,        true}
,  {"this",            no_argument,       &opt_this,        true}
,  {"throw",           no_argument,       &opt_throw,       true}
,  {"trace",           optional_argument, &opt_trace, 0x00400000}
,  {0, 0, 0, 0}                     // (End of option list)
};

static int             opt_a= false;
static int             opt_b= false;
static const char*     opt_c= nullptr;
static const char*     opt_d= nullptr;
static int             opt_e= false;

static const char*     ostr="abc:d::e"; // The getopt_long parameter: optstring

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_args
//
// Purpose-
//       Test arguments
//
//----------------------------------------------------------------------------
static void
   test_args(int argc, char* argv[]) // Test argument handling
{
   debugf("\ntest_args optarg(%s) optind(%d) opterr(%d)\n"
         , optarg, optind, opterr);
   for(int i= 0; i<argc; ++i) {
     debugf("[%2d] '%s'\n", i, argv[i]);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_throw
//
// Purpose-
//       Test exception handling
//
//----------------------------------------------------------------------------
[[noreturn]]
static void
   test_throw( void )               // Test exception handling
{
   throw std::runtime_error("just testing");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       torf
//
// Purpose-
//       Convert condition to "true" or "false" string
//
//----------------------------------------------------------------------------
static inline const char*
   torf(bool condition)
{  return condition ? "true" : "false"; }

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
   Wrapper  tc(opts, ostr);         // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_parm([tr](std::string P, const char* V)
   {
     if( opt_verbose > 1 )
       debugf("on_parm(%s,%s)\n", P.c_str(), V);

     if( P == "feedme" )
       opt_feedme= V;

     else
     if( P == "trace" ) {
       if( V )
         opt_trace= tr->ptoi(V);
     }

     else
     if( P[0] == '-' ) {            // If switch option
       switch( P[1] ) {
         case 'a':
           opt_a= true;
           break;

         case 'b':
           opt_b= true;
           break;

         case 'c':
           opt_c= V;
           break;

         case 'd':
           opt_d= "default d";
           if( V )
             opt_d= V;
           break;

         case 'e':
           opt_e= true;
           break;

         default:
           debugf("%4d Should not occur %c,%d\n", __LINE__, P[1], P[1]);
       }
     }

     else
       debugf("Unexpected parameter '%s'='%s'\n", P.c_str(), V);

     return 0;
   });

   tc.on_info([](void)
   {
     fprintf(stderr,
            "  --args\tDisplay arguments\n"
            "  --debug\tPrint using Debug object\n"
            "  --feedme\tRequired argument\n"
            "  --that\tNo argument\n"
            "  --this\tNo argument\n"
            "  --throw\tThrow an exception\n"
            "  --trace\t{=size} Create internal trace file './trace.mem'\n"

            "  -a\t\tOption control A\n"
            "  -b\t\tOption control B\n"
            "  -c\t\t{argument} Option control C\n"
            "  -d\t\t{=argument} Option control D\n"
            "  -e\t\tOption control E\n"
            );
   });

   tc.on_init([tr](int, char**)
   {
     if( opt_debug )
       debug= tr->init_debug();
     if( opt_trace )
       table= tr->init_trace("./trace.mem", opt_trace);

     return 0;
   });

   tc.on_term([tr](void)
   {
     if( table )
       tr->term_trace(table, opt_trace);
     if( debug )
       tr->term_debug(debug);
   });

   tc.on_main([tr](int argc, char* argv[])
   {
     error_count= 0;

     if( opt_verbose ) {
       debugf("\nOptions:\n");
       debugf("%5s hcdm\n", torf(opt_hcdm));
       debugf("%5d verbose\n", opt_verbose);
       debugf("%5s args\n", torf(opt_args));
       debugf("%5s debug\n", torf(opt_debug));
       debugf("%5s feedme: %s\n", torf(bool(opt_feedme)), opt_feedme);
       debugf("%5s that\n", torf(opt_that));
       debugf("%5s this\n", torf(opt_this));
       debugf("%5s throw\n", torf(opt_throw));
       debugf("%5s trace: %#x\n", torf(opt_trace), opt_trace);

       debugf("%5s -a\n", torf(opt_a));
       debugf("%5s -b\n", torf(opt_b));
       debugf("%5s -c: %s\n", torf(bool(opt_c)), opt_c);
       debugf("%5s -d: %s\n", torf(bool(opt_d)), opt_d);
       debugf("%5s -e\n", torf(opt_e));
     }

     if( opt_args )
       test_args(argc, argv);

     if( opt_debug ) {
       debugf("\n");
       tr->debug("opt_debug");
     }

     if( opt_throw )
       test_throw();

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   try {
     return tc.run(argc, argv);
   } catch(const char* x) {
     debugf("Exception const char*(%s)\n", x);
   } catch(std::exception& x) {
     debugf("Exception exception(%s)\n", x.what());
   } catch(...) {
     debugf("Exception ...\n");
   }

   return 2;
}

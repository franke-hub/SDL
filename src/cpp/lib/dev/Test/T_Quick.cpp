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
//       T_Quick.cpp
//
// Purpose-
//       Quick verification tests.
//
// Last change date-
//       2023/06/04
//
//----------------------------------------------------------------------------
#include <iostream>                 // For std::cout
#include <stdint.h>                 // For standard integer types
#include <string.h>                 // For std::string, size_t

#include <pub/TEST.H>               // For VERIFY macros
#include <pub/Debug.h>              // For pub::Debug, namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/utility.h>            // For pub::utility::dump/visify
#include <pub/Wrapper.h>            // For pub::Wrapper

#include "pub/http/Codec.h"         // For pub::http::Codec, tested

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using namespace PUB::http;
using PUB::utility::dump;
using PUB::utility::visify;
using PUB::Wrapper;
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_case= false; // (Only set if --hcdm --all)
static int             opt_codec= true; // (Unconditionally true)
static int             opt_dirty= false; // --dirty

static struct option   opts[]=      // Options
{  {"all",     optional_argument, nullptr,         0}
,  {"dirty",   no_argument      , &opt_dirty,   true}
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_case
//
// Purpose-
//       Testcase example
//
//----------------------------------------------------------------------------
static inline int
   test_case( void )
{
   if( opt_verbose )
     debugf("\ntest_case\n");

   int error_count= 0;              // Error counter

   error_count += VERIFY( true );   // Dummy test

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_codec
//
// Purpose-
//       Test ~/src/cpp/inc/pub/http/Codec.h
//
//----------------------------------------------------------------------------
static inline int
   test_codec( void )               // Test Codec.h
{
   if( opt_verbose )
     debugf("\ntest_codec:\n");

   int error_count= 0;

   Codec64 base64;
   std::string inp= "abcdefghijklmnopqrstuvwxyz";
   inp= inp + inp + "\r\n";
   inp= inp + inp + inp + inp;
   inp= inp + inp + inp + inp;

   for(int i= 0; i<10; ++i) {
     std::string out= base64.encode(inp);
     std::string ver= base64.decode(out);
     error_count += VERIFY( inp == ver );
     if( error_count || (opt_verbose > 1 && i == 4) ) {
       debugf("out:{{\n%s\n}}\n", visify(out).c_str());
       debugf("inp(%s)\n", visify(inp).c_str());
       debugf("ver(%s)\n", visify(ver).c_str());
       break;
     }

     inp += "X";
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_dirty
//
// Purpose-
//       A quick and dirty test.
//
//----------------------------------------------------------------------------
static inline int
   test_dirty( void )
{
   if( opt_verbose )
     debugf("\ntest_dirty\n");

   int error_count= 0;              // Error counter

   return error_count;
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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_info([]()
   {
     fprintf(stderr, "  --all\t\tRun all regression tests\n"
            );
   });

   tc.on_init([](int argc, char* argv[]) // (Unused in this sample)
   { (void)argc; (void)argv;        // (Unused arguments)

     return 0;
   });

   tc.on_parm([](std::string name, const char* value)
   {
     if( opt_hcdm )
       debugf("on_parm(%s,%s)\n", name.c_str(), value);

     if( name == "all" ) {          // Note: specify --hcdm *BEFORE* --all
       if( opt_hcdm ) {
         opt_case= true;            // (Only set here)
       }
     }

     return 0;
   });

   tc.on_term([]()                  // (Unused in this sample)
   {
   });

   tc.on_main([tr](int argc, char* argv[])
   { (void)argc; (void)argv;        // (Unused arguments)
     if( opt_hcdm || opt_verbose ) {
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       debug_set_mode(Debug::MODE_INTENSIVE);
     }

     int error_count= 0;

     if( opt_case )    error_count += test_case();
     if( opt_codec )   error_count += test_codec();
     if( opt_dirty )   error_count += test_dirty();

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;

   return tc.run(argc, argv);
}

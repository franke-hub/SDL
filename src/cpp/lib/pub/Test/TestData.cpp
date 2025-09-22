//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       TestData.cpp
//
// Purpose-
//       Test Data.h
//
// Last change date-
//       2025/09/20
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string

#include <pub/Debug.h>              // For namespace debugging

#include "pub/Data.h"               // For Data classes, tested
#include <pub/Wrapper.h>            // For class Wrapper

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging functions
using namespace PUB::data;
using PUB::List;
using PUB::Wrapper;
using std::string;

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_name
//
// Purpose-
//       Test PUB::data::Name
//
//----------------------------------------------------------------------------
static int                          // Error count
   test_name(                       // Test PUB::data::Name
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 error_count= 0; // Number of errors encountered

   // Resolve each argument
   for(int argx= optind; argx < argc; argx++) {
     char* C= argv[argx];
     Name name(C);
     std::string error= name.resolve();
     if( opt_verbose ) {
       if( error == "" )              // If no error
         debugf("OK: '%s'= Name.resolve(%s)\n", name.name.c_str(), C);
       else {
         debugf("NG: '%s'= Name.resolve(%s)\n", error.c_str(), C);
       }
     }

     if( error != "" )
       error_count++;
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_path
//
// Purpose-
//       Test PUB::data::Path
//
//----------------------------------------------------------------------------
static int                          // Error count
   test_path(                       // Test PUB::data::Path
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   typedef List<File>::iterator     Flit;

   int                 error_count= 0; // Number of errors encountered

   if( opt_verbose ) {
     // Print each file name in path
     for(int argx= optind; argx < argc; argx++) {
       char* C= argv[argx];
       debugf("\nPath(%s):\n", C);
       Path path(C);
       for(Flit it= path.list.begin(); it != path.list.end(); ++it) {
         debugf(": %s\n", it->name.c_str());
       }
     }
   }

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
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   Wrapper  tc;                     // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   //-------------------------------------------------------------------------
   // Mainline code
   tc.on_main([tr](int argc, char* argv[])
   {
     int error_count= 0;            // Error counter

     if( opt_verbose ) {
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       if( false ) {
         debugf("optind(%d)\n", optind);
         for(int i= 0; i<argc; ++i)
           debugf("[%2d] '%s'\n", i, argv[i]);
       }
     }

     if( argc < (optind + 1) ) {
       debugf("Test name missing: Use 'name' or 'path'\n");
       ++error_count;
     } else {
       const string opt_test= argv[optind];
       --argc;
       ++argv;

       if ( opt_test == "name" ) {
         error_count += test_name(argc, argv);
       } else if( opt_test == "path" ) {
         error_count += test_path(argc, argv);
       } else {
         ++error_count;
         errorf("Invalid test name '%s'\n", opt_test.c_str());
       }
     }

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}

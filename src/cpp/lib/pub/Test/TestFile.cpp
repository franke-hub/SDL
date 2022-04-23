//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestFile.cpp
//
// Purpose-
//       Test Fileman.h (parts untested by ~/src/cpp/Fileman)
//
// Last change date-
//       2022/04/22
//
//----------------------------------------------------------------------------
//#include <stdio.h>
//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>

#include <exception>

#include <pub/config.h>             // For _PUB_NAMESPACE macro
#include <pub/Debug.h>              // For namespace debugging

#include "pub/Fileman.h"            // For Fileman classes, tested
#include <pub/Wrapper.h>            // For class Wrapper

using namespace _PUB_NAMESPACE::debugging; // For debugging functions
using namespace _PUB_NAMESPACE::fileman; // For pub::fileman classes
using _PUB_NAMESPACE::Wrapper;      // For pub::Wrapper class

#define opt_hcdm       pub::Wrapper::opt_hcdm
#define opt_verbose    pub::Wrapper::opt_verbose

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_name
//
// Purpose-
//       Test pub::fileman::Name
//
//----------------------------------------------------------------------------
static int                          // Error count
   test_name(                       // Test pub::fileman::Name
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 error_count= 0; // Number of errors encountered

   // Resolve each argument
   for(int argx= 1; argx < argc; argx++) {
     char* C= argv[argx];
     Name name(C);
     std::string error= name.resolve();
     if( opt_verbose ) {
       if( error == "" )              // If no error
         debugf("OK: '%s'= resolve(%s)\n", name.name.c_str(), C);
       else {
         debugf("NG: '%s'= resolve(%s)\n", error.c_str(), C);
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
     if( opt_verbose )
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

     int error_count= test_name(argc, argv);

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

//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
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
//       Test_mem.cpp
//
// Purpose-
//       Test memory.h function.
//
// Last change date-
//       2026/06/11
//
//----------------------------------------------------------------------------
#include <memory>                   // For std::unique_ptr

#include <pub/Debug.h>              // For pub::debugging methods
#include "pub/memory.h"             // For pub::scoped_ptr, tested
#include <pub/TEST.H>               // For VERIFY macro
#include "pub/Wrapper.h"            // For pub::Wrapper
#include "pub/utility.i"            // For pub::utility conversion routines

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;                   // For Debug objects
using PUB::Wrapper;                 // For Testcase wrapper
using PUB::s2c;                     // String to char* utility

using std::unique_ptr;              // For std::unique_ptr template

using namespace PUB::debugging;


//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more
}; // Generic enum

//----------------------------------------------------------------------------
// Extended options
//----------------------------------------------------------------------------
////// int             opt_hcdm= HCDM;       // (Wrapper built-in)
////// int             opt_verbose= VERBOSE; // (Wrapper built-in)
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"********", no_argument,       nullptr,    0} // --(ignored)
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_case
//
// Purpose-
//       Test case skeleton.
//
//----------------------------------------------------------------------------
static inline int                   // Error counter
   test_case(void)
{
   if( opt_verbose )
     debugf("\ntest_case\n");

   int error_count= 0;

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_unique
//
// Purpose-
//       Test std::unique_ptr
//
//----------------------------------------------------------------------------
static inline int                   // Error counter
   test_unique(void)
{
   if( opt_verbose )
     debugf("\ntest_unique\n");

   int error_count= 0;

   static constexpr const int DIM= 256;

   std::unique_ptr<uint64_t[]> buffer( new uint64_t[DIM] );
   for(size_t i= 0; i<DIM; ++i) {
     buffer[i]= uint64_t(0xfedcba9876543210) + i;
   }

   for(size_t i= 0; i<DIM; ++i) {
     error_count += VERIFY( buffer[i] == (uint64_t(0xfedcba9876543210)+i) );
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
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Mainline code
   tc.on_main([tr](int, char*[])
   {
     if( opt_verbose )
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

     int error_count= 0;

     error_count += test_unique();

     if( opt_verbose || error_count ) {
       debugf("\n");
       tr->report_errors(error_count);
     }

     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;
   setlocale(LC_NUMERIC, "");       // Activates ' thousand separator

   return tc.run(argc, argv);
}

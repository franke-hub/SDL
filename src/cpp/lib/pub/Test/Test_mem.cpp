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
//       2026/07/02
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
// Struct-
//       Thing
//
// Purpose-
//       Test object.
//
//----------------------------------------------------------------------------
struct Thing {
static size_t          bug_count;   // Error counter
static size_t          new_count;   // Constructor counter
static size_t          old_count;   // Destructor counter

char                   ident[32];   // Identifier

   Thing( void )                    // Constructor
{
   ++new_count;
   if( opt_verbose )
     debugf("Thing::Thing()  new(%3zd) old(%3zd)\n", new_count, old_count);

   strcpy(ident, "ITS_A_THING");
}

   ~Thing( void )                   // Destructor
{
   ++old_count;
   if( opt_verbose )
     debugf("Thing::~Thing() new(%3zd) old(%3zd)\n", new_count, old_count);

   bug_count += VERIFY( strcmp(ident, "ITS_A_THING") == 0 );
}
}; // struct Thing

size_t   Thing::bug_count= 0;
size_t   Thing::new_count= 0;
size_t   Thing::old_count= 0;

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

   {{{{                             // Scope boundary
     std::unique_ptr<Thing> thing( new(Thing) );
     error_count += VERIFY( Thing::new_count == 1 );
   }}}}

   error_count += VERIFY( Thing::bug_count == 0 );
   error_count += VERIFY( Thing::new_count == 1 );
   error_count += VERIFY( Thing::old_count == 1 );

   {{{{                             // Scope boundary
     std::unique_ptr<Thing[]> thing( new(Thing[DIM-1]) );
     error_count += VERIFY( Thing::new_count == DIM );
     error_count += VERIFY( Thing::old_count == 1 );
   }}}}

   error_count += VERIFY( Thing::bug_count == 0 );
   error_count += VERIFY( Thing::new_count == DIM );
   error_count += VERIFY( Thing::old_count == DIM );

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

//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_bug.cpp
//
// Purpose-
//       Test debugging methods.
//
// Last change date-
//       2022/04/19
//
//----------------------------------------------------------------------------
#include <errno.h>                  // For errno

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros
#include "pub/Debug.h"              // For Debug, tested
#include <pub/Wrapper.h>            // For class Wrapper

using _LIBPUB_NAMESPACE::Debug;
using namespace _LIBPUB_NAMESPACE::debugging;
using pub::Wrapper;                 // For pub::Wrapper class

#define opt_hcdm       pub::Wrapper::opt_hcdm
#define opt_verbose    pub::Wrapper::opt_verbose

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_bt
//
// Purpose-
//       Test backtrace
//
//----------------------------------------------------------------------------
_LIBPUB_NOINLINE
static void bar() {
   debugf("Backtrace test\n");
   debug_backtrace();
   debugf("\n");
}

_LIBPUB_NOINLINE
static void foo() {
   bar();
}

_LIBPUB_NOINLINE
static void the() {
   foo();
}

_LIBPUB_NOINLINE
static void test_bt() {
   the();
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
     int               argc,        // Argument count (UNUSED)
     char*             argv[])      // Argument array (UNUSED)
{
   //-------------------------------------------------------------------------
   // Initialize
   Wrapper  tc;                     // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_main([tr](int, char*[])
   {
     if( opt_verbose )
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

     int error_count= 0;

     // Test backtrace
     test_bt();

     // Test modes
     debug_set_mode(Debug::MODE_DEFAULT);
     debugf("Standard mode:\n");
     debugf("This appears in %s and %s\n", "TRACE", "STDOUT");
     errorf("This appears in %s and %s\n", "TRACE", "STDERR");
     tracef("This appears in %s ONLY\n",   "TRACE");
     debugh("This appears in %s and %s\n", "TRACE", "STDOUT");
     errorh("This appears in %s and %s\n", "TRACE", "STDERR");
     traceh("This appears in %s ONLY\n",   "TRACE");

     debug_set_mode(Debug::MODE_IGNORE);
     errno= EAGAIN;
     errorp("Ignore mode: This appears in STDERR (even with ignore mode)");
     debugf("Ignore mode:\n");
     errorf("Ignore mode:\n");
     tracef("Ignore mode:\n");
     debugh("Ignore mode:\n");
     errorh("Ignore mode:\n");
     traceh("Ignore mode:\n");

     debug_set_mode(Debug::MODE_INTENSIVE);
     debugf("Intensive mode:\n");
     debugf("This appears in %s and %s\n", "TRACE", "STDOUT");
     errorf("This appears in %s and %s\n", "TRACE", "STDERR");
     tracef("This appears in %s ONLY\n",   "TRACE");
     debugh("This appears in %s and %s\n", "TRACE", "STDOUT");
     errorh("This appears in %s and %s\n", "TRACE", "STDERR");
     traceh("This appears in %s ONLY\n",   "TRACE");

     tr->report_errors(error_count);
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}

//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
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
//       2022/04/24
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
// Extended options
//----------------------------------------------------------------------------
static int             opt_trace= false;   // --backtrace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"backtrace",  no_argument,       &opt_trace,  true} // --backtrace
,  {0, 0, 0, 0}                     // (End of option list)
};

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
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_main([tr](int, char*[])
   {
     if( opt_verbose )
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

     int error_count= 0;

     // Test backtrace
     // Implementation note: backtrace is provided by the boost library and
     // its output varies depending upon the installed version. Since
     // regression testing checks our output, it's not tested by default.
     if( opt_trace )
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
     errno= 0;
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

     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}

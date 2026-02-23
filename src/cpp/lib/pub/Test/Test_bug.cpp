//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2026 Frank Eskesen.
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
//       Test_bug.cpp
//
// Purpose-
//       Test debugging methods.
//
// Last change date-
//       2026/02/23
//
//----------------------------------------------------------------------------
#include <stdexcept>                // For std::runtime_error
#include <cerrno>                   // For errno

#include <pub/TEST.H>               // For test functions and macros
#include "pub/Debug.h"              // For Debug, tested
#include <pub/Wrapper.h>            // For class Wrapper

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;
using namespace PUB::debugging;
using PUB::Wrapper;                 // For pub::Wrapper class

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Extended options
//----------------------------------------------------------------------------
//static struct option   opts[]=      // The getopt_long parameter: longopts
//{  {0, 0, 0, 0}                     // (End of option list)
//};

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
     // Implementation note: backtrace is provided by the boost library and
     // its output varies depending upon the installed version. Since
     // regression testing checks our output, it's not tested by default.
     if( opt_verbose )
       test_bt();

     // Test modes
     debug_set_head(Debug::HEAD_TIME);
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

     // Test throwf (output not checked)
     if( opt_verbose ) {
       bool caught= false;
       try {
         debugf("\n%4d Testing throwf\n", __LINE__);
         throwf("%4d runtime_error", __LINE__);
         error_count += MUST_NOT(fail to throw an exception);
       } catch(std::runtime_error& X) {
         debugf("(Backtrace expected)\n");
         debugf("%4d ..As expected: %s\n", __LINE__, X.what());
         caught= true;
       }
       error_count += VERIFY( caught == true );
     }

     // Test abortf (output not checked)
     if( opt_verbose > 1 ) {
       try {
         debugf("\n%4d Testing abortf\n", __LINE__);
         abortf("%4d abortf test", __LINE__);
         error_count += MUST_NOT(fail to abort);
       } catch(...) {
         error_count += MUST_NOT(fail to abort);
       }
       error_count += MUST_NOT(fail to abort);
     }

     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   setlocale(LC_NUMERIC, "");       // Activates ' thousand separator
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;

   return tc.run(argc, argv);
}

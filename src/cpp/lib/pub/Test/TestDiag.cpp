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
//       TestDiag.cpp
//
// Purpose-
//       Test the Diagnostic functions.
//
// Last change date-
//       2026/10/09
//
// Implementation notes-
//       No verification tests included. Use --verbose to display values.
//
//----------------------------------------------------------------------------
#define _GNU_SOURCE                 // For pthread_getattr_np

#include <string>                   // For std::string

#include <pthread.h>                // For pthread interface
#include <string.h>                 // For strerror

#include "pub/diag-stack.h"         // For pub::diag::Stack
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Wrapper.h>            // For pub::Wrapper

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::Wrapper;
using PUB::diag::Stack;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  USE_ITRACE= false                // Use internal trace?
}; // enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static void*           table= nullptr; // The Trace table

// Extended options
static int             opt_trace= 0; // --trace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"trace",   optional_argument, &opt_trace, 0x00400000} // --trace
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_errno
//
// Purpose-
//       Handle an error
//
//----------------------------------------------------------------------------
static inline int                   // Return code, always 1
   handle_errno(                    // Handle an error
     const char*       f,           // The failing function name
     int               rc)          // ERRNO code
{
   debugf("%s ERROR %d:%s\n", f, rc, strerror(rc));
   return 1;
}

static inline void*                 // Return void*, always nullptr
   handle_thread_errno(             // Handle an error in a thread
     const char*       f,           // The failing function name
     int               rc)          // ERRNO code
{  handle_errno(f, rc); return nullptr; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_thread_thread
//
// Purpose-
//       Test thread attributes
//
//----------------------------------------------------------------------------
static void*                        // (Always nullptr)
   test_thread_thread(void*)
{
   pthread_attr_t attr;             // (Our attributes)
   void*          addr= nullptr;    // (An address attribute)
   size_t         size= 0;          // (A size_t attribute)

   if( opt_verbose )
     debugf("\ntest_thread_thread\n");

   Stack stack;
   if( opt_verbose )
     stack.debug("test_thread_thread");

   int rc= pthread_getattr_np(pthread_self(), &attr);
   if( rc )
     return handle_thread_errno("pthread_getattr_np", rc);

   rc= pthread_attr_getstack(&attr, &addr, &size); // Get stack attributes
   if( rc )
     return handle_thread_errno("pthread_attr_getstack", rc);

   if( opt_verbose )
     debugf("Stack: 0x%.12zx.0x%.8zx\n", intptr_t(addr), size);

   rc= pthread_attr_destroy(&attr); // We're done using the attributes
   if( rc )
     return handle_thread_errno("pthread_attr_destroy", rc);

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_thread
//
// Purpose-
//       Test thread attributes
//
//----------------------------------------------------------------------------
static int                          // Number of errors detected
   test_thread( void )              // Test (display) thread attributes
{
   if( opt_verbose )
     debugf("test_thread\n");

   int error_count= 0;

   pthread_attr_t      attr;        // The pthread attributes
   pthread_t           thread;      // The pthread identifier

   Stack stack;
   if( opt_verbose )
     stack.debug("test_thread");

   int rc= pthread_attr_init(&attr);
   if( rc )
     return handle_errno("pthread_attr_init", rc);

   pthread_create(&thread, &attr, test_thread_thread, nullptr);
   pthread_join(thread, nullptr);

   rc= pthread_attr_destroy(&attr); // We're done using the attributes
   if( rc )
     return handle_errno("pthread_attr_destroy", rc);

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
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_info([]()
   {
     if( USE_ITRACE )
       fprintf(stderr,
              "  --trace\t{=size} Create internal trace file './trace.mem'\n"
              );
   });

   tc.on_parm([tr](std::string P, const char* V)
   {
     if( P == "trace" ) {
       if( V )
         opt_trace= tr->ptoi(V);
     }

     return 0;
   });

   tc.on_init([tr](int, char**)
   {
     debug_set_head(Debug::HEAD_THREAD | Debug::HEAD_TIME);
     if( opt_hcdm )
       debug_set_mode(Debug::MODE_INTENSIVE);

     if( USE_ITRACE && opt_trace )
       table= tr->init_trace("./trace.mem", opt_trace);

     return 0;
   });

   tc.on_term([tr]()
   {
     if( table )
       tr->term_trace(table, opt_trace);
   });

   tc.on_main([tr](int, char**)
   {
     int error_count= 0;
     try {
       if( opt_verbose )
         debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

       error_count += test_thread();
     } catch(const char* x) {
       debugf("FAILED: Exception: const char*(%s)\n", x);
       ++error_count;
     } catch(std::exception& x) {
       debugf("FAILED: Exception: exception(%s)\n", x.what());
       ++error_count;
     } catch(...) {
       debugf("FAILED: Exception: ...\n");
       ++error_count;
     }

     if( opt_verbose || error_count ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return int(error_count != 0);
   });

   //-----------------------------------------------------------------------
   // Run the test
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;
   return tc.run(argc, argv);
}

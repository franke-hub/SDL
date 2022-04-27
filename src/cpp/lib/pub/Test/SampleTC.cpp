//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       SampleTC.cpp
//
// Purpose-
//       Sample test case
//
// Last change date-
//       2022/04/24
//
//----------------------------------------------------------------------------
#include <stdexcept>                // For std::runtime exception
#include <string.h>                 // For strcmp
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Wrapper.h>            // For pub::Wrapper

using namespace pub::debugging;     // For debugging functions
using pub::Wrapper;                 // For pub::Wrapper class

#define opt_hcdm       pub::Wrapper::opt_hcdm
#define opt_verbose    pub::Wrapper::opt_verbose

//----------------------------------------------------------------------------
// Extended options
//----------------------------------------------------------------------------
static int             opt_error= false;   // --error
static const char*     opt_throw= nullptr; // --throw
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"argument", required_argument, nullptr,          0} // --argument
,  {"error",   no_argument,       &opt_error,     true} // --error
,  {"throw",   optional_argument, nullptr,           0} // --throw
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0000
//
// Purpose-
//       Sample test function.
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   test0000( void )                 // Sample test funtion
{  if( opt_verbose > 1 )
     debugf("%4d test0000 (sample test)\n", __LINE__);

   int error_count= 0;
   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_error
//
// Purpose-
//       Test test error.
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   test_error( void )               // Test test error
{  debugf("%4d test_error (always fails)\n", __LINE__);

   return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_throw
//
// Purpose-
//       Test test exception.
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   test_throw( void )               // Test test exception
{  debugf("%4d test_throw (Exception: %s)\n", __LINE__, opt_throw);

   int error_count= 0;

   if( strcmp(opt_throw, "pub") == 0 )
     throw pub::Exception("pub exception test");
   if( strcmp(opt_throw, "std") == 0 )
     throw std::runtime_error("std exception test");
   if( true )
     throw opt_throw;

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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_info([]()
   {
     fprintf(stderr, "  --argument\tTest required_argument specifier\n");
     fprintf(stderr, "  --error\tTest test error\n");
     fprintf(stderr, "  --throw\tTest test exception\n");
   });

   tc.on_init([](int argc, char* argv[]) // (Unused in this sample)
   { (void)argc; (void)argv;        // (Unused arguments)
     return 0;
   });

   tc.on_parm([](std::string name, const char* value)
   {
     if( name == "throw" ) {
       if( value == nullptr )
         value= "std";
       opt_throw= value;
     }

     return 0;
   });

   tc.on_term([]()                  // (Unused in this sample)
   {
   });

   tc.on_main([tr](int argc, char* argv[])
   { (void)argc; (void)argv;        // (Unused arguments)
     if( opt_verbose )
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

     int error_count= 0;

     error_count += test0000();
     if( opt_error )
       error_count += test_error();
     if( opt_throw )
       error_count += test_throw();

     tr->report_errors(error_count);
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}

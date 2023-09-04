//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.cpp
//
// Purpose-
//       RFC7540, RFC7541 unit test
//
// Last change date-
//       2023/09/04
//
//----------------------------------------------------------------------------
#include <cstdint>                  // For uint32_t, uint16_t, ...

#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <string.h>                 // For

// The tested includes
#include <pub/TEST.H>               // For VERIFY, ...
#include <pub/Debug.h>              // For debugging
#include <pub/utility.h>            // For pub::utility::visify
#include <pub/Wrapper.h>            // For pub::Wrapper

// Namespace accessors
#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub objects
using namespace PUB::debugging;     // For debugging subroutines

#include "RFC7540.h"                // For class RFC7540, tested
#include "RFC7541.h"                // For class RFC7541, tested

using PUB::utility::visify;         // For pub::utility::visify method
using PUB::Wrapper;                 // For pub::Wrapper class

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             error_count= 0; // Error counter
static Debug*          debug= nullptr; // The Debug object
static void*           table= nullptr; // The Trace table

//----------------------------------------------------------------------------
// Extended options
//----------------------------------------------------------------------------
extern int             opt_hcdm;    // Hard Core Debug Mode?
extern int             opt_verbose; // Debugging verbosity

static int             opt_debug=    0; // --debug
static int             opt_throw=    0; // --throw
static int             opt_trace=    0; // --trace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"debug",           no_argument,       &opt_debug,       true}
,  {"throw",           no_argument,       &opt_throw,       true}
,  {"trace",           optional_argument, &opt_trace, 0x00400000}
,  {0, 0, 0, 0}                     // (End of option list)
};

static const char*     ostr="";     // The getopt_long parameter: optstring

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_7540
//
// Purpose-
//       Test RFC7540
//
//----------------------------------------------------------------------------
static inline int
   test_7540( void )                // Test RFC7540
{
   if( opt_verbose )
     debugf("\ntest_7540:\n");
   error_count= 0;

   RFC7540 test;                    // The test object
   // NOT CODED YET

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_7541
//
// Purpose-
//       Test RFC7541
//
//----------------------------------------------------------------------------
static inline int
   test_7541( void )                // Test RFC7540
{
   typedef RFC7541::Huff  Huffie;
   typedef RFC7541::octet octet;

   if( opt_verbose )
     debugf("\ntest_7541:\n");
   error_count= 0;

   // Bringup display
// RFC7541::debug("TABLES");

   // Static tests
   VERIFY( RFC7541::encoded_length("") == 0 );
   const Huffie nul;                // Empty Huffie
   VERIFY( nul == nul );            // Self comparison
   Huffie one;
   VERIFY( nul == one );            // Empty to empty comparison

   // Dynamic tests
   RFC7541 test;                    // The test object
   char buffer[256];                // The string test buffer
   for(int i= 0; i<256; ++i)
     buffer[i]= i;
   buffer[0]= 255;
   buffer[255]= 0;

   for(size_t L= 1; L<=256; ++L) {
     std::string sample(buffer, L);

     if( opt_verbose )
       debugf("\nsample '%s'\n", pub::utility::visify(sample).c_str());
     one= test.encode(sample);      // (Also tests copy replacement)

     std::string check= one.decode();
     error_count += VERIFY( check == sample );
     error_count += VERIFY(RFC7541::encoded_length(sample) == one.get_size());
     if( opt_verbose)
       debugf("decode '%s'\n", pub::utility::visify(check).c_str());
     if( error_count ) {
       if( !opt_verbose ) {
         debugf("sample '%s'\n", pub::utility::visify(sample).c_str());
         debugf("decode '%s'\n", pub::utility::visify(check).c_str());
       }
       one.debug("encode/decode error");
       break;
     }

     // Copy test
     Huffie two= one;
     error_count += VERIFY( two.decode() == sample );
     if( error_count ) {
       one.debug("one");
       two.debug("two");
       break;
     }

     // Accessor method tests
     const octet* one_octet= one.get_addr();
     size_t one_size=  one.get_size();
     const octet* two_octet= two.get_addr();
     size_t two_size=  two.get_size();
     VERIFY( one_octet != two_octet );
     VERIFY( one_size  == two_size  );
     VERIFY( memcmp(one_octet, two_octet, two_size) == 0 );

     // String constructor test
     Huffie str(sample);
     error_count += VERIFY( str.decode() == sample );
     if( error_count ) {
       one.debug("one");
       two.debug("str");
       break;
     }

     // Comparison operator test
     VERIFY( one == two );
     str= "Strawberry";
     VERIFY( two != str );
     VERIFY( one != nul );

     // Move constructor/assignment test
     Huffie h03= std::move(two);
     VERIFY( one == h03 );
     VERIFY( two == nul );

     two= std::move(h03);
     VERIFY( one == two );
     VERIFY( h03 == nul );

     if( error_count )
       break;
   }

   return error_count != 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_throw
//
// Purpose-
//       Test exception handling
//
//----------------------------------------------------------------------------
[[noreturn]]
static void
   test_throw( void )               // Test exception handling
{
   throw std::runtime_error("just testing");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       torf
//
// Purpose-
//       Convert condition to "true" or "false" string
//
//----------------------------------------------------------------------------
static inline const char*
   torf(bool condition)
{  return condition ? "true" : "false"; }

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
   //-------------------------------------------------------------------------
   // Initialize
   Wrapper  tc(opts, ostr);         // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_parm([tr](std::string P, const char* V)
   {
     if( opt_verbose > 1 )
       debugf("on_parm(%s,%s)\n", P.c_str(), V);

     if( false )                    // Select parameters in else if() {...}
       ;

     else if( P == "trace" ) {
       if( V )
         opt_trace= tr->ptoi(V);
     }

     else if( P[0] == '-' ) {       // If switch option
       debugf("%4d Should not occur %c,%d\n", __LINE__, P[1], P[1]);
     }

     else
       debugf("Unexpected parameter '%s'='%s'\n", P.c_str(), V);

     return 0;
   });

   tc.on_info([](void)
   {
     fprintf(stderr,
            "  --debug\tPrint using Debug object\n"
            "  --throw\tThrow an exception\n"
            "  --trace\t{=size} Create internal trace file './trace.mem'\n"
            );
   });

   tc.on_init([tr](int, char**)
   {
     if( opt_debug )
       debug= tr->init_debug();
     if( opt_trace )
       table= tr->init_trace("./trace.mem", opt_trace);

     return 0;
   });

   tc.on_term([tr](void)
   {
     if( table )
       tr->term_trace(table, opt_trace);
     if( debug )
       tr->term_debug(debug);
   });

   tc.on_main([tr](int, char**)
   {
     error_count= 0;

     if( opt_verbose ) {
       debugf("\nOptions:\n");
       debugf("%5s hcdm\n", torf(opt_hcdm));
       debugf("%5d verbose\n", opt_verbose);

       debugf("%5s debug\n", torf(opt_debug));
       debugf("%5s throw\n", torf(opt_throw));
       debugf("%5s trace: %#x\n", torf(opt_trace), opt_trace);
     }

     if( opt_debug ) {
       debugf("\n");
       tr->debug("opt_debug");
     }

     if( opt_throw )
       test_throw();

     error_count += test_7540();
     error_count += test_7541();

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   try {
     opt_hcdm= HCDM;
     opt_verbose= VERBOSE;

     return tc.run(argc, argv);
   } catch(std::exception& x) {
     debugf("std::exception(%s)\n", x.what());
   } catch(const char* x) {
     debugf("Exception(const char*(%s))\n", x);
   } catch(...) {
     debugf("Exception ...\n");
   }

   return 2;
}

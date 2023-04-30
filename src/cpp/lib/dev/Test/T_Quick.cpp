//----------------------------------------------------------------------------
//
//       Copyright (c) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       T_Quick.cpp
//
// Purpose-
//       Quick verification tests.
//
// Last change date-
//       2023/04/29
//
//----------------------------------------------------------------------------
#include <iostream>                 // For std::cout
#include <stdint.h>                 // For standard integer types
#include <string.h>                 // For std::string, size_t

#include <pub/TEST.H>               // For VERIFY macros
#include <pub/Debug.h>              // For pub::Debug, namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/utility.h>            // For pub::utility::dump/visify
#include <pub/Statistic.h>          // For pub::Statistic
#include <pub/Wrapper.h>            // For pub::Wrapper

#include "pub/http/Recorder.h"      // For pub::Recorder TODO: <pub/Recorder>
#include "pub/http/Codec.h"         // For pub::http::Codec, tested

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using namespace PUB::http;
using PUB::utility::dump;
using PUB::utility::visify;
using PUB::Wrapper;
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_case= false; // (Only set if --hcdm --all)
static int             opt_codec= true; // (Unconditionally true)
static int             opt_recorder= true; // (Unconditionally true)

static struct option   opts[]=      // Options
{  {"all",     optional_argument, nullptr,         0}
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Struct-
//       SampleRecord
//
// Purpose-
//       Sample record, for pub::Recorder test
//
//----------------------------------------------------------------------------
struct SampleRecord : public pub::Recorder::Record {
pub::statistic::Active stat;        // A named, reported statistic

   SampleRecord(std::string name= "unnamed")
{
   this->name= name;

   on_report([this]() {
//   printf("on_report(%s)\n", this->name.c_str());
     char buffer[128];              // (More than large enough)
     sprintf(buffer, "{%8zd,%8zd,%8zd,%8zd}: "
            , stat.counter.load(), stat.current.load()
            , stat.maximum.load(), stat.minimum.load());
     return std::string(buffer) + this->name;
   }); // on_report

   on_reset([this]() {
     printf("on_reset(%s)\n", this->name.c_str());
     stat.counter.store(0);
     stat.current.store(0);
     stat.maximum.store(0);
     stat.minimum.store(0);
   }); // on_reset
}  // (constructor)
}; // struct SampleRecord

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_case
//
// Purpose-
//       Testcase example
//
//----------------------------------------------------------------------------
static inline int
   test_case( void )
{
   if( opt_verbose )
     debugf("\ntest_case\n");

   int error_count= 0;              // Error counter

   error_count += VERIFY( true );   // Dummy test

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_codec
//
// Purpose-
//       Test ~/src/cpp/inc/pub/http/Codec.h
//
//----------------------------------------------------------------------------
static inline int
   test_codec( void )               // Test Codec.h
{
   if( opt_verbose )
     debugf("\ntest_codec:\n");

   int error_count= 0;

   Codec64 base64;
   std::string inp= "abcdefghijklmnopqrstuvwxyz";
   inp= inp + inp + "\r\n";
   inp= inp + inp + inp + inp;
   inp= inp + inp + inp + inp;

   for(int i= 0; i<10; ++i) {
     std::string out= base64.encode(inp);
     std::string ver= base64.decode(out);
     error_count += VERIFY( inp == ver );
     if( error_count || (opt_verbose > 1 && i == 4) ) {
       debugf("out:{{\n%s\n}}\n", visify(out).c_str());
       debugf("inp(%s)\n", visify(inp).c_str());
       debugf("ver(%s)\n", visify(ver).c_str());
       break;
     }

     inp += "X";
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_recorder
//
// Purpose-
//       Test ~/src/cpp/inc/pub/http/Recorder.h
//
//----------------------------------------------------------------------------
static inline int
   test_recorder( void )
{
   typedef pub::Recorder    Recorder;
   typedef Recorder::Record Record;

   if( opt_verbose )
     debugf("\ntest_recorder:\n");

   int error_count= 0;              // Error counter

   Recorder recorder;
   Recorder::set(&recorder);
   SampleRecord one("one");
   SampleRecord two("two");

   recorder.insert(&one);
   Recorder::get()->insert(&two);   // Using the global Recorder

   // Do something that updates one.stat and two.stat
   one.stat.inc(); one.stat.inc(); one.stat.inc(); one.stat.dec();
   two.stat.inc(); two.stat.inc();

   // Verify the report (Requires opt_verbose)
   if( opt_verbose ) {
     recorder.report([](Record& record) {
       std::cout << record.h_report() << "\n";
     }); // reporter.report

     std::cout << "\nRESET\n";
     Recorder::get()->reset();
     recorder.report([](Record& record) {
       std::cout << record.h_report() << "\n";
     }); // reporter.report

     std::cout << "\nREMOVE\n";
     Recorder::get()->remove(&two); // Using the global Recorder
     recorder.report([](Record& record) {
       std::cout << record.h_report() << "\n";
     }); // reporter.report
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_dirty
//
// Purpose-
//       A quick and dirty test.
//
//----------------------------------------------------------------------------
static inline int
   test_dirty( void )
{
   if( opt_verbose )
     debugf("\ntest_dirty\n");

   int error_count= 0;              // Error counter

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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_info([]()
   {
     fprintf(stderr, "  --all\t\tRun all regression tests\n"
            );
   });

   tc.on_init([](int argc, char* argv[]) // (Unused in this sample)
   { (void)argc; (void)argv;        // (Unused arguments)

     return 0;
   });

   tc.on_parm([](std::string name, const char* value)
   {
     if( opt_hcdm )
       debugf("on_parm(%s,%s)\n", name.c_str(), value);

     if( name == "all" ) {          // Note: specify --hcdm *BEFORE* --all
       if( opt_hcdm ) {
         opt_case= true;            // (Only set here)
       }
     }

     return 0;
   });

   tc.on_term([]()                  // (Unused in this sample)
   {
   });

   tc.on_main([tr](int argc, char* argv[])
   { (void)argc; (void)argv;        // (Unused arguments)
     if( opt_hcdm || opt_verbose ) {
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       debug_set_mode(Debug::MODE_INTENSIVE);
     }

     int error_count= 0;

     if( opt_case )    error_count += test_case();
     if( opt_codec )   error_count += test_codec();
     if( opt_recorder) error_count += test_recorder();
//   if( true )        error_count += test_dirty();

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;

   return tc.run(argc, argv);
}

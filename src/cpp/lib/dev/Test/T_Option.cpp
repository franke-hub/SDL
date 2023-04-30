//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       T_Option.cpp
//
// Purpose-
//       Test http::Options.h
//
// Last change date-
//       2023/04/29
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string

#include <pub/TEST.H>               // For VERIFY macro
#include <pub/Debug.h>              // For debugging classes and functions
#include <pub/Wrapper.h>            // For pub::Wrapper

#include "pub/http/Options.h"       // For pub::http::Options, tested

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub classes
using namespace PUB::debugging;     // For pub debugging functions
using namespace PUB::http;          // For pub::http classes
using PUB::Wrapper;
using namespace std;                // For std classes

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
};

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
typedef Options::const_iterator const_iterator;

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_case= false; // (Only set if --hcdm --all)
static int             opt_dirty= false; // --dirty
static int             opt_main= true; // (Unconditionally true)

static struct option   opts[]=      // Options
{  {"all",     optional_argument, nullptr,         0} // --all
,  {"dirty",   no_argument,       &opt_dirty,   true} // --dirty
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define TRY_CATCH(THIS) \
{{{{ bool caught= false; \
     try { THIS; } \
     catch(std::exception& X) { \
       caught= true; \
       if( opt_verbose ) \
         debugf("catch(%s)\n", #THIS); \
       } \
     error_count += VERIFY( caught ); \
}}}}

#define TRY_VALID(THIS) \
{{{{ bool caught= false; \
     try { THIS; } \
     catch(std::exception& X) { \
       caught= true; \
       if( opt_verbose ) \
         debugf("catch(%s)\n", #THIS); \
       } \
     error_count += VERIFY( !caught ); \
}}}}

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

   Options opts;
   for(const_iterator it= opts.begin(); it != opts.end(); ++it) {
     ++error_count;
     debugf("%4d SHOULD NOT OCCUR: iterator: %s: %s\n", __LINE__
           , it->first.c_str(), it->second.c_str());
   }

   // Test TRY_CATCH
   if( opt_verbose ) {
     if( opt_hcdm ) {
       debugf("Expected: 6 messages:\n%s%s%s%s%s%s%s\n",
              "  catch(throw std::runtime_error('OK')\n",
              "  TRY_CATCH\n",
              "  Error: VERIFY(caught)\n",
              "  TRY_VALID\n",
              "  catch(throw std::runtime_error('NG')\n",
              "  Error: VERIFY(!caught)\n",
              "  (2 errors detected)\n"
              );
     } else {
       debugf("Expected: 2 messages:\n%s%s\n",
              "  catch(throw std::runtime_error('OK')\n",
              "  TRY_VALID\n"
              );
     }
   } else if( opt_hcdm ) {
     debugf("Expected: 2 messages:\n%s%s%s\n",
            "  Error: VERIFY(caught)\n",
            "  Error: VERIFY(!caught)\n",
            "  (2 errors detected)\n"
            );
   }

   TRY_CATCH( throw std::runtime_error("OK") );
   if( opt_hcdm )
     TRY_CATCH(                     // Test: NO EXECEPTION THROWN error
       if( opt_verbose)
         debugf("%4d TRY_CATCH\n", __LINE__);
     )

   TRY_VALID(
     if(opt_verbose) debugf("%4d TRY_VALID\n", __LINE__);
   )
   if( opt_hcdm )                    // Test: EXECPTION THROWN error
     TRY_VALID( throw std::runtime_error("NG") );

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_main
//
// Purpose-
//       All other tests
//
//----------------------------------------------------------------------------
static inline int
   test_main( void )                 // All other tests
{
   if( opt_verbose )
     debugf("\ntest_main:\n");
   int error_count= 0;

   Options opts;
   if( opt_verbose )
     debugf("\ninsert\n");
   error_count += VERIFY( !opts.insert("This", "The THIS value") );
   error_count += VERIFY( !opts.insert("That", "The THAT value") );
   error_count += VERIFY( !opts.insert("Other", "The OTHER value") );

   TRY_VALID(opts.begin()++);       // (Allowed, temporary discarded)
   TRY_VALID(opts.end()++);         // (Allowed, temporary has no effect)
   TRY_VALID(++opts.end());         // (Allowed, temporary has no effect)

   if( opt_verbose )
     debugf("\niterator++\n");
   int i= 0;
   for(const_iterator it= opts.begin(); it != opts.end(); it++) {
     if( opt_verbose )
       debugf("[%d] %s: '%s'\n", i, it->first.c_str(), it->second.c_str());
     switch(i) {
       case 0:
         error_count += VERIFY( it->first == "This" );
         error_count += VERIFY( it->second == "The THIS value" );
         break;

       case 1:
         error_count += VERIFY( it->first == "That" );
         error_count += VERIFY( it->second == "The THAT value" );
         break;

       default:
         error_count += VERIFY( it->first == "Other" );
         error_count += VERIFY( it->second == "The OTHER value" );
         break;
     }
     i++;
   }

   if( opt_verbose )
     debugf("\n++iterator\n");
   i= 0;
   for(const_iterator it= opts.begin(); it != opts.end(); ++it) {
     if( opt_verbose )
       debugf("[%d] %s: '%s'\n", i, it->first.c_str(), it->second.c_str());
     switch(i) {
       case 0:
         error_count += VERIFY( it->first == "This" );
         error_count += VERIFY( it->second == "The THIS value" );
         break;

       case 1:
         error_count += VERIFY( it->first == "That" );
         error_count += VERIFY( it->second == "The THAT value" );
         break;

       default:
         error_count += VERIFY( it->first == "Other" );
         error_count += VERIFY( it->second == "The OTHER value" );
         break;
     }
     ++i;
   }

   error_count += VERIFY(  opts.insert("This", "The THIS replacement value") );
   if( opt_verbose )
     debugf("\nlocate\n");          // (Returns const char*)
   error_count += VERIFY( strcmp(opts.locate("this"),
                                "The THIS replacement value") == 0 );
   error_count += VERIFY( strcmp(opts.locate("THIS"),
                                "The THIS replacement value") == 0 );
   error_count += VERIFY( strcmp(opts.locate("that"),
                                "The THAT value") == 0 );
   error_count += VERIFY( strcmp(opts.locate("other"),
                                "The OTHER value") == 0 );
   error_count += VERIFY( opts.locate("nada")  == nullptr);
   if( opt_verbose ) {
     debugf("opts.locate('this'): '%s')\n",  opts.locate("this"));
     debugf("opts.locate('THIS'): '%s')\n",  opts.locate("THIS"));
     debugf("opts.locate('that'): '%s')\n",  opts.locate("that"));
     debugf("opts.locate('other'): '%s')\n",  opts.locate("other"));
     debugf("opts.locate('nada'): '%s')\n",  opts.locate("nada"));
   }

   if( opt_verbose )
     debugf("\noperator[]\n");      // (Returns string, inserts "" if missing)
   error_count += VERIFY(opts["this"] == "The THIS replacement value" );
   error_count += VERIFY(opts["THIS"] == "The THIS replacement value" );
   error_count += VERIFY(opts["that"] == "The THAT value" );
   error_count += VERIFY(opts["other"] == "The OTHER value" );
   error_count += VERIFY(opts["nada"] == ""); // (Inserts "")
   if( opt_verbose ) {
     debugf("opts['this']: '%s'\n", opts["this"].c_str());
     debugf("opts['THIS']: '%s'\n", opts["THIS"].c_str());
     debugf("opts['that']: '%s'\n", opts["that"].c_str());
     debugf("opts['other']: '%s'\n", opts["other"].c_str());
     debugf("opts['nada']: '%s'\n", opts["nada"].c_str());
   }

   if( opt_verbose )
     debugf("\nremove\n");
   error_count += VERIFY(  opts.remove("nada") );
   error_count += VERIFY( !opts.remove("nada") );
   if( opt_verbose )
     debugf("nada: '%s'\n", opts.locate("nada"));

   if( opt_verbose )
     debugf("\nExceptions, end() handling:\n");
   const_iterator xx= opts.end();
   xx++; xx++; xx++; xx++;
   error_count += VERIFY( xx == opts.end() );
   TRY_CATCH( debugf("%s\n", xx->first.c_str()); )
   TRY_CATCH( debugf("%s\n", (*xx).first.c_str()); )
   TRY_VALID( ++xx )
   TRY_VALID( ++opts.end() )

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
     fprintf(stderr, "  --all\t\tRun all regression tests\n"
            );
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

   tc.on_main([tr](int argc, char* argv[])
   { (void)argc; (void)argv;        // (Unused arguments)
     if( opt_hcdm || opt_verbose ) {
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       debug_set_mode(Debug::MODE_INTENSIVE);
     }

     int error_count= 0;

     if( opt_main )    error_count += test_main();
     if( opt_case )    error_count += test_case();
     if( opt_dirty )   error_count += test_dirty();

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

   return tc.run(argc, argv);
}

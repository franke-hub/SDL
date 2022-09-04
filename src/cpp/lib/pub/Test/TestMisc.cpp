//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestMisc.cpp
//
// Purpose-
//       Miscellaneous tests.
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <functional>
#include <getopt.h>
#include <string.h>

#include <pub/Debug.h>
#include <pub/Exception.h>

// The tested includes
#include "pub/TEST.H"               // For VERIFY, ...
#include "pub/Properties.h"         // For pub::Properties
#include "pub/Statistic.h"          // For pub::Statistic
#include "pub/Tokenizer.h"          // For pub::Tokenizer
#include "pub/Wrapper.h"            // For pub::Wrapper

// Namespace accessors
#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;
using Exception= PUB::Exception;
using IndexException= PUB::IndexException;
using PUB::Wrapper;                 // For pub::Wrapper class

#define opt_verbose    PUB::Wrapper::opt_verbose

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Example
//
// Purpose-
//       Test (Example.h)
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_Example( void )             // Test Example.h
{
   int                 error_count= 0; // Number of errors encountered

   if( opt_verbose )
     debugf("\ntest_Example\n");

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Properties
//
// Purpose-
//       Test Properties.
//
//----------------------------------------------------------------------------
static inline int
   test_Properties( void )          // Test Properties
{
   int error_count= 0;

   if( opt_verbose )
     debugf("\ntest_Properties\n");

   using _LIBPUB_NAMESPACE::Properties;
   typedef Properties::MapIter_t MapIter_t; // Map iterator type
   Properties props;                // Our test object

   std::string s;

   s= "yY";
   props.insert("yY", "yar");
   if( opt_verbose ) {
     debugf("%s: %s\n", s.c_str(), props[ s ]);
     debugf("%s: %s\n", "yY"     , props["yY"]);
     debugf("%s: %s\n", "Yy"     , props.get_property("Yy"));
   }
   error_count += VERIFY(strcmp(props[ s ], "yar") == 0);
   error_count += VERIFY(strcmp(props["yY"], "yar") == 0);
   error_count += VERIFY(strcmp(props.get_property("Yy"), "yar") == 0);

   s= "Nn";
   props.insert("Nn", "nar");
   error_count += VERIFY(strcmp(props[ s ], "nar") == 0);
   error_count += VERIFY(strcmp(props["Nn"], "nar") == 0);
   error_count += VERIFY(strcmp(props.get_property("nN"), "nar") == 0);

   s= "W";
   props.insert(s, "wasp");
   error_count += VERIFY(strcmp(props[ s ], "wasp") == 0);
   error_count += VERIFY(strcmp(props["W"], "wasp") == 0);
   error_count += VERIFY(strcmp(props.get_property("w"), "wasp") == 0);

   error_count += VERIFY(strcmp(props.get_property("Foo", "bar"), "bar") == 0);
   props.insert("foo", "bart s");
   error_count += VERIFY(strcmp(props.get_property("Foo", "bar"), "bart s") == 0);
   props.remove("foo");
   error_count += VERIFY(props.get_property("foo") == nullptr);

   if( opt_verbose ) {
     debugf("\nProperties:\n");
     for(MapIter_t mi= props.begin(); mi != props.end(); mi++) {
       debugf("%s: '%s'\n", mi->first.c_str(), mi->second.c_str());
     }
   }

   //-------------------------------------------------------------------------
   // Verify IndexException raised where expected
   try {
     props.insert("Yy", "yard");
     errorf("%4d Missing IndexException\n", __LINE__);
     error_count++;
   } catch(IndexException& X) {
     if( opt_verbose)
       debugf("%4d Expected IndexException caught: %s\n", __LINE__,
             std::string(X).c_str());
   } catch(std::exception& X) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, X.what());
     error_count++;
   } catch(...) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, "...");
     error_count++;
   }

   try {
     props["foo"];
     errorf("%4d Missing IndexException\n", __LINE__);
     error_count++;
   } catch(IndexException& X) {
     if( opt_verbose)
       debugf("%4d Expected IndexException caught: %s\n", __LINE__,
             std::string(X).c_str());
   } catch(std::exception& X) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, X.what());
     error_count++;
   } catch(...) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, "...");
     error_count++;
   }

   try {
     props.remove("foo");
     errorf("%4d Missing IndexException\n", __LINE__);
     error_count++;
   } catch(IndexException& X) {
     if( opt_verbose)
       debugf("%4d Expected IndexException caught: %s\n", __LINE__,
             std::string(X).c_str());
   } catch(std::exception& X) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, X.what());
     error_count++;
   } catch(...) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, "...");
     error_count++;
   }

   //-------------------------------------------------------------------------
   // Verify Properties.reset() method
   props.reset();
   error_count += VERIFY(props.get_property("S") == nullptr);
   error_count += VERIFY(props.begin() == props.end());

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Statistic
//
// Purpose-
//       Test Statistic.
//
//----------------------------------------------------------------------------
static inline int
   test_Statistic( void )           // Test Statistic
{
   int error_count= 0;

   if( opt_verbose )
     debugf("\ntest_Statistic\n");

   PUB::Statistic stat;
   error_count += VERIFY(stat.inc() == 1 );
   error_count += VERIFY(stat.inc() == 2 );
   error_count += VERIFY(stat.inc() == 3 );
   error_count += VERIFY(stat.inc() == 4 );
   error_count += VERIFY(stat.inc() == 5 );
   error_count += VERIFY(stat.dec() == 4 );
   error_count += VERIFY(stat.dec() == 3 );
   error_count += VERIFY(stat.dec() == 2 );
   error_count += VERIFY(stat.inc() == 3 );

   error_count += VERIFY(stat.counter.load() == 6 );
   error_count += VERIFY(stat.current.load() == 3 );
   error_count += VERIFY(stat.maximum.load() == 5 );
   error_count += VERIFY(stat.minimum.load() == 2 );

   if( opt_verbose ) {
     printf("stat: %ld  %ld,%ld,%ld\n", stat.counter.load()
           , stat.minimum.load() , stat.current.load(), stat.maximum.load());
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Tokenizer
//
// Purpose-
//       Test Tokenizer.
//
//----------------------------------------------------------------------------
static inline int
   test_Tokenizer( void )           // Test Tokenizer
{
   int error_count= 0;

   if( opt_verbose )
     debugf("\ntest_Tokenizer\n");

   using _LIBPUB_NAMESPACE::Tokenizer;
   typedef _LIBPUB_NAMESPACE::Tokenizer::Iterator Iterator;
   Tokenizer izer(" a  b  c  def g "); // Our test object
   Iterator it= izer.begin();
   error_count += VERIFY( it != izer.end() );
   error_count += VERIFY( it() == "a" );
   error_count += VERIFY( (it++)() == "a" );
   error_count += VERIFY( (++it)() == "c" );
   error_count += VERIFY( (++it)() == "def" );
   error_count += VERIFY( it != izer.end() );
   error_count += VERIFY( (++it)() == "g" );
   error_count += VERIFY( ++it == izer.end() );
   error_count += VERIFY( ++it == izer.end() );
   error_count += VERIFY( it() == "" );

   if( opt_verbose ) {
     for(it= izer.begin(); it != izer.end(); ++it)
       printf("%s\n", it().c_str());
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
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   Wrapper  tc;                     // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_main([tr](int, char*[])
   {
     int error_count= 0;

     error_count += test_Properties(); // Test Properties.h
     error_count += test_Statistic(); // Test Statistic.h
     error_count += test_Tokenizer(); // Test Tokenizer.h

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}

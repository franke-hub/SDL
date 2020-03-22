//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
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
//       2020/01/27
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <functional>
#include <getopt.h>
#include <string.h>

#include <pub/Debug.h>
#include <pub/Exception.h>

// The tested includes
#include "pub/Clock.h"
#include "pub/Interval.h"
#include "pub/Properties.h"
#include "pub/Tokenizer.h"

// Helper functions
#include "MUST.H"

// Namespace accessors
using namespace _PUB_NAMESPACE::debugging;
using Exception= _PUB_NAMESPACE::Exception;
using IndexException= _PUB_NAMESPACE::IndexException;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define IFDEBUG(x) { if( opt_debug ) { x }}

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_debug= 0; // --debug (level)
static int             opt_help= false; // --help or error
static int             opt_index;   // Option index

static struct option   OPTS[]=      // Options
{  {"help",    no_argument,       &opt_help,    true}

,  {"debug",   optional_argument, nullptr,      0}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HELP
,  OPT_DEBUG
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       interval_test
//
// Purpose-
//       Tests both Clock.h and Interval.h
//
//----------------------------------------------------------------------------
static inline void
   interval_test(                   // Test Clock.h and/or Interval.h
     std::function<void(void)> starter,
     std::function<double(void)> stopper)
{
   const int           RETRIES= 256;
   const double        TIMEOUT= 1.732; // Clock timeout

   uint32_t count;
   starter();
   double delta= stopper();
   double prior= delta;

   for(count= 0; count<RETRIES; count++) // Try to get a zero interval
   {
     prior= delta;
     delta= stopper();
     if( (delta - prior) == 0.0 )
       break;
   }

   if( count != RETRIES )           // If a zero interval found
   {
     count= 0;
     prior= delta;
     delta= stopper();
     while( (delta - prior) != 0.0 )
     {
       prior= delta;
       delta= stopper();
     }

     while( (delta - prior) == 0.0 )
     {
       count++;
       prior= delta;
       delta= stopper();
     }
   } else {
     count= 0;
     starter();
     delta= stopper();
   }

   IFDEBUG( debugf("%.6g Minimum interval\n", delta); )
   IFDEBUG( debugf("%d Interval count\n", count); )

   prior= delta;
   while( delta < TIMEOUT )
   {
     IFDEBUG( traceh("%.6f\r", delta); )
     prior= delta;
     delta= stopper();
   }
   IFDEBUG( traceh("\n"); )
   IFDEBUG( debugf("%.6f Seconds (%f)\n", delta, TIMEOUT); )
   IFDEBUG( debugf("%.6f Prior\n", prior); )
   IFDEBUG( debugf("%.6g Diff\n", delta - prior); )
}

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
   int                 errorCount= 0; // Number of errors encountered

   debugf("\ntest_Example\n");

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Clock
//
// Purpose-
//       Test Clock.h
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_Clock( void )               // Test Clock.h
{
   int                 errorCount= 0; // Number of errors encountered

   debugf("\ntest_Clock\n");

   _PUB_NAMESPACE::Clock clock;
   double start= clock.now();

   interval_test([clock, start]() mutable {start= clock.now(); },
                 [clock, start]() {return clock.now() - start; });

   IFDEBUG( debugf("%14.3f Clock::now()\n", _PUB_NAMESPACE::Clock::now()); )
   IFDEBUG( debugh("Debug::now()\n"); )
   _PUB_NAMESPACE::Clock one;
   _PUB_NAMESPACE::Clock two;
   _PUB_NAMESPACE::Clock wow;
   two= 3.0;

   wow= one + two;
   errorCount += VERIFY( wow == one + two );
   errorCount += VERIFY( (double)wow == (double)one + (double)two );

   wow= one - two;
   errorCount += VERIFY( wow == one - two );
   errorCount += VERIFY( (double)wow == (double)one - (double)two );

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Interval
//
// Purpose-
//       Test Interval.h
//
//----------------------------------------------------------------------------
static inline int                   // Number of errors encountered
   test_Interval( void )            // Test Interval.h
{
   int                 errorCount= 0; // Number of errors encountered

   debugf("\ntest_Interval\n");

   _PUB_NAMESPACE::Interval interval;

   interval_test([interval]() mutable {interval.start(); },
                 [interval]() mutable {return interval.stop(); });

   return errorCount;
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
   int errorCount= 0;

   debugf("\ntest_Properties\n");

   using _PUB_NAMESPACE::Properties;
   typedef Properties::MapIter_t MapIter_t; // Map iterator type
   Properties props;                // Our test object

   std::string s;

   s= "yY";
   props.insert("yY", "yar");
   if( opt_debug ) {
     debugf("%s: %s\n", s.c_str(), props[ s ]);
     debugf("%s: %s\n", "yY"     , props["yY"]);
     debugf("%s: %s\n", "Yy"     , props.getProperty("Yy"));
   }
   errorCount += VERIFY(strcmp(props[ s ], "yar") == 0);
   errorCount += VERIFY(strcmp(props["yY"], "yar") == 0);
   errorCount += VERIFY(strcmp(props.getProperty("Yy"), "yar") == 0);

   s= "Nn";
   props.insert("Nn", "nar");
   errorCount += VERIFY(strcmp(props[ s ], "nar") == 0);
   errorCount += VERIFY(strcmp(props["Nn"], "nar") == 0);
   errorCount += VERIFY(strcmp(props.getProperty("nN"), "nar") == 0);

   s= "W";
   props.insert(s, "wasp");
   errorCount += VERIFY(strcmp(props[ s ], "wasp") == 0);
   errorCount += VERIFY(strcmp(props["W"], "wasp") == 0);
   errorCount += VERIFY(strcmp(props.getProperty("w"), "wasp") == 0);

   errorCount += VERIFY(strcmp(props.getProperty("Foo", "bar"), "bar") == 0);
   props.insert("foo", "bart s");
   errorCount += VERIFY(strcmp(props.getProperty("Foo", "bar"), "bart s") == 0);
   props.remove("foo");
   errorCount += VERIFY(props.getProperty("foo") == nullptr);

   if( opt_debug ) {
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
     errorCount++;
   } catch(IndexException X) {
     if( opt_debug)
       debugf("%4d Expected IndexException caught: %s\n", __LINE__,
             std::string(X).c_str());
   } catch(std::exception X) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, X.what());
     errorCount++;
   } catch(...) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, "...");
     errorCount++;
   }

   try {
     props["foo"];
     errorf("%4d Missing IndexException\n", __LINE__);
     errorCount++;
   } catch(IndexException X) {
     if( opt_debug)
       debugf("%4d Expected IndexException caught: %s\n", __LINE__,
             std::string(X).c_str());
   } catch(std::exception X) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, X.what());
     errorCount++;
   } catch(...) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, "...");
     errorCount++;
   }

   try {
     props.remove("foo");
     errorf("%4d Missing IndexException\n", __LINE__);
     errorCount++;
   } catch(IndexException X) {
     if( opt_debug)
       debugf("%4d Expected IndexException caught: %s\n", __LINE__,
             std::string(X).c_str());
   } catch(std::exception X) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, X.what());
     errorCount++;
   } catch(...) {
     errorf("%4d Wrong exception type(%s)\n", __LINE__, "...");
     errorCount++;
   }

   //-------------------------------------------------------------------------
   // Verify Properties.reset() method
   props.reset();
   errorCount += VERIFY(props.getProperty("S") == nullptr);
   errorCount += VERIFY(props.begin() == props.end());

   return errorCount;
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
   int errorCount= 0;

   debugf("\ntest_Tokenizer\n");

   using _PUB_NAMESPACE::Tokenizer;
   typedef _PUB_NAMESPACE::Tokenizer::Iterator Iterator;
   Tokenizer izer(" a  b  c  def g "); // Our test object
   Iterator it= izer.begin();
   errorCount += VERIFY( it != izer.end() );
   errorCount += VERIFY( it() == "a" );
   errorCount += VERIFY( (it++)() == "a" );
   errorCount += VERIFY( (++it)() == "c" );
   errorCount += VERIFY( (++it)() == "def" );
   errorCount += VERIFY( it != izer.end() );
   errorCount += VERIFY( (++it)() == "g" );
   errorCount += VERIFY( ++it == izer.end() );
   errorCount += VERIFY( ++it == izer.end() );
   errorCount += VERIFY( it() == "" );

   if( opt_debug ) {
     for(it= izer.begin(); it != izer.end(); ++it)
       printf("%s\n", it().c_str());
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Informational exit
{
   fprintf(stderr, "TestMisc [options]\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --debug\t{=value}\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis example.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 C;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   opterr= 0;                       // Do not write error messages

   while( (C= getopt_long(argc, (char**)argv, ":", OPTS, &opt_index)) != -1 )
     switch( C )
     {
       case 0:
         switch( opt_index )
         {
           case OPT_DEBUG:
             if( optarg )
               opt_debug= atoi(optarg);
             else
               opt_debug= -1;
             break;

           default:
             break;
         }
         break;

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Option requires an argument '%s'.\n",
                           argv[optind-1]);
         else
           fprintf(stderr, "Option requires an argument '-%c'.\n", optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Unknown option '%s'.\n", argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "Unknown option '-%c'.\n", optopt);
         else
           fprintf(stderr, "Unknown option character '0x%x'.\n", optopt);
         break;

       default:
         fprintf(stderr, "%4d SNO ('%c',0x%x).\n", __LINE__, C, C);
         exit( EXIT_FAILURE );
     }

   if( opt_help )
     info();
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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int                 errorCount= 0; // Number of errors encountered

   try {
     parm(argc, argv);

     errorCount += test_Properties(); // Test Properties.h
     errorCount += test_Tokenizer(); // Test Tokenizer.h

     opt_debug= true;               // Following tests require output
     errorCount += test_Clock();    // Test Clock.h
     errorCount += test_Interval(); // Test Interval.h
   } catch(Exception& X) {
     errorCount++;
     debugf("%4d %s\n", __LINE__, std::string(X).c_str());
   } catch(std::exception& X) {
     errorCount++;
     debugf("%4d std::exception(%s)\n", __LINE__, X.what());
   } catch(...) {
     errorCount++;
     debugf("%4d catch(...)\n", __LINE__);
   }

   if( errorCount == 0 )
     debugf("NO errors encountered\n");
   else if( errorCount == 1 )
     debugf("1 error encountered\n");
   else
     debugf("%d errors encountered\n", errorCount);

   return (errorCount != 0);
}

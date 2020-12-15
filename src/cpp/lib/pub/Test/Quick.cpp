//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Quick.cpp
//
// Purpose-
//       Quick verification tests.
//
// Last change date-
//       2020/12/14
//
//----------------------------------------------------------------------------
#include <chrono>
#include <ctype.h>                  // For isprint()
#include <getopt.h>                 // For getopt()
#include <iostream>
#include <new>
#include <random>
#include <string>
#include <thread>

#include <assert.h>
#include <endian.h>
#include <errno.h>                  // For errno, ...
#include <limits.h>                 // For INT_MIN, INT_MAX, ...
#include <stdio.h>
#include <stddef.h>                 // For offsetof
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "MUST.H"                   // Error counting assert
#include "pub/Debug.h"
#include "pub/config.h"             // For _PUB_NAMESPACE
#include "pub/Object.h"
#include "pub/Exception.h"
#include "pub/Latch.h"              // See test_Latch
#include "pub/memory.h"             // See test_atomic_shared_ptr, NOT CODED YET
#include "pub/Signals.h"            // See test_Signals
#include "pub/Trace.h"              // See test_Trace
#include "pub/UTF8.h"               // See test_UTF8
#include "pub/utility.h"            // For _PUB_NAMESPACE::utility
using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef CHECK                       // If defined, use parameter checking
#undef  CHECK                       // SHOULD match Trace.cpp
#endif

#ifndef TRACE                       // If defined, use internal trace
#define TRACE                       // (We test the iftrace macro)
#endif

#define IFDEBUG(x) { if( opt_debug ) {x} }

#include "pub/ifmacro.h"            // Dependent macro

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_case= false; // (Only set if --all)
static int             opt_debug= 0; // --debug
static int             opt_dump= false; // --dump
static int             opt_hcdm= false; // --hcdm
static int             opt_help= false; // --help or error
static int             opt_index;   // Option index
static int             opt_latch= false; // --latch
static int             opt_misc= false; // --misc
static int             opt_must= false; // --must
static int             opt_signals= false; // --signals
static int             opt_trace= false; // --trace
static int             opt_verbose= false; // --verbose
static int             opt_utf8= false; // --utf8
static const char*     utf8_encode= nullptr; // --utf8.encode
static const char*     utf8_decode= nullptr; // --utf8.decode

static struct option   OPTS[]=      // Options
{  {"help",    no_argument,       &opt_help,    true}
,  {"hcdm",    no_argument,       &opt_hcdm,    true}
,  {"dump",    no_argument,       &opt_dump,    true}
,  {"latch",   no_argument,       &opt_latch,   true}
,  {"misc",    no_argument,       &opt_misc,    true}
,  {"must",    no_argument,       &opt_must,    true}
,  {"signals", no_argument,       &opt_signals, true}
,  {"trace",   no_argument,       &opt_trace,   true}
,  {"utf8",    no_argument,       &opt_utf8,    true}
,  {"verbose", no_argument,       &opt_verbose, true}

,  {"all",     optional_argument, nullptr,      0}
,  {"debug",   optional_argument, nullptr,      0}
,  {"utf8.encode", required_argument, nullptr,  0}
,  {"utf8.decode", required_argument, nullptr,  0}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HELP
,  OPT_HCDM
,  OPT_DUMP
,  OPT_LATCH
,  OPT_MISC
,  OPT_MUST
,  OPT_SIGNALS
,  OPT_TRACE
,  OPT_UTF8
,  OPT_VERBOSE
,  OPT_ALL
,  OPT_DEBUG
,  OPT_UTF8_ENCODE
,  OPT_UTF8_DECODE
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       hcdmf
//
// Purpose-
//       Hard Core Debug Mode printf
//
//----------------------------------------------------------------------------
static void
   hcdmf(int line)
{  IFHCDM( debugf("%4d Quick (HCDM)\n", line); ) ELHCDM((void)line;) }

//----------------------------------------------------------------------------
//
// Subroutine-
//       hexchar
//
// Purpose-
//       Get value of HEX character
//
//----------------------------------------------------------------------------
static unsigned                     // HEX value (EXCEPTION if error)
   hexchar(                         // HEX character value
     int               C)           // For this HEX character
{
   if( C >= '0' && C <= '9' )
     return C - '0';

   if( C >= 'a' && C <= 'f' )
     return 10 + C - 'a';

   if( C >= 'A' && C <= 'F' )
     return 10 + C - 'A';

   throwf("Invalid HEX character(%c)", C);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static void
   info( void)                      // Parameter description
{
   fprintf(stderr, "Quick [options]\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --all\t\tRun all regression tests\n"
                   "  --dump\tDump.h debug.out test\n"
                   "  --debug\t{=value}\n"
                   "  --latch\tLatch.h regression test\n"
                   "  --signals\tsignals::Signal.h regression test\n"
                   "  --trace\tTrace.h debug.out test\n"
                   "  --utf8\tUTF8.h regression test\n"
                   "  --utf8.encode\t=test test UTF8::encode()\n"
                   "  --utf8.decode\t=test test UTF8::decode()\n"
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
           case OPT_ALL:
             opt_latch= true;
             opt_case= true;
             // opt_dump= true;     // Select separately (visual debugging)
             opt_misc= true;
             opt_signals= true;
             // opt_trace= true;    // Select separately (visual debugging)
             opt_utf8= true;
             break;

           case OPT_DEBUG:
             if( optarg )
               opt_debug= atoi(optarg);
             else
               opt_debug= -1;
             break;

           case OPT_UTF8_ENCODE:
             utf8_encode= optarg;
             break;

           case OPT_UTF8_DECODE:
             utf8_decode= optarg;
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
//       scdmf
//
// Purpose-
//       Soft Core Debug Mode printf
//
//----------------------------------------------------------------------------
static void
   scdmf(int line)
{  IFDEBUG( debugf("%4d Quick (SCDM)\n", line); ) }

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Case
//
// Purpose-
//       Test case example.
//
//----------------------------------------------------------------------------
static inline int
   test_Case( void )                  // Test case example
{
   debugf("\ntest_Case\n");

   int                 errorCount= 0; // Number of errors encountered

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_dump
//
// Purpose-
//       Test utility::dump.h
//
//----------------------------------------------------------------------------
static inline void
   test_dump(                         // Test utility::dump.h
     const char*       buffer,        // The test buffer
     unsigned int      origin,        // The dump origin
     unsigned int      length,        // The dump length
     unsigned int      offset)        // The (virtual) origin
{
   using pub::utility::dump;

   char* ORIGIN= nullptr;
   ORIGIN += offset;

   FILE* stdbug= Debug::get()->get_FILE();
   fprintf(stdbug, "\n%p[%.2x:%.4x:%.2x]\n", buffer, origin, length, offset);
   dump(stdbug, buffer + origin, length, ORIGIN);

   if( opt_verbose ) {
     fprintf(stdout, "\n%p[%.2x:%.4x:%.2x]\n", buffer, origin, length, offset);
     dump(stdout, buffer + origin, length, ORIGIN);
   }
}

static inline void
   test_dump(                         // Test utility::dump.h
     const char*       buffer,        // The test buffer
     unsigned int      origin,        // The dump origin
     unsigned int      length)        // The dump length
{
   using pub::utility::dump;

   FILE* stdbug= Debug::get()->get_FILE();
   fprintf(stdbug, "\n%p[%.2x:%.4x]\n", buffer, origin, length);
   dump(buffer + origin, length);

   if( opt_verbose ) {
     fprintf(stdout, "\n%p[%.2x:%.4x]\n", buffer, origin, length);
     dump(stdout, buffer + origin, length);
   }
}


static inline int
   test_dump( void )                  // Test utility::dump.h
{
   debugf("\ntest_dump%s\n", opt_verbose ? "" : " (See: debug.out)" );

   int                 errorCount= 0; // Number of errors encountered

   using pub::utility::dump;
   alignas(256) char buffer[256];
   for(int i=  0; i<32; i++)
     buffer[i]= i;
   for(size_t i= 32; i<sizeof(buffer); i++)
     buffer[i]= "0123456789ABCDEF"[i%16];

   test_dump(buffer,  3,   3);
   test_dump(buffer,  3,  29);
   test_dump(buffer, 14,  14);
   test_dump(buffer,  1, 126);
   test_dump(buffer, 33, 126);
   test_dump(buffer,  0, 128);
   test_dump(buffer,  1, 128);

   test_dump(buffer,  3,   3,  3);
   test_dump(buffer,  3,  29,  3);
   test_dump(buffer, 14,  14, 14);
   test_dump(buffer,  1, 126,  1);
   test_dump(buffer,  0, 128,  0);
   test_dump(buffer,  1, 128,  1);

   test_dump(buffer,  0, 256);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Latch
//
// Purpose-
//       (Minimally) test Latch.h
//
//----------------------------------------------------------------------------
static inline int
   test_Latch( void )                 // Test Latch.h
{
   debugf("\ntest_Latch\n");

   int                 errorCount= 0; // Number of errors encountered

   //-------------------------------------------------------------------------
   IFDEBUG( debugf("..Testing: Share/ExclusiveLatch\n"); )
   SharedLatch    shared;
   ExclusiveLatch exclusive(shared);

   {{{{ std::lock_guard<decltype(shared)> lock1(shared);
     errorCount += MUST_EQ(shared.count, 1);
     if( exclusive.try_lock() )
       errorCount += MUST_NOT(Obtain exclusive while shared);

     {{{{ std::lock_guard<decltype(shared)> lock2(shared);
       errorCount += MUST_EQ(shared.count, 2);
     }}}}
     errorCount += MUST_EQ(shared.count, 1);
   }}}}
   errorCount += MUST_EQ(shared.count, 0);

   if( exclusive.try_lock() )
   {
     errorCount += MUST_EQ(shared.count, 0x80000000);
     exclusive.unlock();;
     errorCount += MUST_EQ(shared.count, 0);
   } else {
     errorCount += MUST_NOT(Fail to obtain exclusive latch);
   }

   {{{{ std::lock_guard<decltype(exclusive)> lock(exclusive);
     errorCount += MUST_EQ(shared.count, 0x80000000);
   }}}}
   errorCount += MUST_EQ(shared.count, 0);

   //-------------------------------------------------------------------------
   IFDEBUG( debugf("..Testing: RecursiveLatch\n"); )
   std::thread::id not_thread;
   RecursiveLatch recursive;
   errorCount += MUST_EQ(recursive.latch, not_thread);
   errorCount += MUST_EQ(recursive.count, 0);

   {{{{ std::lock_guard<decltype(recursive)> lock1(recursive);
     errorCount += MUST_EQ(recursive.count, 1);

     {{{{ std::lock_guard<decltype(recursive)> lock2(recursive);
       errorCount += MUST_EQ(recursive.count, 2);
     }}}}

     errorCount += MUST_EQ(recursive.count, 1);
   }}}}
   errorCount += MUST_EQ(recursive.latch, not_thread);
   errorCount += MUST_EQ(recursive.count, 0);

   //-------------------------------------------------------------------------
   IFDEBUG( debugf("..Testing: NonRecursiveLatch\n"); )
   NonRecursiveLatch nonrecursive;
   errorCount += MUST_EQ(nonrecursive.latch, not_thread);

   {{{{ std::lock_guard<decltype(nonrecursive)> lock1(nonrecursive);
     try {
       {{{{ std::lock_guard<decltype(nonrecursive)> lock2(nonrecursive);
         errorCount += MUST_NOT(Recursively hold NonRecursiveLatch);
       }}}}
     } catch(Exception& X) {
       IFDEBUG( debugf("....Expected: %s\n", ((std::string)X).c_str()); )
       errorCount += MUST_EQ(nonrecursive.latch, not_thread);
     }
   }}}}

   //-------------------------------------------------------------------------
   IFDEBUG( debugf("..Testing: NullLatch\n"); )
   NullLatch fake_latch;

   {{{{ std::lock_guard<decltype(fake_latch)> lock1(fake_latch);
        std::lock_guard<decltype(fake_latch)> lock2(fake_latch);
        std::lock_guard<decltype(fake_latch)> lock3(fake_latch);
        std::lock_guard<decltype(fake_latch)> lock4(fake_latch);
   }}}}

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Misc
//
// Purpose-
//       Miscellaneous tests.
//
//----------------------------------------------------------------------------
static inline int
   test_Misc( void )                  // Miscellaneous tests
{
   debugf("\ntest_Misc\n");

   int                 errorCount= 0; // Number of errors encountered

   // Test utility.h (utility::dump tested separately)
   using namespace pub::utility;
   using pub::utility::atoi;
   using pub::utility::atol;
   using pub::utility::atox;

   errno= 0;                          // No error

   errorCount += MUST_EQ(atoi("1234567890"), 1234567890);
   errorCount += MUST_EQ(atol("123456789012345"), 123456789012345L);
   errorCount += MUST_EQ(atox("12abcdefABCDEF"), 0x12abcdefABCDEF);
   errorCount += MUST_EQ(atol("0x1234567890"), 0x1234567890L);
   errorCount += MUST_EQ(atoi("  1234567890  "), 1234567890);

   errorCount += MUST_EQ(strcmp(skip_space("  abcd  "), "abcd  "), 0);
   errorCount += MUST_EQ(strcmp(find_space("abcd  efgh"), "  efgh"), 0);

   errorCount += MUST_EQ(*skip_space("  "), '\0');
   errorCount += MUST_EQ(*find_space("abcdefgh"), '\0');

   errorCount += MUST_EQ(errno, 0);

   errno= 0; atoi("");
   errorCount += MUST_EQ(errno, EINVAL);
   errno= 0; atoi("0x");
   errorCount += MUST_EQ(errno, EINVAL);
   errno= 0; atoi("0x0100000000");
   errorCount += MUST_EQ(errno, ERANGE);

   errno= 0;
   errorCount += MUST_EQ(atoi(" 2147483647"),  2147483647);
   errorCount += MUST_EQ(errno, 0);

   errno= 0;
   errorCount += MUST_EQ(atoi("+2147483647"), +2147483647);
   errorCount += MUST_EQ(errno, 0);

   errno= 0; atoi("2147483648");
   errorCount += MUST_EQ(errno, ERANGE);

   errno= 0;
   errorCount += MUST_EQ(atoi("-2147483648"), INT_MIN);
   errorCount += MUST_EQ(errno, 0);
   IFDEBUG( printf("%4d %d %d\n", __LINE__, errno, atoi("-2147483648")); )

   errno= 0;
   errorCount += MUST_EQ(atoi(" 0x80000000"), INT_MIN);
   errorCount += MUST_EQ(errno, 0);
   IFDEBUG( printf("%4d %d %d %x\n", __LINE__, errno, atoi(" 0x80000000"), atoi(" 0x80000000")); )

   errno= 0; atoi("-2147483649");
   errorCount += MUST_EQ(errno, ERANGE);
   IFDEBUG( printf("%4d %d\n", __LINE__, errno); )

   errno= 0;
   errorCount += MUST_EQ(atol(" 9223372036854775807"), 9223372036854775807L);
   errorCount += MUST_EQ(errno, 0);
   IFDEBUG( printf("%4d %d, %ld\n", __LINE__, errno, atol(" 9223372036854775807")); )

   errno= 0;
   errorCount += MUST_EQ(atol("+9223372036854775807"), +9223372036854775807L);
   errorCount += MUST_EQ(errno, 0);
   IFDEBUG( printf("%4d %d, %ld\n", __LINE__, errno, atol("+9223372036854775807")); )

   errno= 0; atol("9223372036854775808");
   errorCount += MUST_EQ(errno, ERANGE);
   IFDEBUG( printf("%4d %d\n", __LINE__, errno); )

   errno= 0;
   errorCount += MUST_EQ(atol("-9223372036854775808"), LONG_MIN);
   errorCount += MUST_EQ(errno, 0);
   IFDEBUG( printf("%4d %d, %ld\n", __LINE__, errno, atol("-9223372036854775808")); )

   errno= 0;
   errorCount += MUST_EQ(atol(" 0X8000000000000000"), LONG_MIN);
   errorCount += MUST_EQ(errno, 0);
   IFDEBUG( printf("%4d %d, %ld\n", __LINE__, errno, atol(" 0X8000000000000000")); )

   errno= 0; atol("-9223372036854775809");
   errorCount += MUST_EQ(errno, ERANGE);
   IFDEBUG( printf("%4d %d\n", __LINE__, errno); )

   errno= 0; atol(" 0X10000000000000000");
   errorCount += MUST_EQ(errno, ERANGE);
   IFDEBUG( printf("%4d %d\n", __LINE__, errno); )

static constexpr const char* const lazy=
       "The quick Brown fox jumps over the lazy dog.";
static constexpr const char* const good=
       "Now is the time for all GOOD men to come to the aid of their party.";
   errorCount += VERIFY( wildchar::strcmp("*", "anything") == 0);
   errorCount += VERIFY( wildchar::strcmp("*", "") == 0);
   errorCount += VERIFY( wildchar::strcmp("this", "this") == 0);
   errorCount += VERIFY( wildchar::strcmp("this", "that") != 0);
   errorCount += VERIFY( wildchar::strcmp("some*ing", "something") == 0);
   errorCount += VERIFY( wildchar::strcmp("s?me*ing", "someDing") == 0);
   errorCount += VERIFY( wildchar::strcmp("s?me*ing", "soMEDing") != 0);

   errorCount += VERIFY( wildchar::strcasecmp("*", "ANYTHING") == 0);
   errorCount += VERIFY( wildchar::strcasecmp("*", "") == 0);
   errorCount += VERIFY( wildchar::strcasecmp("ThIs", "tHiS") == 0);
   errorCount += VERIFY( wildchar::strcasecmp("this", "that") != 0);
   errorCount += VERIFY( wildchar::strcasecmp("some*ing", "something") == 0);
   errorCount += VERIFY( wildchar::strcasecmp("s?me*ing", "something") == 0);
   errorCount += VERIFY( wildchar::strcasecmp("s?me*ing", "soMEthing") == 0);

   errorCount += VERIFY( wildchar::strcmp("*Brown*dog?", lazy) == 0);
   errorCount += VERIFY( wildchar::strcmp("The*brown*LAZY*", lazy) != 0);
   errorCount += VERIFY( wildchar::strcmp("*dog.", lazy) == 0);
   errorCount += VERIFY( wildchar::strcmp("*DOG*", lazy) != 0);
   errorCount += VERIFY( wildchar::strcmp("The*", lazy) == 0);
   errorCount += VERIFY( wildchar::strcmp("Now*", lazy) != 0);
   errorCount += VERIFY( wildchar::strcmp("Now*", good) == 0);
   errorCount += VERIFY( wildchar::strcmp("Now is the time*to*party?", good) == 0);

   errorCount += VERIFY( wildchar::strcasecmp("*brOWN*dog?", lazy) == 0);
   errorCount += VERIFY( wildchar::strcasecmp("The*brown*LAZY*", lazy) == 0);
   errorCount += VERIFY( wildchar::strcasecmp("*dog.", lazy) == 0);
   errorCount += VERIFY( wildchar::strcasecmp("*DOG*", lazy) == 0);
   errorCount += VERIFY( wildchar::strcasecmp("THE*", lazy) == 0);
   errorCount += VERIFY( wildchar::strcasecmp("NOW*", lazy) != 0);
   errorCount += VERIFY( wildchar::strcasecmp("NOW*", good) == 0);
   errorCount += VERIFY( wildchar::strcasecmp("**NOW* is THE time*to **PARTY**", good) == 0);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Must
//
// Purpose-
//       Test case example, tests MUST functions.
//
//----------------------------------------------------------------------------
static inline int
   test_Must( void )                  // Test Case.h
{
   debugf("\ntest_Must\n");

   int                 errorCount= 0; // Number of errors encountered

   // This tests the MUST.H macros
   int one= 1;
   int two= 1;
   std::thread::id is_thread= std::this_thread::get_id();
   std::thread::id no_thread;

   errorCount += VERIFY(1 == 1);
   errorCount += VERIFY(1 == 2);
   errorCount += MUST_EQ(one, 1);
   errorCount += MUST_EQ(two, 2);
   errorCount += MUST_NOT(Sample error description);
   errorCount += MUST_EQ(is_thread, is_thread);
   errorCount += MUST_EQ((volatile std::thread::id&)no_thread, no_thread);
   errorCount += MUST_EQ((volatile std::thread::id&)is_thread, no_thread);
   errorCount += MUST_EQ(no_thread, is_thread);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Signals
//
// Purpose-
//       Test Signals.h
//
//----------------------------------------------------------------------------
static inline int
   test_Signals( void )             // Test Signals.h
{
   debugf("\ntest_Signals\n");

   int                 errorCount= 0; // Number of errors encountered

   using namespace pub::signals;
   struct Event {                   // Our event type
     float             X;           // X value
     float             Y;           // Y value

     Event(float X_, float Y_) : X(X_), Y(Y_) {}
   }; // struct Event

   static int          A_counter= 0; // Number of A clicks
   static int          B_counter= 0; // Number of B clicks

   using click_conn=   Connector<Event>;
   using click_event=  Event;
   using click_signal= Signal<Event>;

   pub::debugging::options::pub_hcdm= opt_hcdm;

   struct gui_element{
     click_signal clicked;
     void mouse_down(float X,float Y) { Event E(X,Y); clicked.signal(E); }

     gui_element(const char* name= nullptr) : clicked(name) {}
   };

   hcdmf(__LINE__); gui_element A("A"); // The "A" gui_element
   hcdmf(__LINE__); gui_element B("B"); // The "B" gui_element
   hcdmf(__LINE__); click_conn connection_1;
   hcdmf(__LINE__); click_conn connection_2;

   struct Slot_A {
     void operator()(click_event& E) {
       IFDEBUG( debugf("SA: A was counted for %.0f,%.0f\n", E.X, E.Y); )
       A_counter++;
     }
   };

   struct Slot_B {
     void operator()(click_event& E) {
       IFDEBUG( debugf("SB: B was counted for %.0f,%.0f\n", E.X, E.Y); )
       B_counter++;
     }
   };

   scdmf(__LINE__);
   connection_1= A.clicked.connect([](click_event& E) {
       IFDEBUG(  debugf("LA: A was counted for %.0f,%.0f\n", E.X, E.Y); )
       A_counter++;
   });
   scdmf(__LINE__);
   connection_2= B.clicked.connect([](click_event& E) {
       IFDEBUG(  debugf("LB: B was counted for %.0f,%.0f\n", E.X, E.Y); )
       B_counter++;
   });
// A.clicked.debug("A");
// B.clicked.debug("B");

   // A has one connection, B has one connection
   IFHCDM( hcdmf(__LINE__); A.clicked.debug("A");      )
   IFHCDM( hcdmf(__LINE__); B.clicked.debug("B");      )
   IFHCDM( hcdmf(__LINE__); connection_1.debug("c_1"); )
   IFHCDM( hcdmf(__LINE__); connection_2.debug("c_2"); )

   scdmf(__LINE__); A.mouse_down(__LINE__, -1);
   scdmf(__LINE__); B.mouse_down(-1, __LINE__);
   errorCount += MUST_EQ(A_counter, 1);
   errorCount += MUST_EQ(B_counter, 1);
   errorCount += VERIFY(B_counter == 1);

   {
     scdmf(__LINE__);
     auto temporary= A.clicked.connect([](click_event& E){
         IFDEBUG(  debugf("LT: A was counted for %.0f,%.0f\n", E.X, E.Y); )
         A_counter++;
     });
     IFHCDM( hcdmf(__LINE__); A.clicked.debug("A"); )
     // A has two connections, B has one connection
     scdmf(__LINE__); A.mouse_down(1, 0);
     scdmf(__LINE__); B.mouse_down(0, 1);
     errorCount += MUST_EQ(A_counter, 3);
     errorCount += MUST_EQ(B_counter, 2);
   }

   // temporary is out of scope: A has one connection, B has one connection
   // This overwrites the B connection_2 with the A connection_1,
   // leaving connection_1 empty and connection_2 with the A connection.
   scdmf(__LINE__);
   connection_2= std::move(connection_1);

   // A has one connection, B has no connections
   IFHCDM( hcdmf(__LINE__); A.clicked.debug("A"); )
   IFHCDM( hcdmf(__LINE__); B.clicked.debug("B"); )
   scdmf(__LINE__); A.mouse_down(2, 0);
   scdmf(__LINE__); B.mouse_down(0, 2);
   errorCount += MUST_EQ(A_counter, 4);
   errorCount += MUST_EQ(B_counter, 2);

   // Add a Slot_B connection to A.clicked.
   // A.mouse_down drives both l1 and SB connections
   scdmf(__LINE__);
   click_conn more= A.clicked.connect(Slot_B()); // Clicking A counts B!
   IFHCDM( hcdmf(__LINE__); A.clicked.debug("B"); )
   scdmf(__LINE__); A.mouse_down(3, 0); // Increments A_counter and B_counter
   errorCount += MUST_EQ(A_counter, 5);
   errorCount += MUST_EQ(B_counter, 3);

   // B.mouse_down does nothing.
   scdmf(__LINE__); B.mouse_down(0, 3);
   errorCount += MUST_EQ(A_counter, 5);
   errorCount += MUST_EQ(B_counter, 3);

   // This is a usage error.
   // The connection is created but not saved, so it's immediately deleted.
   // Thus it has no effect.
   B.clicked.connect(Slot_B());
   IFHCDM( hcdmf(__LINE__); B.clicked.debug("T"); )
   scdmf(__LINE__); A.mouse_down(4, 0); // Increments A_counter and B_counter
   errorCount += MUST_EQ(A_counter, 6);
   errorCount += MUST_EQ(B_counter, 4);

   // B.mouse_down still does nothing.
   scdmf(__LINE__); B.mouse_down(0, 4); // Might expect B_counter == 5
   errorCount += MUST_EQ(A_counter, 6);
   errorCount += MUST_EQ(B_counter, 4); // But connection does not exist

   //-------------------------------------------------------------------------
   // Test Signal::reset()
   scdmf(__LINE__); A.clicked.reset();
   scdmf(__LINE__); B.clicked.reset();

   scdmf(__LINE__); A.mouse_down(-5, 0);
   scdmf(__LINE__); B.mouse_down(0, -5);
   errorCount += MUST_EQ(A_counter, 6); // (Unchanged)
   errorCount += MUST_EQ(B_counter, 4); // (Unchanged)

   //-------------------------------------------------------------------------
   // Test Connection::reset()
   scdmf(__LINE__);
   connection_1= A.clicked.connect(Slot_A()); // Make connection
   connection_1.reset();            // Break connection

   scdmf(__LINE__); A.mouse_down(-6, 0);
   scdmf(__LINE__); B.mouse_down(0, -6);
   errorCount += MUST_EQ(A_counter, 6); // (Unchanged)
   errorCount += MUST_EQ(B_counter, 4); // (Unchanged)

   //-------------------------------------------------------------------------
   // Test multiple connections
   scdmf(__LINE__);
   A_counter= B_counter= 0;
   click_conn l_array[32];
   for(int i= 0; i<32; i++)
   {
     if( i & 1 )
       l_array[i]= B.clicked.connect(Slot_B());
     else
       l_array[i]= A.clicked.connect(Slot_A());
   }
   IFHCDM( hcdmf(__LINE__); A.clicked.debug("A"); )
   IFHCDM( hcdmf(__LINE__); B.clicked.debug("B"); )
   scdmf(__LINE__); A.mouse_down(16, 0);
   scdmf(__LINE__); B.mouse_down(0, 16);
   errorCount += MUST_EQ(A_counter, 16);
   errorCount += MUST_EQ(B_counter, 16);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Trace
//
// Purpose-
//       Test Trace.h
//
//----------------------------------------------------------------------------
static inline int
   test_Trace( void )               // Test Trace.h
{
using _PUB_NAMESPACE::Trace;

   debugf("\ntest_Trace\n");

   int                 errorCount= 0; // Number of errors encountered
   uint32_t            size;        // Working size

   // Test IFTRACE macro (Reqires: errorCount == 0)
   #ifdef TRACE
     errorCount++;
     IFTRACE(
       errorCount--;
       IFHCDM( debugf("%4d HCDM TRACE defined, IFTRACE active\n", __LINE__); )
     )
     if( errorCount )
       debugf("TRACE defined, but IFTRACE() inactive\n");
   #else
     IFHCDM( debugf("%4d HCDM TRACE undefined\n", __LINE__); )
     IFTRACE(
       errorCount++;
       debugf("TRACE undefined, but IFTRACE() active\n");
     )
   #endif

   // Test IFCHECK macro (Reqires: errorCount == 0)
   #ifdef CHECK
     errorCount++;
     IFCHECK(
       errorCount--;
       IFHCDM( debugf("%4d HCDM CHECK defined, IFCHECK active\n", __LINE__); )
     )
     if( errorCount )
       debugf("CHECK defined, but IFCHECK inactive\n");
   #else
     IFHCDM( debugf("%4d HCDM CHECK undefined\n", __LINE__); )
     IFCHECK(
       debugf("CHECK undefined but IFCHECK active\n");
       errorCount++;
     )
   #endif

   // Define our Trace::Record
   typedef pub::Trace::Record Record;
   Record* record= nullptr;         // Working Record*

   // Allocate the Trace table
   uint32_t table_size= 0x00020000; // Desired table space
   table_size += sizeof(pub::Trace); // Address trim allowance
   table_size += sizeof(pub::Trace); // For header
   table_size += 7;                 // Insure tail trim
   void*    table_addr= malloc(table_size);
   memset(table_addr, 'T', table_size);
   Trace* trace= Trace::make(table_addr, table_size);
   Trace::trace= trace;
   utility::dump(Debug::get()->get_FILE(), table_addr, table_size, table_addr);

   // Initialization tests
   if( sizeof(Trace) != trace->zero ) {
     errorCount++;
     debugf("%4d sizeof(Trace)(%zd) != trace->zero(%d)\n", __LINE__,
            sizeof(Trace), trace->zero);
   }

   //-------------------------------------------------------------------------
   // Test Trace methods, initializing the Trace storage
   tracef("\n");
   for(uint32_t i= 0; i<table_size+12; i++) // 32 wraps + extra
   {
       record= (Record*)Trace::storage_if(sizeof(Record));
       if( record ) {
         record->trace(".FOO", 254);
         uint32_t* offset= (uint32_t*)(record->value);
         *offset= htobe32((char*)offset - (char*)trace);
       }
   }
   trace->dump();                   // Look and see

   // This test is designed to only show interesting records.
   tracef("\nTest wrap clear\n");
   memset((char*)trace + sizeof(Trace), 0, trace->size-sizeof(Trace));
   size= trace->size - 512;
   trace->allocate(size);
   trace->dump();                   // Look and see
   tracef("\n");                    // Look and see (unformatted)
   utility::dump(Debug::get()->get_FILE(), table_addr, table_size, table_addr);

   //-------------------------------------------------------------------------
   // Size error tests
   size= trace->size - trace->zero;
   record= (Record*)trace->allocate(size);
   if( record == nullptr) {
     errorCount++;
     debugf("%4d Full length Record NOT allocated\n", __LINE__);
   }

   IFCHECK(
     record= (Record*)trace->allocate(0);
     if( record ) {
       errorCount++;
       debugf("%4d Zero length Record allocated\n", __LINE__);
     }

     record= (Record*)trace->allocate(size + 1);
     if( record ) {
       errorCount++;
       debugf("%4d Over-length Record allocated\n", __LINE__);
     }

     // Arithmetic overflow error requires an overly large table
     size= Trace::TABLE_SIZE_MAX;
     void* addr= malloc(size);
     if( addr == nullptr )
       debugf("%4d Unable to malloc(%d)\n", __LINE__, size);

     if( addr ) {                   // Check arithmetic overflow?
       Trace* table= Trace::make(addr, size); // We need a mondo table

       // Prepare to create an overflow condition
       void* record= table->allocate(table->size - 512);
       if( record == nullptr) {
         errorCount++;
         debugf("%4d Large Record NOT allocated\n", __LINE__);
       }

       record= table->allocate(4096); // Allocate, arithmetic overflow
       if( record ) {
         errorCount++;
         debugf("%4d Arithmetic overflow not detected\n", __LINE__);
         table->dump();
       }
       free(addr);
     }
   )

   //-------------------------------------------------------------------------
   // Deactivation error tests
   trace->deactivate();
   record= (Record*)Trace::storage_if(sizeof(Record));
   if( record ) {
     errorCount++;
     debugf("%4d Record allocated while trace inactive\n", __LINE__);
   }

   trace->flag[Trace::X_HALT]= 0;   // (Permitted)
   record= (Record*)Trace::storage_if(sizeof(Record));
   if( record == nullptr ) {
     errorCount++;
     debugf("%4d Unable to reactivate trace\n", __LINE__);
   }

   Trace::trace= nullptr;           // Disable global trace
   record= (Record*)Trace::storage_if(sizeof(Record));
   if( record ) {
     errorCount++;
     debugf("%4d Record allocated while Trace::trace == nullptr\n", __LINE__);
   }

   //-------------------------------------------------------------------------
   // Clean up and exit
   free(table_addr);
   if( errorCount == 0 )
     printf("Examine debug.out to verify proper operation\n");

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_UTF8
//
// Purpose-
//       Test UTF8.h
//
//----------------------------------------------------------------------------
static inline int
   test_UTF8( void )                // Test UTF8.h
{
   debugf("\ntest_UTF8\n");

   for(int code= 0; code < 0x00110000; code++)
   {
     if( code == 0x0000D800 ) {
       code= 0x0000DFFF;
       continue;
     }

     unsigned char buffer[32];      // Working buffer
     memset(buffer, 0xff, 8);
     UTF8::Encoder encoder(buffer, sizeof(buffer));
     encoder.encode(code);
     if( false )                    // (Visual verification)
       debugf("%4d HCDM %.6x %zu/%zu [%.2x,%.2x,%.2x,%.2x]\n", __LINE__, code
             , encoder.get_used(), encoder.get_size()
             , buffer[0], buffer[1], buffer[2], buffer[3]);

     unsigned char* addr= buffer + encoder.get_used();
     if( UTF8::inc(buffer) != addr ) {
       debugf("%4d HCDM Got(%p) Expected(%p)\n", __LINE__
             , UTF8::inc(buffer), addr);
       return 1;
     }
     if( UTF8::dec(addr) != buffer ) {
       debugf("%4d HCDM %p != %p [%.2x,%.2x,%.2x,%.2x,%.2x] %p\n", __LINE__
             , UTF8::dec(addr), buffer
             , buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], addr);
       return 1;
     }
     if( UTF8::dec(addr, encoder.get_used()) != buffer ) {
       debugf("%4d HCDM %p != %p [%.2x,%.2x,%.2x,%.2x,%.2x] %p\n", __LINE__
             , UTF8::dec(addr, encoder.get_used()), buffer
             , buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], addr);
       return 1;
     }
     if( code ) {
       if( UTF8::index(buffer, 1) != encoder.get_used() ) {
         debugf("%4d HCDM Got(%zd) Expected(%zd) [%.2x,%.2x,%.2x,%.2x]\n"
               , __LINE__ , UTF8::index(buffer,1), encoder.get_used()
               , buffer[0], buffer[1], buffer[2], buffer[3]);
         return 1;
       }
     } else {
       if( UTF8::index(buffer, 1) != 0 ) { // (Don't go past end of string)
         debugf("%4d HCDM Got(%zd) Expected(%zd) [%.2x,%.2x,%.2x,%.2x]\n"
               , __LINE__ , UTF8::index(buffer,1), encoder.get_used()
               , buffer[0], buffer[1], buffer[2], buffer[3]);
         return 1;
       }
     }

     UTF8::Decoder decoder(buffer, encoder.get_used());
     int test= decoder.decode();
     if( code != test ) {
       debugf("%4d HCDM %.6x [%.2x,%.2x,%.2x,%.2x] %.6x\n", __LINE__
             , code, buffer[0], buffer[1], buffer[2], buffer[3], test);
       return 1;
     }
     if( encoder.get_used() != decoder.get_used() ) {
       debugf("%4d HCDM SIZE: %.6x out(%zd) inp(%zd)\n", __LINE__
             , code, decoder.get_used(), encoder.get_used());
       return 1;
     }
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_UTF8_encode
//
// Purpose-
//       Test UTF8::encode (valuerange 00..10ffff)
//
//----------------------------------------------------------------------------
static inline int
   test_UTF8_encode(int, char**)    // Test UTF8::encode
//   int               argc,        // Argument count (unused)
//   char**            argv)        // Argument array (unused)
{
   debugf("\ntest_UTF8_encode(%s)\n", utf8_encode);

   unsigned char       buffer[32];    // The encode buffer
   memset(buffer, 0xff, sizeof(buffer));
   UTF8::Encoder       encoder(buffer, sizeof(buffer)); // Encoder

   int code= utility::atox(utf8_encode);
   encoder.encode(code);
   debugf("%.6x %zu/%zu [%.2x,%.2x,%.2x,%.2x]\n", code
         , encoder.get_used(), encoder.get_size()
         , buffer[0], buffer[1], buffer[2], buffer[3]);

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_UTF8_decode
//
// Purpose-
//       Test UTF8::decode (ONE encoded character)
//
//----------------------------------------------------------------------------
static inline int
   test_UTF8_decode(int, char**)    // Test UTF8::decode
//   int               argc,        // Argument count (unused)
//   char**            argv)        // Argument array (unused)
{
   debugf("\ntest_UTF8_decode(%s)\n", utf8_decode);

   char                buffer[1024];  // The encode buffer
   unsigned            size= 0;       // Number of bytes used
   int                 errorCount= 0; // Number of errors encountered

   // Fill the buffer
   const char* C= utf8_decode;
   while( size < sizeof(buffer) )
   {
     while( isspace(*C) ) C++;
     if( *C == '\0' )
       break;

     unsigned V= hexchar(*C);
     C++;
     if( *C != '\0' && !isspace(*C) )
     {
       V <<= 4;
       V  += hexchar(*C);
       C++;
     }

     buffer[size++]= V;
   }

   UTF8::Decoder decoder(buffer, size); // Decoder
   try {
     for(;;)
     {
       int code= decoder.decode();
       if( code < 0 )
         break;
       debugf("%.6x\n", code);
     }
     debugf("*DONE*\n");
   } catch( ... ) {
     errorCount++;
     debugf("catch(...)\n");
     throw;
   }

   return errorCount;
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
   unsigned errorCount= 0;
   debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

   try {
     parm(argc, argv);

     if( opt_case )    errorCount += test_Case();
     if( opt_dump )    errorCount += test_dump();
     if( opt_latch )   errorCount += test_Latch();
     if( opt_misc )    errorCount += test_Misc();
     if( opt_must )    errorCount += test_Must();
     if( opt_signals ) errorCount += test_Signals();
     if( opt_utf8 )    errorCount += test_UTF8();
     if( opt_trace )   errorCount += test_Trace();
     if( utf8_encode ) errorCount += test_UTF8_encode(argc, argv);
     if( utf8_decode ) errorCount += test_UTF8_decode(argc, argv);
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

   debugf("\n");
   if( errorCount == 0 )
     debugf("NO errors detected\n");
   else if( errorCount == 1 )
     debugf("1 error detected\n");
   else {
     debugf("%d errors detected\n", errorCount);
     errorCount= 1;
   }

   return errorCount;
}

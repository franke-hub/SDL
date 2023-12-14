//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
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
//       2023/12/14
//
//----------------------------------------------------------------------------
#include <cstdlib>                  // For std::free
#include <ctype.h>                  // For isprint()
#include <iostream>                 // For cout
#include <string>                   // For std::string
#include <thread>                   // For std::thread::id

#include <errno.h>                  // For errno, ...
#include <limits.h>                 // For INT_MIN, INT_MAX, ...
#include <stddef.h>                 // For offsetof

#include "pub/TEST.H"               // For error counting
#include "pub/Debug.h"              // For namespace pub::debugging
#include "pub/diag-pristine.h"      // For namespace pub::debugging
#include "pub/Exception.h"          // For pub::Exception
#include "pub/Latch.h"              // See test_Latch
#include "pub/Reporter.h"           // For pub::Reporter
#include "pub/Signals.h"            // See test_Signals
#include "pub/Statistic.h"          // For pub::Statistic
#include "pub/Thread.h"             // For pub::Thread
#include "pub/Trace.h"              // See test_Trace
#include "pub/utility.h"            // For pub::utility
#include "pub/Wrapper.h"            // For pub::Wrapper

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::dump;
using PUB::Wrapper;

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

#include "pub/ifmacro.h"            // Dependent macro

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_TEST= false; // (Only set if --all)
static int             opt_case= false; // (Only set if --all)
static int             opt_diag= false; // (Only set if --all)
static int             opt_dump= false; // --dump
static int             opt_latch= false; // --latch
static int             opt_misc= false; // --misc
static int             opt_reporter= true; // (Unconditionally true)
static int             opt_signals= false; // --signals
static int             opt_trace= false; // --trace

static struct option   opts[]=      // Options
{  {"all",     optional_argument, nullptr,         0}
,  {"dump",    no_argument,       &opt_dump,    true}
,  {"latch",   no_argument,       &opt_latch,   true}
,  {"misc",    no_argument,       &opt_misc,    true}
,  {"signals", no_argument,       &opt_signals, true}
,  {"trace",   no_argument,       &opt_trace,   true}
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Struct-
//       SampleRecord
//
// Purpose-
//       Sample record, for pub::Reporter test
//
//----------------------------------------------------------------------------
struct SampleRecord : public pub::Reporter::Record {
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
//       hcdml
//
// Purpose-
//       Hard Core Debug Mode: Print line number
//
//----------------------------------------------------------------------------
static void
   hcdml(int line)
{  if( opt_hcdm ) debugf("%4d Quick (HCDM)\n", line); }

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
//       test_diag
//
// Purpose-
//       Test diagnostic diag-pristine.h
//
//----------------------------------------------------------------------------
#pragma GCC diagnostic ignored "-Warray-bounds"
static void
   test_diag_error(void* V)         // Deliberately trash of before and after
{
   char* C= (char*)V;
   *(C -  1)= 0xff;
   *(C + 32)= 0xff;
}
#pragma GCC diagnostic pop

static inline int
   test_diag( void )
{
   if( opt_verbose )
     debugf("\ntest_diag\n");

   struct {
     Pristine before;
     char buffer[32]= {};
     Pristine after;
   } S;

   int error_count= 0;              // Error counter

   if( opt_verbose ) {
     void* V= S.buffer;
     test_diag_error(V);
     debugf("Two error messages expected...\n");
     Pristine::opt_hcdm= true;
   }

   return error_count;
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
   using PUB::utility::dump;

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
   using PUB::utility::dump;

   FILE* stdbug= Debug::get()->get_FILE();
   fprintf(stdbug, "\n%p[%.2x:%.4x]\n", buffer, origin, length);
   dump(buffer + origin, length);

   if( opt_verbose ) {
     fprintf(stdout, "\n%p[%.2x:%.4x]\n", buffer, origin, length);
     dump(stdout, buffer + origin, length);
   }
}


static inline int
   test_dump( void )                  // Test utility::dump
{
   if( opt_verbose )
     debugf("\ntest_dump (See: debug.out)");

   int                 error_count= 0; // Number of errors encountered

   using PUB::utility::dump;
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

   return error_count;
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
   if( opt_verbose )
     debugf("\ntest_Latch\n");

   int                 error_count= 0; // Number of errors encountered

   static constexpr uintptr_t
       HBIT= sizeof(uintptr_t) == 8 ? 0x8000000000000000L : 0x80000000;
   std::thread::id null_id= std::thread::id();
   std::thread::id tid;               // The current recursive.latch.load()

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("..Testing: Latch\n");
   Latch latch;

   latch.lock();
   latch.unlock();

   latch.try_lock();
   latch.unlock();

   try {                            // Test unlock when not held
     latch.unlock();
     error_count += MUST_NOT(Fail to throw an exception);
   } catch(std::runtime_error& X) {
     if( opt_verbose )
       debugf("....Expected: %s\n", X.what());
   }

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("..Testing: RecursiveLatch\n");
   RecursiveLatch recursive;
   tid= recursive.latch.load();
   error_count += MUST_EQ(tid, null_id);
   error_count += MUST_EQ(recursive.count, 0);

   {{{{ std::lock_guard<decltype(recursive)> lock1(recursive);
     error_count += MUST_EQ(recursive.count, 1);

     {{{{ std::lock_guard<decltype(recursive)> lock2(recursive);
       error_count += MUST_EQ(recursive.count, 2);
     }}}}

     error_count += MUST_EQ(recursive.count, 1);
   }}}}
   tid= recursive.latch.load();
   error_count += MUST_EQ(tid, null_id);
   error_count += MUST_EQ(recursive.count, 0);

   try {                            // Test unlock when not held
     recursive.unlock();
     error_count += MUST_NOT(Fail to throw an exception);
   } catch(std::runtime_error& X) {
     if( opt_verbose )
       debugf("....Expected: %s\n", X.what());
   }

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("..Testing: SHR_latch/XCL_latch\n");
   SHR_latch shr;
   XCL_latch xcl(shr);

   {{{{ std::lock_guard<decltype(shr)> lock1(shr);
     error_count += MUST_EQ(shr.count, 1);
//   if( xcl.try_lock() )             // (Deadlock if SHR+XCL on same thread)
//     error_count += MUST_NOT(Obtain exclusive while shared);

     {{{{ std::lock_guard<decltype(shr)> lock2(shr);
       error_count += MUST_EQ(shr.count, 2);
     }}}}
     error_count += MUST_EQ(shr.count, 1);
   }}}}
   error_count += MUST_EQ(shr.count, 0);

   if( xcl.try_lock() )
   {
     error_count += MUST_EQ(shr.count, HBIT);
     xcl.unlock();;
     error_count += MUST_EQ(shr.count, 0);
   } else {
     error_count += MUST_NOT(Fail to obtain exclusive latch);
   }

   {{{{ std::lock_guard<decltype(xcl)> lock(xcl);
     error_count += MUST_EQ(shr.count, HBIT);
   }}}}
   error_count += MUST_EQ(shr.count, 0);
   error_count += MUST_EQ(xcl.thread, null_id);

   // Test release share lock when not held
   try {
     shr.unlock();
     error_count += MUST_NOT(Fail to throw an exception);
   } catch(std::runtime_error& X) {
     if( opt_verbose )
       debugf("....Expected: %s\n", X.what());
   }
   error_count += MUST_EQ(shr.count, 0);

   // Test downgrade. (Note: upgrade not supported)
   xcl.lock();
   error_count += MUST_EQ(shr.count, HBIT);
   error_count += MUST_EQ(xcl.thread, std::this_thread::get_id());

   xcl.downgrade();
   error_count += MUST_EQ(shr.count, 1);
   error_count += MUST_EQ(xcl.thread, null_id);
   error_count += MUST_EQ(shr.count, 1);
   try {                            // Test downgrade when XCL not held
     xcl.downgrade();
     error_count += MUST_NOT(Fail to throw an exception);
   } catch(std::runtime_error& X) {
     if( opt_verbose )
       debugf("....Expected: %s\n", X.what());
   }
   error_count += MUST_EQ(shr.count, 1);
   shr.unlock();
   error_count += MUST_EQ(shr.count, 0);

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("..Testing: TestLatch\n");
   TestLatch testlatch;
   tid= recursive.latch.load();
   error_count += MUST_EQ(tid, null_id);

   {{{{ std::lock_guard<decltype(testlatch)> lock1(testlatch);
     try {
       {{{{ std::lock_guard<decltype(testlatch)> lock2(testlatch);
         error_count += MUST_NOT(Recursively hold TestLatch);
       }}}}
     } catch(std::runtime_error& X) {
       if( opt_verbose )
         debugf("....Expected: %s\n", X.what());
       tid= recursive.latch.load();
       error_count += MUST_EQ(tid, null_id);
     }
   }}}}

   //-------------------------------------------------------------------------
   if( opt_verbose )
     debugf("..Testing: NullLatch\n");
   NullLatch fake_latch;

   {{{{ std::lock_guard<decltype(fake_latch)> lock1(fake_latch);
        std::lock_guard<decltype(fake_latch)> lock2(fake_latch);
        std::lock_guard<decltype(fake_latch)> lock3(fake_latch);
        std::lock_guard<decltype(fake_latch)> lock4(fake_latch);
   }}}}

   return error_count;
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
   if( opt_verbose )
     debugf("\ntest_Misc\n");

   int                 error_count= 0; // Number of errors encountered

   // Test utility.h (utility::dump tested separately)
   using namespace PUB::utility;
   using PUB::utility::atoi;
   using PUB::utility::atol;
   using PUB::utility::atox;

   errno= 0;                          // No error

   error_count += MUST_EQ(atoi("1234567890"), 1234567890);
   error_count += MUST_EQ(atol("123456789012345"), 123456789012345L);
   error_count += MUST_EQ(atox("12abcdefABCDEF"), 0x12abcdefABCDEF);
   error_count += MUST_EQ(atol("0x1234567890"), 0x1234567890L);
   error_count += MUST_EQ(atoi("  1234567890  "), 1234567890);

   error_count += MUST_EQ(strcmp(skip_space("  abcd  "), "abcd  "), 0);
   error_count += MUST_EQ(strcmp(find_space("abcd  efgh"), "  efgh"), 0);

   error_count += MUST_EQ(*skip_space("  "), '\0');
   error_count += MUST_EQ(*find_space("abcdefgh"), '\0');

   error_count += MUST_EQ(errno, 0);

   errno= 0; atoi("");
   error_count += MUST_EQ(errno, EINVAL);
   errno= 0; atoi("0x");
   error_count += MUST_EQ(errno, EINVAL);
   errno= 0; atoi("0x0100000000");
   error_count += MUST_EQ(errno, ERANGE);

   errno= 0;
   error_count += MUST_EQ(atoi(" 2147483647"),  2147483647);
   error_count += MUST_EQ(errno, 0);

   errno= 0;
   error_count += MUST_EQ(atoi("+2147483647"), +2147483647);
   error_count += MUST_EQ(errno, 0);

   errno= 0; atoi("2147483648");
   error_count += MUST_EQ(errno, ERANGE);

   errno= 0;
   error_count += MUST_EQ(atoi("-2147483648"), INT_MIN);
   error_count += MUST_EQ(errno, 0);
   if( opt_hcdm )
     printf("%4d %d %d\n", __LINE__, errno, atoi("-2147483648"));

   errno= 0;
   error_count += MUST_EQ(atoi(" 0x80000000"), INT_MIN);
   error_count += MUST_EQ(errno, 0);
   if( opt_hcdm )
     printf("%4d %d %d %x\n", __LINE__, errno
           , atoi(" 0x80000000"), atoi(" 0x80000000"));

   errno= 0; atoi("-2147483649");
   error_count += MUST_EQ(errno, ERANGE);
   if( opt_hcdm )
     printf("%4d %d\n", __LINE__, errno);

   errno= 0;
   error_count += MUST_EQ(atol(" 9223372036854775807"), 9223372036854775807L);
   error_count += MUST_EQ(errno, 0);
   if( opt_hcdm )
     printf("%4d %d, %ld\n", __LINE__, errno, atol(" 9223372036854775807"));

   errno= 0;
   error_count += MUST_EQ(atol("+9223372036854775807"), +9223372036854775807L);
   error_count += MUST_EQ(errno, 0);
   if( opt_hcdm )
     printf("%4d %d, %ld\n", __LINE__, errno, atol("+9223372036854775807"));

   errno= 0; atol("9223372036854775808");
   error_count += MUST_EQ(errno, ERANGE);
   if( opt_hcdm )
     printf("%4d %d\n", __LINE__, errno);

   errno= 0;
   error_count += MUST_EQ(atol("-9223372036854775808"), LONG_MIN);
   error_count += MUST_EQ(errno, 0);
   if( opt_hcdm )
     printf("%4d %d, %ld\n", __LINE__, errno, atol("-9223372036854775808"));

   errno= 0;
   error_count += MUST_EQ(atol(" 0X8000000000000000"), LONG_MIN);
   error_count += MUST_EQ(errno, 0);
   if( opt_hcdm )
     printf("%4d %d, %ld\n", __LINE__, errno, atol(" 0X8000000000000000"));

   errno= 0; atol("-9223372036854775809");
   error_count += MUST_EQ(errno, ERANGE);
   if( opt_hcdm )
     printf("%4d %d\n", __LINE__, errno);

   errno= 0; atol(" 0X10000000000000000");
   error_count += MUST_EQ(errno, ERANGE);
   if( opt_hcdm )
     printf("%4d %d\n", __LINE__, errno);

   std::thread::id tid= std::this_thread::get_id();
   error_count += VERIFY(utility::to_string(tid)==Thread::get_id_string(tid));
   if( opt_verbose )
     printf("std::thread::id(%s)\n", Thread::get_id_string(tid).c_str());

static constexpr const char* const lazy=
       "The quick Brown fox jumps over the lazy dog.";
static constexpr const char* const good=
       "Now is the time for all GOOD men to come to the aid of their party.";
   error_count += VERIFY( wildchar::strcmp("*", "anything") == 0);
   error_count += VERIFY( wildchar::strcmp("*", "") == 0);
   error_count += VERIFY( wildchar::strcmp("this", "this") == 0);
   error_count += VERIFY( wildchar::strcmp("this", "that") != 0);
   error_count += VERIFY( wildchar::strcmp("some*ing", "something") == 0);
   error_count += VERIFY( wildchar::strcmp("s?me*ing", "someDing") == 0);
   error_count += VERIFY( wildchar::strcmp("s?me*ing", "soMEDing") != 0);

   error_count += VERIFY( wildchar::strcasecmp("*", "ANYTHING") == 0);
   error_count += VERIFY( wildchar::strcasecmp("*", "") == 0);
   error_count += VERIFY( wildchar::strcasecmp("ThIs", "tHiS") == 0);
   error_count += VERIFY( wildchar::strcasecmp("this", "that") != 0);
   error_count += VERIFY( wildchar::strcasecmp("some*ing", "something") == 0);
   error_count += VERIFY( wildchar::strcasecmp("s?me*ing", "something") == 0);
   error_count += VERIFY( wildchar::strcasecmp("s?me*ing", "soMEthing") == 0);

   error_count += VERIFY( wildchar::strcmp("*Brown*dog?", lazy) == 0);
   error_count += VERIFY( wildchar::strcmp("The*brown*LAZY*", lazy) != 0);
   error_count += VERIFY( wildchar::strcmp("*dog.", lazy) == 0);
   error_count += VERIFY( wildchar::strcmp("*DOG*", lazy) != 0);
   error_count += VERIFY( wildchar::strcmp("The*", lazy) == 0);
   error_count += VERIFY( wildchar::strcmp("Now*", lazy) != 0);
   error_count += VERIFY( wildchar::strcmp("Now*", good) == 0);
   error_count += VERIFY( wildchar::strcmp("Now is the time*to*party?", good) == 0);

   error_count += VERIFY( wildchar::strcasecmp("*brOWN*dog?", lazy) == 0);
   error_count += VERIFY( wildchar::strcasecmp("The*brown*LAZY*", lazy) == 0);
   error_count += VERIFY( wildchar::strcasecmp("*dog.", lazy) == 0);
   error_count += VERIFY( wildchar::strcasecmp("*DOG*", lazy) == 0);
   error_count += VERIFY( wildchar::strcasecmp("THE*", lazy) == 0);
   error_count += VERIFY( wildchar::strcasecmp("NOW*", lazy) != 0);
   error_count += VERIFY( wildchar::strcasecmp("NOW*", good) == 0);
   error_count += VERIFY( wildchar::strcasecmp("**NOW* is THE time*to **PARTY**", good) == 0);

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Reporter
//
// Purpose-
//       Test ~/src/cpp/inc/pub/Reporter.h
//
//----------------------------------------------------------------------------
static inline int
   test_Reporter( void )
{
   typedef pub::Reporter    Reporter;
   typedef Reporter::Record Record;

   if( opt_verbose )
     debugf("\ntest_Reporter:\n");

   int error_count= 0;              // Error counter

   Reporter reporter;
   Reporter::set(&reporter);
   SampleRecord one("one");
   SampleRecord two("two");

   reporter.insert(&one);
   Reporter::get()->insert(&two);   // Using the global Reporter

   // Do something that updates one.stat and two.stat
   one.stat.inc(); one.stat.inc(); one.stat.inc(); one.stat.dec();
   two.stat.inc(); two.stat.inc();

   // Verify the report (Requires opt_verbose)
   if( opt_verbose ) {
     reporter.report([](Record& record) {
       std::cout << record.h_report() << "\n";
     }); // reporter.report

     std::cout << "\nRESET\n";
     Reporter::get()->reset();
     reporter.report([](Record& record) {
       std::cout << record.h_report() << "\n";
     }); // reporter.report

     std::cout << "\nREMOVE\n";
     Reporter::get()->remove(&two); // Using the global Reporter
     reporter.report([](Record& record) {
       std::cout << record.h_report() << "\n";
     }); // reporter.report
   }

   return error_count;
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
   if( opt_verbose )
     debugf("\ntest_Signals\n");

   int                 error_count= 0; // Number of errors encountered

   using namespace PUB::signals;
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

   PUB::debugging::options::pub_hcdm= opt_hcdm;

   struct gui_element{
     click_signal clicked;
     void mouse_down(float X,float Y) { Event E(X,Y); clicked.signal(E); }

     gui_element(const char* name= nullptr) : clicked(name) {}
   };

   hcdml(__LINE__); gui_element A("A"); // The "A" gui_element
   hcdml(__LINE__); gui_element B("B"); // The "B" gui_element
   hcdml(__LINE__); click_conn connection_1;
   hcdml(__LINE__); click_conn connection_2;

   struct Slot_A {
     void operator()(click_event& E) {
       A_counter++;
       if( opt_verbose )
         debugf("SA: A was counted for %.0f,%.0f\n", E.X, E.Y);
     }
   };

   struct Slot_B {
     void operator()(click_event& E) {
       B_counter++;
       if( opt_verbose )
         debugf("SB: B was counted for %.0f,%.0f\n", E.X, E.Y);
     }
   };

   hcdml(__LINE__);
   connection_1= A.clicked.connect([](click_event& E) {
       A_counter++;
       if( opt_verbose )
         debugf("LA: A was counted for %.0f,%.0f\n", E.X, E.Y);
   });
   hcdml(__LINE__);
   connection_2= B.clicked.connect([](click_event& E) {
       B_counter++;
       if( opt_verbose )
         debugf("LB: B was counted for %.0f,%.0f\n", E.X, E.Y);
   });
// A.clicked.debug("A");
// B.clicked.debug("B");

   // A has one connection, B has one connection
   IFHCDM( hcdml(__LINE__); A.clicked.debug("A");      )
   IFHCDM( hcdml(__LINE__); B.clicked.debug("B");      )
   IFHCDM( hcdml(__LINE__); connection_1.debug("c_1"); )
   IFHCDM( hcdml(__LINE__); connection_2.debug("c_2"); )

   hcdml(__LINE__); A.mouse_down(__LINE__, -1);
   hcdml(__LINE__); B.mouse_down(-1, __LINE__);
   error_count += MUST_EQ(A_counter, 1);
   error_count += MUST_EQ(B_counter, 1);
   error_count += VERIFY(B_counter == 1);

   {
     hcdml(__LINE__);
     auto temporary= A.clicked.connect([](click_event& E)
     {
       A_counter++;
       if( opt_verbose )
         debugf("LT: A was counted for %.0f,%.0f\n", E.X, E.Y);
     });
     IFHCDM( hcdml(__LINE__); A.clicked.debug("A"); )
     // A has two connections, B has one connection
     hcdml(__LINE__); A.mouse_down(1, 0);
     hcdml(__LINE__); B.mouse_down(0, 1);
     error_count += MUST_EQ(A_counter, 3);
     error_count += MUST_EQ(B_counter, 2);
   }

   // temporary is out of scope: A has one connection, B has one connection
   // This overwrites the B connection_2 with the A connection_1,
   // leaving connection_1 empty and connection_2 with the A connection.
   hcdml(__LINE__);
   connection_2= std::move(connection_1);

   // A has one connection, B has no connections
   IFHCDM( hcdml(__LINE__); A.clicked.debug("A"); )
   IFHCDM( hcdml(__LINE__); B.clicked.debug("B"); )
   hcdml(__LINE__); A.mouse_down(2, 0);
   hcdml(__LINE__); B.mouse_down(0, 2);
   error_count += MUST_EQ(A_counter, 4);
   error_count += MUST_EQ(B_counter, 2);

   // Add a Slot_B connection to A.clicked.
   // A.mouse_down drives both l1 and SB connections
   hcdml(__LINE__);
   click_conn more= A.clicked.connect(Slot_B()); // Clicking A counts B!
   IFHCDM( hcdml(__LINE__); A.clicked.debug("B"); )
   hcdml(__LINE__); A.mouse_down(3, 0); // Increments A_counter and B_counter
   error_count += MUST_EQ(A_counter, 5);
   error_count += MUST_EQ(B_counter, 3);

   // B.mouse_down does nothing.
   hcdml(__LINE__); B.mouse_down(0, 3);
   error_count += MUST_EQ(A_counter, 5);
   error_count += MUST_EQ(B_counter, 3);

   // This is a usage error.
   // The connection is created but not saved, so it's immediately deleted.
   // Thus it has no effect.
   B.clicked.connect(Slot_B());
   IFHCDM( hcdml(__LINE__); B.clicked.debug("T"); )
   hcdml(__LINE__); A.mouse_down(4, 0); // Increments A_counter and B_counter
   error_count += MUST_EQ(A_counter, 6);
   error_count += MUST_EQ(B_counter, 4);

   // B.mouse_down still does nothing.
   hcdml(__LINE__); B.mouse_down(0, 4); // Might expect B_counter == 5
   error_count += MUST_EQ(A_counter, 6);
   error_count += MUST_EQ(B_counter, 4); // But connection does not exist

   //-------------------------------------------------------------------------
   // Test Signal::reset()
   hcdml(__LINE__); A.clicked.reset();
   hcdml(__LINE__); B.clicked.reset();

   hcdml(__LINE__); A.mouse_down(-5, 0);
   hcdml(__LINE__); B.mouse_down(0, -5);
   error_count += MUST_EQ(A_counter, 6); // (Unchanged)
   error_count += MUST_EQ(B_counter, 4); // (Unchanged)

   //-------------------------------------------------------------------------
   // Test Connection::reset()
   hcdml(__LINE__);
   connection_1= A.clicked.connect(Slot_A()); // Make connection
   connection_1.reset();            // Break connection

   hcdml(__LINE__); A.mouse_down(-6, 0);
   hcdml(__LINE__); B.mouse_down(0, -6);
   error_count += MUST_EQ(A_counter, 6); // (Unchanged)
   error_count += MUST_EQ(B_counter, 4); // (Unchanged)

   //-------------------------------------------------------------------------
   // Test multiple connections
   hcdml(__LINE__);
   A_counter= B_counter= 0;
   click_conn l_array[32];
   for(int i= 0; i<32; i++)
   {
     if( i & 1 )
       l_array[i]= B.clicked.connect(Slot_B());
     else
       l_array[i]= A.clicked.connect(Slot_A());
   }
   IFHCDM( hcdml(__LINE__); A.clicked.debug("A"); )
   IFHCDM( hcdml(__LINE__); B.clicked.debug("B"); )
   hcdml(__LINE__); A.mouse_down(16, 0);
   hcdml(__LINE__); B.mouse_down(0, 16);
   error_count += MUST_EQ(A_counter, 16);
   error_count += MUST_EQ(B_counter, 16);

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_TEST
//
// Purpose-
//       Test case example, tests TEST.H functions.
//
//----------------------------------------------------------------------------
static inline int
   test_TEST( void )                  // Test TEST.H
{
   debugf("\ntest_TEST\n");

   int                 error_count= 0; // Number of errors encountered

   // This tests the TEST.H macros, including error cases
   int one= 1;
   int two= 1;
   std::thread::id is_thread= std::this_thread::get_id();
   std::thread::id no_thread;

   error_count += VERIFY(1 == 1);
   error_count += VERIFY(1 == 2);
                 debugf("%4d: Error expected\n", __LINE__ - 1);
   error_count += MUST_EQ(one, 1);
   error_count += MUST_EQ(two, 2);
                 debugf("%4d: Error expected\n", __LINE__ - 1);
   error_count += MUST_NOT(Sample error description);
                 debugf("%4d: Error expected\n", __LINE__ - 1);
   error_count += MUST_EQ(is_thread, is_thread);
   error_count += MUST_EQ((std::thread::id&)no_thread, no_thread);
   error_count += MUST_EQ((std::thread::id&)is_thread, no_thread);
                 debugf("%4d: Error expected\n", __LINE__ - 1);
   error_count += MUST_EQ(no_thread, is_thread);
                 debugf("%4d: Error expected\n", __LINE__ - 1);

   return error_count != 5;
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
//       test_Trace
//
// Purpose-
//       Test Trace.h
//
//----------------------------------------------------------------------------
static inline int
   test_Trace( void )               // Test Trace.h
{
   using _LIBPUB_NAMESPACE::Trace;

   debugf("\ntest_Trace\n");

   int                 error_count= 0; // Number of errors encountered
   uint32_t            size;        // Working size

   // Test IFTRACE macro (Reqires: error_count == 0)
   #ifdef TRACE
     error_count++;
     IFTRACE(
       error_count--;
       IFHCDM( debugf("%4d HCDM TRACE defined, IFTRACE active\n", __LINE__); )
     )
     if( error_count )
       debugf("TRACE defined, but IFTRACE() inactive\n");
   #else
     IFHCDM( debugf("%4d HCDM TRACE undefined\n", __LINE__); )
     IFTRACE(
       error_count++;
       debugf("TRACE undefined, but IFTRACE() active\n");
     )
   #endif

   // Test IFCHECK macro (Reqires: error_count == 0)
   #ifdef CHECK
     error_count++;
     IFCHECK(
       error_count--;
       IFHCDM( debugf("%4d HCDM CHECK defined, IFCHECK active\n", __LINE__); )
     )
     if( error_count )
       debugf("CHECK defined, but IFCHECK inactive\n");
   #else
     IFHCDM( debugf("%4d HCDM CHECK undefined\n", __LINE__); )
     IFCHECK(
       debugf("CHECK undefined but IFCHECK active\n");
       error_count++;
     )
   #endif

   // Define our Trace::Record
   typedef PUB::Trace::Record Record;
   Record* record= nullptr;         // Working Record*

   // Allocate the Trace table
   uint32_t table_size= 0x00020000; // Desired table space
   table_size += sizeof(PUB::Trace); // Address trim allowance
   table_size += sizeof(PUB::Trace); // For header
   table_size += 7;                 // Insure tail trim
   void*    table_addr= malloc(table_size);
   memset(table_addr, 'T', table_size);
   Trace* trace= Trace::make(table_addr, table_size);
   Trace::table= trace;
   utility::dump(Debug::get()->get_FILE(), table_addr, table_size, table_addr);

   // Initialization tests
   if( sizeof(Trace) != trace->zero ) {
     error_count++;
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
     error_count++;
     debugf("%4d Full length Record NOT allocated\n", __LINE__);
   }

   IFCHECK(
     record= (Record*)trace->allocate(0);
     if( record ) {
       error_count++;
       debugf("%4d Zero length Record allocated\n", __LINE__);
     }

     record= (Record*)trace->allocate(size + 1);
     if( record ) {
       error_count++;
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
         error_count++;
         debugf("%4d Large Record NOT allocated\n", __LINE__);
       }

       record= table->allocate(4096); // Allocate, arithmetic overflow
       if( record ) {
         error_count++;
         debugf("%4d Arithmetic overflow not detected\n", __LINE__);
         memset(record, 'R', 4096);
         table->dump();
       }
       free(addr);
     }
   )

   //-------------------------------------------------------------------------
   // Check all Trace::trace methods
   memset(table_addr, 'T', table_size); // Refresh the trace table
   trace= Trace::make(table_addr, table_size);
   Trace::table= trace;

   Trace::Record* R= Trace::trace(32);
   strcpy((char*)R, "Ain't this a dandy little trace record?");

   // Note: The unit field isn't set, so the 'rd?\0' from "record?" remains.
   Trace::trace(".CPU");            // (First byte reserved for CPU ID)

   Trace::trace(".ONE", 0xC0DEC0DE);
   Trace::trace(".TWO", "Code");
   Trace::trace("INFO", 0x0732, "This is trace info"); // (Truncated)
   Trace::trace("UNIT", "unit");
   Trace::trace(".one", ".W01", (void*)0x76543210'ffff0000L);
   Trace::trace(".two", ".W02", (void*)0x76543211'ffff0001L
                              , (void*)0x76543212'ffff0002L);
   Trace::trace("MORE", "more", (void*)0x7654321A'ffff000AL
                              , (void*)0x7654321B'ffff000BL
                              , (void*)0x7654321C'ffff000CL
                              , (void*)0x7654321D'ffff000DL
                              , (void*)0x7654321E'ffff000EL
                              , (void*)0x7654321F'ffff000FL);
   trace->dump();

   //-------------------------------------------------------------------------
   // Deactivation error tests
   trace->deactivate();
   record= (Record*)Trace::storage_if(sizeof(Record));
   if( record ) {
     error_count++;
     debugf("%4d Record allocated while trace inactive\n", __LINE__);
   }

   trace->flag[Trace::X_HALT]= 0;   // (Permitted)
   record= (Record*)Trace::storage_if(sizeof(Record));
   if( record == nullptr ) {
     error_count++;
     debugf("%4d Unable to reactivate trace\n", __LINE__);
   }

   Trace::table= nullptr;           // Disable global trace
   record= (Record*)Trace::storage_if(sizeof(Record));
   if( record ) {
     error_count++;
     debugf("%4d Record allocated while Trace::table == nullptr\n", __LINE__);
   }

   //-------------------------------------------------------------------------
   // Clean up and exit
   free(table_addr);
   if( error_count == 0 )
     printf("Examine debug.out to verify proper operation\n");

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
                     "  --dump\tutility.h dump() test\n"
                     "  --latch\tLatch.h regression test\n"
                     "  --signals\tsignals::Signal.h regression test\n"
                     "  --trace\tTrace.h debug.out test\n"
            );
   });

   tc.on_init([](int argc, char* argv[]) // (Unused in this sample)
   { (void)argc; (void)argv;        // (Unused arguments)
     #ifdef HCDM
       opt_hcdm= true;
     #endif

     return 0;
   });

   tc.on_parm([](std::string name, const char* value)
   {
     if( opt_hcdm )
       debugf("on_parm(%s,%s)\n", name.c_str(), value);

     if( name == "all" ) {          // Note: specify --hcdm *BEFORE* --all
       if( opt_hcdm ) {
         opt_TEST= true;            // (Only set here)
         opt_case= true;            // (Only set here)
         opt_diag= true;            // (Only set here)
       }
       // opt_dump= true;           // Select separately (needs validation)
       opt_diag= true;
       opt_latch= true;
       opt_misc= true;
       opt_signals= true;
       // opt_trace= true;          // Select separately (needs validation)
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

     if( opt_TEST )    error_count += test_TEST();
     if( opt_case )    error_count += test_case();
     if( opt_diag )    error_count += test_diag();
     if( opt_dump )    error_count += test_dump();
     if( opt_latch )   error_count += test_Latch();
     if( opt_misc )    error_count += test_Misc();
     if( opt_reporter) error_count += test_Reporter();
     if( opt_signals ) error_count += test_Signals();
     if( opt_trace )   error_count += test_Trace();
//   if( true )        error_count += test_dirty(); // Optional bringup test

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

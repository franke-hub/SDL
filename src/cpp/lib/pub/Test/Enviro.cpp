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
//       Enviro.cpp
//
// Purpose-
//       Display environmental control variables.
//
// Last change date-
//       2022/05/06
//
//----------------------------------------------------------------------------
#define _FILE_OFFSET_BITS 64        // (Required for LINUX)
// #define _LARGEFILE_SOURCE 1
// #define _LARGEFILE64_SOURCE 1

#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include <exception>
#include <string>
using namespace std;

#include "pub/TEST.H"               // For VERIFY, ...
#include "pub/Clock.h"              // For pub::Clock
#include "pub/Debug.h"              // For namespace pub::debugging
#include "pub/Event.h"              // For pub::Event
#include "pub/Exception.h"          // For pub::Exception, std::exception
#include "pub/Interval.h"           // For pub::Interval
#include "pub/Thread.h"             // For pub::Thread
#include "pub/Wrapper.h"            // For pub::Wrapper

#ifdef _OS_WIN
  #include <Windows.h>
  #if( _MSC_VER > 1200 )
    #define lstat64 _stat64         // No links in _OS_WIN
    #define stat64  _stat64         // stat64 == _stat64
  #else
    #define lstat64 _stat           // No links in _OS_WIN
    #define stat64  _stat           // stat64 == _stat
  #endif
#endif

#ifdef _OS_CYGWIN
#define stat64 stat                 // stat == stat64 in CYGWIN
#endif

#undef _ADDR64
#if defined(_WIN64) || defined(__x86_64__)
  #define _ADDR64
#elif !defined(_CC_MSC) && !defined(_CC_GCC)
  #error "_ADDR64 indeterminate"
#endif

using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;
using pub::Wrapper;                 // For pub::Wrapper class

#define opt_verbose    pub::Wrapper::opt_verbose

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#include "pub/ifmacro.h"            // Verify multiple inclusion

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define concat(x) "" #x ""
#define catcon(x) concat(x)

#define MACROF(x) \
   debugf ("%8.5s %s(%s)\n",  "", #x, catcon(x))

//----------------------------------------------------------------------------
// Internal data areas. Counters external to confuse compiler
//----------------------------------------------------------------------------
volatile unsigned int  int_value= 0;
volatile unsigned long long_value= 0;
volatile double        dbl_value= 0.0;

//----------------------------------------------------------------------------
//
// Class-
//       MyException
//
// Purpose-
//       Extend std::exception with message
//
//----------------------------------------------------------------------------
class MyException : public std::runtime_error {
   using std::runtime_error::runtime_error;
}; // class MyException

//----------------------------------------------------------------------------
//
// Class-
//       Timer
//
// Purpose-
//       Background Thread that sets and clears `running`
//
//----------------------------------------------------------------------------
class Timer : public pub::Thread {
public:
pub::Event             event;       // Test start event
double                 test_time;   // Test run time
static volatile int    running;     // Test running

virtual
~Timer( void ) = default;
Timer(double t) : Thread(), test_time(t)
{  }

virtual void
run( void )
{
   running= true;
   event.post();
   Thread::sleep(test_time);
   running= false;
   event.reset();
}
}; // class Timer
volatile int           Timer::running=false;

//----------------------------------------------------------------------------

//
// Subroutine-
//       torf
//
// Purpose-
//       Returns " TRUE" or "FALSE" string
//
//----------------------------------------------------------------------------
static inline const char*           // " TRUE" or "FALSE"
   torf(                            // True or False test
     unsigned int    const variable)// The test variable
{
   if (variable)                    // If variable is true
     return (" TRUE");              // Indicate true

   else                             // If variable is false
     return ("FALSE");              // Indicate false
}

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

   if( opt_verbose ) {
     debugf("%.6g Minimum interval\n", delta);
     debugf("%d Interval count\n", count);
   }

   prior= delta;
   while( delta < TIMEOUT )
   {
     if( opt_verbose ) { traceh("%.6f\r", delta); }
     prior= delta;
     delta= stopper();
   }
   if( opt_verbose ) {
     traceh("\n");
     debugf("%.6f Seconds (%f)\n", delta, TIMEOUT);
     debugf("%.6f Prior\n", prior);
     debugf("%.6g Diff\n", delta - prior);
   }
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
   int                 error_count= 0; // Number of errors encountered

   debugf("\ntest_Clock\n");

   _PUB_NAMESPACE::Clock clock;
   double start= clock.now();

   interval_test([clock, start]() mutable {start= clock.now(); },
                 [clock, start]() {return clock.now() - start; });

   debugf("%14.3f Clock::now()\n", _PUB_NAMESPACE::Clock::now());
   debugh("Debug::now()\n");

   _PUB_NAMESPACE::Clock one;
   _PUB_NAMESPACE::Clock two;
   _PUB_NAMESPACE::Clock wow;
   two= 3.0;

   wow= one + two;
   error_count += VERIFY( wow == one + two );
   error_count += VERIFY( (double)wow == (double)one + (double)two );

   wow= one - two;
   error_count += VERIFY( wow == one - two );
   error_count += VERIFY( (double)wow == (double)one - (double)two );

   return error_count;
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
   int                 error_count= 0; // Number of errors encountered

   debugf("\ntest_Interval\n");

   _PUB_NAMESPACE::Interval interval;

   interval_test([interval]() mutable {interval.start(); },
                 [interval]() mutable {return interval.stop(); });

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Performance
//
// Purpose-
//       Timing tests, for machine to machine comparisons.
//
// Implementation notes-
//       Measurements within 1% should be considered identical.
//
//----------------------------------------------------------------------------
static inline int                   // (Always 0)
   test_Performance( void )         // Performance
{
   debugf("\ntest_Performance, for machine-to-machine comparison\n");

   Timer timer= 1.0;                // Test timer

   timer.start();
   timer.event.wait();
   while( timer.running )
     int_value++;
   timer.join();

   timer.start();
   timer.event.wait();
   while( timer.running )
     long_value++;
   timer.join();

   timer.start();
   timer.event.wait();
   while( timer.running )
     dbl_value += 1.0;
   timer.join();

   // Tests complete
   debugf("%'16d ints/second\n", int_value);
   debugf("%'16ld longs/second\n", long_value);
   debugf("%'16.0f doubles/second\n", dbl_value);

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       demo_std_exception
//
// Purpose-
//       Demonstrate std::exception usage error
//
// Elaboration-
//       The catch statement should read: catch(exception& x)
//       Since it does not, the exception is COPIED into a std::exception
//       and that exception is the BASE std::exception
//
//       GCC (now) checks for this usage error.
//
//----------------------------------------------------------------------------
static inline int
   demo_std_exception( void )
{
#ifdef _CC_GCC                      // (This DEMOs the usage error)
   #pragma GCC diagnostic ignored "-Wcatch-value"
#endif

   debugf("\n");
   try {
     MyException up("oops");
     throw up;
// } catch(exception& x) {          // Properly coded
   } catch(exception x) {           // Improperly coded catch-value
     if( strcmp(x.what(), "oops") != 0 ) {
       debugf("WHAT(%s) HAPPENED?\n", x.what());
       debugf("WHAT(%s) HAPPENED? was expected\n", "oops");
     }
   }

   debugf("demo_std_exception: demonstrates usage error\n");
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_std_exception
//
// Purpose-
//       Test std::exception
//
//----------------------------------------------------------------------------
static inline int
   test_std_exception( void )       // Test std::exception
{
   debugf("test_std_exception: verifies proper usage\n");

   try {
     MyException up("oops");
     throw up;
   } catch(exception& x) {
     if( strcmp(x.what(), "oops") != 0 )
     {
       debugf("WHAT(%s) HAPPENED?\n", x.what());
       throw "Should Not Occur";
     }
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_stdlib
//
// Purpose-
//       Test cstdlib, stdlib.h
//
// Implementation notes-
//       Expect exception or SEGFAULT if failure
//
//----------------------------------------------------------------------------
static inline int
   test_stdlib( void )              // Test cstdlib, stdlib.h (free)
{
// debugf("test_stdlib, fault if error\n"); // (Silent test)

   ::free(NULL);                    // Test stdlib.h
   std::free(nullptr);              // Test cstdlib

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       environment
//
// Purpose-
//       Verify compilation controls
//
//----------------------------------------------------------------------------
static inline int
   environment( void )              // Verify compilation controls
{
   const char*       chs;           // Pointer to character string
   int               error_count= 0; // Error count

   //-------------------------------------------------------------------------
   // Variables which may be defined
   //-------------------------------------------------------------------------
   debugf("\n\n");
   debugf("Definition variables:\n");
   debugf("__LINE__(%d) __FILE__(%s)\n", __LINE__, __FILE__);
   debugf("\n");

   chs= "NOT";
#ifdef _ADDR64
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_ADDR64");

   chs= "NOT";
#ifdef _LONG_LONG
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_LONG_LONG");

   chs= "NOT";
#ifdef LONGLONG_MIN
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "LONGLONG_MIN");

   chs= "NOT";
#ifdef LONG_LONG_MIN
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "LONG_LONG_MIN");

   chs= "NOT";
#ifdef _ALL_SOURCE
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_ALL_SOURCE");

   chs= "NOT";
#ifdef _ANSI_C_SOURCE
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_ANSI_C_SOURCE");

   chs= "NOT";
#ifdef __GNUC__
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "__GNUC__");

   chs= "NOT";
#ifdef __GNUG__
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "__GNUG__");

   chs= "NOT";
#ifdef _POSIX_SOURCE
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_POSIX_SOURCE");

   chs= "NOT";
  #ifdef _WIN64
    chs= "IS";
  #endif
   debugf("%8.5s defined(%s)\n", chs, "_WIN64");

   chs= "NOT";
#ifdef _XOPEN_SOURCE
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_XOPEN_SOURCE");

   chs= "NOT";
#ifdef _X86_
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_X86_");

   chs= "NOT";
#ifdef __x86_64__
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "__x86_64__");

   //-------------------------------------------------------------------------
   // Windows.h
   //-------------------------------------------------------------------------
   debugf("\nWindows:\n");
   MACROF(_INTEGRAL_MAX_BITS);
   MACROF(_MSC_VER);
   MACROF(WINADVAPI);
   MACROF(WINAPI);
   MACROF(WINVER);

   //-------------------------------------------------------------------------
   // GNU Compiler
   //-------------------------------------------------------------------------
   debugf("\nGCC\n");
   MACROF(__GNUC__);
   MACROF(__GNUG__);
   MACROF(__GCC_ATOMIC_BOOL_LOCK_FREE);
   MACROF(__GCC_ATOMIC_CHAR_LOCK_FREE);
   MACROF(__GCC_ATOMIC_SHORT_LOCK_FREE);
   MACROF(__GCC_ATOMIC_INT_LOCK_FREE);
   MACROF(__GCC_ATOMIC_LONG_LOCK_FREE);
   MACROF(__GCC_ATOMIC_LLONG_LOCK_FREE);
   MACROF(__GCC_ATOMIC_POINTER_LOCK_FREE);
   MACROF(__GNUC_STDC_INLINE__);

   //-------------------------------------------------------------------------
   // BSD/Linux
   //-------------------------------------------------------------------------
   debugf("\n");
   MACROF(__cplusplus);
   MACROF(__FAVOR_BSD);
   MACROF(__KERNEL_STRICT_NAMES);
   MACROF(__LARGE64_FILES);
   MACROF(__POSIX_VISIBLE);
   MACROF(__USE_BSD);
   MACROF(__USE_FILE_OFFSET64);
   MACROF(__USE_GNU);
   MACROF(__USE_ISOC9X);
   MACROF(__USE_LARGEFILE);
   MACROF(__USE_LARGEFILE64);
   MACROF(__USE_MISC);
   MACROF(__USE_POSIX);
   MACROF(__USE_POSIX199309);
   MACROF(__USE_POSIX199506);
   MACROF(__USE_POSIX2);
   MACROF(__USE_REENTRANT);
   MACROF(__USE_SVID);
   MACROF(__USE_UNIX98);
   MACROF(__USE_XOPEN);
   MACROF(__USE_XOPEN_EXTENDED);
   MACROF(_FILE_OFFSET_BITS);
   MACROF(_LARGEFILE_SOURCE);
   MACROF(_LARGEFILE64_SOURCE);
   MACROF(NULL);
   MACROF(lstat);
   MACROF(off_t);

   //-------------------------------------------------------------------------
   // Build controls
   //-------------------------------------------------------------------------
   debugf("\n"); //-----------------------------------------------------------
   chs= "NOT";
#ifdef _CC_GCC
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_CC_GCC");

   chs= "NOT";
#ifdef _CC_MSC
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_CC_MSC");

   chs= "NOT";
#ifdef _CC_XLC
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_CC_XLC");

   debugf("\n"); //-----------------------------------------------------------
   chs= "NOT";
#ifdef _HW_PPC
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_HW_PPC");

   chs= "NOT";
#ifdef _HW_X86
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_HW_X86");

   debugf("\n"); //-----------------------------------------------------------
   chs= "NOT";
#ifdef _OS_BSD
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_OS_BSD");

   chs= "NOT";
#ifdef _OS_CYGWIN
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_OS_CYGWIN");

   chs= "NOT";
#ifdef _OS_DOS
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_OS_DOS");

   chs= "NOT";
#ifdef _OS_LINUX
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_OS_LINUX");

   chs= "NOT";
#ifdef _OS_WIN
   chs= "IS";
#endif
   debugf("%8.5s defined(%s)\n", chs, "_OS_WIN");

   //-------------------------------------------------------------------------
   // pub Library
   //-------------------------------------------------------------------------
   debugf("\n");
   MACROF(_PUB_NAMESPACE);
   MACROF(ATTRIB_NORETURN);

   //-------------------------------------------------------------------------
   // inline.h
   //-------------------------------------------------------------------------
   debugf("\n"); // inline.h
   MACROF(INLINE);
   MACROF(INLINING);

   //-------------------------------------------------------------------------
   // Variables which must be defined
   //-------------------------------------------------------------------------
   struct stat s;
   struct stat64 s64;
   debugf("\n\n");
   debugf("Required variables:\n");
   debugf("\n");
   debugf("%8x INT_MAX\n", INT_MAX);
   debugf("%8lx LONG_MAX\n", LONG_MAX);
   debugf("%8d sizeof(off_t)\n", (int)sizeof(off_t));
   debugf("%8d sizeof(struct stat.st_size)\n", (int)sizeof(s.st_size));
   debugf("%8d sizeof(struct stat64.st_size)\n", (int)sizeof(s64.st_size));
   debugf("%8d sizeof(long)\n", (int)sizeof(long));
   debugf("%8d sizeof(void*)\n", (int)sizeof(void*));

   //-------------------------------------------------------------------------
   // Verify stdint.h
   //-------------------------------------------------------------------------
   error_count += VERIFY(sizeof(  int8_t) == 1);
   error_count += VERIFY(sizeof( uint8_t) == 1);
   error_count += VERIFY(sizeof( int16_t) == 2);
   error_count += VERIFY(sizeof(uint16_t) == 2);
   error_count += VERIFY(sizeof( int32_t) == 4);
   error_count += VERIFY(sizeof(uint32_t) == 4);
   error_count += VERIFY(sizeof( int64_t) == 8);
   error_count += VERIFY(sizeof(uint64_t) == 8);

   //-------------------------------------------------------------------------
   // Environment variables
   //-------------------------------------------------------------------------
   debugf("\nEnvironment variables:\n");
   debugf("%8s HOME(%s)\n", "", getenv("HOME"));
   debugf("%8s HOST(%s)\n", "", getenv("HOST"));
   debugf("%8s JAVA_HOME(%s)\n", "", getenv("JAVA_HOME"));
   debugf("%8s TEMP(%s)\n", "", getenv("TEMP"));
   debugf("%8s USER(%s)\n", "", getenv("USER"));

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
int                                 // Return code
   main(                            // Mainline entry
     int             argc,          // Argument count
     char*           argv[])        // Argument array
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

     setlocale(LC_NUMERIC, "");     // Activates ' thousand separator
     error_count += environment();  // Display compliation environment
     error_count += test_stdlib();  // Test ::free(NULL), std::free(nullptr)

     if( opt_verbose ) {
       error_count += demo_std_exception(); // Demo std::exception usage error
       error_count += test_std_exception(); // Test std::exception
       error_count += test_Clock(); // Test Clock.h
       error_count += test_Interval(); // Test Interval.h
       error_count += test_Performance(); // For machine to machine comparison
     }

     debugf("\n");
     tr->report_errors(error_count);
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}

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
//       Test_event.cpp
//
// Purpose-
//       Test Event.h compilation.
//
// Last change date-
//       2026/03/23
//
// Implementation notes-
//       Used to compile test Event.h and Event_any
//
//       Look at the compiler output to see why TestDisp running Event.h
//       runs in ~12 second and Event_any runs in ~14 seconds.
//
//       (gdb trace might also work.)
//
//----------------------------------------------------------------------------
#include <condition_variable>       // For std::condition_variable types
#include <mutex>                    // For std::unique_lock, std::mutex
#include <thread>                   // For std::this_thread::yield

#include <cstdint>                  // For uint32_t

#include <pub/TEST.H>               // For test functions and macros
#include "pub/Debug.h"              // For namespace pub::debugging
#include "pub/Event.h"              // For Event, *COMPILE* tested
#include "pub/mutex.h"              // For pub::mutex
#include <pub/Wrapper.h>            // For class Wrapper

#define PUB _LIBPUB_NAMESPACE
using PUB::Wrapper;
using namespace PUB::debugging;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       pub::Event_any
//
// Purpose-
//       Event descriptor using std::condition_variable_any.
//
//----------------------------------------------------------------------------
class Event_any {                   // Event descriptor
//----------------------------------------------------------------------------
// Event_any::Attributes
//----------------------------------------------------------------------------
private:
volatile uint32_t      code;        // (31 bit) Post code
std::condition_variable_any
                       cv;          // Event driver
pub::mutex             mutex;       // Protects cv

//----------------------------------------------------------------------------
// Event_any::Constructors/Assignment/Destructor
//----------------------------------------------------------------------------
public:
   Event_any( void )                    // Default constructor
:  code(0), cv(), mutex()
{  }

   ~Event_any( void ) = default;        // Destructor

//----------------------------------------------------------------------------
// Event_any::Methods
//----------------------------------------------------------------------------
bool                                // TRUE iff posted
   has_posted( void ) const         // Is Event in posted state?
{  return code & 0x8000'0000; }     // (High order bit indicates posted)

void
   post(                            // Post event completion
     uint32_t          code= 0)     // (31 bit) completion code
{  std::unique_lock<decltype(mutex)> lock(mutex);

   this->code= code | 0x8000'0000;   // (High order bit indicates posted)
   cv.notify_all();
}

int32_t                             // The event code (Always positive)
   wait( void )                     // Wait for Event
{  std::unique_lock<decltype(mutex)> lock(mutex);

   while( !has_posted() )           // Handle spurious wake-ups
     cv.wait(lock);

   return code & 0x7fff'ffff;       // 31-bit post code
}
}; // class Event_any
_LIBPUB_END_NAMESPACE

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
//       test_event_any
//
// Purpose-
//       Test Event_any
//
//----------------------------------------------------------------------------
_LIBPUB_NOINLINE
static int
   test_event_any( void )
{
   if( opt_verbose )
     debugf("\ntest_event_any\n");

   int error_count= 0;              // Error counter

   {{{{
     PUB::Event_any event;
     event.post();
     event.wait();
   }}}}

   {{{{
     PUB::mutex mutex;
     mutex.lock();
     mutex.unlock();
   }}}}

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_event_std
//
// Purpose-
//       Test Event.h
//
//----------------------------------------------------------------------------
_LIBPUB_NOINLINE
static int
   test_event_std( void )
{
   if( opt_verbose )
     debugf("\ntest_event_std\n");

   int error_count= 0;              // Error counter

   {{{{
     PUB::Event event;
     event.post();
     event.wait();
   }}}}

   {{{{
     std::mutex mutex;
     mutex.lock();
     mutex.unlock();
   }}}}

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
     int               argc,        // Argument count (UNUSED)
     char*             argv[])      // Argument array (UNUSED)
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

     error_count += test_event_any();
     error_count += test_event_std();

     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   setlocale(LC_NUMERIC, "");       // Activates ' thousand separator
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;

   return tc.run(argc, argv);
}

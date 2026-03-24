//----------------------------------------------------------------------------
//
//       Copyright (C) 2026 Frank Eskesen.
//
//       This file is free content, distributed under creative commons CC0,
//       explicitly released into the Public Domain.
//       (See accompanying html file LICENSE.ZERO or the original contained
//       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
//
// SPDX-License-Identifier: CC0-1.0
//----------------------------------------------------------------------------
//
// Title-
//       Test_mutex.cpp
//
// Purpose-
//       Mutex stress test.
//
// Last change date-
//       2026/03/16
//
//----------------------------------------------------------------------------
#include <exception>                // For std::exception
#include <mutex>                    // For std::lock_guard

#include <cstdio>                   // For printf
#include <ctime>                    // For timespec, clock_gettime

#include "pub/mutex.h"              // For pub::mutex

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
static double          opt_runtime= 60.0;

//----------------------------------------------------------------------------
//
// Subroutine-
//       now
//
// Purpose-
//       Return the number of seconds since the PC epoch.
//
//----------------------------------------------------------------------------
static double                       // Seconds since the PC epoch
   now( void )                      // Get the current time
{
   struct timespec     ticker;      // UTC time base

   clock_gettime(CLOCK_REALTIME, &ticker); //
   double seconds= (double)ticker.tv_sec;
   seconds += (double)ticker.tv_nsec / 1000000000.0;
   return seconds;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       do_something
//
// Purpose-
//       Do something that causes CYGWIN handle growth
//
// Implementation notes-
//       If the mutex is static, there is no handle growth. The non-static
//       pub::mutex simulates
//
//       mutex.lock() with or without unlock causes CYGWIN handle growth, and
//       an exception is thrown at about 16.880 million handles.
//
//       mutex.unlock() without lock causes CYGWIN handle growth, but growth
//       stops at about 16.880 million handles. No exception is thrown.
//
//       std::lockguard creates a lock/unlock sequence, throwing an exception
//       at about 16.880 million handles.
//
//----------------------------------------------------------------------------
class Thing {                       // Thing descriptor
std::mutex             mutex;       // Protects cv

public:
   Thing() = default;               // Default constructor
   ~Thing() = default;              // Destructor

bool try_lock() { return mutex.try_lock(); }
void lock()     { mutex.lock(); }
void unlock()   { mutex.unlock(); }
}; // class Thing

static inline void
   do_something_else(void)               // Try to cause CYGWIN handle growth
{
   Thing thing;

   thing.lock();
   thing.unlock();
   std::lock_guard<decltype(thing)> lock(thing);
}

static void
   do_something(void)               // Try to cause CYGWIN handle growth
{
   pub::mutex mutex;

   mutex.lock();
   mutex.unlock();
   std::lock_guard<decltype(mutex)> lock(mutex);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_mutex
//
// Purpose-
//       pub::mutex stress test.
//
//----------------------------------------------------------------------------
static inline void
   test_mutex( void )               // Timing test
{
   printf("%8.3f Runtime\n", opt_runtime);

   double then= now();              // Running timer
   while( (now() - then) < opt_runtime )
     do_something();
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
   main(int, char**)                // Mainline code
{
   try {
     test_mutex();
   } catch(std::exception& x) {
     printf("FAILED: Exception: exception(%s)\n", x.what());
   } catch(...) {
     printf("FAILED: Exception: ...\n");
   }

   return 0;
}

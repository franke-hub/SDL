//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_bug.cpp
//
// Purpose-
//       Test debugging methods.
//
// Last change date-
//       2021/05/23
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "pub/Debug.h"
#include "pub/utility.h"              // Volatile values prevent optimization

using _PUB_NAMESPACE::Debug;
using namespace _PUB_NAMESPACE::debugging;

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pub::Debug      debug;         // Debug object

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_bt
//
// Purpose-
//       Test backtrace
//
// Implementation note-
//       Best results when compiled without optimization.
//       With optimization GCC inlines most of the call stack.
//
//----------------------------------------------------------------------------
static void bar() {
   debugf("Backtrace test\n");
   debug_backtrace();
   debugf("\n");
}

static void foo() {
   bar();
}

static void the() {
   foo();
}

static void test_bt() {
   the();
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
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (UNUSED)
//   char*             argv[])      // Argument array (UNUSED)
{
   // Test backtrace
    test_bt();

   // Test modes
   debug_set_mode(Debug::MODE_DEFAULT);
   debugf("Standard mode:\n");
   debugf("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorf("This appears in %s and %s\n", "TRACE", "STDERR");
   tracef("This appears in %s ONLY\n",   "TRACE");
   debugh("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorh("This appears in %s and %s\n", "TRACE", "STDERR");
   traceh("This appears in %s ONLY\n",   "TRACE");

   debug_set_mode(Debug::MODE_IGNORE);
   errno= ESRCH;
   errorp("Ignore mode: (ignored) This appears in STDERR (only)");
   debugf("Ignore mode:\n");
   errorf("Ignore mode:\n");
   tracef("Ignore mode:\n");
   debugh("Ignore mode:\n");
   errorh("Ignore mode:\n");
   traceh("Ignore mode:\n");

   debug_set_mode(Debug::MODE_INTENSIVE);
   debugf("Intensive mode:\n");
   debugf("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorf("This appears in %s and %s\n", "TRACE", "STDERR");
   tracef("This appears in %s ONLY\n",   "TRACE");
   debugh("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorh("This appears in %s and %s\n", "TRACE", "STDERR");
   traceh("This appears in %s ONLY\n",   "TRACE");

   return 0;                        // Normal completion
}

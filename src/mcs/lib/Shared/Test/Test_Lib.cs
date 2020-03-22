//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_Lib.cs
//
// Purpose-
//       Test miscellaneous library classes.
//
// Last change date-
//       2019/02/15
//
//----------------------------------------------------------------------------
#define USE_DEBUG                   // Create Debug instance?

using System;

using Shared;

//============================================================================
// Test cases
//============================================================================
static class Test {
//----------------------------------------------------------------------------
//
// Method-
//       test_trace
//
// Purpose-
//       Trace.cs minimal verification test.
//
//
//----------------------------------------------------------------------------
static void test_trace( ) {         // Test Trace.cs
   Debug.WriteLine("\ntest_trace (See debug.log)");

   Trace.reset(32);                 // Small trace table

   for(uint i= 0; i<18; i++) {      // Trace with wrap
       string s= String.Format("{0,2} trace line", i);
       Trace.trace(s);
   }

   for(uint i= 18; i<36; i++) {      // Trace with wrap
       Object o= Trace.wrap("{0,2} line {1}", i, "trace");
       Trace.trace(o);
   }

   Trace.dump();
}

//============================================================================
// Test.all: Run all tests
//============================================================================
static public void all( ) {         // Run all tests
   test_trace();                    // Test Trace.cs
}
}  // static class Test

//============================================================================
// PROGRAM: Mainline code
//============================================================================
public class Program {
static void Main(string[] args)     // Mainline code
{
#if USE_DEBUG
   Debug debug= new Debug("debug.log");
#endif

   Test.all();

#if USE_DEBUG
   debug.close();
#endif
}
}   // class Program

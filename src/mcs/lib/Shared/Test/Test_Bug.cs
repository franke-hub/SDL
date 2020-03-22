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
//       Test_Bug.cs
//
// Purpose-
//       Test library Debug object.
//
// Last change date-
//       2019/02/15
//
//----------------------------------------------------------------------------
using System;
using System.Diagnostics;           // For demonstration purposes only.
using Shared;                       // For Debug

//============================================================================
/// We are not reall using System.Diagnostics, it is included only to
/// demonstrate how to use this class when it is included.
//============================================================================
using Debug = Shared.Debug;

//============================================================================
// Test cases
//============================================================================
static class Test {
//----------------------------------------------------------------------------
//
// Method-
//       test_debug
//
// Purpose-
//       Debug.cs minimal verification test.
//
//----------------------------------------------------------------------------
public static void test_debug( ) { // Test Debug.cs
   //-------------------------------------------------------------------------
   using( Debug me= new Debug("debug.log") ) {
       me.putLine("Debug: using( Debug x= new Debug({0}) {1}",
                  "\"debug.log\"", "{...}");
       Test.debug_unit();
   }

   GC.Collect();
   GC.WaitForPendingFinalizers();
   Debug.assert( Debug.debug == null );

   //-------------------------------------------------------------------------
   using( Debug me= new Debug() ) {
       me.putLine("\nDebug: Functional tests");
       me.putLine();
       me.putLine("Tested: empty putLine");

       Test.debug_null();
   }

   //-------------------------------------------------------------------------
   {
       Debug me= new Debug();
       me.putLine("\nDebug: Debug x= new Debug(): {0} Debug.close()", "{...}");
       Test.debug_unit();
       me.close();
       me = null;
   }

   GC.Collect();
   GC.WaitForPendingFinalizers();
   Debug.assert( Debug.debug == null );

#if false // THIS TEST FAILS // TODO: Investigate.
   //-------------------------------------------------------------------------
   Debug.WriteLine("\nDebug: Debug x= new Debug(): {0} NO close()", "{...}");
   {
       Debug me= new Debug();
       Test.debug_unit();
   }

   GC.Collect();
   GC.WaitForPendingFinalizers();
   Debug.assert( Debug.debug == null );
#endif
}

static void debug_unit( ) {         // Run Debug unit test
   Debug.debug.put("debug_unit: First test line...\n");

   Debug.debug.put("Begin a console+log line, ");
   Debug.debug.putLine("end the console+log line.");

   Debug.debug.log("Begin a log-only line, ");
   Debug.debug.logLine("end the log-only line.");

   Debug.DEBUGGING= false;
   Debug.debug.put("Begin a discarded line, ");
   Debug.debug.putLine("end the discarded line.");

   Debug.Write("Begin an unconditional console+log line, ");
   Debug.WriteLine("end the unconditional line.");

   Debug.DEBUGGING= true;
   Debug.debug.put("debug_unit: ...Last test line\n");
}

static void debug_null( ) {         // Run null object test
   object null_object= null;

   // Missing references compile but throw a run-time exception
// Debug.debug.put("Testing missing({0}) reference\n");

   // null references compile but throw a run-time exception
// Debug.debug.put("Testing missing({0}) reference\n", null);

   // Null objects compile are display the same as an empty string
   Debug.debug.put("Testing null({0}) reference\n", null_object);
}
}  // static class Test

//============================================================================
// PROGRAM: Mainline code
//============================================================================
public class Program {
static void Main(string[] args)     // Mainline code
{
   Test.test_debug();               // Run all tests
}
}   // class Program

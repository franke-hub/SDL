//-----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Test_regressed.cs
//
// Purpose-
//       Regression tests.
//
// Last change date-
//       2019/02/15
//
//-----------------------------------------------------------------------------
using System;                       // (Almost always required)

using Shared;                       // For Debug

namespace Sample {                  // The Sample namespace
//=============================================================================
//
// Class-
//       Test_regressed
//
// Purpose-
//       Regression tests.
//
//=============================================================================
class Test_regressed: Test {        // Regression tests
public void test_debug()            // Debug.cs regression test
{
   object oopsie= null;
   string demo= "*DEMO*:";

   Debug.WriteLine("{0} null({1}) {2}", demo, oopsie, "var content");
   Debug.WriteLine();
   Debug.WriteLine("{0} after Debug.WriteLine()", demo);

   Debug.debug.log("{0} ", demo);
   Debug.debug.log();               // (NOP)
   Debug.debug.logLine("Log-only line");

   Debug.debug.put("{0} ", demo);
   Debug.debug.put();               // (NOP)
   Debug.debug.putLine("Console+Log line (after log-only line)");
}

//=============================================================================
// Test_regressed.Test: Regression testing
//=============================================================================
public void Test(object obj)
{
   Debug.WriteLine("START>: Test_regressed");
   test_debug();
   Debug.WriteLine("PASSED: Test_regressed");
}
}  // class Test_regressed
}  // namespace Sample

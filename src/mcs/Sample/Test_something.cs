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
//       Test_something.cs
//
// Purpose-
//       Sample test.
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
//       Test_something
//
// Purpose-
//       Sample test.
//
//=============================================================================
class Test_something: Test {        // Sample test
//=============================================================================
// Test_something.Test: Test something
//=============================================================================
public void Test(object obj)
{
#if false
   Debug.WriteLine("START>: Test_something");
   Debug.WriteLine("PASSED: Test_something");
#endif
}
}  // class Test_something
}  // namespace Sample

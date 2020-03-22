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
//       Test_interface.cs
//
// Purpose-
//       Define interfaces used in Sample.cs and Test_*.cs
//
// Last change date-
//       2019/01/16
//
//-----------------------------------------------------------------------------
///// System;                       // (Almost always required, but not here)

namespace Sample {                  // The Sample namespace
//-----------------------------------------------------------------------------
//
// Interface-
//       Test
//
// Purpose-
//       The testcase interface.
//
//-----------------------------------------------------------------------------
interface Test {                    // The testcase interface
void Test(object o);                // If test fails, Exception thrown
}  // interface Test
}  // namespace Sample

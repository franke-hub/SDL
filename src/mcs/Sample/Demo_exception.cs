//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Demo_exception.cs
//
// Purpose-
//       Test exception handling, among other things.
//
// Last change date-
//       2019/02/15
//
//----------------------------------------------------------------------------
using System;                       // (Almost always required)

using Shared;                       // For Shared.Debug

internal static class Testing {     // So we can define methods
// class Testing.ExpectedException
public class ExpectedException: Exception {
public ExpectedException(string s) : base(s) {}
} // class ExpectedException

//----------------------------------------------------------------------------
//
// Method-
//       Testing.demo_exception
//
// Purpose-
//       Demonstrate exception handling
//
//----------------------------------------------------------------------------
internal static void demo_exception( ) // Demo exception handling
{
   bool passed = false;
   Debug debug = Debug.debug;

   try {
       debug.put("START>: Test_exception\n");
       if( Sample.Utility.ALWAYS_TRUE ) // Avoid compiler warning
           throw new ExpectedException(">>>>>>: Expected Exception");

       throw new Exception("FAILED: Test_exception");
   } catch( ExpectedException e) {
       debug.putLine(e.Message);
       debug.putLine(e.StackTrace);
       debug.flush();
       passed = true;
   } catch( Exception e) {
       debug.putLine(e.Message);
       debug.putLine(e.StackTrace);
       debug.flush();
   } finally {
       if( passed )
           debug.putLine("PASSED: Test_exception");
       else
           throw new Exception("FAILED: Test_exception demo");
   }
}
} // internal class Testing

//============================================================================
//
// Method-
//       Sample.Demo_exception.Test()
//
// Purposes-
//       1) Demonstrate exception handling
//       2) Demonstrate usage of Shared.Debug.
//
// How it works-
//       Namespace "Sample" contains code from Sample.cs and multiple other
//       library source files, including this one. The library, Local.net, is
//       a netmodule built from all source.cs files that are not intended to
//       create executable.exe files. The names of the source files used to
//       build the library are not important.
//
//       The build environment Makefiles in ~/src/mcs/ctl/BSD do most of the
//       work for this. Makefile.BSD in this subdirectory provides additional
//       Makefile tailoring.
//
//       Note: the class Test_exception (derived from interface Test) is
//       visible and useable from outside for the Sample even though it is
//       declared internal. It's used in Howto.cs outside of any namespace.
//
// Using Shared.Debug-
//       See: ~/src/mcs/lib/Shared/Debug.cs, Usage notes.
//
//============================================================================
namespace Sample {
internal class Demo_exception : Test {
public void Test(Object obj)
{
   Testing.demo_exception();
}
} // class Test_exception
} // namespace Sample


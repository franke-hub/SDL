//----------------------------------------------------------------------------
//
//       Copyright (C) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test
//
// Purpose-
//       Testcase base class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Test
//
// Purpose-
//       Testcase base class.
//
//----------------------------------------------------------------------------
class Test
{
//----------------------------------------------------------------------------
//
// Method-
//       Test.testcase
//
// Purpose-
//       Function test.
//
//----------------------------------------------------------------------------
public int                          // Number of errors encountered
   testcase( )                      // Function test
   throws Exception
{
   throw new Exception("Used Abstract Base Class(" + getClass().getName() + ")");
}

//----------------------------------------------------------------------------
//
// Method-
//       Test.test
//
// Purpose-
//       Testcase driver.
//
//----------------------------------------------------------------------------
public int                          // Number of errors encountered
   test( )                          // Testcase driver
   throws Exception
{
   System.out.println("Started...");
   testcase();
   System.out.println("...Complete");

   return 0;
}
} // Class Test


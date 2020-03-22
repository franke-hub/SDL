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
//       ArrayTest.java
//
// Purpose-
//       Test arrays.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       ArrayTest
//
// Purpose-
//       Java language test.
//
//----------------------------------------------------------------------------
public class ArrayTest implements TestCase
{
//----------------------------------------------------------------------------
// ArrayTest.attributes
//----------------------------------------------------------------------------
   int                 errorCount;  // Error counter

//----------------------------------------------------------------------------
//
// Method-
//       ArrayTest.ArrayTest
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ArrayTest( )                     // Constructor
{
   errorCount= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       ArrayTest.driver
//
// Purpose-
//       Test case driver.
//
//----------------------------------------------------------------------------
public int                          // Number of errors encountered
   driver( )                        // Test case driver
   throws Exception
{
   int                 i, j;

   Object[]            array= {     // An array of Objects
       new ArrayTest(),             // [0] Object
       new Object[]  {              // [1] Inner array
           new ArrayTest(),
           this,
           new ArrayTest()
           },
       new ArrayTest(),             // [2]
       new Object[]  {              // [3] Inner array
           new ArrayTest()
           },
       new Object[] {},             // [4] NULL array
       this,
       new ArrayTest()
       };

   if( Main.hcdm )
   {
     System.out.println(getClass().getName() + ".driver()");
     new Show(array);
//   System.out.println("" + array);
//
//   for(i=0; i<array.length; i++)
//   {
//     System.out.println("[" + i + "]: " + array[i]);
//     if( (array[i] instanceof Object[]) )
//     {
//       for(j=0; j<((Object[])array[i]).length; j++)
//       {
//         System.out.println("[" + i + "]" +
//                            "[" + j + "]: " +
//                            ((Object[])array[i])[j]);
//       }
//     }
//   }
   }

   return errorCount;
}
} // Class ArrayTest


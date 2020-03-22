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
//       Main.java
//
// Purpose-
//       Sample code containing multiple classes.
//
// Last change date-
//       2007/01/01
//
// Classes defined-
//       Main          (Testcase driver).
//       Test          (Testcase framework).
//       TestArray     (An actual testcase).
//       TestModulus   (An actual testcase).
//
// Notes-
//       Class Test could have been defined in Test.java and so on.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Java testcase driver.
//
//----------------------------------------------------------------------------
public class Main
{
//----------------------------------------------------------------------------
//
// Method-
//       Main.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   TestArray           array=   new TestArray();
   TestModulus         modulus= new TestModulus();

   array.test();
   modulus.test();
}
} // Class Main

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

//----------------------------------------------------------------------------
//
// Class-
//       TestArray
//
// Purpose-
//       Array test class.
//
//----------------------------------------------------------------------------
class TestArray extends Test
{
//----------------------------------------------------------------------------
// TestArray.attributes
//----------------------------------------------------------------------------
   Object[]            array;       // An array of objects

//----------------------------------------------------------------------------
//
// Method-
//       TestArray.test
//
// Purpose-
//       Testcase driver.
//
//----------------------------------------------------------------------------
public int                          // Number of errors encountered
   test( )                          // Testcase driver
   throws Exception
{
   int                 i;

   if( array == null )
     array= new Integer[32];

   for(i=0; i<array.length; i++)
   {
     array[i]= new Integer(i);
   }

   for(i=0; i<array.length; i++)
   {
     System.out.println(array[i] + "= " + i);
   }

   return 0;
}
} // Class TestArray

//----------------------------------------------------------------------------
//
// Class-
//       TestModulus
//
// Purpose-
//       Modulus test class.
//
//----------------------------------------------------------------------------
class TestModulus extends Test
{
//----------------------------------------------------------------------------
//
// Method-
//       TestModulus.test
//
// Purpose-
//       Testcase driver.
//
//----------------------------------------------------------------------------
public int                          // Number of errors encountered
   test( )                          // Testcase driver
   throws Exception
{
   int                 i;

   System.out.println("Started...");

   for(i= -10; i<= 10; i++)
   {
     System.out.println(i % 8 + "= " + i + " % 8");
   }

   System.out.println("...Complete");

   return 0;
}
} // Class TestModulus


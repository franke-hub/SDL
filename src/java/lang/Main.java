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
//       Language tester.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Java language test.
//
//----------------------------------------------------------------------------
public class Main
{
//----------------------------------------------------------------------------
// Main.attributes
//----------------------------------------------------------------------------
static boolean         debugging= true; // Debugging
static boolean         hcdm= true;  // Hard-Core Debug Mode
static boolean         scdm= true;  // Soft-Core Debug Mode

   String[]            args;        // Argument array

//----------------------------------------------------------------------------
//
// Method-
//       Main.Main
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Main(                            // Constructor
     String[]          args)        // Argument array
{
   this.args= args;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.driver
//
// Purpose-
//       Test case driver.
//
//----------------------------------------------------------------------------
public int                          // Number of errors encountered
   driver( )                        // Test case driver
   throws Exception
{
   TestCase            arrayTest;   // Array test
   int                 errorCount;  // Error counter

   errorCount= 0;

   arrayTest= new ArrayTest();
   errorCount += arrayTest.driver();
   return errorCount;
}

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
{
   Main                main;        // Instance variable
   int                 errorCount;  // Error counter

   errorCount= 0;
   main= new Main(args);
   try {
     errorCount += main.driver();
   } catch(Exception e) {
     errorCount++;
     System.out.println("Main: Exception: " + e);
     e.printStackTrace();
   }

   if( errorCount == 0 )
     System.out.println("NO errors encountered");
   else if( errorCount == 1 )
     System.out.println("1 error encountered");
   else
     System.out.println(errorCount + " errors encountered");
}
} // Class Main


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
//       Main
//
// Purpose-
//       Testcase driver.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.lang.*;
import Test;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Java testcase driver.
//
//----------------------------------------------------------------------------
class Main
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
     String[]       args)           // Argument array
   throws Exception
{
   String           string;
   Modulus          modulus= new Modulus();
// Test             test= new Test();
// Xfer             xfer= new Xfer();
   int              value;

   int              i;

   for(i= 0; i<args.length; i++)
   {
     string= args[i];
     value= Integer.parseInt(string);
     System.out.println(value + "= Integer.parseInt(\"" + string + "\")");
   }

   modulus.test();
// xfer.test();
// test.test();

}
} // Class Main

//----------------------------------------------------------------------------
//
// Class-
//       Modulus
//
// Purpose-
//       Modulus test class.
//
//----------------------------------------------------------------------------
class Modulus extends Test
{
//----------------------------------------------------------------------------
//
// Method-
//       Modulus.test
//
// Purpose-
//       Testcase driver.
//
//----------------------------------------------------------------------------
public int                          // Number of errors encountered
   test( )                          // Testcase driver
   throws Exception
{
   int              i;

   System.out.println("Started...");

   for(i= -10; i<= 10; i++)
   {
     System.out.println(i % 8 + "= " + i + " % 8");
   }

   System.out.println("...Complete");

   return 0;
}
} // Class Modulus


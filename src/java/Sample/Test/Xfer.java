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
//       Xfer
//
// Purpose-
//       Transfer test class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Xfer
//
// Purpose-
//       Transfer test class.
//
//----------------------------------------------------------------------------
class Xfer extends Test
{
//----------------------------------------------------------------------------
//
// Method-
//       Xfer.testcase
//
// Purpose-
//       Function test.
//
//----------------------------------------------------------------------------
public static int                   // Number of errors encountered
   testcase(                        // Function test
     int             v1,            // Value[1]
     int             v2,            // Value[2]
     double          w1,            // Weight[1]
     double          w2)            // Weight[2]
   throws Exception
{
   int               resultant;     // Resultant
   int               sumV;          // Sum of values
   double            sumW;          // Sum of weights

   if( w1 < 0.0 || w2 < 0.0 ) throw new Exception("Botched parameters");
   if( v1 < 0 || v2 < 0 ) throw new Exception("Botched parameters");

   sumV= v1 + v2;
   sumW= w1 + w2;
   if( sumW == 0.0 )
     sumW= 0.5;
   else
     sumW= w1 / sumW;

   resultant= (int)(sumV * sumW);
   resultant /= 100;
   resultant *= 100;
   return resultant - v1;
}

//----------------------------------------------------------------------------
//
// Method-
//       Xfer.test
//
// Purpose-
//       Testcase driver.
//
//----------------------------------------------------------------------------
public int                          // Number of errors encountered
   test( )                          // Testcase driver
   throws Exception
{
   int              old1;
   int              old2;
   int              sumV;
   int              new1;
   int              new2;
   int              xfer;
   double           wgt1;
   double           wgt2;
   double           sumW;

   System.out.println("Started...");

   sumV= 1022;
   sumW= 10.0;
   for(old1= 0; old1 < sumV; old1= old1 + 100)
   {
     for(wgt1= 0.0; wgt1 <= 10.0; wgt1= wgt1 + 1.0)
     {
       old2= sumV - old1;
       wgt2= sumW - wgt1;
       xfer= testcase(old1, old2, wgt1, wgt2);
       System.out.println(xfer + "= test(" +
                          old1 + "," +
                          old2 + "," +
                          wgt1 + "," +
                          wgt2 + ") " +
                          "new1(" + (old1 + xfer) + ") " +
                          "new2(" + (old2 - xfer) + ")" );
     }
     System.out.println("");
   }

   System.out.println("...Complete");

   return 0;
}
} // Class Xfer


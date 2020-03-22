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
//       DateTimeTest.java
//
// Purpose-
//       Test the GregorianDate, ReferenceDate and ReferenceTime classes.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package test.util;

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       DateTimeTest
//
// Purpose-
//       Test the GregorianDate, ReferenceDate and ReferenceTime classes.
//
//----------------------------------------------------------------------------
class DateTimeTest
{
//----------------------------------------------------------------------------
// DateTimeTest.attributes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       DateTimeTest.DateTimeTest
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   DateTimeTest( )                // Constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DateTimeTest.main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   try {
     DayOfWeekTest.main(args);
     GregorianTest.main(args);
   }
   catch( Throwable t ) {
     System.out.println("Exception: " + t);
     t.printStackTrace();
   }
}
} // Class DateTimeTest

//----------------------------------------------------------------------------
//
// Class-
//       DayOfWeekTest
//
// Purpose-
//       Java bringup test.
//
//----------------------------------------------------------------------------
class DayOfWeekTest
{
//----------------------------------------------------------------------------
//
// Method-
//       DayOfWeekTest.main
//
// Purpose-
//       Test driver.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]       args)           // Argument array
   throws Exception
{
   GregorianDate    g= new GregorianDate();
   ReferenceDate    j= new ReferenceDate();

   int              leap;
   int              mm;
   int              dd;
   long             yy;
   int              today, yesterday;
   long             firstYear= -8000;

   System.out.println("");
   System.out.println("DayOfWeekTest.main()");
   g.setMonthDayYear(12,31,firstYear-1);
   yesterday= g.getDayOfWeek();
   for(yy= firstYear; yy<8000; yy++)
   {
     if( (yy%100) == 0 )
       System.out.println("Year(" + yy + ")");

     leap= 0;
     if( GregorianDate.isLeapYear(yy) )
       leap= 1;
     for(mm= 1; mm<=12; mm++)
     {
       for(dd= 1; dd<=GregorianDate.daysPerMonth[leap][mm-1]; dd++)
       {
         if( yy == 1752 && mm == 9 && dd == 3 )
           dd= 14;                  // Sept 14, 1752 follows Sept 2, 1752

         g.setMonthDayYear(mm,dd,yy);
         today= g.getDayOfWeek();
         if( today != (yesterday + 1) % 7 )
         {
           System.out.println("Yesterday(" + yesterday + ") " +
                              "Today(" + today + ")");
           System.out.println(mm + "," + dd + "," + yy );
           System.out.println("JulianDate != yesterday + 1\n");
           return;
         }
         yesterday= today;
       }
     }
   }

   System.out.println("Alrighty, then!");
}
} // Class DayOfWeekTest

//----------------------------------------------------------------------------
//
// Class-
//       GregorianTest
//
// Purpose-
//       Verify the GregorianDate class
//
//----------------------------------------------------------------------------
class GregorianTest
{
//----------------------------------------------------------------------------
//
// Method-
//       GregorianTest.isValid
//
// Purpose-
//       Error analysis and reporting routine.
//
//----------------------------------------------------------------------------
public static void
   isValid(                         // Error reporter
     int            mm,             // Month
     int            dd,             // Day
     long           yy,             // Year
     GregorianDate  g)              // The Gregorian date
   throws Exception
{
   if( g.getMonthOfYear() == mm
       && g.getDayOfMonth() == dd
       && g.getYear() == yy )
     return;

   System.out.println("Month(" + mm + ") Day(" + dd + ") Year(" + yy + ")");
   System.out.println("But GregorianDate");
   System.out.println("Month(" + g.getMonthOfYear() +
                      ") Day(" + g.getDayOfMonth() +
                      ") Year(" + g.getYear() + ")");

   throw new Exception("You had your chance, but you blew it!");
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianTest.isValid
//
// Purpose-
//       Error analysis and reporting routine.
//
//----------------------------------------------------------------------------
public static void
   isValid(                         // Error reporter
     double         time,           // Time
     ReferenceTime  t)              // The ReferenceTime
   throws Exception
{
   if( java.lang.Math.abs(time - t.toDouble()) < .0001 )
     return;

   System.out.println("ReferenceTime(" + t.toDouble() + ") " +
                      "Time(" + time + ") " +
                      "Date(" + t.getDay() + ") " +
                      "NSec(" + t.getNanosecond() + ")");

   throw new Exception("You had your chance, but you blew it!");
}

//----------------------------------------------------------------------------
//
// Method-
//       GregorianTest.main
//
// Purpose-
//       Test driver.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]       args)           // Argument array
   throws Exception
{
   ReferenceTime    t= new ReferenceTime();
   GregorianDate    g= new GregorianDate();
   ReferenceDate    j= new ReferenceDate();

   double           time, check;
   int              leap;
   int              mm;
   int              dd;
   long             yy;
   long             today, yesterday;
   long             firstYear= -8000;

   System.out.println("");
   System.out.println("GregorianTest.main()");
   firstYear= -8000;
   g.setMonthDayYear(12,31,firstYear-1);
   yesterday= j.set(g.getReferenceDate());
   for(yy= firstYear; yy<8000; yy++)
   {
     if( (yy%100) == 0 )
       System.out.println("Year(" + yy + ")");

     leap= 0;
     if( GregorianDate.isLeapYear(yy) )
       leap= 1;
     for(mm= 1; mm<=12; mm++)
     {
       for(dd= 1; dd<=GregorianDate.daysPerMonth[leap][mm-1]; dd++)
       {
         if( yy == 1752 && mm == 9 && dd == 3 )
           dd= 14;                  // Sept 14, 1752 follows Sept 2, 1752

         g.setMonthDayYear(mm,dd,yy);
         if( g.getMonthOfYear() != mm )
           System.out.println("Can't set date right");
         if( g.getDayOfMonth() != dd )
           System.out.println("Can't set date right");
         if( g.getYear() != yy )
           System.out.println("Can't set date right");

         today= j.set(g.getReferenceDate());
//       System.out.println("Julian(" + today + ")= " +
//                       "Gregorian(" + mm + "," + dd + "," + yy + ")");

         if( today != j.getDay() )
           throw new Exception("JulianDate inconsistent");

         if( today != yesterday + 1 )
         {
           System.out.println("Yesterday(" + yesterday + ") " +
                              "Today(" + today + ")");
           System.out.println(mm + "," + dd + "," + yy );
           System.out.println("JulianDate != yesterday + 1\n");
           return;
         }
         yesterday= today;

         g.set(j);
         isValid(mm,dd,yy,g);
       }
     }
   }

   for(time= -8000.0; time <= 8000.0; time += 0.25)
   {
     if( ((long)time % 100) == 0 && time == (double)((long)time) )
       System.out.println("Time(" + time + ")");

     t.set(time);
     isValid(time, t);
   }

   System.out.println("Alrighty, then!");
}
} // Class GregorianTest


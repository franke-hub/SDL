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
//       Julian.java
//
// Purpose-
//       Julian bringup test.
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
//       Julian
//
// Purpose-
//       Julian bringup test.
//
//----------------------------------------------------------------------------
class Julian
{
//----------------------------------------------------------------------------
//
// Method-
//       Julian.main
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

   long             yy;
   int              mm;
   int              dd;

   if( args.length < 3 )
   {
     System.out.println("Arguments: Year month day");
     return;
   }

   yy= Long.parseLong(args[0]);
   mm= Integer.parseInt(args[1]);
   dd= Integer.parseInt(args[2]);

   g.setMonthDayYear(mm, dd, yy);
   j.set(g.getReferenceDate());

   System.out.println("Year(" + yy + ") " +
                      "Month(" + mm + ") " +
                      "Day(" + dd + ")= " +
                      "Julian(" + j.getDay() + ")");
}
} // Class Julian


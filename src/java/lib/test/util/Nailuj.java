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
//       Nailuj.java
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
//       Nailuj
//
// Purpose-
//       Julian bringup test.
//
//----------------------------------------------------------------------------
class Nailuj
{
//----------------------------------------------------------------------------
//
// Method-
//       Nailuj.main
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
   if( args.length == 0 )
   {
     System.out.println("Nailuj Julian-date");
     System.exit(2);
   }

   GregorianDate g= new GregorianDate();
   ReferenceDate j= new ReferenceDate(Long.parseLong(args[0]));

   g.set(j);
   System.out.println("Julian(" + j.getDay() + ")= " +
                      "Year(" + g.getYear() + ") " +
                      "Month(" + g.getMonthOfYear() + ") " +
                      "Day(" + g.getDayOfMonth() + ")");
}
} // Class Julian


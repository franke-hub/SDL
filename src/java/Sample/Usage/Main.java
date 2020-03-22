//----------------------------------------------------------------------------
//
//       Copyright (C) 2008 Frank Eskesen.
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
//       Drive all the tests.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Drive all the tests.
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
//       Test driver.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   System.out.println("SampleLogger");
   SampleLogger.main(null);
   TestChar.main(null);
}
} // Class Main


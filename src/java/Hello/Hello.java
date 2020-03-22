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
//       Hello.java
//
// Purpose-
//       Java bringup test.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Hello
//
// Purpose-
//       Java bringup test.
//
//----------------------------------------------------------------------------
class Hello
{
//----------------------------------------------------------------------------
//
// Method-
//       Hello.main
//
// Purpose-
//       Hello, Java world!
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   System.out.println("Hello Java world!");
   Test00.test();
   Test01.test();
   Test02.test();
   Test03.test();
}
} // Class Hello


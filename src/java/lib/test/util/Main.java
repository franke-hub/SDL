//----------------------------------------------------------------------------
//
//       Copyright (C) 2013 Frank Eskesen.
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
//       Generic test class.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
package test.util;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Test wrapper.
//
//----------------------------------------------------------------------------
class Main
{
//----------------------------------------------------------------------------
// Main.Attributes
//----------------------------------------------------------------------------
static final String[]  stringJulian= {"1946", "7", "30"};
static final String[]  stringNailuj= {"2432032"};

//----------------------------------------------------------------------------
//
// Method-
//       Main.main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   System.out.println("");
   System.out.println("LanguageTest");
   LanguageTest.main(args);

   System.out.println("");
   System.out.println("DateTimeTest");
   if( true )
     System.out.println("!!OMITTED!!");
   else
     DateTimeTest.main(args);

   System.out.println("");
   System.out.println("DebuggingTest");
   DebuggingTest.main(args);

   System.out.println("");
   System.out.println("HashVectorTest");
   HashVectorTest.main(args);

   System.out.println("");
   System.out.println("Julian");
   Julian.main(stringJulian);

   System.out.println("");
   System.out.println("Nailuj");
   Nailuj.main(stringNailuj);

   System.out.println("");
   System.out.println("StreamLoggerTest");
   StreamLoggerTest.main(args);

   System.out.println("");
   System.out.println("StringFormatTest");
   StringFormatTest.main(args);
}
} // Class Main


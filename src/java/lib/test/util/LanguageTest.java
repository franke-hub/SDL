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
//       LanguageTest.java
//
// Purpose-
//       Test the Debug class.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
package test.util;

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       LanguageTest
//
// Purpose-
//       Test the Debug class.
//
//----------------------------------------------------------------------------
class LanguageTest extends Debug {
//----------------------------------------------------------------------------
//
// Method-
//       LanguageTest.LanguageTest
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LanguageTest( )                  // Constructor
{  super();
}

//----------------------------------------------------------------------------
//
// Method-
//       LanguageTest.test0000
//
// Purpose-
//       Language tests
//
//----------------------------------------------------------------------------
public void
   test0000( )
   throws Exception
{
   Object       o= getNullObject();
   LanguageTest t= (LanguageTest)getNullObject();

   if( o instanceof LanguageTest )
     throw new Exception("NULL object fails instanceof");

   if( t instanceof LanguageTest )
     throw new Exception("NULL object fails instanceof");
}

//----------------------------------------------------------------------------
//
// Method-
//       LanguageTest.getNullObject
//
// Purpose-
//       Return a null Object
//
//----------------------------------------------------------------------------
public static Object                // Resultant
   getNullObject( )                 // Return a NULL object
{
   return null;
}

//----------------------------------------------------------------------------
//
// Method-
//       LanguageTest.main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   LanguageTest testcase= new LanguageTest();

   try {
     testcase.test0000();
     System.out.println("Alrighty, then");
   }
   catch( Throwable t ) {
     System.out.println("Exception: " + t);
     t.printStackTrace();
   }
}
} // Class LanguageTest


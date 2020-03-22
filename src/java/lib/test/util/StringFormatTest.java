//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       StringFormatTest.java
//
// Purpose-
//       Test the StringFormat class.
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
//       StringFormatTest
//
// Purpose-
//       Test the StringFormat class.
//
//----------------------------------------------------------------------------
class StringFormatTest extends Debug {
//----------------------------------------------------------------------------
// StringFormatTest.attributes
//----------------------------------------------------------------------------
StringFormat           object;      // The Object to test

//----------------------------------------------------------------------------
//
// Method-
//       StringFormatTest.StringFormatTest
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   StringFormatTest( )              // Constructor
{
   object= new StringFormat();
}

//----------------------------------------------------------------------------
//
// Method-
//       StringFormatTest.testValue
//
// Purpose-
//       Test one value.
//
//----------------------------------------------------------------------------
public void
   testValue(                       // Test one value
     int               value,       // The value to test
     int               fieldwidth,  // The fieldwidth
     int               precision)   // The precision
{
   object.reset();
   object.append(new String("" + value)).append("= ")
         .setRadix(10).append("Dec(")
                      .append(value,fieldwidth,precision).append(") ")
         .setRadix(16).append("Hex(")
                      .append(value,fieldwidth,precision).append(") ");

   debugln("" + object);
}

public void
   testValue(                       // Test one value
     int               value,       // The value to test
     int               fieldwidth)  // The fieldwidth
{
   testValue(value, fieldwidth, 0);
}

public void
   testValue(                       // Test one value
     int               value)       // The value to test
{
   testValue(value, 0, 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       StringFormatTest.test0000
//
// Purpose-
//       Test StringFormat
//
//----------------------------------------------------------------------------
public void
   test0000( )                      // Testcase driver
   throws Exception
{
   int               value;

   for(value= -20; value <=20; value++)
     testValue(value);

   testValue(0x80000000);
   testValue(0xfedcba98);
   testValue(0x76543210);
   testValue(0x01234567);
   testValue(0x89abcdef);
   testValue(0xfe000000,4);
   testValue(0x000000fe,4);
   testValue(0xfe000000,4,4);
   testValue(0x000000fe,4,4);
}

//----------------------------------------------------------------------------
//
// Method-
//       StringFormatTest.main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   StringFormatTest    testcase= new StringFormatTest();

   try {
     testcase.test0000();
   }
   catch( Throwable t ) {
     System.out.println("Exception: " + t);
     t.printStackTrace();
   }
}
} // Class StringFormatTest


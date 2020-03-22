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
//       HashVectorTest.java
//
// Purpose-
//       Test the HashVector class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package test.util;

import java.util.Enumeration;
import java.util.Vector;

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       HashVectorTest
//
// Purpose-
//       Test the HashVector class.
//
//----------------------------------------------------------------------------
class HashVectorTest
{
//----------------------------------------------------------------------------
// HashVectorTest.attributes
//----------------------------------------------------------------------------
   HashVector          object;      // The Object to test

//----------------------------------------------------------------------------
//
// Method-
//       HashVectorTest.HashVectorTest
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   HashVectorTest( )                // Constructor
{
   object= new HashVector();
}

//----------------------------------------------------------------------------
//
// Method-
//       HashVectorTest.showHash
//
// Purpose-
//       Display HashVector
//
//----------------------------------------------------------------------------
public void
   showHash(                        // Test HashVector
     String            key)         // Key string
{
   Enumeration         e;
   Vector              v;

   v= object.getVector(key);
   System.out.println("");
   System.out.println("Key: " + key);
   for(e= v.elements(); e.hasMoreElements(); )
   {
     System.out.println("Val: " + e.nextElement());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HashVectorTest.test0000
//
// Purpose-
//       Test HashVector
//
//----------------------------------------------------------------------------
public void
   test0000( )                      // Testcase driver
   throws Exception
{
   String            adj= new String("adjective");
   String            adverb= new String("adverb");
   String            article= new String("article");
   String            noun= new String("noun");
   String            prep= new String("preposition");
   String            verb= new String("verb");

   object.insert(new String("that"), adj);
   object.insert(new String("this"), adj);

   object.insert(new String("a"), article);
   object.insert(new String("an"), article);
   object.insert(new String("the"), article);

   object.insert(new String("to"), adverb);
   object.insert(new String("very"), adverb);

   object.insert(new String("other"), noun);
   object.insert(new String("ring"), noun);
   object.insert(new String("sort"), noun);
   object.insert(new String("this"), noun);
   object.insert(new String("that"), noun);

   object.insert(new String("of"), prep);
   object.insert(new String("for"), prep);

   object.insert(new String("be"), verb);
   object.insert(new String("is"), verb);
   object.insert(new String("ring"), verb);
   object.insert(new String("sort"), verb);

   showHash("a");
   showHash("bogus");
   showHash("other");
   showHash("ring");
   showHash("sort");
   showHash("that");
}

//----------------------------------------------------------------------------
//
// Method-
//       HashVectorTest.main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   HashVectorTest      testcase= new HashVectorTest();

   try {
     testcase.test0000();
   }
   catch( Throwable t ) {
     System.out.println("Exception: " + t);
     t.printStackTrace();
   }
}
} // Class HashVectorTest


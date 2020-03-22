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
//       DebuggingTest.java
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
//       ObjectArrayElement
//
// Purpose-
//       An ObjectArray Object
//
//----------------------------------------------------------------------------
class ObjectArrayElement extends MapDebugAdaptor {
//----------------------------------------------------------------------------
// ObjectArrayElement.Attributes
//----------------------------------------------------------------------------
Object                 element;     // The "Element"

//----------------------------------------------------------------------------
// ObjectArrayElement.Constructors
//----------------------------------------------------------------------------
public
   ObjectArrayElement(              // Constructor
     Object            element)     // Associated Object
{  super();
   this.element= element;
}

//----------------------------------------------------------------------------
// ObjectArrayElement.Methods (overrides MapDebug method)
//----------------------------------------------------------------------------
public void
   debug(                           // Mapped reference debugging display
     DebugMap          map)         // Associated DebugMap object
{
   super.debug(map);

   map.debug(".. element: ", element);
   if( element instanceof Object[] )
   {
     Object[] iA= (Object[])element;
     for(int i= 0; i<iA.length; i++)
     {
       Object iO= iA[i];
       map.debug(".... [" + i + "]: ", iO);
       if( iO instanceof Object[] )
       {
         Object[] jA= (Object[])iO;
         debugf(".... [" + i + "] length: " + jA.length + "\n");
         for(int j= 0; j<jA.length; j++)
         {
           Object jO= jA[j];
           map.debug(".... [" + i + "][" + j + "]: ", jO);
         }
       }
     }
   }
}
} // Class ObjectArrayElement

//----------------------------------------------------------------------------
//
// Class-
//       ObjectArray
//
// Purpose-
//       DebugMap test using an array of ObjectArrayElements
//
//----------------------------------------------------------------------------
class ObjectArray extends MapDebugAdaptor {
//----------------------------------------------------------------------------
// ObjectArray.attributes
//----------------------------------------------------------------------------
ObjectArrayElement[]   array;       // ObjectArrayElement array

//----------------------------------------------------------------------------
//
// Method-
//       ObjectArray.ObjectArray
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ObjectArray( )                   // Constructor
{  super();

   ObjectArrayElement foo= new ObjectArrayElement(new String("FOO"));
   ObjectArrayElement bar= new ObjectArrayElement(new String("BAR"));

   array= new ObjectArrayElement[] {
          new ObjectArrayElement(new String("first")),
          null,
          bar,
          new ObjectArrayElement(new Object[] {
              new String("first middle"),
              null,
              new ObjectArrayElement[] {
                  new ObjectArrayElement(new String("first inner")),
                  new ObjectArrayElement(this),
                  new ObjectArrayElement(new Debug()),
                  foo, bar,
                  new ObjectArrayElement(this),
                  new ObjectArrayElement(new String("last  inner"))
                  },
              new String("peni  middle"),
              new String("last  middle")
              }),
          new ObjectArrayElement(new String("this")),
          new ObjectArrayElement(new String("that")),
          null,
          new ObjectArrayElement(new String("other")),
          foo, foo, foo,
          new ObjectArrayElement(new String("last"))
          };
}

//----------------------------------------------------------------------------
// ObjectArray.Methods (override Debug methods)
//----------------------------------------------------------------------------
public void
   debug(                           // Memory reference debugging display
     DebugMap          map)         // Associated DebugMap object
{
   super.debug(map);

   debugf(".. array[" + array.length + "]\n");
   for(int i= 0; i<array.length; i++)
   {
     ObjectArrayElement e= array[i];
     map.debug(".. [" + i + "] ", e);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ObjectArray.show
//
// Purpose-
//       Display an object
//
//----------------------------------------------------------------------------
public static void
   show(                            // Display an object
     Object          o)             // The Object to display
   throws Exception
{
   Debug.debugf(o.toString()+"\n");
}
} // Class ObjectArray

//----------------------------------------------------------------------------
//
// Class-
//       DebuggingTest
//
// Purpose-
//       Test the Debug class.
//
//----------------------------------------------------------------------------
class DebuggingTest extends Debug {
//----------------------------------------------------------------------------
//
// Method-
//       DebuggingTest.DebuggingTest
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   DebuggingTest( )                 // Constructor
{  super();
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingTest.show
//
// Purpose-
//       Display an object
//
//----------------------------------------------------------------------------
public static void
   show(                            // Display an object
     Object          o)             // The Object to display
   throws Exception
{
   Debug.debugf(o.toString()+"\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingTest.test0000
//
// Purpose-
//       Test Debug methods
//
//----------------------------------------------------------------------------
public void
   test0000( )                      // Testcase driver
   throws Exception
{
   MapDebugAdaptor   mda= new MapDebugAdaptor();
   Object[]          o;

   System.out.println("Verify these outputs:");
   System.out.println("MDA.getObjectString: " + mda.getObjectString());
   System.out.println("MDA.getObjectSN:     " + mda.getObjectSN());
   System.out.println("MDA.getReference:    " + mda.getReference());

   debugf("Message to trace and to stdout\n");
   errorf("Message to trace and to stderr\n");
   printf("Message to stdout (only)\n");
   tracef("Message to trace (only)\n");

   o= new Object[] {
        new String("first"),
        new Object[] {
            new String("first middle"),
            null,
            new Object[] {
                new String("first inner"),
                this,
                new Debug(),
                this,
                new String("last  inner")
                },
            new String("peni  middle"),
            new String("last  middle")
            },
        new String("this"),
        new String("that"),
        new String("other"),
        new String("last")
      };
   show(o);
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingTest.test0001
//
// Purpose-
//       Test Debug, DebugMap
//
//----------------------------------------------------------------------------
public void
   test0001( )                      // Testcase driver
   throws Exception
{
   debugf("\n");
   debugf("test0001 started\n");

   DebugMap map= new DebugMap();
   ObjectArray array= new ObjectArray();
   show(array);

   map.debug(array);                // Load the map
   map.unwind();                    // Unwind (dump) the map, extending it
   debugf("test0001 complete\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingTest.test9999
//
// Purpose-
//       Test Debug stack trace methods
//
//----------------------------------------------------------------------------
public void
   test9999( )                      // Testcase driver
   throws Exception
{
   debugf("\n");
   debugf("test9999 started, throws RuntimeException\n");

   throw new RuntimeException("Testing completed normally");
// Debug.shouldNotOccur();          // Compiler detects unreachable
}

//----------------------------------------------------------------------------
//
// Method-
//       DebuggingTest.main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   DebuggingTest testcase= new DebuggingTest();

   try {
     testcase.test0000();
     testcase.test0001();
     testcase.test9999();
   }
   catch( Throwable t ) {
     System.out.println("Exception: " + t);
     t.printStackTrace();
   }
}
} // Class DebuggingTest


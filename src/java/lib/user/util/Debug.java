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
//       Debug.java
//
// Purpose-
//       Standard debugging methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util;

import java.io.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Debug
//
// Purpose-
//       Standard debugging methods.
//
//----------------------------------------------------------------------------
public class Debug {                // Debugging Object
//-------------------------------------------------------------------------
// Debug.Attributes
//-------------------------------------------------------------------------
static public boolean  HCDM = false;// Hard Core Debug Mode?
static public boolean  SCDM = false;// Soft Core Debug Mode?
static public boolean  INFO = true; // Verbose Mode?

//-------------------------------------------------------------------------
//
// Method-
//    Debug.Debug
//
// Purpose-
//    Constructor.
//
//-------------------------------------------------------------------------
public
   Debug( )                         // Default constructor
{
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.printf
//
// Purpose-
//       Debugging display.
//
//-------------------------------------------------------------------------
public static synchronized void
   printf(                          // Debugging display
     String            string)      // String
{
   System.out.print(string);
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.debugf
//
// Purpose-
//       Debugging display.
//
//-------------------------------------------------------------------------
public static synchronized void
   debugf(                          // Debugging display
     String            string)      // String
{
   System.out.print(string);
   tracef(string);
}

public static synchronized void
   debugln(                         // Debugging display (Add "\n")
     String            string)      // String
{
   debugf(string + "\n");
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.errorf
//
// Purpose-
//       Debugging error display.
//
//-------------------------------------------------------------------------
public static synchronized void
   errorf(                          // Debugging display
     String            string)      // String
{
   System.err.print(string);
   tracef(string);
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.tracef
//
// Purpose-
//       Debugging trace.
//
//-------------------------------------------------------------------------
public static synchronized void
   tracef(                          // Debugging display
     String            string)      // String
{
   BufferedWriter      writer;

   try {
     writer=  new BufferedWriter(
                 new FileWriter("Debug.log", true)
                 );
     writer.write(string);
     writer.close();
   } catch(Exception e) {
     System.err.println("Error writing: 'Debug.log'");
     System.err.print(string);
   }
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.getClassName
//
// Purpose-
//       Extract the class name.
//
//-------------------------------------------------------------------------
public static String                // The Class name
   getClassName(                    // Get the Class name
     Object            object)      // For this object
{
   if( object == null )
     return("NULL");

   return(object.getClass().getName());
}

public String                       // The Class name
   getClassName( )                  // Get the Class name
{
   return getClassName(this);
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.getObjectString
//
// Purpose-
//       Extract the object toString().
//
//-------------------------------------------------------------------------
public String                       // The Object toString
   getObjectString( )               // Get Object toString
{
   return super.toString();
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.debugException
//
// Purpose-
//       Print all the Exception information.
//
//-------------------------------------------------------------------------
public static void
   debugException(                  // Display Exception information
     Exception         e)           // For this Exception
{
   debugf("Exception: " + e.toString() + "\n");
   e.printStackTrace();
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.describe
//
// Purpose-
//       Describe an Object.
//
//-------------------------------------------------------------------------
public static String                // Description
   describe(                        // Get Object description
     Object            object)      // For this object
{
   if( object == null )
     return("NULL");

   return("Class(" + getClassName(object) +
        "),String(" + object.toString() + ")");
}

public String                       // Description
   describe( )                      // Get Object description
{
   return describe(this);
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.shouldNotOccur
//
// Purpose-
//       A "should not occur" condition did.
//
//-------------------------------------------------------------------------
public static void
   shouldNotOccur( )                // Should not occur
{
   try {
     throw new RuntimeException("Should not occur");
   } catch(RuntimeException e) {
     debugException(e);
     throw e;
   }
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.toHex
//
// Purpose-
//       Convert address to hexidecimal representation.
//
//-------------------------------------------------------------------------
public static String                // Resultant
   toHex(                           // Convert integer to hexidecimal
     int               value)       // The integer
{
   StringFormat        format= new StringFormat();

   return format.setRadix(16).append(value,8,8).toString();
}

public static String                // Resultant
   toHex(                           // Convert long to hexidecimal
     long              value)       // The long
{
   StringFormat        format= new StringFormat();

   return format.setRadix(16).append(value,16,16).toString();
}

//-------------------------------------------------------------------------
//
// Method-
//       Debug.verify
//
// Purpose-
//       Verify that some condition is true.
//
// Notes-
//       Changed from assert() for 1.4 compatibility.
//
//-------------------------------------------------------------------------
public static void
   verify(                          // Verify validity
     boolean           expression)  // Boolean expression
{
   if( expression )
     return;

   try {
     throw new RuntimeException("Verify failure");
   } catch(RuntimeException e) {
     debugException(e);
     throw e;
   }
}
} // class Debug


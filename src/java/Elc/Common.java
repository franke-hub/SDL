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
//       Common.java
//
// Purpose-
//       Define the Common singleton Object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;
import user.util.logging.*;

//----------------------------------------------------------------------------
//
// Class-
//       Common
//
// Purpose-
//       The Common singleton Object
//
//----------------------------------------------------------------------------
class Common                        // Common data
{
//----------------------------------------------------------------------------
// Common.Controls
//----------------------------------------------------------------------------
final static String    prefix= "ELC";
final static String    propName= prefix + "_common.pro";

//----------------------------------------------------------------------------
// Common.Attributes (external)
//----------------------------------------------------------------------------
ObjectStore            db;          // The Object Store database
StreamLogger           logger;      // The Logger

static Object          singleton= new Object(); // A synchronization Object
                                    // used in the creation of Singletons

//----------------------------------------------------------------------------
// Common.Attributes (internal)
//----------------------------------------------------------------------------
private static Common  common;      // The Common singleton Object

//----------------------------------------------------------------------------
//
// Method-
//       Common.Common
//
// Purpose-
//       Private constructor
//
//----------------------------------------------------------------------------
private
   Common( )                        // Private constructor
{
   Exception           exception= null; // Captured Exception
   Properties          properties= new Properties(); // The Common Properties

   //-------------------------------------------------------------------------
   // Get Properties
   try
   {
     FileInputStream inp= new FileInputStream(propName);
     properties.load(inp);
   } catch(Exception x) {
     exception= x;
   }

   //-------------------------------------------------------------------------
   // Set Logger
   {{{{
     String name= properties.getProperty("Common.Logger", prefix + "_logger");
     logger= new StreamLogger(name);
     logger.log("Logging started");

     if( exception != null )
     {
       if( exception instanceof FileNotFoundException )
         logger.log("Common.Properties(" + propName + ") not found");
       else
         logger.log("Common.Properties(" + propName + ")", exception);
     }
   }}}}

   //-------------------------------------------------------------------------
   // Set Common; Required for initialization of Objects below.
   common= this;

   //-------------------------------------------------------------------------
   // Set ObjectStore
   {{{{
     String prop= properties.getProperty("ObjectStore.Properties", propName);
     db= new ObjectStore(prop);
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Common.check
//
// Purpose-
//       Check common objects
//
//----------------------------------------------------------------------------
void
   check( )                         // Check Common objects
{
   db.check();
}

//----------------------------------------------------------------------------
//
// Method-
//       Common.close
//
// Purpose-
//       Finalize the Common
//
//----------------------------------------------------------------------------
void
   close( )                         // Close the Common
{
   //-------------------------------------------------------------------------
   // Log Common values
   logger.log("Common.close()");

   //-------------------------------------------------------------------------
   // Remove the Singleton
   check();
   db= null;
   common= null;
   logger= null;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common.get
//
// Purpose-
//       Check the Object Store.
//
//----------------------------------------------------------------------------
public static Common                // The Common singleton Object
   get( )                           // Get the Common singleton
{
   if( common == null )
   {
     synchronized (singleton)
     {{{{
       if( common == null )
         common= new Common();
     }}}}
   }

   return common;
}
}; // class Common


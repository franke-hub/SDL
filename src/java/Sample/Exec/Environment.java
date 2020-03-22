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
//       Environment.java
//
// Purpose-
//       Extract the environment properties.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Environment
//
// Purpose-
//       Java testcase driver.
//
//----------------------------------------------------------------------------
public class Environment
{
//----------------------------------------------------------------------------
//
// Method-
//       Environment.getProperties
//
// Purpose-
//       Extract the environment Properties
//
//----------------------------------------------------------------------------
protected static Properties         // Environment properties
   tryProperties( )                 // Get Properties
   throws Exception
{
   Runtime             runtime= Runtime.getRuntime();
   Process             process;
   BufferedReader      reader;
   Properties          properties;
   String              line;

   //-------------------------------------------------------------------------
   // Run the command
   //-------------------------------------------------------------------------
   process= runtime.exec("env");       // Run the command

   //-------------------------------------------------------------------------
   // Pull the input into the properties
   //-------------------------------------------------------------------------
   properties= new Properties();
   reader= new BufferedReader(new InputStreamReader(process.getInputStream()));
   for(;;)
   {
     line= reader.readLine();
     if( line == null )
       break;

     if( line.length() == 0 )
       continue;

     if( (line.charAt(0) >= 'a' && line.charAt(0) <= 'z')
         || (line.charAt(0) >= 'A' && line.charAt(0) <= 'Z') )
     {
       int split= line.indexOf('=');
       if( split > 0 )
       {
         String name= line.substring(0, split);
         String prop= line.substring(split+1);
         properties.setProperty(name,prop);
       }
     }
   }
   process.waitFor();

   //-------------------------------------------------------------------------
   // Return the properties
   //-------------------------------------------------------------------------
   return properties;
}

public static Properties            // Environment properties
   getProperties( )                 // Get Properties
{
   Properties          properties;

   properties= null;
   try {
     properties= tryProperties();
   } catch( Exception e ) {
     System.err.println("Failed to get Properties: " + e);
     e.printStackTrace();
     throw new RuntimeException("Botched");
   }

   return properties;
}

//----------------------------------------------------------------------------
//
// Method-
//       Environment.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   Properties          properties;
   String[]            defaultArgs= {"PWD", "PATH", "ERROR"};

   if( args.length == 0 )
     args= defaultArgs;

   properties= getProperties();
   for(int i= 0; i<args.length; i++)
   {
     System.out.println("[" + args[i] + "] '"
                        + properties.getProperty(args[i]) + "'");
   }
}
} // Class Environment


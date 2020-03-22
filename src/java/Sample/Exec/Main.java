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
//       Main.java
//
// Purpose-
//       Sample code to demonstrate Runtime.exec().
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Java testcase driver.
//
//----------------------------------------------------------------------------
public class Main
{
//----------------------------------------------------------------------------
//
// Method-
//       Main.main
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
   Runtime             runtime= Runtime.getRuntime();
   Process             doit;
   InputStream         output;

   //-------------------------------------------------------------------------
   // Run the command
   //-------------------------------------------------------------------------
   doit= runtime.exec("env");       // Run the command

   //-------------------------------------------------------------------------
   // Note: The command operates silently (no output) unless it is explicitly
   //       displayed.  The following code displays the command output...
   //-------------------------------------------------------------------------
   output= doit.getInputStream();
   System.out.println("Output:");
   for(;;)
   {
     int inp= output.read();
     if( inp < 0 )
       break;

     System.out.print((char)inp);
   }
   doit.waitFor();                  // Wait for the command to complete
   System.out.println("did it, code " + doit.exitValue()); // It's done
}
} // Class Main


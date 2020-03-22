//----------------------------------------------------------------------------
//
//       Copyright (C) 2008 Frank Eskesen.
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
//       Sample RFC 977 Network News Transfer Protocol (NNTP) server.
//       Includes RFC 2980 extensions.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.util.*;

public class Main
{
//----------------------------------------------------------------------------
// Main.Constants for parameterization
//----------------------------------------------------------------------------
final int              PORT_NUMBER= 65025;

//----------------------------------------------------------------------------
// Main.Attributes
//----------------------------------------------------------------------------
int                    port;        // The listener port
Server                 server[];    // Server List

//----------------------------------------------------------------------------
//
// Method-
//       Main.Main
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Main(                            // Constructor
     String            args[])      // Argument list
{
   server= new Server[10];
   for(int i= 0; i<server.length; i++)
     server[i]= null;

   port= PORT_NUMBER;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.debugf
//
// Purpose-
//       Debugging printf.
//
//----------------------------------------------------------------------------
public static void
   debugf(
     String            string)
{
   System.out.print("Main: " + string);
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.server
//
// Purpose-
//       Drive Server threads.
//
//----------------------------------------------------------------------------
public void
   server( )
   throws Exception
{
   ServerSocket        listen;      // Listener Socket

   int                 i;

   // Status message

   // Finalize any threads
   for(i= 0; i<server.length; i++)
   {
     if( server[i] != null && server[i].fsm == Server.FSM_DONE )
     {
       debugf("[" + i + "] Complete\n");
       server[i].join();
       server[i]= null;
     }
   }

   // Find an available thread
   for(i= 0; i<server.length; i++)
   {
     if( server[i] == null )
     {
       debugf("[" + i + "] Ready\n");
       listen= new ServerSocket(port); // Create the Listener
       Socket talk= listen.accept(); // Accept the first connection
       server[i]= new Server(i, talk);
       server[i].start();
       listen.close();
       debugf("[" + i + "] Selected\n");
       break;
     }
   }

   if( i >= server.length )
   {
     debugf("Busy\n");
     Thread.sleep(30000);
   }
}

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
   Main                main= new Main(args);

   for(;;)
   {
     try {
       main.server();
     } catch(Exception e) {
     }
   }
}
} // Class Main


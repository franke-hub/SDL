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
//       Wilbur.java
//
// Purpose-
//       Wilbur data gatherer.
//
// Last change date-
//       2007/01/01
//
// Parameter options-
//       --hcdm         Hard Core Debug Mode
//       --iodm         I/O Debug Mode
//       --scdm         Soft Core Debug Mode
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Wilbur
//
// Purpose-
//       Wilbur tests.
//
//----------------------------------------------------------------------------
class Wilbur extends Debug {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
boolean                HCDM= false; // Hard Core Debug Mode?
boolean                IODM= false; // I/O Debug Mode?
boolean                SCDM= false; // Soft Core Debug Mode?

//----------------------------------------------------------------------------
//
// Method-
//       Wilbur.init
//
// Purpose-
//       Initialize parameters.
//
//----------------------------------------------------------------------------
public void
   init(                            // Initialize parameters
     String[]          args)        // Argument array
   throws Exception
{
   System.out.println("Starting Wilbur...");

   for(int i= 0; i<args.length; i++)
   {
     if( args[i].equalsIgnoreCase("--hcdm") )
       HCDM= true;
     else if( args[i].equalsIgnoreCase("--iodm") )
       IODM= true;
     else if( args[i].equalsIgnoreCase("--scdm") )
       SCDM= true;
     else
       throw new Exception("Invalid argument(" + args[i] + ")");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Wilbur.wilbur
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public void
   wilbur(                          // Mainline code
     String[]          args)        // Argument array
   throws Exception
{
   ClientThread client= new ClientThread("Client"); // Client thread
   ServerThread server= new ServerThread("Server"); // Server thread

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   init(args);

   //-------------------------------------------------------------------------
   // Start the threads
   //-------------------------------------------------------------------------
   server.start();
   client.start();

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   server.join();
   client.join();
}

//----------------------------------------------------------------------------
//
// Method-
//       Wilbur.main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   Wilbur wilbur= new Wilbur();

   try {
     wilbur.wilbur(args);
   } catch(Exception X) {
     debugException(X);
   }
}
} // class Wilbur


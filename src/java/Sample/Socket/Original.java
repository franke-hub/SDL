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
//       Original.java
//
// Purpose-
//       Sample client or server.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.util.*;
import user.util.*;

public class Original
{
//----------------------------------------------------------------------------
// Original.Constants for parameterization
//----------------------------------------------------------------------------
public boolean         unused       = true; // Not used yet

public String          message[]=
   { "start 5 message 1"
   , "start 5 message 2"
   , "wait"
   , "invalid command"
   , "start xx invalid count"
   , "start 50 dynamic parameters"
   , "wait"
   };

//----------------------------------------------------------------------------
// Original.Attributes
//----------------------------------------------------------------------------
public boolean         isClient     = true; // TRUE iff client
public boolean         isVerbose    = true; // TRUE iff verbose mode
public String          hostName     = null; // The name of the target host
public int             port         = 0;    // The target port

public Socket          talk         = null; // Talk Socket
public BufferedReader  reader       = null; // Socket input stream
public PrintWriter     writer       = null; // Socket output stream

//----------------------------------------------------------------------------
//
// Method-
//       Original.Original
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Original(
     String            args[])      // Argument list
   throws Exception
{
   isClient=  true;                 // Set the defaults
   hostName=  "127.0.0.1";
   port=      65025;

   if( args.length > 0 )            // [0] client | server
   {
     if( args[0].equals("client") )
       isClient= true;

     else if( args[0].equals("server") )
       isClient= false;

     else
       info();
   }
   else
     info();

   if( args.length > 1 )            // [1] hostName
     hostName= args[1];

   if( args.length > 2 )            // [2] Port number
     port= Integer.parseInt(args[2]);
}

//----------------------------------------------------------------------------
//
// Method-
//       Original.info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
public static void
   info( )
   throws Exception
{
   System.out.println("Sample socket application");
   System.out.println("");
   System.out.println("java Original {client|server} <hostname <hostport> >");
   System.exit(1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Original.send
//
// Purpose-
//       Send message on Socket.
//
//----------------------------------------------------------------------------
public void
   send(
     String            message)     // The message
   throws Exception
{
   writer.println(message);
   writer.flush();

   if( isVerbose )
   {
     if( isClient )
       System.out.println("Client wrote: '" + message + "'");
     else
       System.out.println("Server wrote: '" + message + "'");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Original.receive
//
// Purpose-
//       Receive message answer on Socket.
//
//----------------------------------------------------------------------------
public String                       // The response message
   receive( )
   throws Exception
{
   String              result;      // Resultant

   result= reader.readLine();
   if( isVerbose )
   {
     if( isClient )
       System.out.println("Client  read: '" + result + "'");
     else
       System.out.println("Server  read: '" + result + "'");
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Original.client
//
// Purpose-
//       Client.
//
//----------------------------------------------------------------------------
public void
   client( )
   throws Exception
{
   String              string;      // Working String
   Date                start;       // Timing start
   Date                finis;       // Timing finish

   int                 i;

   talk= new Socket(hostName, port); // Create the Socket
   reader= new BufferedReader(
           new InputStreamReader(talk.getInputStream())
                             );
   writer= new PrintWriter(
           new BufferedWriter(
           new OutputStreamWriter(talk.getOutputStream())
                          )  );

   //-------------------------------------------------------------------------
   send("TEST");
   string= "";
   while( !string.equals("OK") )
     string= receive();

   //-------------------------------------------------------------------------
   for(i= 0; i<message.length; i++)
   {
     send(message[i]);
     receive();
   }

   //-------------------------------------------------------------------------
   send("TIME");
   isVerbose= false;
   start= new Date();
   for(i= 0; i<100000; i++)
     send(new String("Message " + i));
   send(".");
   string= receive();
   finis= new Date();
   isVerbose= true;

   System.out.println("Client: read: " + string);
   System.out.println("Client: Time: " + (double)(finis.getTime() - start.getTime())/1000.0 + " seconds");
   System.out.println("Client: Complete");

   talk.close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Original.server
//
// Purpose-
//       Server.
//
//----------------------------------------------------------------------------
public void
   server( )
   throws Exception
{
   ServerSocket        server;      // Connection Socket
   String              response;    // Output String
   String              string;      // Input String

   int                 i;

   server= new ServerSocket(port);  // Create the Listener
   System.out.println("Ready");

   talk= server.accept();           // Accept the first connection
   reader= new BufferedReader(
           new InputStreamReader(talk.getInputStream())
                             );
   writer= new PrintWriter(
           new BufferedWriter(
           new OutputStreamWriter(talk.getOutputStream())
                          )  );
   for(i= 0;;i++)
   {
     string= receive();
     if( string == null )
       break;

     response= "OK";
     if( string.equals("TEST") )
     {
       StringBuffer sb= new StringBuffer();

       for(int x= 0; x<256; x++)
       {
         char c= (char)x;
         sb.append(c);
       }

       writer.println(sb.toString());
       writer.flush();
     }

     if( string.equals("TIME") )
     {
       isVerbose= false;
       response= "<done>";
       for(;;)
       {
         string= receive();
         if( string == null )
           break;

         if( string.equals(".") )
           break;
       }
       isVerbose= true;
     }

     send(response);
   }

   System.out.println("Server: End of file");
   System.out.println("Server: Complete");

   talk.close();
   server.close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Original.main
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
   Original            main= new Original(args);

   if( main.isClient )
     main.client();
   else
     main.server();

   System.gc();
   System.runFinalization();
   System.gc();
}
} // Class Main


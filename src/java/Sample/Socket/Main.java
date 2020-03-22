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

public class Main
{
//----------------------------------------------------------------------------
// Main.Constants for parameterization
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
// Main.Attributes
//----------------------------------------------------------------------------
public boolean         isClient     = true; // TRUE iff client
public boolean         isVerbose    = true; // TRUE iff verbose mode
public String          hostName     = null; // The name of the target host
public int             port         = 0;    // The target port

public Socket          talk         = null; // Talk Socket
public BufferedInputStream
                       reader       = null; // Socket input stream
public BufferedOutputStream
                       writer       = null; // Socket output stream

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
   Main(
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
//       Main.info
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
   System.out.println("java Main {client|server} <hostname <hostport> >");
   System.exit(1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.send
//
// Purpose-
//       Send message on Socket.
//
//----------------------------------------------------------------------------
public void
   send(
     byte[]            message,     // The message buffer
     int               length)      // The message length
   throws Exception
{
   writer.write(message, 0, length);
   writer.flush();
}

public void
   send(
     String            message)     // The message
   throws Exception
{
   int                 length= message.length();
   byte[]              buffer= new byte[length+1];

   for(int i= 0; i<length; i++)
     buffer[i]= (byte)message.charAt(i);
   buffer[length]= '\n';

   send(buffer, length+1);

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
//       Main.receive
//
// Purpose-
//       Receive message answer on Socket.
//
//----------------------------------------------------------------------------
public String                       // The response message
   receive( )
   throws Exception
{
   StringBuffer        result= new StringBuffer(); // Resultant

   for(;;)
   {
     int C= reader.read();
     if( C == '\n' )
       break;

     if( C < 0 )
     {
       if( result.length() == 0 )
         return null;

       break;
     }

     if( C != '\r' )
       result.append((char)C);
   }

   if( isVerbose )
   {
     if( isClient )
       System.out.println("Client  read: '" + result + "'");
     else
       System.out.println("Server  read: '" + result + "'");
   }

   return result.toString();
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.client
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
   reader= new BufferedInputStream(talk.getInputStream());
   writer= new BufferedOutputStream(talk.getOutputStream());

   //-------------------------------------------------------------------------
   send("TEST");
   System.out.print("===============>");
   int prior= 0;
   byte[] buffer= new byte[1];
   for(;;)
   {
     int C= reader.read();
     if( prior == 'O' )
     {
       if( C == 'K' )
         break;

       System.out.write('O');
     }
     prior= C;

     if( prior != 'O' )
     {
       buffer[0]= (byte)C;
       System.out.write(buffer, 0, 1);
     }
   }
   System.out.println("<===============");

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
//       Main.server
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
   reader= new BufferedInputStream(talk.getInputStream());
   writer= new BufferedOutputStream(talk.getOutputStream());

   for(i= 0;;i++)
   {
     string= receive();
     if( string == null )
       break;

     response= "OK";
     if( string.equals("TEST") )
     {
       byte[] ba= new byte[256];

       for(int x= 0; x<256; x++)
         ba[x]= (byte)x;

       send(ba, 256);
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

   if( main.isClient )
     main.client();
   else
     main.server();

   System.gc();
   System.runFinalization();
   System.gc();
}
} // Class Main


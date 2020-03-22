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
//       Sample MulticastSocket client or server.
//
// Last change date-
//       2007/01/01
//
// Notes (port number)-
//       In order to send a datagram, both the address and port number must be
//       specified.  Only recipients who have matching datagram ports open will
//       see group messages sent to a port.  In that respect, each port is like
//       a separate group.
//
//       For this sample, the client port and server port need not differ.  If
//       they are the same the server sees all the messages it writes and the
//       client sees all the join messages.
//
// Notes (multicast)-
//       There is no restriction on running multiple "clients" or "servers"
//       on one or more machines.  More applications result in more messages.
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
static final String    groupAddress= "224.9.0.8"; // Group multicast address

static final String    hello= "JOIN: ";
static final String    message[]=
   { "Server message 1 - first"
   , "Server message 2"
   , "Middle message"
   , "Server message 4"
   , "Server message 5 - last"
   };

//----------------------------------------------------------------------------
// Main.Attributes
//----------------------------------------------------------------------------
public Date            startTime;   // Application start time
public boolean         isClient=    true; // TRUE iff client
public boolean         isVerbose=   true; // TRUE iff verbose mode

public int             clientPort= 12345; // Client port number
public int             serverPort= 54321; // Server port number
public long            activeTime=     0; // Run time (milliseconds)

public MulticastSocket talk;        // DatagramSocket
public InetAddress     addr;        // InternetAddress

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
   isVerbose= true;
   startTime= new Date();

   if( args.length > 0 )            // [0] client | server
   {
     if( args[0].equals("client") )
       isClient= true;

     else if( args[0].equals("server") )
       isClient= false;

     else
       info();
   }

   if( args.length > 1 )            // [1] runtime (in seconds)
     activeTime= Long.parseLong(args[1]) * 1000;

   if( args.length > 2 )            // [2] client-port
     clientPort= Integer.parseInt(args[2]);

   if( args.length > 3 )            // [3] server-port
     serverPort= Integer.parseInt(args[3]);
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
   System.out.println("Sample MulticastSocket application");
   System.out.println("");
   System.out.println("java Main {server|client} "
                    + "<<runtime-seconds> "
                    + "<<client-port> "
                    + "<server-port>> >");
   System.out.println("");
   System.out.print(
     "Usage:\n"
   + "1) Start the server application on one host\n"
   + "2) On this or other hosts (singly or multiply) start the client\n"
   + "\n"
   + "Each joining application will cause the server to broadcast its list\n"
   + "of static messages and each client (except for the new one) to broadcast\n"
   + "an \"I'm here too\" message\n"
   + "Clients write all messages received\n"
   + "\n"
   + "Servers and clients remain active for 5 minutes.\n"
   );
   System.exit(1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.send
//
// Purpose-
//       Send Datagram.
//
//----------------------------------------------------------------------------
public void
   send(                            // Send a Datagram
     Datagram          source)      // The Datagram
   throws Exception
{
   talk.setTimeToLive(1);
   talk.send(source.get());

   if( isVerbose )
   {
     if( isClient )
       System.out.println("Client wrote: '" + source + "'");
     else
       System.out.println("Server wrote: '" + source + "'");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.receive
//
// Purpose-
//       Receive datagram on Socket.
//
//----------------------------------------------------------------------------
public Datagram                     // Datagram
   receive(                         // Receive datagram
     Datagram          result)      // Source/result datagram
   throws Exception
{
   talk.setSoTimeout(3000);
   try {
     talk.receive(result.get());
   } catch( SocketTimeoutException e ) {
     result= null;
   }

   if( isVerbose && result != null )
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
   Datagram            dg;          // Working Datagram
   String              iam;         // WHOAMI String

   talk= new MulticastSocket(clientPort);
   addr= InetAddress.getByName(groupAddress);
   iam= InetAddress.getLocalHost().toString();
   talk.joinGroup(addr);
   System.out.println(" CLIENT: " + iam);
   System.out.println("   ADDR: " + groupAddress + ":" + clientPort);
   System.out.println(" SERVER: " + groupAddress + ":" + serverPort);
   if( activeTime == 0 )
     activeTime= 30000;

   // Join the conversation
   dg= new Datagram(hello + iam);
   dg.set(addr,serverPort);
   send(dg);

   // Client loop
   dg= new Datagram();
   dg.set(addr, clientPort);
   for(;;)
   {
     receive(dg);
     Date now= new Date();
     if( (now.getTime() - startTime.getTime()) >= activeTime )
       break;
   }

   talk.leaveGroup(addr);
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
   Datagram            dg;          // Working Datagram
   String              iam;         // WHOAMI String
   String              string;      // Working String

   talk= new MulticastSocket(serverPort);
   addr= InetAddress.getByName(groupAddress);
   iam= InetAddress.getLocalHost().toString();
   talk.joinGroup(addr);
   System.out.println(" SERVER: " + iam);
   System.out.println("   ADDR: " + groupAddress + ":" + serverPort);
   System.out.println(" CLIENT: " + groupAddress + ":" + clientPort);
   if( activeTime == 0 )
     activeTime= 300000;

   // Server loop
   Datagram recvDG= new Datagram();
   recvDG.set(addr, serverPort);
   for(;;)
   {
     dg= receive(recvDG);
     Date now= new Date();
     if( (now.getTime() - startTime.getTime()) >= activeTime )
       break;

     if( dg == null )
       continue;

     string= dg.toString();
     if( string.startsWith(hello) )
     {
       for(int i= 0; i<message.length; i++)
       {
         dg= new Datagram(message[i]);
         dg.set(addr,clientPort);
         send(dg);
       }
     }
   }

   talk.leaveGroup(addr);
   talk.close();
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

class Datagram {
//----------------------------------------------------------------------------
//
// Class-
//       Datagram
//
// Purpose-
//       Simplified DatagramPacket -- easier to use.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Datagram.Attributes
//----------------------------------------------------------------------------
DatagramPacket         dg= new DatagramPacket(new byte[4096], 4096);

//----------------------------------------------------------------------------
//
// Method-
//       Datagram.Datagram
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   Datagram( )                      // Constructor
{
}

public
   Datagram(                        // Constructor
     String            string)      // From String
{
   dg= new DatagramPacket(string.getBytes(), string.length());
}

//----------------------------------------------------------------------------
//
// Method-
//       Datagram.get
//
// Purpose-
//       Get the underlying DatagramPacket
//       (Because DatagramPacket is foolishly final.)
//
//----------------------------------------------------------------------------
public DatagramPacket               // Resultant
   get( )                           // Get DatagramPacket
{
   return dg;
}

//----------------------------------------------------------------------------
//
// Method-
//       Datagram.set
//
// Purpose-
//       Set methods.
//
//----------------------------------------------------------------------------
public void
   set(                             // Set Address and port
     InetAddress       address,     // The Address
     int               port)        // The port
{
   dg.setAddress(address);
   dg.setPort(port);
}

//----------------------------------------------------------------------------
//
// Method-
//       Datagram.toString
//
// Purpose-
//       String representation: includes address and message
//
//----------------------------------------------------------------------------
public String                       // Resultant
   toString( )                      // Convert to String
{
   return new String(dg.getData(), dg.getOffset(), dg.getLength());
}
} // Class Main


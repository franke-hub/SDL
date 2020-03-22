//----------------------------------------------------------------------------
//
//       Copyright (C) 2016 Frank Eskesen.
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
//       Man-in-the middle server.
//
// Last change date-
//       2016/01/01
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.util.*;

import user.util.*;
import user.util.logging.*;

public class Main {
//----------------------------------------------------------------------------
// Main.Constants for parameterization
//----------------------------------------------------------------------------
final static String    LOGGER_NAME= "logger";

final static int       INPUT_PORT=  9191;
// al static String    OUTPUT_HOST= "192.168.1.240:3030"; // Jena
final static String    OUTPUT_HOST= "192.168.1.240:7777"; // Wilbur/Brian
// al static String    OUTPUT_HOST= "192.168.1.240:8080"; // Tomcat

//----------------------------------------------------------------------------
// Main.Attributes
//----------------------------------------------------------------------------
StreamLogger           logger;      // The logger
int                    ident;       // Connection identifier

String                 output_host; // The host we are connecting to
int                    output_port; // The port we are connecting to

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
   logger= new StreamLogger(LOGGER_NAME);
   logger.log("Logging started");

   output_host= OUTPUT_HOST;
   if( args.length > 0 )
     output_host= args[0];

   int x= output_host.indexOf(":");
   if( x <= 0 )
   {
     debugf("Invalid parameter: '" + output_host + "', no \":\"");
     throw new RuntimeException("Invalid parameter");
   }

   output_port= Integer.parseInt(output_host.substring(x+1));
   output_host= output_host.substring(0, x);

   ident= 0;
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
public void
   debugf(
     String            string)
{
   logger.log("Main: " + string);
   System.out.println("Main: " + string);
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.run
//
// Purpose-
//       Drive Server threads.
//
//----------------------------------------------------------------------------
public void
   run(                             // Run Reader/Writer
     ServerSocket      listen)      // The Listener Socket
   throws Exception
{
   debugf("Waiting for connection: " + listen.getLocalPort() +
          " for '" + output_host + ":" + output_port + "'");
   Socket inp= listen.accept();     // Accept the first connection
   Socket out= new Socket(output_host, output_port); // Create secondary

   ident++;
   String identString= String.format("%2d ", ident);
   debugf("listen.accept: " + identString + " "
          + "H(" + inp.getLocalAddress() + ":" + inp.getLocalPort() +  ") "
          + "N(" + out.getInetAddress() + ":" + output_port + ")");

   Middle reader= new Middle(logger, identString + "H>>>>N: ", inp, out);
   Middle writer= new Middle(logger, identString + "H<<<<N: ", out, inp);

   if( true  ) // Test new code
   {
     Waiter w= new Waiter(logger, identString, reader, writer, inp, out);
     w.setDaemon(true);
     w.start();
     w= null;
   }
   else  // Run working old code
   {
     reader.start();
     writer.start();

     reader.join();
     writer.join();

     inp.close();
     out.close();
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

   ServerSocket listen= new ServerSocket(INPUT_PORT); // Create the Listener
   for(;;)
   {
     try {
       main.run(listen);
     } catch(Exception e) {
       main.debugf("Exception: " + e);
     }
   }
}
} // Class Main

//----------------------------------------------------------------------------
//
// Class-
//       Waiter
//
// Purpose-
//       During bringup, the Middle threads were made daemons and the code
//       stopped working. This was an attempt to fix this, but it turned out
//       that the real problem was the listener port was duplicated.
//       This seems to help correlate the closing of the input and output
//       sockets a little bit better.
//
//----------------------------------------------------------------------------
class Waiter extends Thread {
StreamLogger           logger;      // Logger
String                 prefix;      // Message prefix

Middle                 reader;
Middle                 writer;
Socket                 inp;
Socket                 out;

public void
   debugf(
     String            string)
{
   logger.log(prefix + string);
   System.out.println(prefix + string);
}


public
   Waiter(
     StreamLogger      logger,      // The logger
     String            prefix,      // Message prefix
     Middle            reader,
     Middle            writer,
     Socket            inp,
     Socket            out)
{
   this.logger= logger;
   this.prefix= prefix + "Waiter: ";

   this.reader= reader;
   this.writer= writer;
   this.inp= inp;
   this.out= out;
}

public void
   run( )
{
debugf("started");
   try {
     reader.start();
     writer.start();

     reader.join();
     writer.join();

     inp.close();
     out.close();
   } catch(Exception e) {
     debugf("Exception: " + e);
   }
debugf("complete");
}
} // class Waiter


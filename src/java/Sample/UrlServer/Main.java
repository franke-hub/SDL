//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2020 Frank Eskesen.
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
//       Demonstrate simple HTTP server.
//
// Last change date-
//       2020/01/14
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.text.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Simple HTTP server.
//
//----------------------------------------------------------------------------
public class Main {
//----------------------------------------------------------------------------
// Main.Attributes
//----------------------------------------------------------------------------
public ServerSocket    listen;      // Listener Socket
public Socket          talk;        // Talk Socket
public BufferedReader  reader;      // Socket input stream
public PrintWriter     writer;      // Socket output stream
public int             port;        // The target port

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
   port=  8080;
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
   System.out.print(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.twoDigit
//
// Purpose-
//       Return two digit string value.
//
//----------------------------------------------------------------------------
public static String
   twoDigit(
     int               i)
{
   String              result= "" + i;

   if( result.length() == 1 )
     result= "0" + i;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main.getGmtDate
//
// Purpose-
//       Return the GMT date/time string.
//
//----------------------------------------------------------------------------
public static String
   getGmtDate( )
{
   String[]            dow= {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
   String[]            moy= {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
   Calendar            calendar= Calendar.getInstance(TimeZone.getTimeZone("GMT"));

   calendar.setTime(new Date());
   return dow[calendar.get(Calendar.DAY_OF_WEEK)-1]
          + ", " + moy[calendar.get(Calendar.MONTH)]
          + " "  + calendar.get(Calendar.DAY_OF_MONTH)
          + " "  + calendar.get(Calendar.YEAR)
          + " "  + twoDigit(calendar.get(Calendar.HOUR_OF_DAY))
          + ":"  + twoDigit(calendar.get(Calendar.MINUTE))
          + ":"  + twoDigit(calendar.get(Calendar.SECOND))
          + " GMT";
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
     String            message)     // The message
   throws Exception
{
   writer.print(message + "\r\n");
   writer.flush();
   debugf("send: \'" + message + "\'\n");
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
   String              result;      // Resultant

   result= reader.readLine();
   debugf("recv: \'" + result + "\'\n");
   return result;
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
   String              command;     // Input command
   double              elapsed;     // Elapsed time
   String              string;      // Input/response String
   Date                timeStart;   // Starting time
   Date                timeFinal;   // Ending time
   int                 x;           // String index

   int                 i;

   listen= new ServerSocket(port);  // Create the Listener
   debugf("Ready\n");

   talk= listen.accept();           // Accept the first connection
   reader= new BufferedReader(
           new InputStreamReader(talk.getInputStream())
                             );
   writer= new PrintWriter(
           new BufferedWriter(
           new OutputStreamWriter(talk.getOutputStream())
                          )  );

   timeStart= new Date();
   string= receive();               // Receive request
   for(;;)                          // Receive request properties
   {
     string= receive();
     if( string == null )
     {
       debugf("End of file\n");
       break;
     }

     if( string.equals("") )
       break;
   }

   send("HTTP/1.1 200 OK");
   send("Content-Type: text/html;charset=ISO-8859-1");
   send("Date: " + getGmtDate() + "");
   send("Server: UrlServer/0.0");
   send("");

   send("<html>");
   send("<head>");
   send("  <title>UrlServer WEB page</title>");
   send("</head>");
   send("<body>");
   send("  THIS IS IT!");
   send("</body>");
   send("</html>");

   timeFinal= new Date();
   Thread.sleep(2);
   writer.close();
   reader.close();
   talk.close();
   listen.close();

   elapsed= timeFinal.getTime() - timeStart.getTime();
   elapsed /= 1000.0;
   debugf("Elapsed: " + elapsed + " seconds\n");
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

   if( true  ) for(;;) main.server();

   main.server();
   System.out.println("Main complete");
   System.gc();
   System.runFinalization();
   System.gc();
}
} // Class Main


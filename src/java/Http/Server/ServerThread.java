//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ServerThread.java
//
// Purpose-
//       Server thread request handler.
//
// Last change date-
//       2020/01/12
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.net.*;
import java.text.*;
import java.util.*;

public class ServerThread extends Thread {
//----------------------------------------------------------------------------
// Attributes
//----------------------------------------------------------------------------
public BufferedReader  reader;      // Socket input stream
public Server          server;      // Our Server
public Socket          talk;        // Talk Socket
public PrintWriter     writer;      // Socket output stream

//----------------------------------------------------------------------------
// Static response data
//----------------------------------------------------------------------------
static String[] body=
{  "<html>"
,  "<head>"
,  "  <title>UrlServer WEB page</title>"
,  "</head>"
,  "<body>"
,  "  THIS IS IT!"
,  "</body>"
,  "</html>"
};

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread.ServerThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   ServerThread(                    // Constructor
     Server            server,      // The associated Server
     Socket            socket)      // The associated Socket
{
   this.server= server;
   this.talk= socket;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread.debugf
//
// Purpose-
//       Debugging printf + newline;
//
//----------------------------------------------------------------------------
public static void
   debugf(
     String            string)
{
   System.out.println(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread.twoDigit
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
//       ServerThread.getGmtDate
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
//       ServerThread.send
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
   debugf("send: \'" + message + "\'");
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread.receive
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
   debugf("recv: \'" + result + "\'");
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread.run
//
// Purpose-
//       HTTP request handler.
//
//----------------------------------------------------------------------------
public void
   tryrun( )
   throws Exception
{
   String              string;      // Input/response String

   int                 i;

   reader= new BufferedReader(
           new InputStreamReader(talk.getInputStream())
                             );
   writer= new PrintWriter(
           new BufferedWriter(
           new OutputStreamWriter(talk.getOutputStream())
                          )  );

   Date timeStart= new Date();
// string= receive();               // Receive request
   for(;;)                          // Receive request properties
   {
     string= receive();
     if( string == null )
     {
       debugf("End of file");
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

   for(i= 0; i<body.length; i++)
     send(body[i]);

   Date timeFinal= new Date();
   Thread.sleep(2);
   writer.close();
   reader.close();
   talk.close();

   double elapsed= timeFinal.getTime() - timeStart.getTime();
   elapsed /= 1000.0;
   debugf("Elapsed: " + elapsed + " seconds\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread.run
//
// Purpose-
//       Run, handling exceptions
//
//----------------------------------------------------------------------------
public void
   run( )
{
   try {
     tryrun();
   } catch(Exception X) {
     debugf("Exception: " + X);
   }

}
} // Class ServerThread

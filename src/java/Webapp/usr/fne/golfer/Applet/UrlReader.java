//----------------------------------------------------------------------------
//
//       Copyright (C) 2008-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       UrlReader.java
//
// Purpose-
//       URL Reader.
//
// Last change date-
//       2020/01/15
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
//       UrlReader
//
// Purpose-
//       URL Reader.
//
//----------------------------------------------------------------------------
public class UrlReader
{
public static void debug(String string) {
   System.err.println(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       UrlReader.open
//
// Purpose-
//       Open a URL.
//
//----------------------------------------------------------------------------
public static HttpURLConnection     // Resultant HttpURLConnection
   open(                            // Read from a URL
     String            urlString,   // (The URL string)
     String            method)      // (The access method, "GET" or "POST")
   throws Exception
{
   debug("UrlReader.open(" + urlString + "," + method + ")");

   URL                 url;         // URL
   HttpURLConnection   conn;        // URLConnection

   url= new URL(urlString);
// HttpURLConnection.setFollowRedirects(false); // Not allowed in Applets
   conn= (HttpURLConnection)url.openConnection();
   conn.setRequestMethod(method);
   conn.setRequestProperty("user-agent", "Mozilla 5.0 (Test URL reader)");

   return conn;
}

public static HttpURLConnection     // Resultant HttpURLConnection
   open(                            // Read from a URL
     String            urlString)   // (The URL string)
   throws Exception
{
   return open(urlString, "GET");
}

//----------------------------------------------------------------------------
//
// Method-
//       UrlReader.readURL
//
// Purpose-
//       BRINGUP: Read a URL.
//
//----------------------------------------------------------------------------
public static StringBuffer          // Resultant
   readURL(                         // Read from a URL
     String            urlString,   // (The URL string)
     String            method)      // (The access method, "GET" or "POST")
{
   debug("UrlReader.readURL(" + urlString + "," + method + ")");

   StringBuffer        out= new StringBuffer();

   BufferedReader      inp;         // The connection's input stream
   URL                 url;         // URL
   HttpURLConnection   conn;        // URLConnection
   String              string;      // Working string

   try {
     conn= open(urlString, method);
     inp= new BufferedReader(new InputStreamReader(conn.getInputStream()));
     for(;;)
     {
       string= inp.readLine();
       if( string == null )
         break;

       out.append(string + "\n");
     }
     inp.close();
   } catch( Exception e ) {
     System.err.println("READURL Failure: " + e);
     e.printStackTrace();
     out= null;
   }

   return out;
}

public static StringBuffer          // Resultant
   readURL(                         // Read from a URL
     String            urlString)   // (The URL string)
{
   return readURL(urlString, "GET");
}
} // Class UrlReader


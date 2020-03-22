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
//       Demonstrate simple HTTP client.
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

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Main
//
// Purpose-
//       Simple HTTP client.
//
//----------------------------------------------------------------------------
class Main extends Debug {
//----------------------------------------------------------------------------
//
// Method-
//       Main.readURL
//
// Purpose-
//       Load a URL.
//
//----------------------------------------------------------------------------
public static void
   readURL(                         // Read from a URL
     String            urlString,   // (The URL string)
     String            method)      // (The access method, "GET" or "POST")
   throws Exception
{
   BufferedReader      inp;         // The connection's input stream
   URL                 url;         // URL
   HttpURLConnection   conn;        // URLConnection
   String              string;      // Working string

   if( SCDM ) debugf("\nreadURL('"+urlString+"')..\n");

   try {
     url= new URL(urlString);
     HttpURLConnection.setFollowRedirects(true);
     conn= (HttpURLConnection)url.openConnection();
     conn.setRequestMethod(method);
     conn.setRequestProperty("user-agent", "Mozilla 5.0 (Test URL reader)");

     inp= new BufferedReader(new InputStreamReader(conn.getInputStream()));
     for(;;)
     {
       string= inp.readLine();
       if( string == null )
         break;

       debugf(string + "\n");
     }
     inp.close();
   } catch( Exception e ) {
     debugf("READURL Failure: " + e + "\n");
     e.printStackTrace();
   }

   if( SCDM ) debugf("..readURL()\n=======================\n");
}

public static void
   readURL(                         // Read from a URL
     String            urlString)   // (The URL string)
   throws Exception
{
   readURL(urlString, "GET");
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
   int                 i;

   SCDM= true;
   Main.readURL("http://localhost:8080/sample/Sample?A");
   Main.readURL("http://localhost:8080/sample/Sample?B=queryString");
   Main.readURL("http://localhost:8080/sample/Sample?errorString");
   Main.readURL("http://localhost:8080/sample/Sample?postString", "POST");
   for(i= 0; i<args.length; i++)
     Main.readURL(args[i]);
}
} // Class Main


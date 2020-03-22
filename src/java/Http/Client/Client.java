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
//       Client.java
//
// Purpose-
//       HTTP Client.
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

import user.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       Client
//
// Purpose-
//       HTTP Client.
//
//----------------------------------------------------------------------------
class Client extends Debug {
//----------------------------------------------------------------------------
//
// Method-
//       Client.readURL
//
// Purpose-
//       Load a URL.
//
//----------------------------------------------------------------------------
public void
   readURL(                         // Read from a URL
     String            urlString,   // (The URL string)
     String            method)      // (The access method, "GET" or "POST")
   throws Exception
{
   BufferedReader      inp;         // The connection's input stream
   URL                 url;         // URL
   HttpURLConnection   conn;        // URLConnection
   String              string;      // Working string

   int                 i;

   debugf("\nreadURL('"+urlString+"')..\n");

   try {
     url= new URL(urlString);
     HttpURLConnection.setFollowRedirects(true);
     conn= (HttpURLConnection)url.openConnection();
     conn.setRequestMethod(method);
     conn.setRequestProperty("user-agent", "Mozilla 5.0 (Test URL reader)");

     inp= new BufferedReader(new InputStreamReader(conn.getInputStream()));
     debugf(conn.getHeaderField(0) + "\n");
     for(i= 1; ; i++) {
       String key= conn.getHeaderFieldKey(i);
       if( key == null )
         break;
       String val= conn.getHeaderField(i);

       debugf(key + ": " + val + "\n");
     }
     debugf("\n");

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

   debugf("..readURL() =======================\n");
}

public void
   readURL(                         // Read from a URL
     String            urlString)   // (The URL string)
   throws Exception
{
   readURL(urlString, "GET");
}
} // Class Client

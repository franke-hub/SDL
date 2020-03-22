//----------------------------------------------------------------------------
//
//       Copyright (C) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       MainReader.java
//
// Purpose-
//       Standalone News Reader.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
package usr.fne.newsreader;

import java.io.*;
import java.lang.*;
import java.net.*;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.*;

import usr.fne.common.*;
public class MainReader implements LoggingService {
//----------------------------------------------------------------------------
// MainReader.Attributes
//----------------------------------------------------------------------------
static final String    FILE_PROFILE=  "newsreader.pro"; // Profile

//----------------------------------------------------------------------------
// MainReader.Attributes
//----------------------------------------------------------------------------
protected boolean      debug;       // Debugging control
protected LoggingService
                       logger;      // Our logger
protected int          verbose;     // Verbosity control
protected long         runtime;     // RUNTIME property

protected NewsReader   reader;      // News Reader

//----------------------------------------------------------------------------
//
// Method-
//       MainReader.MainReader
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   MainReader( )                    // Constructor
   throws Exception
{
   String              string;

   // Initialize for debugging
   logger= new MainLogger(FILE_PROFILE);
   this.debug=   logger.getDebug();
   this.verbose= logger.getVerbose();

   if( debug ) log("MainReader()..");

   // Initialize properties
   Properties props= new Properties();
   try {
     props.load(new FileInputStream(FILE_PROFILE));
   } catch( Exception e ) {
     log("init: Error loading: " + FILE_PROFILE, e);
     throw new Exception("Initialization failed");
   }

   runtime= Integer.parseInt(props.getProperty("runtime","30"))*1000;

   // Initialize
   reader= new NewsReader(this, props);

   if( debug ) log("..MainReader()");
}

//----------------------------------------------------------------------------
//
// Method-
//       MainReader.LoggingService
//
// Purpose-
//       Implement LoggingService methods.
//
//----------------------------------------------------------------------------
public boolean
   getDebug( )
{
   return debug;
}

public int
   getVerbose( )
{
   return verbose;
}

public void
   log(
     String            string)
{
   logger.log("MainReader: " + string);
}

public void
   log(
     String            string,
     Throwable         e)
{
   logger.log("MainReader: " + string, e);
}

//----------------------------------------------------------------------------
//
// Method-
//       MainReader.client
//
// Purpose-
//       Client.
//
//----------------------------------------------------------------------------
public void
   client( )
   throws Exception
{
   reader.init();

   NewsDriver thread= new NewsDriver(reader);
   thread.start();

   try {
     Thread.sleep(runtime);
   } catch( Exception e ) {
     log("client", e);
   }

   // Terminate the client Thread
   thread.abandon();

   try {
     thread.join();
   } catch(Exception e) {
     log("client", e);
   }
   thread= null;

   reader.term();
}

//----------------------------------------------------------------------------
//
// Method-
//       MainReader.test
//
// Purpose-
//       Test code.
//
//----------------------------------------------------------------------------
public static void
   test(                            // Testing code
     String[]          args)        // Argument array
   throws Exception
{
   String              string;

   string= "EEE MMM dd HH:mm:ss yyyy z";
   System.out.println("Parse string: '" + string + "'");
   SimpleDateFormat df= new SimpleDateFormat(string);
   System.out.println("Parse string: '" + df.toPattern() + "'");
   df.setLenient(true);
   System.out.println("Parse string: '" + df.toPattern() + "'");

   Date             date1= new Date();
   string= df.format(date1);
   System.out.println(string);

   Date             date2= df.parse(string);
   System.out.println(date1 + " :: " + date2);

   TimeZone         gmt= TimeZone.getTimeZone("GMT");
   df.setTimeZone(gmt);
   string= df.format(date1);
   System.out.println(string);

   Date             date3= df.parse(string);
   System.out.println(date1 + " :: " + date3);

   Date             date4= df.parse(date1.toString());
   System.out.println(date1 + " :: " + date4);
}

//----------------------------------------------------------------------------
//
// Method-
//       MainReader.main
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
   System.out.println("Main..");

   if( false )
   {
     System.out.println("Main.test()..");
     try {
       test(args);
     } catch( Exception e ) {
       System.out.println(e);
       e.printStackTrace();
     }
     System.out.println("..Main.test()");
   }

   MainReader r= new MainReader();
   r.client();

   System.out.println("..Main");
}
} // Class MainReader


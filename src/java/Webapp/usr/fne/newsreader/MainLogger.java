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
//       MainLogger.java
//
// Purpose-
//       Standalone News Reader LoggingService.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
package usr.fne.newsreader;

import java.io.*;
import java.lang.*;
import java.util.*;

import usr.fne.common.*;

class MainLogger implements LoggingService {
//----------------------------------------------------------------------------
// MainLogger.Attributes
//----------------------------------------------------------------------------
public static MainLogger
                       logger;      // Last MainLogger

protected boolean      debug;       // Debugging control
protected int          verbose;     // Verbosity control

protected FileWriter   logFile;     // The log file

//----------------------------------------------------------------------------
//
// Method-
//       MainLogger.MainLogger
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   MainLogger(                      // Constructor
     String            fileName)    // Properties file name
   throws Exception
{
   // Properties for debugging
   Properties props= new Properties();
   try {
     props.load(new FileInputStream(fileName));
   } catch( Exception e ) {
     log("init: Error loading: " + fileName, e);
     throw new Exception("Initialization failed");
   }

   debug= true;
   verbose= 0;
   verbose= Integer.parseInt(props.getProperty("debug","0"));
   if( verbose == 0 )
     debug= false;

   logFile= new FileWriter(props.getProperty("logfile","logfile.out"), true);
   logger= this;
}

//----------------------------------------------------------------------------
//
// Method-
//       MainLogger.LoggingService
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
   Date date= new Date();
   try {
     logFile.write("MainLogger: " + date.getTime() + ": " + string + "\n");
     logFile.flush();
   } catch( Exception e ) {
     e.printStackTrace();
     System.err.println("MainLogger: " + date.getTime() + ": " + string);
   }
}

public void
   log(
     String            string,
     Throwable         t)
{
   Date date= new Date();
   try {
     logFile.write("MainLogger: " + date.getTime() + ": " + string + "\n");
     logFile.write("Exception: " + t + "\n");
     logFile.flush();
   } catch( Exception e ) {
     e.printStackTrace();
     System.err.println("MainLogger: " + date.getTime() + ": " + string);
     System.err.println("Exception: " + t);
   }

   System.err.println("MainLogger: " + date.getTime() + ": " + string);
   t.printStackTrace();
}
} // Class MainLogger


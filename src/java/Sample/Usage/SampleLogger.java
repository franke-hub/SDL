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
//       SampleLogger.java
//
// Purpose-
//       Use a Logger.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
import java.util.logging.Filter;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.LogRecord;

public class SampleLogger extends Logger
{
//----------------------------------------------------------------------------
//
// Method-
//       SampleLogger.SampleLogger
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   SampleLogger( )                  // Constructor
{
   super("user.test.logger", null);

   setUseParentHandlers(false);
   setLevel(Level.FINER);
   setFilter(new SampleFilter());
   addHandler(new SampleHandler());
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleLogger.log
//
// Purpose-
//       Log a message.
//
//----------------------------------------------------------------------------
public void
   log(                             // Log a message
     String            message)     // This one
   throws Exception
{
   log(Level.OFF, message);
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleLogger.main
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
   SampleLogger        logger;      // Logger Object

   logger= new SampleLogger();

   logger.entering("SampleLogger", "main");
   logger.log("Log level DEFAULT");
   logger.log(Level.OFF    , "Log level OFF    ");
   logger.log(Level.SEVERE , "Log level SEVERE ");
   logger.log(Level.WARNING, "Log level WARNING");
   logger.log(Level.INFO   , "Log level INFO   ");
   logger.log(Level.CONFIG , "Log level CONFIG ");
   logger.log(Level.FINE   , "Log level FINE   ");
   logger.log(Level.FINER  , "Log level FINER  ");
   logger.log(Level.FINEST , "Log level FINEST ");
   logger.log(Level.ALL    , "Log level ALL    ");
   logger.exiting("SampleLogger", "main");

   System.out.println("OFF    : " + Level.OFF    .intValue());
   System.out.println("SEVERE : " + Level.SEVERE .intValue());
   System.out.println("WARNING: " + Level.WARNING.intValue());
   System.out.println("INFO   : " + Level.INFO   .intValue());
   System.out.println("CONFIG : " + Level.CONFIG .intValue());
   System.out.println("FINE   : " + Level.FINE   .intValue());
   System.out.println("FINER  : " + Level.FINER  .intValue());
   System.out.println("FINEST : " + Level.FINEST .intValue());
   System.out.println("ALL    : " + Level.ALL    .intValue());
}
} // Class SampleLogger

class SampleFilter implements Filter
{
//----------------------------------------------------------------------------
//
// Method-
//       SampleFilter.isLoggable
//
// Purpose-
//       Implement Filter interface
//
//----------------------------------------------------------------------------
public boolean                      // Resultant (TRUE)
   isLoggable(                      // Is LogRecord Loggable?
     LogRecord         record)      // The LogRecord
{
   return true;
}
} // Class SampleFilter

class SampleHandler extends Handler
{
//----------------------------------------------------------------------------
//
// Method-
//       SampleHandler.SampleHandler
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   SampleHandler( )                 // Constructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleHandler.close
//
// Purpose-
//       Close the Handler
//
//----------------------------------------------------------------------------
public void
   close( )                         // Close the Handler
{
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleHandler.flush
//
// Purpose-
//       Flush the Handler
//
//----------------------------------------------------------------------------
public void
   flush( )                         // Flush the Handler
{
}

//----------------------------------------------------------------------------
//
// Method-
//       SampleHandler.publish
//
// Purpose-
//       Write a LogRecord
//
//----------------------------------------------------------------------------
public void
   publish(                         // Write a LogRecord
     LogRecord         record)      // The LogRecord
{
   System.out.println("Handler: " + record.getMessage());
}
} // Class SampleHandler


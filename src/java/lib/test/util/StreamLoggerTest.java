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
//       StreamLoggerTest.java
//
// Purpose-
//       Test the StreamLogger class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package test.util;

import java.util.logging.Level;

import user.util.logging.*;

//----------------------------------------------------------------------------
//
// Class-
//       StreamLoggerTest
//
// Purpose-
//       Test the StreamLogger class.
//
//----------------------------------------------------------------------------
class StreamLoggerTest
{
//----------------------------------------------------------------------------
//
// Method-
//       StreamLoggerTest.main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
public static void
   main(                            // Mainline code
     String[]          args)        // Argument array
{
   StreamLogger        logger;      // Logger Object

   logger= new StreamLogger();

   logger.entering("StreamLogger", "main");
   logger.log("Log level DEFAULT");
   logger.log("Log level EXCEPTION", new Exception("StandardException"));
   logger.log(Level.OFF    , "Log level OFF    ");
   logger.log(Level.SEVERE , "Log level SEVERE ");
   logger.log(Level.WARNING, "Log level WARNING");
   logger.log(Level.INFO   , "Log level INFO   ");
   logger.log(Level.CONFIG , "Log level CONFIG ");
   logger.log(Level.FINE   , "Log level FINE   ");
   logger.log(Level.FINER  , "Log level FINER  ");
   logger.log(Level.FINEST , "Log level FINEST ");
   logger.log(Level.ALL    , "Log level ALL    ");

   logger.log("OFF    : " + Level.OFF    .intValue());
   logger.log("SEVERE : " + Level.SEVERE .intValue());
   logger.log("WARNING: " + Level.WARNING.intValue());
   logger.log("INFO   : " + Level.INFO   .intValue());
   logger.log("CONFIG : " + Level.CONFIG .intValue());
   logger.log("FINE   : " + Level.FINE   .intValue());
   logger.log("FINER  : " + Level.FINER  .intValue());
   logger.log("FINEST : " + Level.FINEST .intValue());
   logger.log("ALL    : " + Level.ALL    .intValue());

   logger.exiting("StreamLogger", "main");
}
} // Class StreamLoggerTest


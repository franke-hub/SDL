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
//       StreamLogger.java
//
// Purpose-
//       Use a Logger.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util.logging;

import java.lang.StackTraceElement;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.LogRecord;

//----------------------------------------------------------------------------
//
// Class-
//       StreamLogger
//
// Purpose-
//       Stream logger.
//
//----------------------------------------------------------------------------
public class StreamLogger extends Logger
{
//----------------------------------------------------------------------------
// StreamLogger.ATTRIBUTES
//----------------------------------------------------------------------------
protected static final String LOGGER_NAME="user.util";

//----------------------------------------------------------------------------
//
// Method-
//       StreamLogger.StreamLogger
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   StreamLogger(                    // Constructor
     String            fileName)    // Package (file) name
{
   super(fileName, null);

   setUseParentHandlers(false);
   setLevel(Level.FINER);
   addHandler(new StreamHandler(fileName));
}

public
   StreamLogger( )                  // Constructor
{
   this(LOGGER_NAME);
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamLogger.log
//
// Purpose-
//       Log a message.
//
//----------------------------------------------------------------------------
public void
   log(                             // Log a message
     String            message)     // The message
{
   LogRecord           record= new LogRecord(Level.OFF, message);

   record.setLoggerName(LOGGER_NAME);
   record.setSourceClassName(null);
   record.setSourceMethodName(null);
   log(record);
}

public void
   log(                             // Log a message
     String            message,     // The message
     Throwable         t)           // Associated exception
{
   t.printStackTrace();
   log(message);
   log(t.toString());
   StackTraceElement[] trace= t.getStackTrace();
   for(int i=0; i<trace.length; i++)
     log("\tat " + trace[i].toString());
}
} // Class StreamLogger


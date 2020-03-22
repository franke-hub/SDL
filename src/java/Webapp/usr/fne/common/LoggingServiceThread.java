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
//       LoggingServiceThread.java
//
// Purpose-
//       Thread that implements LoggingService
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
package usr.fne.common;

import java.lang.*;
import java.util.*;

//----------------------------------------------------------------------------
//
// Class-
//       LoggingServiceThread
//
// Purpose-
//       Utility LoggingServiceThread.
//
//----------------------------------------------------------------------------
public class LoggingServiceThread
   extends Thread implements LoggingService
{
//----------------------------------------------------------------------------
// LoggingServiceThread.Attributes
//----------------------------------------------------------------------------
protected LoggingService
                       logger;      // Logger
protected boolean      debug;       // DEBUG attribute
protected int          verbose;     // VERBOSE attribute
protected String       header;      // Header message

//----------------------------------------------------------------------------
//
// Method-
//       LoggingServiceThread.LoggingServiceThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   LoggingServiceThread(            // LoggingServiceThread
     LoggingService    logger,      // Source LoggingService
     String            header)      // Log message prefix
{
   super();
   init(logger, header);
}

public
   LoggingServiceThread(            // LoggingServiceThread
     Runnable          runnable,    // For Thread(Runnable)
     LoggingService    logger,      // Source LoggingService
     String            header)      // Log message prefix
{
   super(runnable);
   init(logger, header);
}

public
   LoggingServiceThread(            // LoggingServiceThread
     Runnable          runnable,    // For Thread(Runnable,String)
     String            string,      // For Thread(Runnable,String)
     LoggingService    logger,      // Source LoggingService
     String            header)      // Log message prefix
{
   super(runnable,string);
   init(logger, header);
}

public
   LoggingServiceThread(            // LoggingServiceThread
     String            string,      // For Thread(String)
     LoggingService    logger,      // Source LoggingService
     String            header)      // Log message prefix
{
   super(string);
   init(logger, header);
}

public
   LoggingServiceThread(            // LoggingServiceThread
     ThreadGroup       group,       // For Thread(ThreadGroup,Runnable)
     Runnable          runnable,    // For Thread(ThreadGroup,Runnable)
     LoggingService    logger,      // Source LoggingService
     String            header)      // Log message prefix
{
   super(group,runnable);
   init(logger, header);
}

public
   LoggingServiceThread(            // LoggingServiceThread
     ThreadGroup       group,       // For Thread(ThreadGroup,Runnable,String)
     Runnable          runnable,    // For Thread(ThreadGroup,Runnable,String)
     String            string,      // For Thread(ThreadGroup,Runnable,String)
     LoggingService    logger,      // Source LoggingService
     String            header)      // Log message prefix
{
   super(group,runnable,string);
   init(logger, header);
}

public
   LoggingServiceThread(            // LoggingServiceThread
     ThreadGroup       group,       // For Thread(ThreadGroup,Runnable,String,long)
     Runnable          runnable,    // For Thread(ThreadGroup,Runnable,String,long)
     String            string,      // For Thread(ThreadGroup,Runnable,String,long)
     long              stack,       // For Thread(ThreadGroup,Runnable,String,long)
     LoggingService    logger,      // Source LoggingService
     String            header)      // Log message prefix
{
   super(group,runnable,string,stack);
   init(logger, header);
}

public
   LoggingServiceThread(            // LoggingServiceThread
     ThreadGroup       group,       // For Thread(ThreadGroup,String)
     String            string,      // For Thread(ThreadGroup,String)
     LoggingService    logger,      // Source LoggingService
     String            header)      // Log message prefix
{
   super(group,string);
   init(logger, header);
}

protected void
   init(                            // Initialize
     LoggingService    logger,      // Source LoggingService
     String            header)      // Log message prefix
{
   this.header= header;
   this.logger= logger;
   debug= logger.getDebug();
   verbose= logger.getVerbose();
}

//----------------------------------------------------------------------------
//
// Method-
//       LoggingServiceThread.LoggingService
//
// Purpose-
//       Write a message to the Servlet log.
//
//----------------------------------------------------------------------------
public boolean                      // The DEBUG attribute
   getDebug( )                      // Get DEBUG attribute
{
   return debug;
}

public int                          // The VERBOSE attribute
   getVerbose( )                    // Get VERBOSE attribute
{
   return verbose;
}

public void
   log(                             // Write a log message
     String            message,     // The log message
     Throwable         throwable)   // Associated exception
{  
   logger.log(header + ": " + message, throwable);
}

public void
   log(                             // Write a log message
     String            message)     // The log message
{  
   logger.log(header + ": " + message);
}
} // class LoggingServiceThread


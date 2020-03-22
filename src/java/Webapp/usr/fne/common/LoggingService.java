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
//       LoggingService.java
//
// Purpose-
//       Define a simple logging service.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
package usr.fne.common;

//----------------------------------------------------------------------------
//
// Interface-
//       LoggingService
//
// Purpose-
//       Define a simple logging service.
//
//----------------------------------------------------------------------------
public interface LoggingService
{
//----------------------------------------------------------------------------
//
// Method-
//       LoggingService.getDebug
//
// Purpose-
//       Extract the debug attribute.
//
//----------------------------------------------------------------------------
public boolean                      // The DEBUG attribute
   getDebug( );                     // Get DEBUG attribute

//----------------------------------------------------------------------------
//
// Method-
//       LoggingService.getVerbose
//
// Purpose-
//       Extract the verbosity attribute.
//
//----------------------------------------------------------------------------
public int                          // The VERBOSE attribute
   getVerbose( );                   // Get VERBOSE attribute

//----------------------------------------------------------------------------
//
// Method-
//       LoggingService.log
//
// Purpose-
//       Write a message to the log.
//
//----------------------------------------------------------------------------
public void
   log(                             // Write a log message
     String            message);    // The log message

public void
   log(                             // Write log message
     String            message,     // The log message
     Throwable         e);          // Associated exception
} // Interface LoggingService


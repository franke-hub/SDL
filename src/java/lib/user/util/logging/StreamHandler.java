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
//       StreamHandler.java
//
// Purpose-
//       Stream log handler -- Writes to stream file.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
package user.util.logging;

import java.io.FileWriter;
import java.io.BufferedWriter;
import java.lang.StringBuffer;
import java.util.Date;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogRecord;

//----------------------------------------------------------------------------
//
// Class-
//       StreamHandler
//
// Purpose-
//       Stream logger.
//
//----------------------------------------------------------------------------
public class StreamHandler extends Handler
{
//----------------------------------------------------------------------------
// StreamHandler.Attributes
//----------------------------------------------------------------------------
protected String       fileName;    // File name
protected BufferedWriter
                       writer;      // File writer

//----------------------------------------------------------------------------
//
// Method-
//       StreamHandler.StreamHandler
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
public
   StreamHandler(                   // Constructor
     String            fileName)    // Package name
{
// System.out.println("Construct("+fileName+")");

   this.fileName= fileName+ ".log";

   try {
     writer= new BufferedWriter(
                   new FileWriter(this.fileName, true)
                               );
   } catch(Exception e) {
     String message= "Error opening: '" + this.fileName + "'";
     System.err.println(message + " Exception: " + e);
     throw new RuntimeException(message);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamHandler.close
//
// Purpose-
//       Close the Handler
//
//----------------------------------------------------------------------------
public void
   close( )                         // Close the Handler
{
// System.out.println("close()");

   if( writer == null )
     return;

   try {
     writer.close();
     writer= null;
   } catch(Exception e) {
     String message= "Error closing: '" + fileName + "'";
     System.err.println(message + " Exception: " + e);
     throw new RuntimeException(message);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamHandler.flush
//
// Purpose-
//       Flush the Handler
//
//----------------------------------------------------------------------------
public void
   flush( )                         // Flush the Handler
{
// System.out.println("flush()");

   try {
     writer.flush();
   } catch(Exception e) {
     String message= "Error flushing: '" + fileName + "'";
     System.err.println(message + " Exception: " + e);
     throw new RuntimeException(message);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamHandler.publish
//
// Purpose-
//       Write a LogRecord
//
//----------------------------------------------------------------------------
public void
   publish(                         // Write a LogRecord
     LogRecord         record)      // The LogRecord
{
   StringBuffer        buffer= new StringBuffer();
   String              string;

// System.out.println("publish(" + record.getMessage() + ")");

   try {
     buffer.append("" + record.getMillis());
//// string= record.getLoggerName();
////   buffer.append(": " + string);
     string= record.getSourceClassName();
     if( string != null )
       buffer.append(": " + string + "." + record.getSourceMethodName());
     buffer.append(": " + record.getMessage());
     writer.write(buffer + "\n");
   } catch(Exception e) {
     String message= "Error writing: '" + fileName + "'";
     System.err.println("Root cause: " + message + ", Exception: " + e);
     e.printStackTrace();
     throw new RuntimeException(message);
   }
   flush();
}
} // Class StreamHandler


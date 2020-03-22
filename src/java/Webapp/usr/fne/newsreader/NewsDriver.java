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
//       NewsDriver.java
//
// Purpose-
//       Java News Reader driver.
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
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

import usr.fne.common.*;

//----------------------------------------------------------------------------
//
// Class-
//       NewsDriver
//
// Purpose-
//       Change the state of a client.
//
//----------------------------------------------------------------------------
class NewsDriver extends LoggingServiceThread {
//----------------------------------------------------------------------------
// NewsDriver.Attributes
//----------------------------------------------------------------------------
protected NewsReader   reader;      // Parent NewsReader
protected boolean      abandoned;   // TRUE iff Thread has been abandoned

//----------------------------------------------------------------------------
//
// Method-
//       NewsDriver.NewsDriver
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
public
   NewsDriver(                      // Client Thread
     NewsReader        reader)      // Associated NewsReader
{
   super(reader,"Thread");

   this.reader= reader;
   this.abandoned= false;
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsDriver.abandon
//
// Purpose-
//       Abandon this Thread.
//
//----------------------------------------------------------------------------
public void
   abandon( )                       // Abandon the Thread
{
   if( debug ) log("abandon()");

   abandoned= true;
   post();
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsDriver.post
//
// Purpose-
//       Post the thread.
//
//----------------------------------------------------------------------------
protected synchronized void
   post( )                          // Activate the thread
{
   if( verbose > 5 ) log("NewsDriver.post()");

   notify();
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsDriver.round
//
// Purpose-
//       Round a value upwards.
//
//----------------------------------------------------------------------------
public long                         // Rounded value
   round(                           // Round a value
     long              value,       // The value to round
     long              modulus)     // Upward rounding value
{
   value += (modulus-1);            // Step over next higher value
   value /= modulus;                // Number of moduli
   value *= modulus;                // Rounded value
   return value;
}

//----------------------------------------------------------------------------
//
// Method-
//       NewsDriver.run
//
// Purpose-
//       Drive the Reader.client method.
//
//----------------------------------------------------------------------------
public void
   run( )                           // Run the Thread
{
   long                interval;    // Client delay interval, in milliseconds
   long                lastUpdate;  // Last update time
   long                nextUpdate;  // Next update time

   if( debug ) log("run()..");

   nextUpdate= 0;
   interval= reader.getInterval();
   while( !abandoned )
   {
     Date now= new Date();
     if( nextUpdate > now.getTime() )
     {
       synchronized (this) {
         try {
           if( verbose > 5 ) log("NewsDriver.run: waiting");
           wait(nextUpdate - now.getTime());
           if( verbose > 5 ) log("NewsDriver.run: running");
         } catch( Exception e ) {
           log("run: Exception", e);
           break;
         }
       }
     }
     now= new Date();
     lastUpdate= nextUpdate;
     nextUpdate= round(now.getTime(),interval);
     if( nextUpdate <= lastUpdate )
       nextUpdate= lastUpdate + interval;

     try {
       if( !abandoned )
         reader.client();
     } catch( Exception e ) {
       log("run: Exception", e);
     }
   }

   abandoned= true;
   if( debug ) log("..run()");
}
} // class NewsDriver


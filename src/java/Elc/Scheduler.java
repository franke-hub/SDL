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
//       Scheduler.java
//
// Purpose-
//       The Scheduler singleton drives all WorkList objects.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       See WorkList.java.
//       See WhenItem.java.
//
//----------------------------------------------------------------------------
import java.io.*;
import java.lang.*;
import java.util.*;

import user.util.*;
import user.util.logging.*;

//----------------------------------------------------------------------------
//
// Class-
//       Scheduler
//
// Purpose-
//       Schedule WorkList actions
//
//----------------------------------------------------------------------------
class Scheduler {                   // Scheduler descriptor
//----------------------------------------------------------------------------
// Scheduler.Controls
//----------------------------------------------------------------------------
final static boolean   HCDM= false; // Hard Core Debug Mode?
final static boolean   IODM= HCDM;  // I/O Debug Mode?
final static boolean   SCDM= HCDM;  // Soft Core Debug Mode?

static StreamLogger    logger= Common.get().logger; // The Logger

//----------------------------------------------------------------------------
//
// Class-
//       Scheduler.Driver
//
// Purpose-
//       Define the Scheduler Driver Thread
//
//----------------------------------------------------------------------------
class Driver extends Thread {       // The Scheduler Thread
//----------------------------------------------------------------------------
// Scheduler.Driver.Attributes
//----------------------------------------------------------------------------
WorkList               workList= null; // The current WorkList
boolean                operational= true; // TRUE while operational

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.Driver.Driver
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
void
   Driver( )                        // Construct the Thread
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.Driver.select
//
// Purpose-
//       Select this Thread for Work
//
//----------------------------------------------------------------------------
synchronized void
   select(                          // Select this Thread
     WorkList          workList)    // WorkList
{
   if( HCDM )
     logger.log("Scheduler.Driver(" + toString() +
                ").select(" + workList.toString() + ")");

   this.workList= workList;
   notify();
}

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.Driver.shutdown
//
// Purpose-
//       Shut down this Thread
//
//----------------------------------------------------------------------------
synchronized void
   shutdown( )                      // Shut down
{
   if( HCDM )
     logger.log("Scheduler.Driver(" + toString() + ").shutdown()");

   operational= false;
   notify();
}

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.Driver.run
//
// Purpose-
//       Drive the Thread
//
//----------------------------------------------------------------------------
public void
   run( )                           // Operate the Thread
{
   if( HCDM )
     logger.log("Scheduler.Driver(" + toString() + ").run()");

   for(;;)                          // Process Work
   {
     synchronized (this)
     {{{{
       while( workList == null )
       {
         if( !operational )
           return;

         if( HCDM )
           logger.log("Scheduler(" + toString() +
                      ").Driver.run() wait...");
         try {
           wait();
         } catch(Exception x) {
         }
         if( HCDM )
           logger.log("...Scheduler(" + toString() +
                      ").Driver.run() wait");
       }
     }}}}

     if( HCDM )
       logger.log("Scheduler(" + toString() + ").Driver.run() " +
                  "WorkList(" + workList.toString() + ")");
     workList.empty();
     workList= null;
   }
}
}; // class Scheduler.Driver

//----------------------------------------------------------------------------
//
// Class-
//       Scheduler.Expand
//
// Purpose-
//       Define the Scheduler Expansion Thread
//
//----------------------------------------------------------------------------
class Expand extends Thread {       // The Scheduler expansion Thread
//----------------------------------------------------------------------------
// Scheduler.Expand.Attributes
//----------------------------------------------------------------------------
WorkList               workList= null; // The current WorkList

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.Expand.Expand
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Expand(                          // Construct the Thread
     WorkList          workList)    // The WorkList to process
{
   this.workList= workList;
   setDaemon(true);
}

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.Expand.run
//
// Purpose-
//       Drive the Thread
//
//----------------------------------------------------------------------------
public void
   run( )                           // Operate the Thread
{
   if( HCDM )
     logger.log("Scheduler.Expand(" + toString() + ").run(" +
                workList.toString() + ")");

   workList.empty();                // Process Work
}
}; // class Scheduler.Expand

//----------------------------------------------------------------------------
// Scheduler.Attributes
//----------------------------------------------------------------------------
Driver[]               pool;        // The Thread pool

//----------------------------------------------------------------------------
// Scheduler.Static attributes
//----------------------------------------------------------------------------
protected static Scheduler
                       scheduler= null; // The Scheduler singleton

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.Scheduler
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
protected
   Scheduler( )                     // Default constructor
{
   pool= new Driver[16];
   for(int i= 0; i<pool.length; i++)
   {
     pool[i]= new Driver();
     pool[i].start();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.close
//
// Purpose-
//       Close (shut down) the Scheduler.
//
//----------------------------------------------------------------------------
public void
   close( )                         // Close the Scheduler
{
   if( HCDM )
     logger.log("Scheduler.close()");

   synchronized (Common.get().singleton)
   {{{{
     scheduler= null;
   }}}}

   for(int i=0; i<pool.length; i++)
   {
     try {
       pool[i].shutdown();
     } catch(Exception x) {
     }
   }

   for(int i=0; i<pool.length; i++)
   {
     try {
       pool[i].join(1000);
     } catch(Exception x) {
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.get
//
// Purpose-
//       Get the Scheduler singleton.
//
//----------------------------------------------------------------------------
public static Scheduler             // The Scheduler Singleton
   get( )                           // Get Scheduler Singleton
{
   if( scheduler == null )
   {
     synchronized (Common.get().singleton)
     {{{{
       if( scheduler == null )
         scheduler= new Scheduler();
     }}}}
   }

   return scheduler;
}

//----------------------------------------------------------------------------
//
// Method-
//       Scheduler.schedule
//
// Purpose-
//       Schedule a WorkList.
//
//----------------------------------------------------------------------------
public void
   schedule(                        // Schedule a WorkList
     WorkList          list)        // The WorkList
{
   if( HCDM )
     logger.log("Scheduler.schedule(" + list.toString() + ")");

   for(int i= 0; i<pool.length; i++)
   {
     synchronized (pool[i])
     {{{{
       if( pool[i].workList == null )
       {
         pool[i].select(list);
         return;
       }
     }}}}
   }

   // There are no Threads available.
   Thread t= new Expand(list);      // Run an Expansion Daemon Thread
   t.start();
}
} // class Scheduler


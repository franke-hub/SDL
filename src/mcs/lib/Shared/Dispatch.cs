//-----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Dispatch.cs
//
// Purpose-
//       Work dispatcher.
//
// Last change date-
//       2019/02/15
//
//-----------------------------------------------------------------------------
#undef  USE_TRACE                   // Dispatcher.trace active?

using System;
using System.Collections.Generic;   // For List
using System.Threading;             // For Thread

using Shared;                       // (Library namespace)

namespace Shared.Dispatch {         // The Dispatcher namespace
//-----------------------------------------------------------------------------
//
// Class-
//       Dispatch
//
// Purpose-
//       Work dispatcher.
//
//-----------------------------------------------------------------------------
public class Dispatch {             // Work dispatcher
//-----------------------------------------------------------------------------
// Dispatch.Attributes
//-----------------------------------------------------------------------------
TimersThread           timers= null; // The timers Thread

//-----------------------------------------------------------------------------
// Dispatch.Constructor
//-----------------------------------------------------------------------------
public
   Dispatch( )                      // Constructor
{
   timers= new TimersThread();
}

//-----------------------------------------------------------------------------
// Dispatch.cancel()
//
// Call this method to cancel a timer work Item. If cancelled, the associated
// Dispatch Item COMPLETES with a completion code of CC_ERROR.
//-----------------------------------------------------------------------------
public void
   cancel(                          // Cancel delay
     Object            token)       // Cancellation token
{  timers.cancel(token); }

//-----------------------------------------------------------------------------
// Dispatch.delay()
//
// The associated Dispatch Item completes after the specified number of seconds
// with a completion code of CC_NORMAL. The delay may be cancelled by calling
// the cancel() function with the resultant cancellation token.
//-----------------------------------------------------------------------------
public Object                       // Cancellation token
   delay(                           // Delay for
     double            time,        // This many seconds, then
     Item              item)        // Complete this work Item
{  return timers.delay(time, item); }

//-----------------------------------------------------------------------------
// Dispatch.trace()
//
// Trace Dispatch event.
//-----------------------------------------------------------------------------
public static void
   trace(                           // Trace Dispatch event
     string            text)        // (Associated text)
{
#if USE_TRACE
   if( Trace.Ready ) {              // If Trace active
       Trace.trace(text);
   }
#endif
}

//-----------------------------------------------------------------------------
// Dispatch.wait()
//
// Terminate Dispatch processing, then wait for all associated work to
// complete. No new work will be processed after this function is called.
//-----------------------------------------------------------------------------
public void
   wait( )                          // Wait for all work to complete
{
   // Terminate worker threads
   timers.stop();
   WorkerPool.reset();

   // Wait for completion
   timers.join();
}
}  // class Dispatch

//-----------------------------------------------------------------------------
//
// Class-
//       TimerToken
//
// Purpose-
//       Timer list element.
//
//-----------------------------------------------------------------------------
class TimerToken {                  // Timer list element
public double          time;        // Proposed completion time
public Item            item;        // The work Item
}  // class TimerToken

//-----------------------------------------------------------------------------
//
// Class-
//       TimersThread
//
// Purpose-
//       The Dispatch Timers Thread
//
//-----------------------------------------------------------------------------
class TimersThread {                // The Timers control Thread
bool                   operational= true;
List<TimerToken>       pending= new List<TimerToken>(); // Pending events
ManualResetEvent       post= new ManualResetEvent(false); // Work event
Thread                 thread= null; // The associated Thread

public TimersThread( )
{
   thread= new Thread(new ThreadStart(run));
   thread.Start();
}

public void
   cancel(                          // Cancel this
     Object            o)           // Cancellation token
{
   TimerToken tt= o as TimerToken;  // Cast to TimerToken
   if( tt != null ) {               // If it *IS* a TimerToken
       bool removed;                // TRUE iff removed
       lock(pending) {
           removed= pending.Remove(tt);
       }

       if( removed ) {              // If removed
           tt.item.post((int)Item.CC.PURGE); // Indicate purged
       }
   }
}

public Object                       // Cancellation token
   delay(                           // Delay for
     double            seconds,     // This many seconds, then
     Item              item)        // Complete this work Item
{
   if( seconds < 0.015625 ) {       // If interval too short
       item.post();                 // It's already done
       return null;                 // And can't be cancelled
   }

   TimerToken tt= new TimerToken();
   tt.time= Utility.tod() + seconds;
   tt.item= item;

   lock(pending) {
       if( !operational ) {
           item.post((int)Item.CC.PURGE); // Indicate purged
           return null;
       }

       int M= pending.Count;
       bool insert= true;
       for(int i= 0; i<M; i++) {
           if( tt.time < pending[i].time ) {
               insert= false;
               pending.Insert(i, tt);
               break;
           }
       }
       if( insert )
           pending.Add(tt);

       post.Set();                  // Time to wake up
   }

   return tt;
}

public void run( )                  // Handle work
{
   Dispatch.trace("TimersThread running...");
   while( operational ) {
       Dispatch.trace("TimersThread operational");

       // Drive all expired timers
       int delay= 60000;            // Wait delay (milliseconds)
       for(;;) {
           TimerToken tt= null;
           lock(pending) {
               if( pending.Count > 0 ) {
                   double now= Utility.tod();
                   tt= pending[0];
                   if( tt.time > now ) {
                       if( (tt.time - now) < 60.0 )
                           delay= (int)((tt.time - now) * 1000.0);

                       tt= null;
                       break;
                   }

                   pending.RemoveAt(0);
               }
           }
           if( tt == null )
               break;

           tt.item.post();          // Indicate complete
       }

       Dispatch.trace(String.Format("TimersThread wait({0})", delay));
       post.WaitOne(delay);         // Wait for event
       post.Reset();                // Reset for next event
   }

   // Before we go, purge all pending requests
   lock(pending) {
       while( pending.Count > 0 ) {
           TimerToken tt= pending[0];
           pending.RemoveAt(0);
           tt.item.post((int)Item.CC.PURGE); // Indicate purged
       }
   }

   Dispatch.trace("...TimersThread terminated");
}

public void join( )                 // Join the thread
{
   Dispatch.trace("TimersThread.join()");
   thread.Join();                   // (Pass-through)
}

public void stop( )                 // Stop the thread
{
   Dispatch.trace("TimersThread.stop()");
   operational= false;
   post.Set();
}
}  // class TimersThread
}  // namespace Shared.Dispatch

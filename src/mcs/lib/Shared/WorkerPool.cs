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
//       WorkerPool.cs
//
// Purpose-
//       Worker ThreadPool.
//
// Last change date-
//       2019/02/15
//
//-----------------------------------------------------------------------------
#define USE_BACKGROUND              // Use background threads?

using System;
using System.Collections.Generic;   // For List
using System.Threading;             // For Thread

using Shared;                       // (Library namespace)

namespace Shared {                  // The Library namespace
//-----------------------------------------------------------------------------
//
// Interface-
//       Worker
//
// Purpose-
//       Define something a WorkerThread calls.
//
//-----------------------------------------------------------------------------
public interface Worker {           // The Worker interface
void run( );                        // The Worker method
}

//-----------------------------------------------------------------------------
//
// Class-
//       WorkerThread
//
// Purpose-
//       Define the WorkerPool Thread.
//
//-----------------------------------------------------------------------------
internal class WorkerThread {       // The WorkerPool Thread
bool                   operational= true; // Operational state
ManualResetEvent       @event= new ManualResetEvent(false); // Work event
Thread                 thread= null; // The associated Thread
Worker                 worker= null; // The associated Worker

internal WorkerThread(Worker worker= null) // The associated Worker
{
   this.worker= worker;

   thread= new Thread(new ThreadStart(work));
#if USE_BACKGROUND
   thread.IsBackground= true;
#endif
   thread.Start();
}

//-----------------------------------------------------------------------------
// WorkerThread.Methods
//-----------------------------------------------------------------------------
internal void drive(                // Drive work
     Worker            worker)      // With this Worker
{
   this.worker= worker;
   @event.Set();
}

internal void join( )               // Join the thread
{
   thread.Join();                   // (Pass-through)
}

internal void stop( )               // Stop the thread
{
   operational= false;
   @event.Set();
}

internal void work( )               // Handle work
{
   while( operational ) {
       if( worker != null ) {
           worker.run();
           worker= null;
       }

       WorkerPool.done(this);       // Return this WorkerThread to the pool

       @event.WaitOne();            // Wait for event
       @event.Reset();              // Reset for next event
   }
}
}  // class WorkerThread

//-----------------------------------------------------------------------------
//
// Class-
//       WorkerMain
//
// Purpose-
//       Define the WorkerPool main Worker
//
//-----------------------------------------------------------------------------
internal class WorkerMain {         // The WorkerPool Main Thread Queue<WorkerThread>    spare= new Queue<WorkerThread>();
Queue<WorkerThread>    spare= new Queue<WorkerThread>();

internal void release(              // Release
     WorkerThread      worker)      // This WorkerThread
{
#if USE_BACKGROUND
   worker.stop();

#else
   int count;

   lock(spare) {
       count= spare.Count;
       spare.Enqueue(worker);
   }

   if( count == 0 ) {
       ThreadPool.QueueUserWorkItem(work);
   }
#endif
}

internal void work(Object ignored)  // Handle work
{
   WorkerThread[] array;            // "spare" converted to an array

   lock(spare) {
       array= spare.ToArray();
       spare.Clear();
   }

   foreach( WorkerThread thread in array ) {
       thread.stop();
       thread.join();
   }
}
}  // class WorkerMain

//-----------------------------------------------------------------------------
//
// Static class-
//       WorkerPool
//
// Purpose-
//       The Worker ThreadPool.
//
//-----------------------------------------------------------------------------
public static class WorkerPool {    // The WorkerThread Thread pool
//-----------------------------------------------------------------------------
// WorkerPool.Attributes
//-----------------------------------------------------------------------------
static int             MAX_THREADS= 128; // The maximum number of pooled threads
static public int      MAX_RUNNING { get { return max_running; } }
static public int      MAX_SPARE   { get { return max_spare; } }

static WorkerThread[]  pool= new WorkerThread[MAX_THREADS]; // The pool
static WorkerMain      main= new WorkerMain(); // WorkerMain auxiliary Thread
static int             max_running= 0; // Maximum number of running threads
static int             max_spare= 0; // Maximum number of spare threads
static Object          mutex= new Object(); // Locking object
static int             running;     // The current number of running threads
static int             workers= 0;  // Total number of work() invocations
static int             spare;       // The current number of spare threads

//-----------------------------------------------------------------------------
// WorkerPool.debug()
//
// Debugging diagnostics
//-----------------------------------------------------------------------------
public static void
   debug( )                         // Diagnostics
{
   Debug.WriteLine("WorkerPool.debug()");
   Debug.WriteLine("{0,16} Threads",     workers);
   Debug.WriteLine("{0,16} Running",     running);
   Debug.WriteLine("{0,16} Max_Running", max_running);
   Debug.WriteLine("{0,16} Spare",       spare);
   Debug.WriteLine("{0,16} Max_Spare",   max_spare);
}

//-----------------------------------------------------------------------------
// WorkerPool.done()
//
// Process WorkerThread completion
//-----------------------------------------------------------------------------
internal static void
   done(                            // Work complete
     WorkerThread      worker)      // For this WorkerThread
{
   lock(mutex) {                    // Single-threaded access
       running--;

       if( spare >= pool.Length ) {
           main.release(worker);
       } else {
           pool[spare++]= worker;
           if( spare > max_spare )
               max_spare= spare;
       }
   }
}

//-----------------------------------------------------------------------------
// WorkerPool.reset()
//
// Empty the WorkerPool.
//-----------------------------------------------------------------------------
public static void
   reset( )                         // Reset the WorkerPool
{
   lock(mutex) {                    // Single-threaded access
       for(int i= 0; i<spare; i++) {
           main.release(pool[i]);
       }

       spare= 0;
   }
}

//-----------------------------------------------------------------------------
// WorkerPool.work()
//
// Process work using a WorkerThread.
//-----------------------------------------------------------------------------
public static void
   work(                            // Process work
     Worker            worker)      // Using this Worker
{
   lock(mutex) {                    // Single-threaded access
       workers++;

       if( spare > 0 ) {
           WorkerThread thread= pool[--spare];
           thread.drive(worker);
       } else {
           new WorkerThread(worker);
       }

       running++;
       if( running > max_running )
           max_running= running;
   }
}
}  // class WorkerPool
}  // namespace Shared

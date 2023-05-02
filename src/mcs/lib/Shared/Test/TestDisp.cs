//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestDisp.cs
//
// Purpose-
//       Test Dispatcher classes.
//
// Last change date-
//       2019/02/15
//
//----------------------------------------------------------------------------
#define USE_PROGRESS_COUNTER        // Use progress counter?

using System;

using Shared;                       // Test Library
using Shared.Dispatch;              // Test Library: Dispatcher

//============================================================================
// Test cases
//============================================================================
static class Test {
//----------------------------------------------------------------------------
//
// Class-
//       PassAlongTask
//
// Purpose-
//       Pass work to next Task in list.
//
//----------------------------------------------------------------------------
class PassAlongTask : Task {        // The "pass-along" Task
Task                   next;        // Next Task in list

public PassAlongTask(Task next) : base()
{
   this.next= next;
}

public override void
   work(                            // Process
     Item              item)        // This work Item
{
   next.enqueue(item);
}
}  // class PassAlongTask

//----------------------------------------------------------------------------
//
// Method-
//       test_bringup
//
// Purpose-
//       Dispatcher bringup test.
//
//----------------------------------------------------------------------------
static void test_bringup( )         // Test Dispatch.cs
{
   Dispatch disp= new Dispatch();
   Task task= new Task();
   Wait wait= new Wait();
   Item item= new Item(wait);

   object tt= disp.delay(1.125, item);
   disp.cancel(tt);
   wait.wait();
   Dispatch.trace(String.Format("tc item.cc({0})", item.cc));
   wait.reset();

   tt= disp.delay(1.125, item);
   wait.wait();
   Dispatch.trace(String.Format("tt item.cc({0})", item.cc));
   wait.reset();

   task.enqueue(item);
   wait.wait();
   Dispatch.trace(String.Format("nq item.cc({0})", item.cc));

   task.reset();
   disp.wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       test_timing
//
// Purpose-
//       Dispatch timing test.
//
//
//----------------------------------------------------------------------------
static void test_timing( )          // Test Dispatch.cs
{
// Dispatch            disp= new Dispatch();

   Task                final;       // The final Task
   Task[]              task;        // The pass-along Task array
   Item[]              item;        // The Item array
   Wait[]              wait;        // The Wait array

   // Set defaults
   int loops= 10240;                // Number of major iterations
   int multi= 160;                  // Number of elements queued per iteration
   int tasks= 120;                  // Number of pass-along Tasks

#if false
   loops= 100;                      // (In liu of parameter analysis)
#endif

   Debug.WriteLine("{0,16} LOOPS", loops);
   Debug.WriteLine("{0,16} MULTI", multi);
   Debug.WriteLine("{0,16} TASKS", tasks);

   // Create the Task array
   final= new Task();
   task= new Task[tasks];

   Task prior= final;
   for(int i= tasks-1; i>=0; i--) {
       Task t= new PassAlongTask(prior);
       prior= t;
       task[i]= t;
   }

   // Create the Item and Wait arrays
   item= new Item[multi];
   wait= new Wait[multi];
   for(int i= 0; i<multi; i++) {
       wait[i]= new Wait();
       item[i]= new Item(wait[i]);
   }

   // Run the test
   double start= Utility.tod();
   for(int loop= 0; loop < loops; loop++) {
       #if USE_PROGRESS_COUNTER
           if( (loop % 100) == 0 )
               Console.Write("{0,16} <<<<\r", loop);
       #endif

       for(int m= 0; m<multi; m++) {
           task[0].enqueue(item[m]);
       }

       for(int m= 0; m<multi; m++) {
           wait[m].wait();
           wait[m].reset();
       }
   }
   #if USE_PROGRESS_COUNTER
       Console.Write("{0,32}\r", "");
   #endif

   double now= Utility.tod();
   double elapsed= now - start;
   Debug.WriteLine("{0,16:0.000} elapsed", elapsed);
   double ops= (double)tasks + 1.0;
   ops *= (double)multi;
   ops *= (double)loops;
   Debug.WriteLine("{0,16:0.000} ops/second", ops/elapsed);

   WorkerPool.debug();
   WorkerPool.reset();
// disp.wait();
}

//============================================================================
// Test.all: Run all tests
//============================================================================
static public void all( ) {         // Run all tests
// test_bringup();                  // Bringup test
   test_timing();                   // Timing test
}
}  // static class Test

//============================================================================
// PROGRAM: Mainline code
//============================================================================
public class Program {
static void Main(string[] args)     // Mainline code
{
   Debug debug= new Debug("debug.log");

   Trace.start();                   // Run internal trace

   Test.all();

   Trace.stop();                    // Stop internal trace
   Trace.dump();                    // Dump internal trace

   debug.close();
}
}   // class Program

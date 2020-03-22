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
//       Test_threading.cs
//
// Purpose-
//       Multithreading tests.
//
// Last change date-
//       2019/02/15
//
// Implementation notes-
//       Question:
//         Why does static bool USE_LOCKING= Options.USE_LOCKING get the
//         correct value of Options.USE_LOCKING?
//       Answer: (When invoked from Sample.cs)
//         Options.parm() is called BEFORE Sample sample= new Sample(), and
//         new Test_threading() is called when the tests[] array is built in
//         the implied Sample() constructor.
//       However:
//         This construction order is not guaranteed. Options.USE_LOCKING
//         *MUST* be set before Test_threading.Test() is called, but setting
//         it before the Test_threading object is created is *NOT* required.
//         The static initialization of Test_threading.USE_LOCKING is not
//         needed. The initialization in Test() is needed.
//
//-----------------------------------------------------------------------------
using System;                       // (Almost always required)
using System.Threading;             // System Threading library

using Shared;                       // For Debug

namespace Sample {                  // The Sample namespace
//=============================================================================
//
// Class-
//       Test_threading
//
// Purpose-
//       Multithreading tests.
//
//=============================================================================
class Test_threading: Test {        // Multithreading tests
bool                   USE_LOCKING= Options.USE_LOCKING;

string                 info= "*INFO*:";
object                 mutex= new object(); // Used for locking
string                 pass= "PASSED:";
Random                 random= new Random();

static string[]        name=        // Thread names
{  "one", "two", "333", "444"};     // Enough names for now

int                    run_count= 0; // Run() counter

static double tod() { return Utility.tod(); }
void run() {run_count++;}           // A Simple runnable method
void multi_run()                    // A run method for multiple threads
{  run();                           // Update the counter

   Thread me= Thread.CurrentThread;
   string id= me.Name;

   int delay;                       // A random delay
   for(int i= 0; i<8; i++)
   {
     // Without knowing Random.Next internals, a lock is advisable
     lock(random) { delay= random.Next(1,500); } // Delay in millisconds
     if( USE_LOCKING )
         lock(mutex) { lock(mutex) { // (recursive locking allowed)
             Thread.Sleep(delay);
             if( Options.level > 1 && id == "one" )
                 Debug.WriteLine();
         } } // (recursive locking allowed, and demonstrated)
     else
         Thread.Sleep(delay);

     if( Options.level > 1 )
         Debug.WriteLine("{0} {1:0.000} {2} [{3,2}] delay({4,3})",
                         info, tod(), id, i, delay);
   }

   if( Options.level > 1 )
       Debug.WriteLine("{0} {1:0.000} {2} Complete", info, tod(), id);
}

void bringup()                      // Multithreading bringup test
{
   if( Options.level > 1 ) Debug.WriteLine("{0} bringup...", info);

   Thread t= new Thread(new ThreadStart(run)); // A Thread that runs run()
   t.Start();                       // Start the Thread
   t.Join();                        // Wait for thread completion

   Debug.assert( run_count == 1 );  // Verify thread operation
   if( Options.level > 1 ) Debug.WriteLine("{0} ...bringup", pass);
}

void test_00()                      // Test multiple threads
{
   if( Options.level > 1 )
       Debug.WriteLine("{0} test_00... USE_LOCKING({1})", info, USE_LOCKING);
   double elapsed= tod();

   run_count= 0;
   int max_thread= 4;               // The thread count
   ThreadStart runner= new ThreadStart(multi_run);
   Thread[] thread= new Thread[max_thread]; // The Thread array

   for(int i= 0; i<max_thread; i++) { // Create 'em
       thread[i]= new Thread(runner); // Already done
       thread[i].Name= name[i];
   }

   if( Options.level > 1 )
       Debug.WriteLine("{0} {1:0.000} Starting threads", info, tod());
   for(int i= 0; i<max_thread; i++) // Start 'em
       thread[i].Start();

   for(int i= 0; i<max_thread; i++) // Join 'em
       thread[i].Join();

   elapsed= tod() - elapsed;
   if( Options.level > 1 )
       Debug.WriteLine("{0} {1:0.000} elapsed", info, elapsed);
   if( Options.level > 1 ) Debug.WriteLine("{0} ...test_00", pass);
}

//=============================================================================
// Test_threading.Test: Test multithreading
//=============================================================================
public void Test(object obj)
{
   Debug.WriteLine("START>: Test_threading");
   USE_LOCKING= Options.USE_LOCKING; // Local USE_LOCKING option
   bringup();                       // Run bringup test
   test_00();                       // Run simple test
   Debug.WriteLine("PASSED: Test_threading");
}
}  // class Test_threading
}  // namespace Sample

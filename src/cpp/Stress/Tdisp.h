//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/Stress/Tdisp.h
//
// Purpose-
//       ~/Stress/Tdisp.cpp customization.
//
// Last change date-
//       2022/03/11
//
//----------------------------------------------------------------------------
#ifndef S_TDISP_H_INCLUDED
#define S_TDISP_H_INCLUDED

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // compilation controls
{  HCDM= false                      // Hard Core Debug Mode?

,  ITERATIONS= 10'240               // Default iterations/Task
,  TASK_COUNT=      1               // Default Task_count
,  TRACE_SIZE= 0x01000000           // Default trace table size

,  DISP_ITEMS=    160               // Default DispItems / iteration
,  DISP_TASKS=    120               // Default DispTasks / Thread
}; // enum compilation controls

//----------------------------------------------------------------------------
// Task interaction controls
//----------------------------------------------------------------------------
#include "Common.h"                 // Defines class Main, class Task

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static char*     const max_page= (char*)(uintptr_t(-1)); // Largest address

//----------------------------------------------------------------------------
// Module options: (Defaults enumerated in .h, options initialized in .cpp)
//----------------------------------------------------------------------------
static unsigned        opt_ditem= DISP_ITEMS; // Number of DispItems/iteration
static unsigned        opt_dtask= DISP_TASKS; // Number of DispTasks/Thread

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static uint64_t        elapsed;     // (Global) elapsed test time
static unsigned        thread_index= 0; // The global thread index

//----------------------------------------------------------------------------
//
// Class-
//       PassAlongTask
//
// Purpose-
//       Pass work to next Task in list.
//
//----------------------------------------------------------------------------
class PassAlongTask : public DispTask {
protected:
DispTask*              next;        // Next Task in list

public:
virtual
   ~PassAlongTask( void )
{
// IFHCDM( debugf("~PassAlongTask(%p) %2d\n", this, index); )
}

   PassAlongTask(
     DispTask*         next)
: DispTask()
, next(next)
{ }

virtual void
   work(
     DispItem*         item)
{  next->enqueue(item); }           // Give the work to the next Task
}; // class PassAlongTask

//----------------------------------------------------------------------------
//
// Class-
//       Thread
//
// Purpose-
//       Common test driver thread.
//
//---------------------------------------------------------------------------
class Thread : public Task {        // Thread test driver
//----------------------------------------------------------------------------
// Thread::Attributes
//----------------------------------------------------------------------------
public:
uint16_t               task= 0;     // Our Task index
uint16_t               _0001[3];    // Unused, reserved for alignment

// Dispatcher objects
DispTask               FINAL;       // The final DispTask
DispTask**             TASK;        // The PassAlongTask array
DispItem**             ITEM;        // The DispItem array
DispWait**             WAIT;        // The DispWait array

//----------------------------------------------------------------------------
// Thread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Thread( void )                  // Destructor
{
   // Cleanup
   FINAL.reset();
   for(unsigned i= 0; i<opt_dtask; i++) {
     TASK[i]->reset();
     delete TASK[i];
   }

   for(unsigned i= 0; i<opt_ditem; i++) {
     delete ITEM[i];
     delete WAIT[i];
   }

   delete WAIT;
   delete ITEM;
   delete TASK;
}

   Thread(                          // Constructor
     const char*       ident,       // The thread identifier
     unsigned          index)       // The thread index
:  Task(ident), task(index)
{
   // Create the Task array
   DispTask* prior= &FINAL;
   TASK= new DispTask*[opt_dtask];
   for(int i= opt_dtask-1; i >=0; i--) {
     DispTask* task= new PassAlongTask(prior);
     prior= task;
     TASK[i]= task;
   }

   // Create the ITEM and WAIT arrays
   ITEM= new DispItem*[opt_ditem];
   WAIT= new DispWait*[opt_ditem];
   for(unsigned i= 0; i < opt_ditem; i++) {
     WAIT[i]= new DispWait();
     ITEM[i]= new DispItem(0, WAIT[i]);
   }
}

//----------------------------------------------------------------------------
// Thread::Methods
//----------------------------------------------------------------------------
public:
virtual void
   test( void )                     // Run the test
{  if( HCDM ) debugf("Thread(%s)::test()\n", ident);

   //-------------------------------------------------------------------------
   // Run the test
   for(iteration= 1; iteration <= opt_iterations; iteration++) {
//   Record* record= (Record*)Trace::table->allocate_if(sizeof(Record));
//   if( record == nullptr )        // If trace inactive
//     break;                       // Abort test

     // Progress display
     if( (iteration % (opt_iterations/10)) == 0 ) {
       if( opt_verbose >= 2 && iteration < opt_iterations )
         debugf("%4d Thread(%s)  %'12zd of %'12zd\n", __LINE__, ident
                , iteration, opt_iterations);
     }

     for(unsigned i= 0; i < opt_ditem; i++)
       TASK[0]->enqueue(ITEM[i]);

     for(unsigned i= 0; i < opt_ditem; i++) {
       WAIT[i]->wait();
       WAIT[i]->reset();
     }
   }
}
}; // class Thread

//============================================================================
// Deferred method implementations
//----------------------------------------------------------------------------
// Main::make (Create Task superclass)
//----------------------------------------------------------------------------
Task* Main::make(const char* ident)
{  return new Thread(ident, thread_index++); }

//----------------------------------------------------------------------------
// Main::stats (Statistics display)
//----------------------------------------------------------------------------
void
   Main::stats( void )              // Statistics display
{  if( HCDM ) debugf("\nstatistics()\n");

   //-------------------------------------------------------------------------
   // Diagnostics
   if( opt_verbose >= 3 ) {         // If tracing
     //-----------------------------------------------------------------------
     // Dump the trace table
     debugf("\nTrace::table(%p)->dump() (See debug.out)\n", Trace::table);
     Trace::table->dump();
     if( opt_hcdm ) debug_flush();  // (Force dump completion)

     //-----------------------------------------------------------------------
     // Dispatcher diagnostics
     DispDisp::debug();
   }

   //-------------------------------------------------------------------------
   // Task completion status display
   debugf("\n");
   double ops= (double)opt_dtask + 1.0;
   ops *= (double)opt_ditem;

   double total= 0.0;
   for(int i= 0; i<opt_multi; i++) { // Display the thread status
     Thread* t= (Thread*)task_array[i];
     t->iteration--;                // (Iteration starts at one)

     double secs= (double)t->time/(double)GIGA_VALUE;
     double oper= ops * (double)t->iteration;
     double mega= oper/(double)MEGA_VALUE;
     double mega_sec= mega/secs;
     debugf("%'16.3f Mop/sec, Thread(%s) %'8.3f Mop in %8.3f sec\n"
            , mega_sec, t->ident , mega, secs);

     total += mega_sec;
   }
   debugf("%'16.3f Mop/sec, Thread(.TOT)\n", total);
   debugf("%'16.3f Test seconds\n", (double)elapsed / (double)GIGA_VALUE);
}
#endif // S_TDISP_H_INCLUDED

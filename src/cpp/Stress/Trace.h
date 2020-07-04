//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/Stress/Trace.h
//
// Purpose-
//       ~/Stress/Trace.cpp customization.
//
// Last change date-
//       2020/07/04
//
//----------------------------------------------------------------------------
#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // compliation controls
{  HCDM= false                      // Hard Core Debug Mode?

,  ITERATIONS= 10'000'000           // Default iterations/Task
,  TASK_COUNT= 4                    // Default Task_count
,  TRACE_SIZE= 0x01000000           // Default trace table size
}; // enum comilation controls

//----------------------------------------------------------------------------
// Task interaction controls
//----------------------------------------------------------------------------
#include "Common.h"                 // Defines class Main, class Task

//----------------------------------------------------------------------------
//
// struct-
//       Record
//
// Purpose-
//       In memory trace record descriptor.
//
//----------------------------------------------------------------------------
struct Record {                     // A standard (POD) trace record
enum { SIZE= 4 };                   // Sizeof ident

char                   ident[SIZE]; // The trace type identifier
uint32_t               unused;      // (Not used here)
uint64_t               clock;       // The UTC epoch clock, in nanoseconds
uint64_t               value[2];    // Sequence, inverted sequence

void
   debug(                           // Display the record
     int               line)        // Caller's line number
{  debugh("%4d Record(%p) debug()\n", line, this); } // NOT CODED YET

const char*                       // Get record identity (STATIC BUFFER)
   getIdent( void )
{  static char buffer[SIZE + 4];
   memcpy(buffer, ident, SIZE);
   buffer[SIZE]= '\0';
   return buffer;
}

uint32_t                          // Offset from Trace::trace
   offset( void )                 // Of this Record
{  return Trace::trace->offset(this); }

void
   trace(                           // Initialize the Record
     const char*       ident)       // The trace type identifier
{
   this->clock= epoch_nano();       // (Ordered initialization)
   memcpy(this->ident, ident, SIZE); // (Last)
}
}; // struct Record

//----------------------------------------------------------------------------
//
// Class-
//       Thread
//
// Purpose-
//       Common test driver thread.
//
//----------------------------------------------------------------------------
class Thread : public Task {        // Thread test driver
//----------------------------------------------------------------------------
// Thread::Attributes
//----------------------------------------------------------------------------
public:
uint64_t               pass;        // Passed operation counter
uint64_t               fail;        // Failed operation counter

//----------------------------------------------------------------------------
// Thread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Thread( void ) = default;       // Default destructor
   Thread(                          // Constructor
     const char*       ident)       // The thread identifier
:  Task(ident), pass(0), fail(0) { }

//----------------------------------------------------------------------------
// Thread::Methods
//----------------------------------------------------------------------------
public:
virtual void
   test( void )                     // Run the test
{  if( HCDM ) tracef("Thread(%s)::test()\n", ident);

   //-------------------------------------------------------------------------
   // Run the test
   for(iteration= 0; iteration < opt_iterations; iteration++) {
     Record* record= (Record*)Trace::trace->allocate_if(sizeof(Record));
     if( record ) {                 // Bringup test
       pass++;
       record->value[0]=  pass;
       record->value[1]= ~pass;
       record->trace(ident);
     } else {
       fail++;
     }
   }
}
}; // class Thread

//----------------------------------------------------------------------------
//
// struct-
//       TraceCounter
//
// Purpose-
//       TraceCounter descriptor.
//
//----------------------------------------------------------------------------
struct TraceCounter {               // A trace counter
enum { DIM= 32 };                   // Dimensionalityof traceCounter
enum { SIZE= Record::SIZE };        // Sizeof ident
static int             used;        // The number of TraceCounters used

// TraceCounter instance attributes ------------------------------------------
char                   ident[SIZE]; // Trace identifier
uint32_t               count;       // Number of entries
Record*                prior;       // Prior record
uint64_t               value;       // Prior record sequence number

// TraceCounter static methods -----------------------------------------------
static void
   update(Record*);                 // Update Record counters

static void
   sort( void );                    // Sort the traceCounter array

// TraceCounter instance methods ---------------------------------------------
void
   debug( void )                    // Display instance
{
   debugf("%.8x:%s %'10d %'12lu\n", prior->offset(), getIdent(), count, value);
}

const char*                       // Get record identity (STATIC BUFFER)
   getIdent( void )
{  static char buffer[SIZE + 4];
   memcpy(buffer, ident, Record::SIZE);
   buffer[SIZE]= '\0';
   return buffer;
}
}; // struct TraceCounter

//============================================================================
// External data areas
//----------------------------------------------------------------------------
int                    TraceCounter::used= 0; // No counters used
static TraceCounter    traceCounter[TraceCounter::DIM];

//============================================================================
// Deferred method implementations (Require traceCounter and/or Record)
//----------------------------------------------------------------------------
// TraceCounter::sort Sort the traceCounter array
//----------------------------------------------------------------------------
void
   TraceCounter::sort( void )       // Sort the traceCounter array
{
   for(unsigned i= 1; i<used; i++) { // Bubble sort is good enough
     for(unsigned j= i+1; j<used; j++) {
       if( memcmp(traceCounter[i].ident, traceCounter[j].ident, SIZE) > 0 ) {
         TraceCounter temp= traceCounter[i];
         traceCounter[i]= traceCounter[j];
         traceCounter[j]= temp;
       }
     }
   }
}

//----------------------------------------------------------------------------
// TraceCounter::update (Update associated TraceCounter)
//----------------------------------------------------------------------------
void
   TraceCounter::update(            // Update TraceCounter
     Record*           record)      // For this Record
{  // if( HCDM ) traceh("%4d TC.update(%x)\n", __LINE__, record->offset() );
   for(unsigned i= 1; i<DIM; i++) {
     if( memcmp(record->ident, traceCounter[i].ident, SIZE) == 0 ) {
       traceCounter[i].count++;     // Count the hit

       // Validate the entry
       if( (traceCounter[i].value + 1) != record->value[0] ) { // Sequential
         Record* prior= traceCounter[i].prior;
         debugf("%4d HCDM.h this(%x) last(%x)\n", __LINE__
                , record->offset(), prior->offset() );
         throwf("HCDM.h:%d %s Expected(%zd) This(%zd) Last(%zd)", __LINE__
                , record->getIdent(), traceCounter[i].value + 1
                , record->value[0], prior->value[0] );
       }
       if( record->value[0] != ~record->value[1] ) { // Self-check
         debugf("%4d HCDM.h this(%x)\n", __LINE__ , record->offset());
         throwf("HCDM.h:%d %s V[0](%zx) V[1](%zx)", __LINE__
                , record->getIdent(), record->value[0], record->value[1] );
       }

       traceCounter[i].value= record->value[0];
       traceCounter[i].prior= record;

       return;
     }
   }

   if( used >= DIM ) {              // If no rooom for more
     traceCounter[0].count++;       // Count the miss
     return;
   }

   if( used == 0 ) {                // If first miss
     memcpy(traceCounter[0].ident, ".???", SIZE); // Overflow
     traceCounter[0].count= 0;
     traceCounter[0].prior= (Record*)Trace::trace;
     traceCounter[0].value= 0;
     used++;
   }

   if( record->ident[0] != '.' || record->ident[1] != '0' ) {
     traceCounter[0].count++;
     traceCounter[0].prior= record;
     return;
   }

   memcpy(traceCounter[used].ident, record->ident, SIZE);
   traceCounter[used].count= 1;
   traceCounter[used].prior= record;
   traceCounter[used].value= record->value[0];

   used++;
}

//----------------------------------------------------------------------------
// Main::make (Create Task superclass)
//----------------------------------------------------------------------------
Task* Main::make(const char* ident) { return new Thread(ident); }

//----------------------------------------------------------------------------
// Main::stats (Statistics display)
//----------------------------------------------------------------------------
void
   Main::stats( void )              // Statistics display
{  if( HCDM ) debugf("\nstatistics()\n");

   if( opt_verbose >= 1 ) {         // If verbose, dump trace table
     debugf("\n");
     debugf("Trace::trace(%p)->dump() (See debug.out)\n", Trace::trace);
     Trace::trace->dump();
     if( opt_hcdm ) debug_flush();  // (Force log completion)
   }

   unsigned count= 0;               // Number of trace records

   // Trace table analysis
   Trace* trace= Trace::trace;      // The Trace object
   Record* origin= (Record*)((char*)trace + trace->zero);
   Record* middle= (Record*)((char*)trace + trace->next);
   Record* ending= (Record*)((char*)trace + trace->size);

// One-time validators (OK)
// debugf("%4d HCDM.m (%p,%p,%p)\n", __LINE__, origin, middle, ending);
// debugf("%4d HCDM.m (%x,%x,%x)\n", __LINE__
//        , origin->offset(), middle->offset(), ending->offset());

   Record* record= middle;
   while( record < ending ) {
     if( record->ident[0] == '.' ) { // If valid Record
       TraceCounter::update(record);
       count++;
     }

     record++;
   }

   record= origin;
   while( record < middle ) {
     if( record->ident[0] == '.' ) { // If valid Record
       TraceCounter::update(record);
       count++;
     }

     record++;
   }

   // Trace table analysis display
   debugf("\n");
   debugf("Trace.wrap(%'lu), next(0x%.8x), last(0x%.8x), size(0x%.8x)\n"
          , trace->wrap, trace->next.load(), trace->last, trace->size );
   debugf("-------- Current trace table --------\n");
   debugf("  offset:Type      Count        Value\n");
   size_t total= 0;                 // Number of valid records
   TraceCounter::sort();            // Sort the table
   for(unsigned i= 0; i<TraceCounter::used; i++) {
     traceCounter[i].debug();
     total += traceCounter[i].value;
   }

   debugf("Records/Pass: %'10u %'12zd\n", count, total);

   //-------------------------------------------------------------------------
   // Task completion status display
   debugf("\n");
   for(unsigned i= 0; i<opt_multi; i++) { // Bubble sort the task_array
     Thread* it= (Thread*)task_array[i];
     for(unsigned j= i+1; j<opt_multi; j++) {
       Thread* jt= (Thread*)task_array[j];
       if( jt->time < it->time ) {
         task_array[i]= jt;
         task_array[j]= it;
         it= jt;
       }
     }
   }

   for(unsigned i= 0; i<opt_multi; i++) { // Display the thread status
     Thread* it= (Thread*)task_array[i];

     double secs= (double)it->time/(double)GIGA_VALUE;
     debugf("Thread(%s) %7.3f sec %'12zd pass, %'12zd fail\n", it->ident
            , secs, it->pass, it->fail);
     if( false ) {                  // Display per second values?
       double pass_sec= (double)it->pass/secs;
       double mega_sec= pass_sec/(double)MEGA_VALUE;
       debugf(">>>> Rating: %7.3f Mps\n", mega_sec); // MegaOps / second
     }
   }
}
#endif // TRACE_H_INCLUDED

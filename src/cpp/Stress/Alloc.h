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
//       ~/Stress/Alloc.h
//
// Purpose-
//       ~/Stress/Alloc.cpp customization.
//
// Last change date-
//       2020/07/04
//
//----------------------------------------------------------------------------
#ifndef S_ALLOC_H_INCLUDED
#define S_ALLOC_H_INCLUDED

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // compliation controls
{  HCDM= false                      // Hard Core Debug Mode?

,  ITERATIONS= 1'000'000            // Default iterations/Task
,  SIZE_ALLOC= 1024                 // Default maximum allocation size
,  SLOT_COUNT= 8192                 // Default slot count
,  TASK_COUNT= 4                    // Default Task_count
,  TRACE_SIZE= 0x01000000           // Default trace table size
}; // enum comilation controls

//----------------------------------------------------------------------------
// Task interaction controls
//----------------------------------------------------------------------------
#include "Common.h"                 // Defines class Main, class Task

//----------------------------------------------------------------------------
// Module options: (Defaults enumerated in .h, options initialized in .cpp)
//----------------------------------------------------------------------------
static pub::Allocator* allocator= nullptr; // The selected allocator
static const char*     opt_alloc= "std"; // The selected allocator's name
static unsigned        opt_slots= SLOT_COUNT; // The number of slots/Thread
static unsigned        opt_smaxs= SIZE_ALLOC; // Maximum allocation size

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char*     const max_page= (char*)(uintptr_t(-1)); // Largest address
static unsigned        thread_index= 0; // The global thread index

//----------------------------------------------------------------------------
//
// Struct-
//       Record
//
// Purpose-
//       In memory trace record descriptor.
//
//----------------------------------------------------------------------------
// Trace Record identifiers
static const char*     ID_FIND= ".NEW"; // Storage allocation
static const char*     ID_FREE= ".DEL"; // Storage release

struct Record {                     // A standard (POD) trace record
enum { SIZE= 4 };                   // Sizeof ident

char                   ident[SIZE]; // The trace type identifier
uint16_t               task;        // The Task index
uint16_t               slot;        // The Slot index
uint64_t               clock;       // The UTC epoch clock, in nanoseconds
void*                  memory;      // The Slot storage address
size_t                 length;      // The Slot storage length

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
     const char*       ident,       // The trace type identifier
     int               task,        // The Task index
     int               slot,        // The Slot index
     void*             addr,        // The storage address
     size_t            size)        // The storage length
{
   this->task= task;
   this->slot= slot;
   this->clock= epoch_nano();
   this->memory= addr;
   this->length= size;

   memcpy(this->ident, ident, SIZE); // (Last)
}
}; // struct Record

//----------------------------------------------------------------------------
//
// Struct-
//       Slot
//
// Purpose-
//       Slot data descriptor.
//
// Implementation note-
//       Length must be a power of two.
//
//----------------------------------------------------------------------------
struct Slot_word {                  // A Slot validator
uintptr_t              pos;         // The origin address
uintptr_t              neg;         // ~pos
}; // struct Slot_word

struct Slot {                       // Slot data descriptor
Slot_word*             addr;        // Slot data address
size_t                 size;        // Slot data length

void
   find(                            // Allocate slot data
     size_t            size)        // Of this length
{
   size +=  (sizeof(Slot_word) - 1); // Round up
   size &= ~(sizeof(Slot_word) - 1); // Truncate down
   this->size= size;

   Slot_word* addr= (Slot_word*)allocator->get(size); // Allocate the storage
   uintptr_t pos= uintptr_t(addr);  // The address, as is
   uintptr_t neg= ~pos;             // The inverted address
   this->addr= addr;

   size /= sizeof(Slot_word);       // The number of Slot_words
   for(size_t i= 0; i<size; i++) {  // Set slot verifier
     addr->pos= pos;
     addr->neg= neg;
     ++addr;
   }
}

void
   free(unsigned index)             // Release slot[index]
{
   if( is_valid(index) == false )   // Verify the data
     throw "Verification fault";

   allocator->put(addr, size);      // Release the storage
   this->addr= nullptr;
   this->size= 0;
}

int                                 // TRUE iff slot is valid
   is_valid(unsigned index)         // Verify slot[index]
{
   Slot_word* addr= this->addr;     // The storage address
   if( addr == nullptr )            // If not allocated
     return true;                   // Free slots always valid

   uintptr_t pos= uintptr_t(addr);  // The address, as is
   uintptr_t neg= ~pos;             // The inverted address

   size_t size= this->size / sizeof(Slot_word); // The number of Slot_words
   for(size_t i= 0; i<size; i++) {
     if( addr->pos != pos ) {
       debugh("%4d slot[%u].pos(%.16lx), not(%.16lx)\n", __LINE__, index
              , addr->pos, pos);
       return false;
     }
     if( addr->neg != neg ) {
       debugh("%4d slot[%u].neg(%.16lx), not(%.16lx)\n", __LINE__, index
              , addr->neg, neg);
       return false;
     }

     memcpy(addr, "FREEfreeFREEfree", sizeof(Slot_word));
     ++addr;
   }

   return true;
}
}; // struct Slot
static_assert( (sizeof(Slot) & (sizeof(Slot)-1)) == 0, "Slot size not 2**n");
static_assert( sizeof(Slot) == sizeof(Slot_word), "Slot_word size invalid");

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
Slot*               slot_array= nullptr; // The slot array
uint16_t            task= 0;        // Our Task index
uint16_t            _0001[3];       // Unused, reserved for alignment

// Statistics
size_t              num_find= 0;    // Number of find requests
size_t              num_free= 0;    // Number of free requests

size_t              max_size= 0;    // Maximum allocation bytes
size_t              now_size= 0;    // Current allocation bytes

unsigned            max_slot= 0;    // Maximum number of slots used
unsigned            now_slot= 0;    // Current number of slots used

// Randomizer controls
std::random_device  rd;             // Use to seed generator
std::mt19937        mt;             // Standard mersenne_twister_engine
std::uniform_int_distribution<>
                    ud_size;        // Uniform size distributor
std::uniform_int_distribution<>
                    ud_slot;        // Uniform slot distributor

//----------------------------------------------------------------------------
// Thread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Thread( void )                  // Destructor
{
   if( slot_array ) free(slot_array);
   slot_array= nullptr;
}

   Thread(                          // Constructor
     const char*       ident,       // The thread identifier
     unsigned          index)       // The thread index
:  Task(ident), task(index)
,  rd(), mt(rd()), ud_size(1, opt_smaxs), ud_slot(0, opt_slots - 1)
{
   slot_array= (Slot*)malloc(opt_slots * sizeof(Slot));
   if( slot_array == nullptr ) throw std::bad_alloc();
   for(size_t i= 0; i<opt_slots; i++) {
     slot_array[i].addr= nullptr;
     slot_array[i].size= 0;
   }
}

//----------------------------------------------------------------------------
// Thread::Methods
//----------------------------------------------------------------------------
public:
char*                               // The next storage page, if any
   next_page(                       // Get next storage page
     char*             after)       // After this page
{
   char* min_page= max_page;
   for(unsigned slot= 0; slot<opt_slots; slot++) {
     char* slot_page= (char*)slot_array[slot].addr;
     slot_page= (char*)(uintptr_t(slot_page) & PAGE_MASK);
     if( slot_page > after && slot_page < min_page )
       min_page= slot_page;
   }

   if( min_page == max_page )       // If no more pages remain
     min_page= nullptr;

   return min_page;
}

size_t                              // The size of this storage area
   this_size(                       // Get size of this storage area
     char*             page)        // Starting at this page
{
   size_t size= PAGE_SIZE;          // (This page counts)
   for(;;) {                        // Find consecutive pages
     char* next= next_page(page);   // Find next page
     page += PAGE_SIZE;             // The next contiguous page
     if( next != page )             // If discontiguous
       break;

     size += PAGE_SIZE;             // Contiguous area found
   }

   return size;
}

virtual void
   test( void )                     // Run the test
{  if( HCDM ) debugf("Thread(%s)::test()\n", ident);

   //-------------------------------------------------------------------------
   // Run the test
   for(iteration= 1; iteration <= opt_iterations; iteration++) {
     Record* record= (Record*)Trace::trace->allocate_if(sizeof(Record));
     if( record == nullptr )      // If trace inactive
       break;                     // Abort test

     // Progress display
     if( (iteration % (opt_iterations/10)) == 0 ) {
       if( opt_verbose >= 1 && iteration < opt_iterations )
         debugf("%4d Thread(%s)  %'12zd of %'12zd\n", __LINE__, ident
                , iteration, opt_iterations);
     }

     unsigned S= ud_slot(mt);     // Select a slot index
     Slot* slot= slot_array + S;  // The associated Slot
     if( slot->addr ) {           // If allocated
       now_size -= slot->size;    // (Decrement *before* free)
       now_slot--;                // Slot available

       record->trace(ID_FREE, task, S, slot->addr, slot->size);
       slot->free(S);             // Remove it

       num_free++;                // Count free operation
     } else {                     // If not allocated
       size_t size= ud_size(mt);  // Select a size
       slot->find(size);          // Allocate slot storage
       record->trace(ID_FIND, task, S, slot->addr, slot->size);

       now_size += slot->size;    // Increment *after* find
       if( now_size > max_size ) max_size= now_size;
       now_slot++;                // Slot in use
       if( now_slot > max_slot ) max_slot= now_slot;

       num_find++;                // Count find operation
     }
   }
}
}; // class Thread

//----------------------------------------------------------------------------
// next_page: (Global) next storage page
//----------------------------------------------------------------------------
static inline char*                 // The next storage page, if any
   next_page(                       // Get next storage page
     char*             after)       // After this page
{
   char* min_page= max_page;
   for(unsigned i= 0; i<opt_multi; i++) {
     Thread* t= (Thread*)task_array[i];
     char* page= t->next_page(after);
     if( page > after && page < min_page ) {
       min_page= page;
     }
   }

   if( min_page == max_page )       // If no more pages remain
     min_page= nullptr;

   return min_page;
}

//----------------------------------------------------------------------------
// slot_zero: (Global) The lowest allocated slot address
//----------------------------------------------------------------------------
static inline char*                 // The lowest allocated slot address
   slot_zero( void )                // The lowest allocated slot address
{
   char* min_slot= max_page;
   for(unsigned i= 0; i<opt_multi; i++) {
     Thread* t= (Thread*)task_array[i];
     for(unsigned s= 0; s<opt_slots; s++) {
       char* addr= (char*)t->slot_array[s].addr;
       if( addr && addr < min_slot )
          min_slot= addr;
     }
   }

   if( min_slot == max_page )
     min_slot= nullptr;

   return min_slot;
}

//----------------------------------------------------------------------------
// this_size: (Global) size of storage area
//----------------------------------------------------------------------------
static inline size_t                // The size of this storage area
   this_size(                       // Get size of this storage area
     char*             page)        // Starting at this page
{
   bool found= true;
   size_t tot_size= 0;              // Total contiguous size
   while( found ) {                 // Repeat for each contiguous area
     found= false;
     size_t max_size= 0;
     for(unsigned i= 0; i<opt_multi; i++) {
       Thread* t= (Thread*)task_array[i];
       size_t size= t->this_size(page);
       if( size > max_size ) {
         max_size= size;
       }
     }

     tot_size += max_size;          // (Always non-zero)
     page += max_size;              // (Where this area ends)

     // Now we need to see if any thread has storage allocated where this
     // area ends. If so, we start again.
     if( page == next_page(page - PAGE_SIZE) ) {
       found= true;
     }
   }

   return tot_size;
}

//----------------------------------------------------------------------------
// thread_sort: Sort the Thread array
//----------------------------------------------------------------------------
static inline void                  // (Currently unused)
   unused_sort( void )              // Sort the Thread array
{
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
}

//============================================================================
// Deferred method implementations (Require traceCounter and/or Record)
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
   for(unsigned i= 0; i<opt_multi; i++) { // Verify the slot table
     Thread* t= (Thread*)task_array[i];
     for(unsigned s= 0; s<opt_slots; s++) { // Verify the slot table
       if( ! t->slot_array[s].is_valid(s) ) { // If invalid slot
         opt_verbose= 3;            // Debugging required
         debugf("%4d ERROR: Invalid slot data [%u][%u]\n", __LINE__, i, s);
         break;                     // (Only display one fault per thread)
       }
     }
   }

   if( opt_verbose >= 2 ) {
     Debug* debug= Debug::get();
     FILE* file= debug->get_FILE();

     //-----------------------------------------------------------------------
     // Dump the trace table
     debugf("\nTrace::trace(%p)->dump() (See debug.out)\n", Trace::trace);
     Trace::trace->dump();
     if( opt_hcdm ) debug_flush();  // (Force dump completion)

     //-----------------------------------------------------------------------
     // Dump the (combined) slot table
     tracef("\n");
     for(unsigned i= 0; i<opt_multi; i++) { // Dump the slot table (by Thread)
       Thread* t= (Thread*)task_array[i];
       for(unsigned slot= 0; slot<opt_slots; slot++) {
         if( t->slot_array[slot].addr )
           tracef("[%2d][%6d] 0x%.12zx.%.8zx\n", i, slot
                  , vtos(t->slot_array[slot].addr), t->slot_array[slot].size);
       }
     }

     //-----------------------------------------------------------------------
     // Dump all pages with allocated storage (merged across Threads)
     tracef("\n%.16zX  (slot_zero)\n", vtos(slot_zero()) );
     char* last= nullptr;           // Start at the beginning
     for(;;) {                      // Dump pages with storage
       char* page= next_page(last);
       if( page == nullptr )
         break;

       if( last )                   // If there's a gap
         tracef("\n%.16zX  to %.16zX, GAP\n\n", vtos(last), vtos(page) - 1);

       size_t size= this_size(page);
       pub::utility::dump(file, page, size);
       last= page + size;
     }
   }

   //-------------------------------------------------------------------------
   // Task completion status display
   debugf("\n");
   double total= 0.0;
   for(unsigned i= 0; i<opt_multi; i++) { // Display the thread status
     Thread* t= (Thread*)task_array[i];
     t->iteration--;                      // (Iteration starts at one)

     double secs= (double)t->time/(double)GIGA_VALUE;
     double iter_sec= (double)t->iteration/secs;
     double mega_ops= iter_sec/(double)MEGA_VALUE;
     debugf("Thread(%s) %7.3f Mop/sec, %'12zd Ops in %8.3f sec\n", t->ident
            , mega_ops, t->iteration, secs);

     total += mega_ops;
   }
   debugf("Thread(.TOT) %7.3f Mop/sec\n", total);

   //-----------------------------------------------------------------------
   // Display Thread  statistics
   for(unsigned i= 0; i<opt_multi; i++) { // Dump the thread state
     Thread* t= (Thread*)task_array[i];
     debugf("\nThread(%s)::stats()\n", t->ident);
     debugf("..num_find(%'zd) num_free(%'zd)\n", t->num_find, t->num_free);
     debugf("..max_size(%'zd) max_slot(%'u)\n", t->max_size, t->max_slot);
     debugf("..now_size(%'zd) now_slot(%'u)\n", t->now_size, t->now_slot);
   }
}
#endif // S_ALLOC_H_INCLUDED

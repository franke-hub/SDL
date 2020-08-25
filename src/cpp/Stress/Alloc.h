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
//       2020/08/23
//
//----------------------------------------------------------------------------
#ifndef S_ALLOC_H_INCLUDED
#define S_ALLOC_H_INCLUDED

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define USE_GLOBAL_SLOT true        // Use global slot_array?
#define USE_RANDOM_SEED true        // Use Random Seed?

enum // compilation controls
{  HCDM= false                      // Hard Core Debug Mode?

,  ITERATIONS= 1'000'000            // Default iterations/Task
,  SIZE_ALLOC= 1024                 // Default maximum allocation size
,  SIZE_BLOCK= 256                  // Default block allocation size
,  SLOT_COUNT= 8192                 // Default slot count
,  TASK_COUNT= 4                    // Default Task_count
,  TRACE_SIZE= 0x01000000           // Default trace table size
}; // enum compilation controls

//----------------------------------------------------------------------------
// Task interaction controls
//----------------------------------------------------------------------------
#include "Common.h"                 // Defines class Main, class Task

//----------------------------------------------------------------------------
// Module options: (Defaults enumerated in .h, options initialized in .cpp)
//   Implementation note: opt_minsz must be >= sizeof(Slot)
//----------------------------------------------------------------------------
static pub::Allocator* allocator= nullptr; // The selected allocator
static const char*     opt_alloc= "std"; // The selected allocator's name
static unsigned        opt_slots= SLOT_COUNT; // The number of slots/Thread
static unsigned        opt_maxsz= SIZE_ALLOC; // Maximum allocation size
static unsigned        opt_minsz= 16;    // Minimum allocation size

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char*     const max_page= (char*)(uintptr_t(-1)); // Largest address
static unsigned        thread_index= 0; // The global thread index

#if USE_GLOBAL_SLOT
struct Slot;                        // Forward reference
static Slot*           slot_array= nullptr; // The slot array

typedef std::atomic_int64_t atomic_int64_t;

static atomic_int64_t  num_find;    // Number of find requests
static atomic_int64_t  num_free;    // Number of free requests
static atomic_int64_t  num_lock;    // Number of lock collisions

static atomic_int64_t  max_size;    // Maximum allocation bytes
static atomic_int64_t  now_size;    // Current allocation bytes

static atomic_int64_t  max_slot;    // Maximum number of slots used
static atomic_int64_t  now_slot;    // Current number of slots used
#endif

//----------------------------------------------------------------------------
// truncate: Remove low order bits from an address
//----------------------------------------------------------------------------
static inline void*                 // The address, rounded down
   truncate(                        // Remove low order address bits
     void*             address)     // From this address
{
   uintptr_t result= uintptr_t(address) & uintptr_t(~0x0007);
   return (void*)result;
}

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
static const char*     ID_FIND= ".GET"; // Storage allocation
static const char*     ID_FREE= ".PUT"; // Storage release

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

const char*                         // Get record identity (STATIC BUFFER)
   getIdent( void )
{  static char buffer[SIZE + 4];
   memcpy(buffer, ident, SIZE);
   buffer[SIZE]= '\0';
   return buffer;
}

uint32_t                            // Offset from Trace::trace
   offset( void )                   // Of this Record
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
static const Slot_word FREE_SLOT= {0xC0ffeeCafeC0ffee, 0xeeffc0fecaeeffc0};

struct Slot {                       // Slot data descriptor
Slot_word*             addr;        // Slot data address
size_t                 size;        // Slot data length

// Slot( void ) : addr(nullptr), size(0) {} // Constructor (unused)
// ~Slot( void ) { if( addr ) { free(); } // Destructor    (unused)

unsigned                            // The Slot index
   get_index( void );               // Get Slot index (implementation deferred)

bool                                // TRUE iff lock obtained
   try_lock( void )                 // Attempt to get lock
{
   std::atomic<uintptr_t>* atomic_addr= (std::atomic<uintptr_t>*)(&addr);
   uintptr_t old= atomic_addr->load();
   old &= uintptr_t(~0x0007);
   return atomic_addr->compare_exchange_strong(old, (old | 1));
}

void
   unlock(                          // Release the lock
     void*             word)        // Setting this address
{
   std::atomic<uintptr_t>* atomic_addr= (std::atomic<uintptr_t>*)(&addr);
   atomic_addr->store(uintptr_t(word));
}

void*                               // The allocated slot address
   find(                            // Allocate slot data
     size_t            size)        // Of this length
{
// size +=  (sizeof(Slot_word) - 1); // Round up
// size &= ~(sizeof(Slot_word) - 1); // Truncate down
   this->size= size;

   void* addr= allocator->get(size); // Allocate the storage
   uintptr_t pos= uintptr_t(addr);  // The address, as is
   uintptr_t neg= ~pos;             // The inverted address

   Slot_word* word= (Slot_word*)addr;
   size /= sizeof(Slot_word);       // The number of (complete) Slot_words
   for(size_t i= 0; i<size; i++) {  // Set slot verifier
     word->pos= pos;
     word->neg= neg;
     ++word;
   }

   unlock(addr);                    // (Set the address, releasing the lock)
   return addr;
}

void
   free( void )                    // Release slot
{
   void* addr= truncate(this->addr); // (Remove lock bit)
   if( ! is_valid() ) {            // Verify the data
     pub::utility::dump(addr, this->size);
     debug_flush();

     throw "Verification fault";
   }

   // Mark slot storage free
   Slot_word* word= (Slot_word*)addr;
   size_t size= this->size / sizeof(Slot_word); // The number of Slot_words
   for(size_t i= 0; i<size; i++) {
     word[i]= FREE_SLOT;
   }

   allocator->put(addr, this->size); // Release the storage

   this->size= 0;
   unlock(nullptr);
}

int                                 // TRUE iff slot is valid
   is_valid( void )                 // Verify this Slot
{
   void* addr= truncate(this->addr); // (Remove lock bit)
   if( addr == nullptr )            // If not allocated
     return true;                   // Free slots always valid

   uintptr_t pos= uintptr_t(addr);  // The address, as is
   uintptr_t neg= ~pos;             // The inverted address

   Slot_word* word= (Slot_word*)addr;
   size_t size= this->size / sizeof(Slot_word); // The number of Slot_words
   for(size_t i= 0; i<size; i++) {
     if( word->pos != pos ) {
       debugf("%4d slot[%4u][%4zd].pos(0x%.16lx), not(0x%.16lx)\n", __LINE__
              , get_index(), i, word->pos, pos);
       return false;
     }
     if( word->neg != neg ) {
       debugf("%4d slot[%4u][%4zd].neg(0x%.16lx), not(0x%.16lx)\n", __LINE__
              , get_index(), i, word->neg, neg);
       return false;
     }
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
#if USE_GLOBAL_SLOT == false
Slot*                  slot_array= nullptr; // The slot array
#endif

uint16_t               task= 0;     // Our Task index
uint16_t               _0001[3];    // Unused, reserved for alignment

// Statistics
#if USE_GLOBAL_SLOT == false
size_t                 num_find= 0; // Number of find requests
size_t                 num_free= 0; // Number of free requests
size_t                 num_lock= 0; // Number of lock collisions

size_t                 max_size= 0; // Maximum allocation bytes
size_t                 now_size= 0; // Current allocation bytes

unsigned               max_slot= 0; // Maximum number of slots used
unsigned               now_slot= 0; // Current number of slots used
#endif

// Randomizer controls
std::random_device     rd;          // Use to seed generator
std::mt19937           mt;          // Standard mersenne_twister_engine
std::uniform_int_distribution<>
                       ud_size;     // Uniform size distributor
std::uniform_int_distribution<>
                       ud_slot;     // Uniform slot distributor

//----------------------------------------------------------------------------
// Thread::Constructor
//----------------------------------------------------------------------------
public:
   Thread(                          // Constructor
     const char*       ident,       // The thread identifier
     unsigned          index)       // The thread index
:  Task(ident), task(index)
#if USE_RANDOM_SEED
,  rd(), mt(rd()), ud_size(opt_minsz, opt_maxsz), ud_slot(0, opt_slots - 1)
#else // Use repeatable result
,  rd(), mt(732), ud_size(opt_minsz, opt_maxsz), ud_slot(0, opt_slots - 1)
#endif
{
   if( opt_hcdm ) debugf("Thread(%p)::Thread\n", this);

#if USE_GLOBAL_SLOT == false
   slot_array= (Slot*)malloc(opt_slots * sizeof(Slot));
   if( slot_array == nullptr ) throw std::bad_alloc();
   for(size_t i= 0; i<opt_slots; i++) {
     Slot& slot= slot_array[i];
     slot.addr= nullptr;
     slot.size= 0;
   }
#endif
}

//----------------------------------------------------------------------------
// Thread::Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Thread( void )                  // Destructor
{
   if( opt_hcdm ) debugf("Thread(%p)::~Thread\n", this);

#if USE_GLOBAL_SLOT == false
   if( slot_array ) {
     for(size_t S= 0; S<opt_slots; S++) { // Release any allocated slots
       Slot& slot= slot_array[S];
       if( slot.addr ) {
         Record* record= (Record*)Trace::trace->allocate_if(sizeof(Record));
         if( record ) record->trace(ID_FREE, task, S, slot.addr, slot.size);
         slot.free();
       }
     }

     free(slot_array);
     slot_array= nullptr;
   }
#endif
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

   if( true  ) {                    // TODO: REMOVE.
     for(unsigned i= 0; i<opt_slots; i++) { // Verify slot.get_index
       if( slot_array[i].get_index() != i )
         debugf("%5d= slot[%5d].get_index()\n", slot_array[i].get_index(), i);
     }
   }

   //-------------------------------------------------------------------------
   // Run the test
   for(iteration= 1; iteration <= opt_iterations; iteration++) {
     Record* record= (Record*)Trace::trace->allocate_if(sizeof(Record));
     if( record == nullptr )        // If trace inactive
       break;                       // Abort test

     // Progress display
     if( (iteration % (opt_iterations/10)) == 0 ) {
       if( opt_verbose >= 2 && iteration < opt_iterations )
         debugf("%4d Thread(%s)  %'12zd of %'12zd\n", __LINE__, ident
                , iteration, opt_iterations);
     }

     unsigned S;                    // Slot index
     Slot* slot;                    // Slot address
     for(;;) {
       S= ud_slot(mt);              // Select a slot index
       slot= slot_array + S;        // Address the associated Slot
       if( slot->try_lock() )       // And lock it
         break;

       num_lock++;                  // Count lock interference
     }

     void* slot_addr= truncate(slot->addr);
     if( slot_addr ) {              // If allocated
       now_size -= slot->size;      // (Decrement *before* free)
       now_slot--;                  // Slot available

       record->trace(ID_FREE, task, S, slot_addr, slot->size);
       slot->free();                // Remove it, freeing the lock

       num_free++;                  // Count free operation
     } else {                       // If not allocated
       size_t size= ud_size(mt);    // Select a size
       slot_addr= slot->find(size); // Allocate slot storage
       record->trace(ID_FIND, task, S, slot_addr, slot->size);

#if USE_GLOBAL_SLOT
       now_size += size;            // Increment *after* find
       if( now_size.load() > max_size.load() ) max_size.store(now_size);
       now_slot++;                  // Slot in use
       if( now_slot.load() > max_slot.load() ) max_slot.store(now_slot);

       num_find++;                  // Count find operation
#else
       now_size += size;            // Increment *after* find
       if( now_size > max_size ) max_size= now_size;
       now_slot++;                  // Slot in use
       if( now_slot > max_slot ) max_slot= now_slot;

       num_find++;                  // Count find operation
#endif
     }
   }
}
}; // class Thread

//----------------------------------------------------------------------------
// Slot::get_index: Get current Slot index (only called if error encountered)
//----------------------------------------------------------------------------
unsigned
   Slot::get_index( void )          // Get current Slot index
{
#if USE_GLOBAL_SLOT
   unsigned index= this - slot_array;
#else
   unsigned index= opt_slots;       // Default, invalid slot index
   Thread* thread= dynamic_cast<Thread*>(Thread::current());
   if( thread )
     index= this - thread->slot_array;
#endif

   return index;
}

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
#if USE_GLOBAL_SLOT
   char* min_slot= max_page;
   for(unsigned s= 0; s<opt_slots; s++) {
     char* addr= (char*)slot_array[s].addr;
     if( addr && addr < min_slot )
        min_slot= addr;
   }
#else
   char* min_slot= max_page;
   for(unsigned i= 0; i<opt_multi; i++) {
     Thread* t= (Thread*)task_array[i];
     for(unsigned s= 0; s<opt_slots; s++) {
       char* addr= (char*)t->slot_array[s].addr;
       if( addr && addr < min_slot )
          min_slot= addr;
     }
   }
#endif

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
#if USE_GLOBAL_SLOT
   for(unsigned s= 0; s<opt_slots; s++) { // Verify the slot table
     if( ! slot_array[s].is_valid() ) { // If invalid slot
       opt_verbose= 5;            // Debugging required
       break;                     // (Only display one fault per thread)
     }
   }
#else
   for(unsigned i= 0; i<opt_multi; i++) { // Verify the slot table
     Thread* t= (Thread*)task_array[i];
     for(unsigned s= 0; s<opt_slots; s++) { // Verify the slot table
       if( ! t->slot_array[s].is_valid() ) { // If invalid slot
         opt_verbose= 5;            // Debugging required
         break;                     // (Only display one fault per thread)
       }
     }
   }
#endif

   if( opt_verbose >= 3 ) {         // If tracing
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
#if USE_GLOBAL_SLOT
     for(unsigned slot= 0; slot<opt_slots; slot++) {
       if( slot_array[slot].addr )
         tracef("[%6d] 0x%.12zx.%.8zx\n", slot
                , vtos(slot_array[slot].addr), slot_array[slot].size);
     }
#else
     for(unsigned i= 0; i<opt_multi; i++) { // Dump the slot table (by Thread)
       Thread* t= (Thread*)task_array[i];
       for(unsigned slot= 0; slot<opt_slots; slot++) {
         if( t->slot_array[slot].addr )
           tracef("[%2d][%6d] 0x%.12zx.%.8zx\n", i, slot
                  , vtos(t->slot_array[slot].addr), t->slot_array[slot].size);
       }
     }
#endif

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
     t->iteration--;                // (Iteration starts at one)

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
#if USE_GLOBAL_SLOT
   debugf("..num_lock(%'zd) (collisions)\n", num_lock.load());
   debugf("..num_find(%'zd) num_free(%'zd)\n"
         , num_find.load(), num_free.load());
   debugf("..max_size(%'zd) max_slot(%'zd)\n"
         , max_size.load(), max_slot.load());
   debugf("..now_size(%'zd) now_slot(%'zd)\n"
         , now_size.load(), now_slot.load());
#else
   for(unsigned i= 0; i<opt_multi; i++) { // Dump the thread state
     Thread* t= (Thread*)task_array[i];
     debugf("\nThread(%s)::stats()\n", t->ident);
     debugf("..num_lock(%'zd) (collisions)\n", t->num_lock);
     debugf("..num_find(%'zd) num_free(%'zd)\n", t->num_find, t->num_free);
     debugf("..max_size(%'zd) max_slot(%'u)\n", t->max_size, t->max_slot);
     debugf("..now_size(%'zd) now_slot(%'u)\n", t->now_size, t->now_slot);
   }
#endif
}
#endif // S_ALLOC_H_INCLUDED

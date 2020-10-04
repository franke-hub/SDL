//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Ref.cpp
//
// Purpose-
//       Ref method implementations.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <condition_variable>       // Used with collector thread
#include <mutex>                    // Used with collector thread
#include <sstream>                  // Used to identify thread_id

#include <com/Debug.h>              // For debugging

#include "obj/Object.h"
#include "obj/Allocator.h"
#include "obj/Semaphore.h"
#include "obj/Statistic.h"
#include "obj/Thread.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <obj/ifmacro.h>

#define MAX_PAGE_CACHE       2      // Maximum number of cached pages
#define USE_DEBUGGING_THREAD false  // Run background debugging thread?
#define USE_HCDM             false  // Use HCDM debugging
#define USE_OBJECT_CHECKING  false  // Check object validity?

#ifdef HCDM
  #undef  USE_HCDM
  #define USE_HCDM           true   // Use HCDM debugging
#endif

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Struct-
//       RefLink
//
// Purpose-
//       Reference link descriptor.
//
//----------------------------------------------------------------------------
enum { REFPOOL= 4096 };             // Number of pre-allocated elements
struct RefLink {                    // Reference link descriptor
   RefLink*            refLink;     // Next RefLink in list
   Object*             object;      // Associated Object*
}; // struct RefLink

//----------------------------------------------------------------------------
// Internal data areas
// NOTE: We depend upon the order of construction being the compliation
// order (top to bottom) and the order of destruction to be that order
// reversed (bottom to top.)
//----------------------------------------------------------------------------
// Controls
static bool            operational= true; // Ref::collect operational

// The list of objects that need to be deleted.
static Allocator       mgmt(sizeof(RefLink), 4*REFPOOL, REFPOOL); // RefLink allocator
static std::atomic<RefLink*>
                       head(nullptr); // The RECLAIM list

// Controls for garbage colletion recursion control
static std::atomic<std::thread::id>
                       collect_id;  // Current collector thread_id
static std::thread::id garbage_id;  // The garbage collector thread_id
static std::thread::id null_id;     // The NULL std::thread::id

// Controls used to synchronize garbage colletion completion
static Latch           synch_gc;    // Garbage collection synchronization

static std::condition_variable
                       fg_cv;       // The foreground condition variable
static std::mutex      fg_mutex;    // Protects fg_cv
static std::atomic<unsigned>
                       fg_post(0);  // Current fg_cv post state
static std::atomic<unsigned>
                       fg_wait(0);  // Current fg_cv wait state

// Information only used when USE_HCDM == true
static std::atomic<unsigned>
                       fg_coll(0);  // Current collector state
static RefLink*        fg_link= nullptr;
static RefLink*        fg_next= nullptr;
static Object*         fg_obj=  nullptr;

// Statistical counters
STATISTIC              stat_addQ(0); // Number of reclaim list adds
STATISTIC              stat_delQ(0); // Number of reclaim list deletes

STATISTIC              stat_coll(0); // Number of collect calls
STATISTIC              stat_list(0); // Number of reclaim list empties
STATISTIC              stat_post(0); // Number of collector thread post calls
STATISTIC              stat_redo(0); // Number of collect redo operations
STATISTIC              stat_wait(0); // Number of collector thread wait calls
STATISTIC              stat_xit0(0); // Number of collect type 0 exits
STATISTIC              stat_xit1(0); // Number of collect type 1 exits
STATISTIC              stat_xit2(0); // Number of collect type 2 exits
STATISTIC              stat_xit3(0); // Number of collect type 3 exits

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
std::atomic<size_t>    Ref::object_count(0); // Referenced Object counter

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_count_over
//       handle_count_under
//
// Purpose-
//       Handle reference counter overflow/underflow.
//
//----------------------------------------------------------------------------
static void
   handle_count_over(               // Handle reference count overflow
     Object*           object)      // For this Object
{
   char buffer[64];                 // "Object(0x123456789abcdef0).reference overflow

   sprintf(buffer, "Object(%p).reference overflow", object);
   Exception::abort("%s", buffer);
}

static void
   handle_count_under(              // Handle reference count underflow
     Object*           object)      // For this Object
{
   char buffer[64];                 // "Object(0x123456789abcdef0).reference underflow

   sprintf(buffer, "Object(%p).reference unerflow", object);
   Exception::abort("%s", buffer);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       id_string
//
// Purpose-
//       Get string representation for collect_id
//
//----------------------------------------------------------------------------
static inline std::string           // The associated string representation
   id_string( void )                // Get collect_id string representation
{  return Thread::get_id_string(collect_id.load()); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       nop
//
// Purpose-
//       Avoid compiler complaints for unused debugging controls.
//
//----------------------------------------------------------------------------
#if USE_HCDM // Only used for debugging variables
static const void* nop_check= &nop_check;
static inline void
   nop(void* thing)                 // Fool compiler
{
   if( thing == nop_check ) Exception::abort("Should not occur");
}
#endif

//----------------------------------------------------------------------------
// USE_OBJECT_CHECKING implementation [[Only valid for bringup debugging]]
// ** WARNING ** Does not work properly. [[ Does not map all storage ]]
//----------------------------------------------------------------------------
#if USE_OBJECT_CHECKING
static struct Object_checker {      // Object checker class
intptr_t               origin= 0;   // Object address range origin
intptr_t               ending= 0;   // Object address ending value

intptr_t               stack;       // A stack address
intptr_t               local;       // A local address

   Object_checker( void )           // Constructor
{
   intptr_t length= size_t(0x80000000); // 2GB checking range

   Object* one= new Object();
   Object* two= new Object();

   if( intptr_t(two) > intptr_t(one) ) // If heap expands upward
   {
     ending= intptr_t(one) + length;
     origin= intptr_t(one);
   }
   else
   {
     ending= intptr_t(one);
     origin= ending - length;
   }

   stack= intptr_t(&one);           // A stack address
   local= intptr_t(this);           // A static address

   delete two;
   delete one;

debugf("Object_checker one(%p) two(%p)\n", one, two);
debugf("Object_checker %p::%p\n", (char*)origin, (char*)ending);
}

inline void
   check_object(                    // Validate
     Object*           object)      // This object address
{
   intptr_t addr= intptr_t(object); // Get integer address
   if( addr >= origin && addr <= ending ) // If in range
     return;

// It shouldn't be possible to get here before static construction completes
// if( origin == 0 )                // If we got here before construction
//   return;

   debugf("INVALID OBJECT(%p) DETECTED\n", object);
   debugf("Valid(%p::%p) Stack(%p) Local(%p)\n", (char*)origin, (char*)ending,
          (char*)stack, (char*)local);
// throw Exception("Invalid Object*");
}
} checker; // static struct Object_checker

inline void check_object(Object* O) {checker.check_object(O);}
#else
inline void check_object(Object*) { } // No object checking (Unused parameter)
#endif

//----------------------------------------------------------------------------
// USE_COLLECTOR_THREAD implementation
//----------------------------------------------------------------------------
static std::thread     thread;      // The Ref_Collector thread

static class Ref_Collector {        // The Ref collector thread
std::atomic<int>       fsm;         // The current state
Semaphore              semaphore;   // Synchronization semaphore

public:
   ~Ref_Collector( void )
{
   IFHCDM( debugf("Ref_Collector::~Ref_Collector\n"); )

   try {
     Ref::collect();
     operational= false;
     post();
     thread.join();
   } catch(Exception& X) {
     debugf("%4d Ref catch(%s(%s))\n", __LINE__, X.string().c_str(), X.what());
   } catch(std::exception& X) {
     debugf("%4d Ref catch(std::exception(%s)\n", __LINE__, X.what());
   } catch(...) {
     debugf("%4d Ref catch(...)\n", __LINE__);
   }
}

   Ref_Collector( void )
:  fsm(0)
{
   IFHCDM( debugf("Ref_Collector::Ref_Collector\n"); )

   std::thread t(start, this);
   thread= std::move(t);
   post();
}

inline void debug( void )
{
   debugf("Ref_Collector(%p)::debug\n", this);
   debugf("..operational(%s) fsm(%d)\n",
          operational ? "true" : "false", fsm.load());
}

inline void run( void )
{
   if( USE_HCDM )
     debugf("Ref_Collector::run %s\n",
            Thread::get_id_string(std::this_thread::get_id()).c_str());

   garbage_id= thread.get_id();     // Set our thread_id
   while( operational )
   {
     IFHCDM( debugf("Ref_Collector::run.wait..\n"); )
     fsm.store(1);
     wait();
     IFHCDM( debugf("..Ref_Collector::run.wait\n"); )
     fsm.store(2);

     Ref::collect();
   }

   IFHCDM( debugf("..Ref_Collector::run\n"); )
}

static void start(Ref_Collector* thread)
{
   try {
     thread->wait();                // Wait for ::thread initialization

     thread->run();
   } catch(Exception& X) {
     debugf("%4d Ref catch(%s(%s))\n", __LINE__, X.string().c_str(), X.what());
   } catch(std::exception& X) {
     debugf("%4d Ref catch(std::exception(%s)\n", __LINE__, X.what());
   } catch(...) {
     debugf("%4d Ref catch(...)\n", __LINE__);
   }
}

void post( void )                   // Indicate work available
{
   IFHCDM( debugf("Ref_Collector::post()\n"); )
   statistic(stat_post);            // Increment post counter

   semaphore.post();
}

void wait( void )                   // Wait for work
{
   IFHCDM( debugf("Ref_Collector::wait()\n"); )
   statistic(stat_wait);            // Increment wait counter

   semaphore.wait();
}
}                      collector;   // The garbage collector controller

//----------------------------------------------------------------------------
//
// Class-
//       Ref_DebugThread
//
// Purpose-
//       Debugging thread.
//
//----------------------------------------------------------------------------
#if USE_DEBUGGING_THREAD
class Ref_DebugThread : public Thread {
//----------------------------------------------------------------------------
// Ref_DebugThread::Attributes
//----------------------------------------------------------------------------
public:
int                    operational= true; // Thread operational

// Controls used to synchronize termination
std::mutex             thread_mutex; // Protects thread_cv
std::condition_variable
                       thread_cv;   // The foreground condition variable

//----------------------------------------------------------------------------
// Ref_DebugThread::Constructors
//----------------------------------------------------------------------------
public:
   ~Ref_DebugThread( void )
{  terminate(); }

   Ref_DebugThread( void ) : Thread()
,  thread_mutex(), thread_cv()
{  start();
}

//----------------------------------------------------------------------------
// Ref_DebugThread::Methods
//----------------------------------------------------------------------------
void
   _run( void )                     // Background debugging thread
{
   debugf("Ref_DebugThread started\n");

   // Operate the thread
   detach();
   while( operational )
   {
     debugf("Ref_DebugThread waiting:\n");
     std::unique_lock<decltype(thread_mutex)> lock(thread_mutex);
//   std::cv_status timeout=
         thread_cv.wait_for(lock, std::chrono::seconds(60));

     if( operational )
     {
       debugf("Ref_DebugThread status:\n");
       Ref::debug_static();
     }
   }

   debugf("Ref_DebugThread exiting\n");
}

virtual void
   run( void )                      // Run debugging thread
{
   try {
     _run();
   } catch(Exception& X) {
     debugf("%4d catch(%s) what(%s)\n", __LINE__, X.string().c_str(), X.what());
   } catch(std::exception& X) {
     debugf("%4d catch(std::exception) what(%s)\n", __LINE__, X.what());
   } catch(...) {
     debugf("%4d catch(...)\n", __LINE__);
   }
}

void
   terminate( void )                // Terminate the thread
{
   operational= false;

   std::unique_lock<decltype(thread_mutex)> lock(thread_mutex);
   thread_cv.notify_all();
}
}; // class Ref_DebugThread

static Ref_DebugThread debugThread; // Our debug thread
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Ref::debug_static
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Ref::debug_static( void )        // Debugging display
{  debugf("Ref::debug_static operational(%s) object(%zd)\n",
          operational ? "true" : "false", object_count.load());

if( false )
   {{{{
     const char* mess= "";
     #ifndef HCDM_0001
       mess= "NOT ";
     #endif
     debugf("..HCDM_0001(%sDEFINED)\n", mess);
   }}}}

   debugf("..MAX_PAGE_CACHE(%d)\n", MAX_PAGE_CACHE);

   debugf("..USE_DEBUGGING_THREAD(%s)\n"
          "..USE_HCDM(%s) USE_OBJECT_CHECKING(%s)\n",
          USE_DEBUGGING_THREAD ? "true" : "false",
          USE_HCDM ? "true" : "false",
          USE_OBJECT_CHECKING ? "true" : "false");

   if( USE_HCDM )
     debugf("..thread(%s) fsm(%d).%zd, link(%p), next(%p), obj(%p)\n"
            , id_string().c_str(), fg_coll.load()
            , stat_delQ.load(), fg_link, fg_next, fg_obj);

   debugf("..addQ(%zd) delQ(%zd) coll(%zd) list(%zd) redo(%zd)\n"
          "..xit0(%zd) xit1(%zd) xit2(%zd) xit3(%zd)\n"
          , stat_addQ.load(), stat_delQ.load()
          , stat_coll.load(), stat_list.load(), stat_redo.load()
          , stat_xit0.load(), stat_xit1.load()
          , stat_xit2.load(), stat_xit3.load());

   debugf("..post(%zd) wait(%zd) waitFSM(%d) postFSM(%d)\n"
          , stat_post.load(), stat_wait.load()
          , fg_wait.load(), fg_post.load());

   debugf("..fsm(%d) head(%p) collect_id(%s)\n"
          , fg_coll.load(), head.load(), id_string().c_str());

   mgmt.debug();
   collector.debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       Ref::collect
//
// Purpose-
//       Perform garbage collection.
//
// Implementation note-
//       This is where *ALL* Object garbage collection work is driven.
//
//----------------------------------------------------------------------------
void
   Ref::collect( void )             // Run garbage collector
{
   IFHCDM( debugf("Ref()::collect() %s\n", id_string().c_str()); )

   RefLink*            newRefLink;  // New* RefLink
   RefLink*            oldRefLink;  // Old* RefLink

   // At this point we have added the object to the reclaim list but haven't
   // yet decided whether we need to perform the reclaim ourselves.
   statistic(stat_coll);            // Increment invocation  counter
   if( head.load() == nullptr )     // If the reclaim list is already empty
   {
     statistic(stat_xit0);
     return;
   }

   std::thread::id new_id= std::this_thread::get_id();
   std::thread::id old_id= collect_id.load();
   while( old_id == null_id )
   {
     if( collect_id.compare_exchange_weak(old_id, new_id) )
       break;
   }

   if( old_id != null_id )
   {
     statistic(stat_xit1);
     return;
   }

   newRefLink= head.exchange(nullptr);
   if( newRefLink == nullptr )
   {
     statistic(stat_xit2);
     collect_id.store(null_id);
     return;
   }

   // Empty the reclaim list
   fg_coll.store(1);
   for(;;)
   {
     #if USE_HCDM
       RefLink* prevOld= nullptr;
       RefLink* prevNew= nullptr;
     #endif

     statistic(stat_list);
     while( newRefLink )
     {
       try {
         if( USE_HCDM ) fg_coll.store(3);
         oldRefLink= newRefLink;
         newRefLink= oldRefLink->refLink;

         #if USE_HCDM
           mgmt.check(oldRefLink);
           if( newRefLink) mgmt.check(newRefLink);
           fg_coll.store(4);
           fg_link= oldRefLink;
           fg_next= newRefLink;
           fg_obj=  oldRefLink->object;
           check_object(oldRefLink->object);
           fg_coll.store(666);
         #endif

         IFHCDM( debugf("Ref()::delete(%p)\n", oldRefLink->object); )
         delete oldRefLink->object;
       } catch(...) {
         // This does not catch recursion bugs, alas
         // We could just go on, ignoring the failing delete but
         // a debugging abort seems like the way to go.
         Exception::abort("Delete object(%p) failure\n",
                          oldRefLink->object);
       }

       #if USE_HCDM
         prevNew= newRefLink;
         prevOld= oldRefLink;
         nop( prevNew );
         nop( prevOld );
         fg_coll.store(5);
         mgmt.put(oldRefLink);
         fg_coll.store(6);
       #else
         mgmt.put(oldRefLink);
       #endif

       statistic(stat_delQ);
     }

     newRefLink= head.exchange(nullptr);
     if( newRefLink == nullptr )
       break;

     statistic(stat_redo);
   }

   fg_coll.store(0);
   collect_id.store(null_id);
   statistic(stat_xit3);

   // Drive any threads waiting for garbage collection completion
   fg_post.store(1);

   std::unique_lock<decltype(fg_mutex)> lock(fg_mutex);
   fg_cv.notify_all();
}

//----------------------------------------------------------------------------
//
// Method-
//       Ref::gc
//
// Purpose-
//       Wait for garbage collection completion.
//
//----------------------------------------------------------------------------
bool                                // TRUE iff garbage collected
   Ref::gc( void )                  // Wait for garbage collection completion
{
   bool result= false;              // Default: nothing to collect

   std::unique_lock<decltype(fg_mutex)> lock(fg_mutex);

   while( operational
       && (head.load() != nullptr || collect_id.load() != null_id) )
   {
     result= true;
     if( fg_post.load() )
       break;

     fg_wait.store(1);
     fg_cv.wait(lock);
     fg_wait.store(0);
   }

   fg_post.store(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Ref::set
//
// Purpose-
//       Update the Object*.
//
// Implementation note-
//       This is where *ALL* Object garbage collection work is driven.
//
//----------------------------------------------------------------------------
void
   Ref::set(                        // Update the Object*
     Object*           newObject)   // With this Object*
{
   IFHCDM(
     debugf("Ref(%p)::set(%p=>%p) %s\n", this,
            newObject, object.load(), id_string().c_str());
   )

   int32_t             oldCount;    // Old reference counter
   int32_t             newCount;    // New reference counter
   RefLink*            oldRefLink;  // Old -> head
   RefLink*            newRefLink;  // New RefLink*
   Object*             oldObject;   // Old -> Object

   oldObject= object;
   while( !object.compare_exchange_strong(oldObject, newObject) )
     ;

   if( (void*)oldObject == (void*)newObject ) // If unchanged
     return;

   //-------------------------------------------------------------------------
   // Increment the new Object's reference counter, checking for overflow.
   // Note: In case the old Object deletion chain contains the penultimate
   // reference to the new Object, this MUST be done before deleting the
   // old Object.
   if( newObject != nullptr )       // If a new Object exists
   {
     check_object(newObject);       // Check when inserting object

     oldCount= newObject->references;
     newCount= oldCount + 1;
     if( newCount <= 0 ) handle_count_over(newObject);
     while( !newObject->references.compare_exchange_weak(oldCount, newCount) )
     {
       newCount= oldCount + 1;
       if( newCount <= 0 ) handle_count_over(newObject);
     }

     if( config::Ref::USE_OBJECT_COUNT )
       if( oldCount == 0 )
         count_object(+1);
   }

   //-------------------------------------------------------------------------
   // Decrement the new Object's reference counter, checking for underflow.
   // Delete it if becomes zero.
   if( oldObject != nullptr )       // If an old Object exists
   {
     oldCount= oldObject->references;
     newCount= oldCount - 1;
     if( newCount < 0 ) handle_count_under(oldObject);
     while( !oldObject->references.compare_exchange_weak(oldCount, newCount) )
     {
       newCount= oldCount - 1;
       if( newCount < 0 ) handle_count_under(oldObject);
     }

     if( newCount == 0 )            // If no more references exist
     {
       if( config::Ref::USE_OBJECT_COUNT )
         count_object(-1);

       {{{{ // Fix: 2018/06/05 (See .README)
         if( std::this_thread::get_id() != garbage_id )
         {
            std::lock_guard<decltype(synch_gc)> lock(synch_gc);

            #if defined(_OS_LINUX)
              // This mechanism always prevents the number of extended REF
              // pages from getting out of hand.

              // On Cygwin, however, thread starvation almost always occurs,
              // causing an unacceptable performance penalty.
              if( mgmt.get_used_pages() > MAX_PAGE_CACHE )
                while( gc() )
                  ;
            #else
              // This mechanism does not work for Linux systems.

              // On Cygwin, however, it works well. Extended REF page
              // allocation does not occur and thread starvation is avoided.
              if( mgmt.get_used_pages() > MAX_PAGE_CACHE )
                collect();
            #endif
         }
       }}}}

       // We cannot simply delete the Object beause it can contain final Refs
       // to Objects that recursively contain final Refs to Objects and so on.
       // If this situation occurs it would be possible to run out of stack
       // space before we return.
       // To prevent this, we add use an atomic reclaim list.
       statistic(stat_addQ);
       newRefLink= (RefLink*)mgmt.get();
       newRefLink->object= oldObject;
       oldRefLink= head.load();
       newRefLink->refLink= oldRefLink;
       while( !head.compare_exchange_weak(oldRefLink, newRefLink) )
         newRefLink->refLink= oldRefLink;

       // The object is on the reclaim list.
       // Drive the collector thread if the list was empty.
       if( oldRefLink == nullptr )
         collector.post();
     }
   }
}
} // namespace _OBJ_NAMESPACE


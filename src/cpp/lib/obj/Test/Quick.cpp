//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Quick.cpp
//
// Purpose-
//       Quick, minimal tests.
//
// Last change date-
//       2021/07/24
//
//----------------------------------------------------------------------------
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "com/Debug.h"
#include "obj/Object.h"
#include "obj/Array.h"
#include "obj/Latch.h"
#include "obj/List.h"
#include "obj/String.h"
#include "obj/Thread.h"
using namespace _OBJ_NAMESPACE;

#include "Thing.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef TEST_COMPILE_ERRORS
#undef  TEST_COMPILE_ERRORS         // If defined, test compile error detection
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       id_string
//
// Purpose-
//       Get id string of std::thread::id
//
//----------------------------------------------------------------------------
static inline std::string           // Resultant string
   id_string(                       // Get id string
     std::thread::id&  id)          // For this id
{  return Thread::get_id_string(id);
}

static inline std::string           // Resultant string
   id_string(                       // Get id string
     volatile std::thread::id&  id) // For this id
{  return id_string(const_cast<std::thread::id&>(id));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Collect
//
// Purpose-
//       Complete garbage collection between tests
//
//----------------------------------------------------------------------------
static inline int
   test_Collect( void )             // Run garbage collector
{
   while( Ref::gc() )               // Complete any prior garbage collection
     ;

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_CompileErrors
//
// Purpose-
//       Verify compile-time error detection.
//
//----------------------------------------------------------------------------
static inline int
   test_CompileErrors( void )       // Verify compile-time error detection
{
   Object* object= new Object();    // Simple Object
   Ref     ref;                     // Simple Ref
   String* string= new String("S"); // Simple String

   Ref o_ref= object;               // (Do not delete Object during test)
   Ref s_ref= string;               // (Do not delete String during test)

   Ref_t<String> stringRef;         // OK: Empty
   Ref_t<String> stringRef0(stringRef); // OK: Valid copy constructor
   stringRef= stringRef0;           // OK: Types are the same
   stringRef= *string;              // OK: Types are the same
   stringRef= string;               // OK: Types are the same
   stringRef.set(string);           // OK: Types are the same

   ref= stringRef;                  // OK: Downcast allowed
   ref= *string;                    // OK: Downcast allowed
   ref= string;                     // OK: Downcast allowed

#ifdef TEST_COMPILE_ERRORS
   Ref_t<String> stringRef1= new Object(); // ERROR: Not a String
   Ref_t<String> stringRef2= new Exception(); // ERROR: Not a String

   Ref_t<String> stringRef3(*object); // ERROR: Use of deleted function
   Ref_t<String> stringRef4(object); // ERROR: Use of deleted function
   stringRef= ref;                  // ERROR: Use of deleted function

   stringRef= *object;              // ERROR: Use of deleted function
   stringRef= object;               // ERROR: Use of deleted function
   stringRef.set(object);           // ERROR: Use of deleted function
#endif

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Object
//
// Purpose-
//       (Minimally) Test Object.h
//
//----------------------------------------------------------------------------
static inline int
   test_Object( void )              // Test Object.h
{
   debugf("\nNow testing Object.h, Ref.h\n");

   int errorCount= 0;              // Error counter
   size_t old_object_count= Ref::get_object_count();
   debugf("%3zd= get_object_count@%4d\n", Ref::get_object_count(), __LINE__);
   if( true ) {
     Ref r= new Object();
     if( obj::config::Ref::USE_OBJECT_COUNT
         && Ref::get_object_count() != (old_object_count+1) )
     {
       errorCount++;
       debugf("ERROR: Ref::get_object_count(%zd) != old_object_count(%zd)+1\n",
              Ref::get_object_count(), old_object_count);
     }
     debugf("Object(%s)\n", (*r).string().c_str());
   }

   // Left scope, object should be gone.
   test_Collect();                  // Insure garbage collection completed

   if( Ref::get_object_count() != old_object_count )
   {
     errorCount++;
     debugf("ERROR: Ref::get_object_count(%zd) != old_object_count(%zd)\n",
            Ref::get_object_count(), old_object_count);
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Array
//
// Purpose-
//       Test Array.h
//
//----------------------------------------------------------------------------
static inline int
   test_Array( void )               // Test Array.h
{
#ifdef USE_THING_OBJ
   debugf("\nNow testing Array.h\n");

   int errorCount= 0;              // Error counter
   size_t old_object_count= Ref::get_object_count();
   debugf("%3zd= get_object_count@%4d\n", Ref::get_object_count(), __LINE__);
   if( false ) {                    // Simple array
     Array_t<Thing, 32> array;      // The test array
     // Ref ref= array;             // MUST NOT reference a stack Object

     for(std::size_t i= 0; i<array.size(); i++)
       array[i]= new Thing(i);

     for(std::size_t i= 0; i<array.size(); i++)
       array[i]->check(__LINE__, i);

     debugf("%3zd= get_object_count@%4d\n", Ref::get_object_count(), __LINE__);
   }

   if( true  ) {                    // Monster array
     #undef  THING_COUNT
     #define THING_COUNT  100000
     typedef Array_t<Thing, THING_COUNT>  ARRAY_OBJ;
     typedef Array_t<Thing, THING_COUNT>* ARRAY_PTR;
     typedef Ref_t<ARRAY_OBJ>             ARRAY_REF;

     ARRAY_REF thing_array;
     ARRAY_PTR ptr= new ARRAY_OBJ();
     debugf("ARRAY_PTR(%p)\n", ptr);
     thing_array= ptr;
     thing_array= nullptr;

     thing_array= new ARRAY_OBJ();
     for(size_t i= 0; i<(*thing_array).size(); i++)
       (*thing_array)[i]= new Thing();
   }

   // Complete garbage collection
   debugf("%3zd= get_object_count@%4d\n", Ref::get_object_count(), __LINE__);
   test_Collect();                  // Insure garbage collection completed
   debugf("%3zd= get_object_count@%4d\n", Ref::get_object_count(), __LINE__);

   if( Ref::get_object_count() != old_object_count )
   {
     errorCount++;
     debugf("ERROR: Ref::get_object_count(%zd) != old_object_count(%zd)\n",
            Ref::get_object_count(), old_object_count);
   }

   return errorCount;
#else
   return 0;
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Exception
//
// Purpose-
//       (Minimally) Test Exception.h
//
//----------------------------------------------------------------------------
static inline int
   test_Exception( void )           // Test Exception.h
{
   debugf("\nNow testing Exception.h\n");

   int errorCount= 0;              // Error counter
   try {
       throw NullPointerException("test"); // Reverse these statements for jollies
       throw NullPointerException();       // Reverse these statements for jollies
   } catch(NullPointerException& X) {
       debugf("Caught expected NPE(%s) %s\n", X.what(), X.string().c_str());
   } catch(std::exception& X) {
       errorCount++;
       debugf("ERROR: Caught std(%s)\n", X.what());
   } catch(...) {
       errorCount++;
       debugf("ERROR: Caught(...)\n");
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Latch
//
// Purpose-
//       (Minimally) Test Latch.h
//
//----------------------------------------------------------------------------
static inline int
   test_Latch( void )               // Test Latch.h
{
   debugf("\nNow testing Latch.h\n");

   SharedLatch    shared;
   ExclusiveLatch exclusive(shared);
   debugf("shared.count(%u)\n", shared.count);
   {{{{ std::lock_guard<decltype(shared)> lock1(shared);
     debugf("shared.count(%u)\n", shared.count);
     if( exclusive.try_lock() )
       throw Exception("Obtained exclusive while shared");

     {{{{ std::lock_guard<decltype(shared)> lock2(shared);
       debugf("shared.count(%u)\n", shared.count);
     }}}}
     debugf("shared.count(%u)\n", shared.count);
   }}}}
   debugf("shared.count(%u)\n", shared.count);

   if( exclusive.try_lock() )
   {
     debugf("shared.count(%x)\n", shared.count);
     exclusive.unlock();;
     debugf("shared.count(%x)\n", shared.count);
   } else {
     throw Exception("Unable to obtain exclusive latch");
   }

   {{{{ std::lock_guard<decltype(exclusive)> lock(exclusive);
     debugf("Exclusive lock_guard\n");
     debugf("shared.count(%x)\n", shared.count);
   }}}}
   debugf("shared.count(%x)\n", shared.count);

   debugf("Exclusive try_lock\n");
   if( exclusive.try_lock() )
   {
     debugf("shared.count(%x)\n", shared.count);
     exclusive.unlock();;
     debugf("shared.count(%x)\n", shared.count);
   } else {
     throw Exception("Unable to obtain exclusive latch");
   }

   debugf("\n");
   RecursiveLatch recursive;
   debugf("RecursiveLatch(%s) count(%u)\n", id_string(recursive.latch).c_str(), recursive.count);
   assert( recursive.count == 0 );

   {{{{ std::lock_guard<decltype(recursive)> lock1(recursive);
     debugf("RecursiveLatch(%s) count(%u)\n", id_string(recursive.latch).c_str(), recursive.count);
     assert( recursive.count == 1 );

     {{{{ std::lock_guard<decltype(recursive)> lock2(recursive);
       debugf("RecursiveLatch(%s) count(%u)\n", id_string(recursive.latch).c_str(), recursive.count);
       assert( recursive.count == 2 );
     }}}}

     debugf("RecursiveLatch(%s) count(%u)\n", id_string(recursive.latch).c_str(), recursive.count);
     assert( recursive.count == 1 );
   }}}}
   debugf("RecursiveLatch(%s) count(%u)\n", id_string(recursive.latch).c_str(), recursive.count);
   assert( recursive.count == 0 );

   return 0;                        // Exception if error
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_List
//
// Purpose-
//       (Minimally) Test List.h
//
//----------------------------------------------------------------------------
static inline int
   test_List( void )                // Test List.h
{
   debugf("\nNow testing List.h\n");

   int errorCount= 0;
   class _Link_ : public List<_Link_>::Link {
     public: int i;
   };
   List<_Link_> list;

   for(int i= 0; i<8; i++)
   {
     _Link_* link= new _Link_();
     link->i= i;

     list.fifo(link);
   }

   int count= 0;
   _Link_* link= list.reset();
   while( link )
   {
     if( count != link->i )
     {
       errorCount++;
       debugf("ERROR: ");
     }
     debugf("[%d] %d\n", count++, link->i);

     link= link->get_next();
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_String
//
// Purpose-
//       (Minimally) Test String.h
//
//----------------------------------------------------------------------------
static inline int
   test_String( void )              // Test String.h
{
   debugf("\nNow testing String.h\n");

   int errorCount= 0;              // Error counter
   obj::String s1("foobar ");
   obj::String s2(s1);
   obj::String s3= s2;
   std::cout << s1 << s2 << s3 << std::endl;

   std::cout << s1.hashf() << " hashf(" << s1 << ")\n";
   std::cout << s2.hashf() << " hashf(" << s2 << ")\n";
   std::cout << s3.hashf() << " hashf(" << s3 << ")\n";
   std::cout << obj::String("this").hashf()  << " hashf(this)\n";
   std::cout << obj::String("that").hashf()  << " hashf(that)\n";
   std::cout << obj::String("other").hashf() << " hashf(other)\n";

   try {
     Object foo;
     std::cout << (s1 == foo) << " (ERROR IF YOU SEE THIS)\n";
     errorCount++;
   } catch(CompareCastException& X) {
     std::cout << "Caught expected CompareCastException(" << X.what() << "): "
               << X << std::endl;
   } catch(...) {
     errorCount++;
     debugf("ERROR: Caught(...)\n");
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Thread
//
// Purpose-
//       (Minimally) Test Thread.h
//
//----------------------------------------------------------------------------
class TestThread : public Thread {
   using Thread::Thread;

const char*            name;        // The Thread's name

public:
   TestThread(const char* _name) : Thread(), name(_name) {}

virtual void
   run( void )                      // Test a thread
{
   debugf("Thread(%s) started\n", name);

   for(int i= 0; i<5; i++)
   {
     sleep(1);
     debugf("Thread(%s) running\n", name);
   }

// debugf("Thread(%s) complete\n", name);
}
}; // class TestThread

static inline int
   test_Thread( void )              // Test Thread.h
{
   debugf("\nNow testing Thread.h\n");

   Ref_t<Thread> one = new TestThread("one");
   Ref_t<Thread> two = new TestThread("two");

   (*one).start();
   two->start();

   one->join();
   (*two).join();

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(int, char**)                // Mainline code
//   int             argc,          // Argument count
//   char*           argv[])        // Argument array
{
   int               errorCount= 0; // Error counter

   try {
     errorCount += test_CompileErrors();
     errorCount += test_Collect();
     errorCount += test_Object();
     errorCount += test_Array();
     errorCount += test_Exception();
     errorCount += test_Latch();
     errorCount += test_List();
     errorCount += test_String();
     errorCount += test_Thread();
   } catch(Exception& X) {
     errorCount++;
     debugf("%4d %s(%s)\n", __LINE__, X.string().c_str(), X.what());
   } catch(std::exception& X) {
     errorCount++;
     debugf("%4d std::exception(%s)\n", __LINE__, X.what());
   } catch(...) {
     errorCount++;
     debugf("%4d catch(...)\n", __LINE__);
   }

   if( false ) {
     debugf("Running Ref::gc..\n");
     test_Collect();                // Insure garbage collection completed
     debugf("..Ref::gc finished\n");

     Ref::debug_static();
     Thing::debug_static();
   }

   if( errorCount == 0 )
     debugf("NO errors detected\n");
   else if( errorCount == 1 )
     debugf("1 error detected\n");
   else {
     debugf("%d errors detected\n", errorCount);
     errorCount= 1;
   }

   return 0;
}


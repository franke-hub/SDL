//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Trace.cpp
//
// Purpose-
//       Trace object methods.
//
// Last change date-
//       2020/06/09
//
//----------------------------------------------------------------------------
#include <atomic>                   // Atomic operation required
#include <mutex>                    // For std::lock_guard
#include <new>                      // For std::bad_alloc
#include <stdio.h>
#include <stdlib.h>                 // For aligned_alloc, ...
#include <string.h>                 // For memcpy

#include <pub/Debug.h>              // For debugging
#include <pub/utility.h>

#include "pub/Trace.h"
using namespace _PUB_NAMESPACE::debugging; // For debugging

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
enum { ALIGNMENT= Trace::ALIGNMENT }; // Storage allocation required alignemnt

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Trace*                 Trace::trace= nullptr; // The common Trace object

//----------------------------------------------------------------------------
//
// Method-
//       Trace::Trace
//
// Purpose-
//       Constructor.
//
// Implementation notes-
//       make() validates alignment and size.
//
//----------------------------------------------------------------------------
   Trace::Trace(                    // Constructor
     uint32_t          size)        // Size of trace area
:  next(0)
,  top(0)
,  bot(0)
,  size(0)
{
   uintptr_t           header;      // Effective header size

   header= sizeof(Trace);
   header +=  (ALIGNMENT - 1);
   header &= ~(ALIGNMENT - 1);

   top=  header;
   next= header;
   bot=  size;
   this->size= size - header;

   wrap[0]= 0;
   wrap[1]= 0;
   wrap[2]= 0;
   wrap[3]= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::make
//
// Purpose-
//       Allocate the Trace object.
//
//----------------------------------------------------------------------------
Trace*                              // -> Trace instance
   Trace::make(                     // Allocate the Trace object
     size_t            size)        // Size of trace area
{
   if( size > MAXIMUM_SIZE )        // Silently enforce size restrictions
     size= MAXIMUM_SIZE;
   if( size < MINIMUM_SIZE )
     size= MINIMUM_SIZE;
   size +=  (ALIGNMENT - 1);
   size &= ~(ALIGNMENT - 1);

   void* addr= aligned_alloc(ALIGNMENT, size); // Allocate the Trace
   if( addr == nullptr )            // If allocation fails
     throw std::bad_alloc();        // Bad allocation
   memset(addr, 0, size);           // Initialize the Trace area

   Trace* trace= new(addr) Trace(size);
   return trace;
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::take
//
// Purpose-
//       Atomically take and reset the commont the Trace object.
//
//----------------------------------------------------------------------------
Trace*                              // -> Trace instance
   Trace::take( void )              // Allocate the Trace object
{
   std::atomic<Trace*>* atomic_trace= (std::atomic<Trace*>*)&trace;

   Trace* oldPointer= atomic_trace->load();
   while( !atomic_trace->compare_exchange_weak(oldPointer, nullptr) )
     ;

   return oldPointer;
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::allocate
//
// Purpose-
//       Allocate a trace record.
//
//----------------------------------------------------------------------------
Trace::Record*                      // -> Trace record
   Trace::allocate(                 // Allocate a trace record
     uint32_t          size)        // Size of trace record
{
   Record*             ptrRecord;   // -> Record

   uint32_t            newV;        // New value
   uint32_t            oldV;        // Old value
   uint32_t            wrapped;     // Non-zero iff wrapped

   size +=  (ALIGNMENT - 1);
   size &= ~(ALIGNMENT - 1);
   if ( size == 0 || size > this->size )
     return nullptr;

   std::atomic<uint32_t>* atomic_next= (std::atomic<uint32_t>*)&next;
   oldV= atomic_next->load();
   for(;;)
   {
     wrapped= 0;
     newV= oldV + size;
     ptrRecord= (Record*)((char*)this + oldV);
     if ( newV > bot )
     {
       wrapped= oldV;
       ptrRecord= (Record*)((char*)this + top);
       newV= top + size;
     }

     if( atomic_next->compare_exchange_strong(oldV, newV) )
       break;
   }

   // Indicate return record not initialized
   *(uint32_t*)ptrRecord= *(uint32_t*)(".000");
   if( wrapped )                    // Handle wrap
   {
     std::atomic<uint32_t>* atomic_wrap= (std::atomic<uint32_t>*)wrap;

     // If there were empty slots skipped at the top, clean the top
     if( wrapped < bot )
     {
       Record* record= (Record*)(char*)this + wrapped;
       memset(record, 0, bot-wrapped);
       *(uint32_t*)record= *((uint32_t*)(".END"));
     }

     for(int i=3; i>=0; i--)
     {
       oldV= atomic_wrap[i].load();
       do
       { newV= oldV + 1;
       } while( !atomic_wrap[i].compare_exchange_strong(oldV, newV) );

       if( newV != 0 )
         break;
     }
   }

   return ptrRecord;
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::dump
//
// Purpose-
//       Dump the trace table.
//
//----------------------------------------------------------------------------
void
   Trace::dump( void ) const        // Dump the trace table
{
   Debug* debug= Debug::get();
   std::lock_guard<decltype(*debug)> lock(*debug);

   tracef("Trace(%p)::dump\n", this);
   tracef("..top(%8x) next(%.8x) bot(%.8x) size(%.8x)\n",
          top, next, bot, size);
   tracef("..wrap %.8x %.8x %.8x %.8x\n", wrap[0], wrap[1], wrap[2], wrap[3]);
   ::pub::utility::dump(debug->get_FILE(), this, bot, nullptr);
}
}  // namespace _PUB_NAMESPACE

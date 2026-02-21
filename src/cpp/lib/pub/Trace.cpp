//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Trace.cpp
//
// Purpose-
//       Trace object methods.
//
// Last change date-
//       2026/02/05
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE                 // For sched_getcpu
#define _GNU_SOURCE
#endif

#include <atomic>                   // For std::atomic
#include <mutex>                    // For std::lock_guard
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memcpy, strncpy
#include <ctime>                    // For clock_gettime

#include <sched.h>                  // For sched_getcpu
#include <unistd.h>                 // For sysconf

#include <pub/Debug.h>              // For debugging
#include "pub/Trace.h"              // For pub::Trace, implemented
#include <pub/utility.h>            // For pub::utility::dump

using namespace _LIBPUB_NAMESPACE::debugging; // For debugging

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Compile-time options
//----------------------------------------------------------------------------
enum // Compile-time options. We rely on optimization to elide unused code.
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  USE_CHECK= false                 // Check for should not occur conditions?
}; // Compile-time options

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Trace*                 Trace::table= nullptr; // The common Trace object

//----------------------------------------------------------------------------
//
// Method-
//       Trace::Record::set_cpuid
//
// Purpose-
//       Replace ident[0] with cpu id
//
// Implementation notes-
//       If and when _GNU_SOURCE isn't required, move this to Trace.h
//
//----------------------------------------------------------------------------
void
   Trace::Record::set_cpuid( void ) // Replace ident[0] with cpu id
{  ident[0]= sched_getcpu(); }

//----------------------------------------------------------------------------
//
// Method-
//       Trace::Trace
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Trace::Trace(                    // Constructor
     uint32_t          size)        // Length of trace table
{
   unsigned zero= sizeof(Trace);    // The first record offset
   static_assert( (sizeof(Trace) & (ALIGNMENT-1)) == 0, "Trace.h ALIGNMENT");
// zero +=  (ALIGNMENT - 1);        // Not needed, static_assert verifies it
// zero &= ~(ALIGNMENT - 1);
// size -= zero;

   this->zero= zero;
   this->last= size;
   this->size= size;
   this->next= zero;
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::make
//
// Purpose-
//       Initialize the Trace table
//
// Implementation notes-
//       Storage and size alignment are done here, before construction
//
//----------------------------------------------------------------------------
Trace*                              // -> Trace instance
   Trace::make(                     // Initialize the Trace table
     void*             addr,        // Address of trace table
     size_t            size)        // Length of trace table
{
   if( HCDM  ) debugf("%4d HCDM Trace.cpp HCDM active\n", __LINE__);
   if( USE_CHECK && false )
     debugf("%4d HCDM Trace.cpp USE_CHECK active\n", __LINE__);

   if( addr == nullptr              // Reject invalid parameters
       || size < TABLE_SIZE_MIN || size > TABLE_SIZE_MAX )
     throw std::bad_alloc();

   unsigned diff= (uintptr_t)addr & (ALIGNMENT-1);
   if( diff != 0 ) {                // If misaligned storage
     diff= ALIGNMENT - diff;
     size -= diff;
     addr= (void*)((uintptr_t)addr + diff);
   }
   size &= ~(ALIGNMENT - 1);

   memset(addr, 0, size);           // Initialize to zeros
   Trace* trace= new(addr) Trace((uint32_t)size); // Construct the Trace object
   trace->flag[X_OFFSET]= diff;     // (No real need for this field.)

   return trace;
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::static_debug
//
// Purpose-
//       Display static debug information.
//
//----------------------------------------------------------------------------
void
   Trace::static_debug(             // Static debug information
     const char*       info)        // Caller information
{
   debugf("Trace(%p)::static_debug(%s)\n", table, info);
   debugf("..HCDM(%s) USE_CHECK(%s)\n", pub::b2c(HCDM), pub::b2c(USE_CHECK));
   if( table )
     debugf("..next(0x%.8x) size(0x%.8x) zero(0x%.2x) last(0x%.8x) wrap(%lu)\n"
           , table->next.load(), table->size, table->zero, table->last
           , table->wrap);
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::allocate
//
// Purpose-
//       Allocate storage.
//
// Implementation note-
//       Performance critical path.
//       A nullptr resultant only occurs as a result of an application error
//
//----------------------------------------------------------------------------
_LIBPUB_HOT
void*                               // Resultant
   Trace::allocate(                 // Allocate a trace record
     uint32_t          size)        // of this length
{
   void*               result;      // Resultant

   uint32_t            newV;        // New value
   uint32_t            oldV;        // Old value
   uint32_t            last;        // Last oldV when wrapped

   size +=  (ALIGNMENT - 1);
   size &= ~(ALIGNMENT - 1);
   if( USE_CHECK || true ) {        // Check size parameter?
     // Size checks are always enabled:
     //   size == 0: An all too common mistake
     //   size > (available size): The for(;;) loop never exits
     // this->zero is rounded size of header, always the header size
//   if ( size == 0 || size > (this->size - this->zero) ) // If too large
     if ( size == 0 || size > (this->size - sizeof(Trace) ) ) // If too large
       throw std::bad_alloc();      // Parameter error
   } // if( USE_CHECK || true )

   oldV= next.load();
   for(;;) {
     last= 0;                       // Indicate not wrapped
     newV= oldV + size;             // Arithmetic overflow is a user error
     if( USE_CHECK ) {              // Check for arithmetic overflow?
       // Arithmetic overflow can only occur when the size parameter plus the
       // size of the table is greater than UINT32_MAX.
       // This is simple for an application to avoid and while the checking
       // is small, it's not zero.
       if( newV < size )            // If arithmetic overflow
         return nullptr;
     }

     result= (char*)this + oldV;
     if ( newV > this->size ) {     // If wrap
       last= oldV;
       result= (char*)this + zero;
       newV= size + zero;
     }

     if( next.compare_exchange_weak(oldV, newV) )
       break;
   }

   if( HCDM ) {
     // Clarifies record initialization in progress state
     memset(result, 0, size);         // Indicate record not initialized
     memcpy(result, ".000", 4);       // Clarify: record not initialized
     // ((Record*)result)->trace(".000", -1); // Alternate clarify option
   }

   if( last ) {                     // Handle wrap
     wrap++;                        // Update the wrap count
     this->last= last;              // Last valid location

     // Clean up any unused space between last and this->size (zombie data)
     // This cleanup makes the last trace entry in the trace table slightly
     // easier to look at, and doesn't occur in the normal case.
     if( last < this->size ) {      // If zombie data exists
       char* atlast= (char*)this + last;
       memset(atlast, 0, this->size - last);
       memcpy(atlast, ".END", 4);
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::dump
//
// Purpose-
//       Dump the trace table.
//
// Implementation note-
//       For readability the global Debug mutex is held during the entire
//       dump process.
//
//----------------------------------------------------------------------------
void
   Trace::dump( void ) const        // Dump the trace table
{
   Debug* debug= Debug::get();
   std::lock_guard<decltype(*debug)> lock(*debug);

   tracef("Trace(%p)::dump\n", this);
   tracef("..next(0x%.8x) size(0x%.8x) zero(0x%.2x) last(0x%.8x) wrap(%lu)\n"
         , next.load(), size, zero, last, wrap);
   utility::dump(debug->get_FILE(), this, size, nullptr);
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::start
//
// Purpose-
//       Resume tracing (if the trace table is present.)
//
//----------------------------------------------------------------------------
void
   Trace::start( void )             // Resume tracing
{
   if( table ) {
     table->reactivate();
     Trace::trace(".SYS", "<go>", ":start");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Trace::stop
//
// Purpose-
//       Suspend tracing (if the trace table is present.)
//
//----------------------------------------------------------------------------
void
   Trace::stop( void )              // Suspend tracing
{
   if( table ) {
     Trace::trace(".SYS", "stop", ":stop");
     table->deactivate();
   }
}
}  // namespace _LIBPUB_NAMESPACE

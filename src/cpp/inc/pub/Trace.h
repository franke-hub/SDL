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
//       Trace.h
//
// Purpose-
//       Trace table storage allocator.
//
// Last change date-
//       2020/08/04
//
// Usage notes-
//       The Trace object allocates storage sequentially from itself, wrapping
//       when a request cannot otherwise be satisfied. This is intended for use
//       as a trace table item allocator. Applications may find other uses.
//
//       The Trace object is thread-safe and process-safe. Trace tables may be
//       allocated in shared memory and shared between processes. Process's
//       Trace::trace may differ.
//
//       A trace table SHOULD be large enough that so that table wrapping does
//       not cause initialization interference.
//
// Implementation notes-
//       Applications are responsible for trace table allocation/release.
//       The Trace object is contained within the trace table. The entire
//       trace table is initialized using make().
//
//       Trace storage allocation is performance critical. Therefore, this
//       methods DOES NOT normally check for application errors. There is
//       NO defined application mechanism to enable this checking.
//
//       Specifically, Trace::allocate MIGHT NOT check whether:
//         * flag[X_HALT] == 0      (allocate_if provides this check)
//         * Parameter size == 0    (Always an application error)
//         * Parameter size > (this->size - sizeof(Trace))
//         * Possible arithemetic overflow: size > (size + this->next)
//           This condition can occur with large trace tables and large
//           allocation requests. To prevent this, applications MUST NOT
//           allow the size parameter plus the size of the trace table to
//           exceed UINT32_MAX.
//
// Sample usage-
//       void* storage= malloc(desired_size); // Unaligned is OK. make trims
//       Trace::trace= pub::Trace::make(storage, desired_size);
//
//       struct Record : public pub::Trace::Record { // Your Record
//         : // Your data goes here
//       };
//       :
//       Record* record= (Record*)Trace::storage_if(sizeof(Record));
//       if( record ) {             // If trace is active
//         : // Initialize your data
//         record->trace(".xxx"); // Initialize the trace identifier + clock
//       }
//       :
//       free(Trace::trace); Trace::trace= nullptr; // Done with trace table
//
//----------------------------------------------------------------------------
#ifndef _PUB_TRACE_H_INCLUDED
#define _PUB_TRACE_H_INCLUDED

#include <atomic>                   // For std::atomic_uint64_t, ...
#include <stdint.h>                 // For uint32_t

#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Trace
//
// Purpose-
//       In memory trace object descriptor.
//
//----------------------------------------------------------------------------
class Trace {                       // Trace object
//----------------------------------------------------------------------------
// Trace::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  ALIGNMENT= 32                    // Table/Record alignment (informative)
,  TABLE_SIZE_MAX=   0x00FFFFFF00UL // Maximum allowed table size
,  TABLE_SIZE_MIN=   0x0000010000UL // Minimum allowed table size
}; // enum

//----------------------------------------------------------------------------
// Trace::Attributes
//----------------------------------------------------------------------------
static Trace*          trace;       // Common Trace instance (Application controlled)

//// Applications can, but normally do not access these fields
uint64_t               wrap;        // The wrap counter
uint32_t               _0008;       // Reserved
uint8_t                flag[4];     // Control flags
uint32_t               zero;        // Offset: Trace table origin
uint32_t               last;        // Offset: Last trace entry before wrap
uint32_t               size;        // Offset: Size of trace table storage
std::atomic_uint32_t   next;        // Offset: Next trace entry

enum FLAG_X                         // Flag index
{  X_HALT= 0                        // The HALT flag. If non-zero, halt
,  X_OFFSET= 3                      // Alignment offset adjustment
}; // enum FLAG_X

//----------------------------------------------------------------------------
// Trace::Record (POD: Plain Old Data)
//----------------------------------------------------------------------------
public:
struct Record {                     // A standard (POD) trace record
char                   ident[4];    // The trace type identifier
uint32_t               unit;        // The trace unit identifier
uint64_t               clock;       // The UTC epoch clock, in nanoseconds
char                   value[16];   // Data values (For smallest Record)

void
   trace(                           // Initialize the Record type identifier
     const char*       ident);      // The trace type identifier (+clock)

void
   trace(                           // Initialize the Record identifiers
     const char*       ident,       // The trace type identifier (+clock)
     uint32_t          unit);       // The trace unit identifier
}; // struct Record

//----------------------------------------------------------------------------
// Trace::Constructors
//----------------------------------------------------------------------------
public:
   ~Trace( void ) {}                // Destructor

protected:                          // Applications MUST use make
   Trace(                           // Constructor
     uint32_t          size);       // Size of trace area, including *this

   Trace(const Trace&) = delete;    // Disallowed copy constructor
Trace& operator=(const Trace&) = delete; // Disallowed assignment operator

public:
//----------------------------------------------------------------------------
//
// Method-
//       make
//
// Purpose-
//       Initialize the Trace table
//
// Usage notes-
//       Applications control the allocation and deletion of Trace table
//       storage. There is no alignment restriction on the storage area.
//       The resultant Trace object, however, is always ALIGNMENT aligned.
//       The Trace object is at the (aligned) beginning of the storage area.
//
//       This method DOES NOT set Trace::trace. If desired, an application
//       may set this perhaps using Trace::trace= make(addr, size);
//
//----------------------------------------------------------------------------
static Trace*                       // The Trace object
   make(                            // Create a Trace object from
     void*             addr,        // Storage area
     size_t            size);       // Storage length

//----------------------------------------------------------------------------
// Trace::Accessors
//----------------------------------------------------------------------------
public:
bool is_active( void )              // Is trace active?
{  return flag[X_HALT] == 0; }

//----------------------------------------------------------------------------
// Trace::Methods
//----------------------------------------------------------------------------
public:
// allocate: Allocate storage
void*                               // Resultant
   allocate(                        // Allocate a trace record
     uint32_t          size);       // of this length

// allocate_if: Allocate storagae, nullptr if is_active() == false
void*                               // -> Trace record
   allocate_if(                     // Allocate a trace record
     uint32_t          size)        // of this length
{  if( is_active() ) return allocate(size); else return nullptr; }

// deactivate: Globally deactivate this Trace object
void
   deactivate( void )               // Halt tracing
{  flag[X_HALT]= true; }            // is_active() now returns false

// dump: Create an unformatted hex dump file using Debug::tracef
//   Note: the global Debug lock is held while dumping.
void
   dump( void ) const;              // Dump the trace table

uint32_t                            // Offset of record
   offset(                          // Get offset of
     void*             record)      // This record
{  return (char*)record - (char*)this; }

// storage_if: Static storage allocator (with status checking)
static void*                        // The storage, nullptr if inactive
   storage_if(                      // Conditionally allocate storage
     uint32_t          size)        // of this length
{  if( trace ) return trace->allocate_if(size); else return nullptr; }
}; // class Trace
}  // namespace _PUB_NAMESPACE
#endif // _PUB_TRACE_H_INCLUDED
